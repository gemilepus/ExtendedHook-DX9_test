//AgeOfEmpireHook.dll

#include "windows.h"
#include "d3dx9.h"
#include "ExtendedHook.h"

#pragma comment(lib, "d3dx9.lib")


void start_hooking();
void WriteText(IDirect3DDevice9* d3ddev, LPCTSTR text, long x, long y, long width, long height);
int times_load = 0;

typedef DWORD D3DCOLOR;

IDirect3DDevice9* DeviceInterface;

//hook Direct3DCreate9
typedef IDirect3D9* (WINAPI* pDirect3DCreate9) (UINT SDKVersion);
EHOOKSTRUCT api_Direct3DCreate9;
IDirect3D9* WINAPI Direct3DCreate9_Hook(UINT SDKVersion);

//hook CreateDevice
typedef HRESULT(APIENTRY* pCreateDevice)(
    IDirect3D9* pDev,
    UINT Adapter,
    D3DDEVTYPE DeviceType,
    HWND hFocusWindow,
    DWORD BehaviorFlags,
    D3DPRESENT_PARAMETERS* pPresentationParameters,
    IDirect3DDevice9** ppReturnedDeviceInterface
    );
EHOOKSTRUCT api_CreateDevice;
HRESULT APIENTRY CreateDevice_hook(IDirect3D9* pDev, UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow,
    DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters,
    IDirect3DDevice9** ppReturnedDeviceInterface);

//Hook EndScene
typedef HRESULT(WINAPI* pEndScene)(IDirect3DDevice9* pDevInter);
EHOOKSTRUCT api_EndScene;
HRESULT WINAPI EndScene_hook(IDirect3DDevice9* pDevInter);

BOOL APIENTRY DllMain(HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        start_hooking();
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

void start_hooking() {


    if (InstallEHook("Direct3DCreate9", L"d3d9.dll", &api_Direct3DCreate9, &Direct3DCreate9_Hook) == false) {
        MessageBox(0, L"Error while hooking Direct3DCreate9", L"Hooker", MB_OK | MB_ICONWARNING);
    }
    return;
}



IDirect3D9* WINAPI Direct3DCreate9_Hook(UINT SDKVersion) {
    IDirect3D9* pDev = ((pDirect3DCreate9)api_Direct3DCreate9.adr_new_api)(SDKVersion);

    _asm pushad
    DWORD* vtable = (DWORD*)*((DWORD*)pDev); //VTABLE
    if (times_load == 1) { //the first time d3d9.dll is used, isn't for the game making, we need the second
        InstallEHookEx((void*)vtable[16], &api_CreateDevice, &CreateDevice_hook);
    }
    times_load += 1;
    _asm popad

    return pDev;
}

HRESULT APIENTRY CreateDevice_hook(IDirect3D9* pDev, UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow,
    DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters,
    IDirect3DDevice9** ppReturnedDeviceInterface) {

    HRESULT final = ((pCreateDevice)api_CreateDevice.adr_new_api)(pDev, Adapter, DeviceType, hFocusWindow,
        BehaviorFlags, pPresentationParameters, ppReturnedDeviceInterface);

    _asm pushad
    DWORD* DevInterface = (DWORD*)*((DWORD*)*ppReturnedDeviceInterface); //VTABLE
    InstallEHookEx((void*)DevInterface[42], &api_EndScene, &EndScene_hook); //EndScene
    _asm popad

    return final;
}

HRESULT WINAPI EndScene_hook(IDirect3DDevice9* pDevInter) {
    _asm pushad
    WriteText(pDevInter, L"AGE OF EMPIRES EXTENDED HOOK BY ROSDEVIL", 20, 20, 300, 50);
    if (GetAsyncKeyState(VK_F1))WriteText(pDevInter, L"Hooked functions:\n - CreateDevice\n - EndScene\n", 20, 50, 150, 100);
    _asm popad

    return ((pEndScene)api_EndScene.adr_new_api)(pDevInter);
}


void WriteText(IDirect3DDevice9* d3ddev, LPCTSTR text, long x, long y, long width, long height) {
    ID3DXFont* m_font;

    D3DXCreateFont(d3ddev, 15, 0, FW_BOLD, 0, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, TEXT("Arial"), &m_font);
    D3DCOLOR fontColor1 = D3DCOLOR_XRGB(255, 0, 0);

    RECT space;
    space.top = y;
    space.left = x;
    space.right = width + x;
    space.bottom = height + y;

    m_font->DrawText(NULL, text, -1, &space, 0, fontColor1);
    m_font->Release();
}