#pragma once
#include <string>
#include <windows.h>

namespace Core {
    class SystemUtils {
    public:
        // Ejecuta un comando de sistema (CMD) de forma totalmente oculta
        static bool ExecuteHiddenCommand(const std::wstring& command);

        // Verifica si el proceso tiene privilegios de Administrador
        static bool IsRunAsAdmin();

        // Obtiene la ruta del ejecutable actual
        static std::wstring GetExecutablePath();

    private:
        // Helper interno para convertir comandos de string a formato compatible con Windows
        static std::wstring BuildCommandString(const std::wstring& cmd);
    };
}