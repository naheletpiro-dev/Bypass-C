#include <windows.h>
#include <iostream>
#include <shellapi.h>
#include <d3d11.h>
#include "../include/gui/Render.h"
#include "../include/gui/Menu.h"

// --- FUNCIÓN DE ELEVACIÓN DE PRIVILEGIOS (UAC) ---
bool RequestAdminPrivileges() {
    BOOL bIsElevated = FALSE;
    HANDLE hToken = NULL;
    if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
        TOKEN_ELEVATION elevation;
        DWORD dwSize;
        if (GetTokenInformation(hToken, TokenElevation, &elevation, sizeof(elevation), &dwSize)) {
            bIsElevated = elevation.TokenIsElevated;
        }
    }
    if (hToken) CloseHandle(hToken);

    if (!bIsElevated) {
        wchar_t szPath[MAX_PATH];
        if (GetModuleFileNameW(NULL, szPath, MAX_PATH)) {
            SHELLEXECUTEINFOW sei = { sizeof(sei) };
            sei.cbSize = sizeof(sei);
            sei.lpVerb = L"runas";
            sei.lpFile = szPath;
            sei.hwnd = NULL;
            sei.nShow = SW_NORMAL;
            sei.fMask = SEE_MASK_NOCLOSEPROCESS;

            if (ShellExecuteExW(&sei)) return false; 
        }
    }
    return bIsElevated != 0;
}

// --- PUNTO DE ENTRADA PRINCIPAL ---
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow) {
    
    // 1. Elevación (Indispensable para el motor de bypass)
    if (!RequestAdminPrivileges()) return 0;

    // 2. Soporte DPI para que el menú no se vea borroso
    SetProcessDPIAware();

    // 3. Inicializar el motor gráfico
    Gui::Render renderEngine;
    if (!renderEngine.Initialize(L"Scanneler - Ghost Protocol")) {
        MessageBoxA(NULL, "Error critico: El dispositivo DirectX 11 no pudo iniciarse.", "Scanneler System", MB_ICONERROR);
        return -1;
    }

    // 4. CARGA SEGURA DE RECURSOS
    // Obtenemos el puntero del dispositivo DESPUÉS de Initialize
    ID3D11Device* device = renderEngine.GetDevice(); 
    
    if (device != nullptr) {
        // Intentamos cargar la imagen. 
        // Si la función LoadLogoTexture está bien blindada (con el fallback que te pasé),
        // no importa si el archivo no existe; el programa NO crasheará.
        Gui::Menu::LoadLogoTexture(device, "assets/Scanneler.png");
    }

    // 5. Bucle de Renderizado
    // Si llegamos aquí, el programa debería mantenerse abierto
    try {
        renderEngine.Run();
    } catch (...) {
        OutputDebugStringA("[!] Error fatal durante el ciclo de renderizado.\n");
    }

    return 0;
}