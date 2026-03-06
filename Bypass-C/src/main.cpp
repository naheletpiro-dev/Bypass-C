#include <windows.h>
#include <iostream>
#include <shellapi.h>
#include <vector>
#include <map>
#include <mutex>
#include "../include/gui/Render.h"
#include "../include/gui/Menu.h"

// --- INICIALIZACIÓN DE TODAS LAS VARIABLES ESTÁTICAS DE MENU ---
// Nota: Solo usamos tipos estándar de C++ aquí para evitar el error de contexto de ImGui al arrancar.
namespace Gui {
    // Estado y Lenguaje
    AppState Menu::currentState = AppState::SplashScreen;
    std::string Menu::currentLang = "EN";
    std::map<std::string, std::map<std::string, std::string>> Menu::languages;

    // Splash Screen
    float Menu::splashProgress = 0.0f;
    int Menu::splashStep = 0;
    float Menu::pulseDirection = 1.0f;
    float Menu::glowWidth = 0.0f;

    // Alertas
    bool Menu::showAlert = false;
    bool Menu::alertIsError = false;
    std::string Menu::alertTitle = "";
    std::string Menu::alertMessage = "";

    // Login
    char Menu::userBuffer[256] = "";
    char Menu::passBuffer[256] = "";
    char Menu::keyBuffer[256] = "";
    std::string Menu::loginStatus = "SYSTEM IDLE";
    bool Menu::isLoginError = false;
    bool Menu::isAdmin = false;

    // Redeem
    char Menu::redeemKey[256] = "";
    char Menu::redeemUser[256] = "";
    char Menu::redeemPass[256] = "";
    char Menu::redeemConf[256] = "";
    std::string Menu::redeemStatus = "";

    // Main / Settings
    bool Menu::showSettings = false;
    bool Menu::deepScanEnabled = false;
    float Menu::matrixDrops[50] = { 0 };
    bool Menu::isBindingHotkey = false;
    int Gui::Menu::bypassHotkey = 0;

    // Admin Center
    std::vector<UserNode> Menu::cachedUsers;
    int Menu::keyTierIndex = 1;
    int Menu::keyAmount = 1;
    std::string Menu::generatedKeysOutput = "";
    bool Menu::showHwidResetModal = false;
    char Menu::hwidResetTarget[256] = "";
    bool Menu::showDeleteModal = false;
    std::string Menu::userToDelete = "";

    // Bypass / Ghost Protocol
    std::wstring Menu::targetPath = L"";
    std::vector<std::string> Menu::consoleLogs;
    std::mutex Menu::logMutex; 
    bool Menu::isWiping = false;
    bool Menu::showKamikazeModal = false;
}

// Función para solicitar privilegios de Administrador (UAC)
bool RequestAdminPrivileges() {
    BOOL isAdmin = FALSE;
    HANDLE hToken = NULL;
    if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
        TOKEN_ELEVATION elevation;
        DWORD cbSize = sizeof(TOKEN_ELEVATION);
        if (GetTokenInformation(hToken, TokenElevation, &elevation, sizeof(elevation), &cbSize)) {
            isAdmin = elevation.TokenIsElevated;
        }
    }
    if (hToken) CloseHandle(hToken);

    if (!isAdmin) {
        wchar_t szPath[MAX_PATH];
        if (GetModuleFileNameW(NULL, szPath, ARRAYSIZE(szPath))) {
            SHELLEXECUTEINFOW sei = { sizeof(sei) };
            sei.cbSize = sizeof(sei);
            sei.lpVerb = L"runas"; 
            sei.lpFile = szPath;
            sei.hwnd = NULL;
            sei.nShow = SW_NORMAL;
            sei.fMask = SEE_MASK_NOCLOSEPROCESS;
            if (!ShellExecuteExW(&sei)) return false;
            exit(0); 
        }
    }
    return true;
}

// PUNTO DE ENTRADA PRINCIPAL
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow) {
    // 1. Auto-Elevación: Necesario para que el Cleaner acceda a registros protegidos
    if (!RequestAdminPrivileges()) return 0;

    // 2. Pre-Arranque: Soporte para monitores de alta resolución
    SetProcessDPIAware();

    // 3. Inicializar el motor gráfico (DirectX 11 + ImGui)
    // El orden interno de Initialize ahora protege el contexto de ImGui
    Gui::Render renderEngine;
    if (!renderEngine.Initialize(L"Scanneler - Ghost Protocol")) {
        MessageBoxA(NULL, "Fallo critico al iniciar el motor grafico.", "Scanneler Error", MB_ICONERROR);
        return -1;
    }

    // 4. Ejecutar bucle de renderizado
    renderEngine.Run();

    return 0;
}