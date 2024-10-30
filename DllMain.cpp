#include <cstdio>
#include <string>
#include <cstdint>
#include <Windows.h>

#include "MinHook/MinHook.h"

#define LOG(fmt, ...) printf(fmt "\n", ##__VA_ARGS__)

void SetupConsole()
{
    AllocConsole();
    SetConsoleTitleA("T.ME/WERGITY_MODS");
    freopen_s(reinterpret_cast<FILE**>(stdout), "CONOUT$", "w", stdout);

    for (int i = 0; i < 15; i++)
    {
        LOG("TG: @Wergity_Mods");
    }
}

void InjectKey()
{
    HKEY key;
    bool keyInjected = false;
    std::string cheatKey = "Astral_abcdefg";

    LONG result = RegCreateKeyExA(
        HKEY_LOCAL_MACHINE,
        "SOFTWARE\\AstraL",
        0,
        NULL,
        REG_OPTION_NON_VOLATILE,
        KEY_WRITE,
        NULL,
        &key,
        NULL
    );

    if (result == ERROR_SUCCESS)
    {
        result = RegSetValueExA(
            key,
            "Key",
            0,
            REG_SZ,
            (const BYTE*)cheatKey.c_str(),
            cheatKey.size()
        );

        RegCloseKey(key);
        keyInjected = result == ERROR_SUCCESS;
    }

    if (keyInjected)
    {
        LOG("Cheat key injected, press login button after patches applied successfully");
    }
    else
    {
        LOG("Failed inject cheat key, type it manually: %s", cheatKey.c_str());
    }
}

void* (*oGetResponse)(const char*, void*, void*, void*);
void* GetResponse(const char* a1, void* a2, void* a3, void* a4)
{
    /*
    {"data":"1bf458c9b7c060b6f88ed1833aae806ca0e4869cc5bf39f2d8240278d184fc9d453fdac39c2de44f6b2defdb870115944a0c9a59c7dc30767904efcf60e854bb35c8a81061272ee621d331282f2d8a0d"}
    {'status': 'success', 'code': '1', 'date': '2024-10-28 22:08:03'}

    stealing the "date" parameter from the original response and adding to the current response
    */

    std::string data = std::string(a1);

    char buffer[172];
    sprintf_s(
        buffer,
        "%s%s",
        "{\"data\":\"1bf458c9b7c060b6f88ed1833aae806ca0e4869cc5bf39f2d8240278d184fc9d453fdac39c2de44f6b2defdb87011594",
        data.substr(105, 66).c_str()
    );
    
    return oGetResponse(buffer, a2, a3, a4);
}

uintptr_t GetCheatBase()
{
    uintptr_t bs4 = uintptr_t(LoadLibraryA("C:\\Program Files\\BlueStacks_bgp64\\opengl32.dll"));
    if (bs4)
    {
        return bs4;
    }

    uintptr_t bs5 = uintptr_t(LoadLibraryA("C:\\Program Files\\BlueStacks_nxt\\opengl32.dll"));
    if (bs5)
    {
        return bs5;
    }

    uintptr_t opengl32 = uintptr_t(GetModuleHandleA("opengl32.dll"));
    if (opengl32)
    {
        return opengl32;
    }

    return 0;
}

bool Patch()
{
    uintptr_t cheat_base = GetCheatBase();
    if (!cheat_base)
    {
        LOG("opengl32.dll not found; wtf?");
        return false;
    }
    LOG("cheat base found at %p", (void*)cheat_base);

    uint8_t JMP = 0xEB;
    void* dateVerification = (void*)(cheat_base + 0x21383);

    DWORD oProtect = 0;
    if (!VirtualProtect(dateVerification, sizeof(JMP), PAGE_EXECUTE_READWRITE, &oProtect))
    {
        return false;
    }

    *(uint8_t*)dateVerification = JMP;
    if (!VirtualProtect(dateVerification, sizeof(JMP), PAGE_EXECUTE_READ, &oProtect))
    {
        return false;
    }
        
    if (MH_CreateHook((void*)(cheat_base + 0x20320), (void*)GetResponse, (void**)&oGetResponse) != MH_OK)
    {
        LOG("Failed create GetResponse hook");
        return false;
    }

    return true;
}

void Crack()
{
    SetupConsole();
    InjectKey();

    if (MH_Initialize() != MH_OK)
    {
        LOG("Failed init hook-system");
        return;
    }

    if (!Patch())
    {
        return;
    }

    if (MH_EnableHook(MH_ALL_HOOKS) != MH_OK)
    {
        LOG("Failed enable hook-system");
        return;
    }
    LOG("Patched, enjoy!");

    HWND console = GetConsoleWindow();
    FreeConsole();

    Sleep(15000);
    ShowWindow(console, SW_HIDE);
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    if (fdwReason == DLL_PROCESS_ATTACH)
    {
        CreateThread(0, 0, LPTHREAD_START_ROUTINE(Crack), 0, 0, 0);
    }

    return TRUE;
}