#include <cstdio>
#include <string>
#include <cstdint>
#include <Windows.h>

#include "MinHook/MinHook.h"

// {"data":"1bf458c9b7c060b6f88ed1833aae806ca0e4869cc5bf39f2d8240278d184fc9d453fdac39c2de44f6b2defdb870115944a0c9a59c7dc30767904efcf60e854bb35c8a81061272ee621d331282f2d8a0d"}
// {'status': 'success', 'code': '1', 'date': '2024-10-28 22:08:03'}

// stealing the "date" parameter from the original response and adding to the current response
void* (*oGetResponse)(const char*, void*, void*, void*);
void* GetResponse(const char* a1, void* a2, void* a3, void* a4)
{
    std::string data = std::string(a1);
    std::string success_data = "{\"data\":\"1bf458c9b7c060b6f88ed1833aae806ca0e4869cc5bf39f2d8240278d184fc9d453fdac39c2de44f6b2defdb87011594";
    success_data += data.substr(105, 66);

    a1 = success_data.c_str();
    return oGetResponse(a1, a2, a3, a4);
}

void SetupConsole()
{
    AllocConsole();
    SetConsoleTitleA("T.ME/WERGITY_MODS");
    freopen_s(reinterpret_cast<FILE**>(stdout), "CONOUT$", "w", stdout);
}

void Crack()
{
    SetupConsole();

    printf("waiting opengl32.dll...\n");
    uintptr_t opengl32;
    do
    {
        Sleep(1000);
        opengl32 = uintptr_t(GetModuleHandleA("opengl32.dll"));
    } while (!opengl32);
    printf("opengl32.dll located at %p\n", (void*) opengl32);

    printf("Patching...\n");
    if (MH_Initialize() != MH_OK)
    {
        printf("Failed init hook-system\n");
        return;
    }

    if (MH_CreateHook((void*)(opengl32 + 0x20320), (void*)GetResponse, (void**)&oGetResponse) != MH_OK)
    {
        printf("Failed create GetResponse hook\n");
        return;
    }
    
    if (MH_EnableHook(MH_ALL_HOOKS) != MH_OK)
    {
        printf("Failed enable hook-system\n");
        return;
    }

    printf("Patched, enjoy!\n");

    Sleep(3000);
    system("cls");
    for (int i = 0; i < 5; i++)
    {
        printf("TG: @Wergity_Mods\n");
    }
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    if (fdwReason == DLL_PROCESS_ATTACH)
    {
        CreateThread(0, 0, LPTHREAD_START_ROUTINE(Crack), 0, 0, 0);
    }

    return TRUE;
}