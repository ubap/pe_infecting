/*
	jakub trzebiatowski
	idea release
	
	VS 2012, link with:
	/SECTION:.text,ERWKP 
*/

#define GET_PARAM_(f) PARAM_ ## f : mov eax, 1337;

uint8_t* payLoad;
uint32_t payLoad_length;
int32_t SelfCopyingFunction(uint32_t param1 /*access via GET_PARAM_(1)*/)
{
	_asm
	{
		jmp end;
start:
		GET_PARAM_(1);

		jmp eax;
end:
		// calc payload length
		mov eax, end;
		sub eax, start;
		mov payLoad_length, eax;
		mov eax, start;
		mov payLoad, eax;

		// todo: PRE PROCESSOR LOOP tru params:
		mov eax, param1;
		mov PARAM_1+1, eax;

	};
}