#include "antidebug.h"
#include "xorstr.hpp"
#include "apihash.hpp"
#include <tlhelp32.h>

// Definición de tipos para funciones nativas
typedef NTSTATUS(NTAPI* pNtSetInformationThread)(
    HANDLE ThreadHandle,
    UINT ThreadInformationClass,
    PVOID ThreadInformation,
    ULONG ThreadInformationLength
);

bool AntiDebug::IsProcessHackerPresent() {
    auto pCreateSnapshot = API_CALL("kernel32.dll", 0x7E3A1B2C, CreateToolhelp32Snapshot);
    if (!pCreateSnapshot) return false;

    HANDLE hSnap = pCreateSnapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnap == INVALID_HANDLE_VALUE) return false;

    PROCESSENTRY32W pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32W);

    auto pProcess32First = API_CALL("kernel32.dll", 0x2A4B5C6D, Process32FirstW);
    auto pProcess32Next = API_CALL("kernel32.dll", 0x3D4E5F6A, Process32NextW);

    if (pProcess32First && pProcess32First(hSnap, &pe32)) {
        do {
            // SOLUCIÓN AL ERROR 254: Asignamos a variables intermedias
            // Esto ayuda al compilador a entender que XorStrW es una expresión, no un tipo.
            const wchar_t* target1 = XorStrW(L"processhacker.exe");
            const wchar_t* target2 = XorStrW(L"SystemInformer.exe");

            if (_wcsicmp(pe32.szExeFile, target1) == 0 || 
                _wcsicmp(pe32.szExeFile, target2) == 0) {
                CloseHandle(hSnap);
                return true;
            }
        } while (pProcess32Next && pProcess32Next(hSnap, &pe32));
    }

    CloseHandle(hSnap);
    return false;
}

bool AntiDebug::IsDebuggerPresent() {
    auto pIsDebuggerPresent = API_CALL("kernel32.dll", 0x4F5A6B7C, ::IsDebuggerPresent);
    return pIsDebuggerPresent ? pIsDebuggerPresent() : false;
}

void AntiDebug::HideThread() {
    // Usamos el APIResolver directamente para evitar conflictos con la macro en funciones NT
    auto NtSetInfoThread = APIResolver::get_api<pNtSetInformationThread>("ntdll.dll", 0x5B2D3E4F);

    if (NtSetInfoThread) {
        // 0x11 = ThreadHideFromDebugger
        NtSetInfoThread(GetCurrentThread(), 0x11, NULL, 0);
    }
}

void AntiDebug::Protect() {
    HideThread();
    if (IsDebuggerPresent() || IsProcessHackerPresent()) {
        // Salida forzosa indetectable
        auto pTerminate = API_CALL("kernel32.dll", 0x12345678, TerminateProcess); // Asegúrate de tener el hash real
        if (pTerminate) pTerminate(GetCurrentProcess(), 0);
        else exit(0);
    }
}