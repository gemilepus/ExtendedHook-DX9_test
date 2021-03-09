//
//EXTENDED CODE OVERWRITE HOOKING METHOD BY ROSDEVIL
//
//#include "stdafx.h" //cancel this if you are working with Dev-Cpp
#include "windows.h"
#include <iostream>
#include "ExtendedHook.h"

bool InstallEHook(LPCSTR API, LPCTSTR lib, PEHOOKSTRUCT  EHookA, void * redit){

 HMODULE dll = GetModuleHandle(lib);
 if (dll <= 0) dll = LoadLibrary(lib);
 DWORD * ApiAdr = (DWORD *)GetProcAddress(dll, API);
  
 return InstallEHookEx(ApiAdr, EHookA, redit);
}

bool InstallEHookEx(void * TargetAddress, PEHOOKSTRUCT EHookA, void * redit){
 
 DWORD * ApiAddress = (DWORD*)TargetAddress;
 if (ApiAddress <= 0) return false;
 return CustomEHook(ApiAddress, EHookA, redit, Reckon_bytes_jmp(ApiAddress));
}

bool CustomEHook(void * TargetAddress, PEHOOKSTRUCT EHookA, void * redit, unsigned int bytes_jmp){
 DWORD * ApiAdr = (DWORD*)TargetAddress;
 
 EHookA->adr_init = ApiAdr;
 EHookA->adr_redirect = (DWORD *)redit;

 EHookA->bytes_size = bytes_jmp;

 LPVOID MyFuncAddress = VirtualAllocEx(GetCurrentProcess(), NULL, EHookA->bytes_size + 5, MEM_RESERVE|MEM_COMMIT, PAGE_EXECUTE_READWRITE);
 WriteProcessMemory(GetCurrentProcess(), MyFuncAddress, ApiAdr, EHookA->bytes_size, NULL);

 EHookA->adr_new_api = (DWORD*)MyFuncAddress;

 long sizejmp = (DWORD)redit - (DWORD)ApiAdr - 5;
 DWORD oldIP, newP;

 if (VirtualProtect(ApiAdr, 5, PAGE_EXECUTE_READWRITE, &oldIP)==0) return false;
 *(char *) ApiAdr = 0xE9;
 *(long *)((unsigned long)ApiAdr + 1) = sizejmp;
 VirtualProtect(ApiAdr, 5, oldIP, &newP);


 long backjmp = ((DWORD)ApiAdr + EHookA->bytes_size) - ((DWORD)MyFuncAddress + EHookA->bytes_size + 5);

 *(char*)((unsigned long)MyFuncAddress + EHookA->bytes_size) = 0xE9;
 *(int *)((unsigned long)MyFuncAddress + EHookA->bytes_size + 1) = backjmp;
 
 return true;
}


void * UninstallEHook(PEHOOKSTRUCT EHookA){
  DWORD oldIP;
  if (VirtualProtect(EHookA->adr_init, 5, PAGE_EXECUTE_READWRITE, &oldIP) == 0) return NULL;
  WriteProcessMemory(GetCurrentProcess(), EHookA->adr_init, (void*)EHookA->adr_new_api, EHookA->bytes_size, NULL);
  VirtualFreeEx(GetCurrentProcess, (void*)EHookA->adr_new_api, EHookA->bytes_size + 5, NULL);
  EHookA->adr_new_api = 0;
  EHookA->adr_redirect = 0;
  EHookA->bytes_size = 0;
  VirtualProtect(EHookA->adr_init, 5, oldIP, &oldIP);
  return EHookA->adr_init;
}

//windows API mostly can be hook by replacing the first 5 bytes without screwing the rest of the code.
unsigned int Reckon_bytes_jmp(void * adr){
 
 unsigned int counter = 0;

 while (5 > counter){
     void * address = (void*)((unsigned long)adr + counter);
     char c = *(char*)address;
     short int s = *(short int*)address;

     if (s == 0xffffff8b) counter += 2;//(0x8BFF) mov edi, edi
     if (s == 0xffffEC8B) counter += 2;//(0x8BEC) mov ebp, esp
	 if (s == 0x25ff) counter += 6; //jmp dword ptr [DS:]
     if (c == 0xE9) counter += 5; //(0xE9 0x00000000) jmp address
     if (c == 0x55) counter += 1;//(0x55) push ebp
     if (c == 0xffffffE8) counter += 5;//(0xE8 0x00000000) call address
     if (c == 0x68) counter += 5;//(0x68 0x00000000) push address
     if (c == 0x6a) counter += 2;//(0x6A 0x00) push onebyte 
	 if (c == 0xa1) counter += 5;//(0xA1 )/mov eax, 0xadress
 }
 return counter;
}
