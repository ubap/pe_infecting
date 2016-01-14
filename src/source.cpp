/*
	jakub trzebiatowski
	idea release
*/

#define PARAM_ 1337
#define PARAM_SIZE_ 4
uint8_t* payLoad;
uint32_t payLoad_length;
int32_t SelfCopyingFunction(uint32_t param /*access via PARAM_*/)
{
	_asm
	{
		jmp end;
start:
		mov eax, PARAM_;
param_addr:
		jmp eax;
end:
		// calc payload length
		mov eax, end;
		sub eax, start;
		mov payLoad_length, eax;
		mov eax, start;
		mov payLoad, eax;


		mov eax, param;
		mov param_addr-PARAM_SIZE_, eax;

	};
}