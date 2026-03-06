#include "../../include/gui/Render.h"
#include "../../include/gui/Menu.h"
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>
#include <d3d11.h>

// Variables globales para DirectX
static ID3D11Device* g_pd3dDevice = nullptr;
static ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
static IDXGISwapChain* g_pSwapChain = nullptr;
static ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;

// Prototipos de funciones auxiliares de Windows
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace Gui {
    Render::Render() {}
    Render::~Render() {}

    bool Render::Initialize(const wchar_t* title) {
        // 1. Crear la clase de ventana
        WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"ScannelerClass", nullptr };
        RegisterClassExW(&wc);
        
        // 2. Crear la ventana física
        HWND hwnd = CreateWindowW(wc.lpszClassName, title, WS_OVERLAPPEDWINDOW, 100, 100, 1280, 800, nullptr, nullptr, wc.hInstance, nullptr);

        // 3. Inicializar Direct3D 11
        DXGI_SWAP_CHAIN_DESC sd;
        ZeroMemory(&sd, sizeof(sd));
        sd.BufferCount = 2;
        sd.BufferDesc.Width = 0;
        sd.BufferDesc.Height = 0;
        sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.OutputWindow = hwnd;
        sd.SampleDesc.Count = 1;
        sd.Windowed = TRUE;

        D3D_FEATURE_LEVEL featureLevel;
        D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, nullptr, 0, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);

        // Crear Render Target
        ID3D11Texture2D* pBackBuffer;
        g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
        g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
        pBackBuffer->Release();

        // --- CAMBIO CRÍTICO: INICIALIZAR IMGUI ANTES DE MOSTRAR LA VENTANA ---
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

        ImGui_ImplWin32_Init(hwnd);
        ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

        // Ahora que ImGui está listo, mostramos la ventana
        ShowWindow(hwnd, SW_SHOWDEFAULT);
        UpdateWindow(hwnd);

        return true;
    }

    void Render::Run() {
        bool done = false;
        while (!done) {
            MSG msg;
            while (PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
                if (msg.message == WM_QUIT) done = true;
            }
            if (done) break;

            // Validación de seguridad: No renderizar si el contexto se perdió
            if (ImGui::GetCurrentContext() == nullptr) continue;

            // Iniciar frame de ImGui
            ImGui_ImplDX11_NewFrame();
            ImGui_ImplWin32_NewFrame();
            ImGui::NewFrame();

            // Dibujar Menú
            Gui::Menu::Draw(); 

            // Renderizado final
            ImGui::Render();
            const float clear_color_with_alpha[4] = { 0.05f, 0.05f, 0.05f, 1.0f };
            g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
            g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
            ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

            g_pSwapChain->Present(1, 0); 
        }

        // Limpieza
        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
    }

    // Función de manejo de mensajes corregida
    LRESULT CALLBACK Render::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        // PROTECCIÓN: Si el contexto no existe, no pasar mensajes a ImGui
        if (ImGui::GetCurrentContext() != nullptr) {
            if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
                return true;
        }

        switch (msg) {
            case WM_SIZE:
                if (g_pd3dDevice != nullptr && wParam != SIZE_MINIMIZED) {
                    // Aquí podrías redimensionar el render target si fuera necesario
                }
                return 0;
            case WM_SYSCOMMAND:
                if ((wParam & 0xfff0) == SC_KEYMENU) return 0;
                break;
            case WM_DESTROY:
                PostQuitMessage(0);
                return 0;
        }
        return DefWindowProcW(hWnd, msg, wParam, lParam);
    }
}