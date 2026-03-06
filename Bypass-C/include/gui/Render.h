#pragma once
#include <windows.h>
#include <d3d11.h>
#include "Menu.h" // Incluimos nuestra interfaz principal

namespace Gui {
    class Render {
    public:
        Render();
        ~Render();

        // Funciones que llamamos desde main.cpp
        bool Initialize(LPCWSTR windowTitle);
        void Run();

    private:
        HWND hwnd;
        WNDCLASSEXW wc;
        
        // Instancia de nuestro menú (toda la lógica visual que hicimos antes)
        Menu mainMenu; 

        // Punteros básicos de DirectX 11
        ID3D11Device* pd3dDevice;
        ID3D11DeviceContext* pd3dDeviceContext;
        IDXGISwapChain* pSwapChain;
        ID3D11RenderTargetView* mainRenderTargetView;

        // Métodos internos de inicialización gráfica
        bool CreateDeviceD3D(HWND hWnd);
        void CleanupDeviceD3D();
        void CreateRenderTarget();
        void CleanupRenderTarget();

        // Manejador de eventos de la ventana de Windows
        static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    };
}