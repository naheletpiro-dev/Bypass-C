#pragma once
#include <windows.h>
#include <d3d11.h>
#include "Menu.h"

namespace Gui {
    class Render {
    public:
        Render();
        ~Render();

        // Inicializa la ventana y DirectX 11
        bool Initialize(LPCWSTR windowTitle);
        
        // Bucle principal de renderizado
        void Run();

        // --- FUNCIÓN CRÍTICA PARA CARGAR EL LOGO ---
        // Permite obtener el dispositivo D3D11 desde fuera de la clase
        ID3D11Device* GetDevice() { return pd3dDevice; }

    private:
        HWND hwnd;
        WNDCLASSEXW wc;
        
        // Instancia del menú con toda nuestra lógica de frames
        Menu mainMenu; 

        // Punteros de la infraestructura de DirectX 11
        ID3D11Device* pd3dDevice;
        ID3D11DeviceContext* pd3dDeviceContext;
        IDXGISwapChain* pSwapChain;
        ID3D11RenderTargetView* mainRenderTargetView;

        // Métodos internos de gestión de recursos de video
        bool CreateDeviceD3D(HWND hWnd);
        void CleanupDeviceD3D();
        void CreateRenderTarget();
        void CleanupRenderTarget();

        // Procesador de mensajes de Windows (teclado, ratón, cerrar ventana)
        static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    };
}