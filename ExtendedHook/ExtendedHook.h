//
//EXTENDED CODE OVERWRITE HOOKING METHOD BY ROSDEVIL
//
#ifndef _EHOOK
#define _EHOOK

typedef struct _EHOOKSTRUCT{

DWORD * adr_init; 
//Stores the original address

DWORD * adr_redirect; 
//Stores the address of the hook function

DWORD * adr_new_api; 
//Stores the address of the NEW API

DWORD bytes_size; 
//Number of bytes copied

}EHOOKSTRUCT, *PEHOOKSTRUCT;

//Not to use by Coder
unsigned int Reckon_bytes_jmp(void * adr);

//Functions to use
void * UninstallEHook(PEHOOKSTRUCT EHookA);
bool InstallEHookEx(void * TargetAddress, PEHOOKSTRUCT EHookA, void * redit);
bool InstallEHook(LPCSTR API, LPCTSTR lib, PEHOOKSTRUCT  EHookA, void * redit);

bool CustomEHook(void * TargetAddress, PEHOOKSTRUCT EHookA, void * redit, unsigned int bytes_jmp);

#endif

/*
EXTENDED CODE OVERWRITING
the concept of this approach is to locate the address of the original API function 
and to change first few bytes of this function with a JMP instruction that redirects 
the call to the custom supplied API function, moreover we copy the first five bytes in a new location
in memory with an added JMP back to [API address + 5], so to perform the API.

To say the truth, under windows (32bit) we are extremly lucky because quite all APIs start with the 
same sequence of opcodes:

mov edi, edi   (2 bytes)
push ebp       (1 byte)
mov ebp, esp   (2 bytes)

or some other APIs (like LdrLoadDll) start with a [push 0x00000000] which is exately 5 bytes. 
*/