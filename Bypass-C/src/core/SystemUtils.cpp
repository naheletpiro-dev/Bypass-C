#include "../../include/core/SystemUtils.h"

namespace Core {
    bool SystemUtils::ExecuteHiddenCommand(const std::wstring& command) {
        STARTUPINFOW si = { sizeof(si) };
        PROCESS_INFORMATION pi;
        si.dwFlags = STARTF_USESHOWWINDOW;
        si.wShowWindow = SW_HIDE; // Esto oculta la ventana negra

        // Preparamos el comando para CMD
        std::wstring fullCommand = L"cmd.exe /c " + command;

        if (CreateProcessW(NULL, &fullCommand[0], NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
            WaitForSingleObject(pi.hProcess, INFINITE);
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
            return true;
        }
        return false;
    }
}