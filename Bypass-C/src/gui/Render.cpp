#include "../../include/gui/Render.h"
#include "../../include/gui/Menu.h"
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>
#include <d3d11.h>

// Prototipo externo para ImGui
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace Gui {
    Render::Render() {
        hwnd = nullptr;
        pd3dDevice = nullptr;
        pd3dDeviceContext = nullptr;
        pSwapChain = nullptr;
        mainRenderTargetView = nullptr;
    }

    Render::~Render() {
        CleanupDeviceD3D();
    }

    bool Render::Initialize(const wchar_t* title) {
        // 1. Registro de Clase
        WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"ScannelerClass", nullptr };
        RegisterClassExW(&wc);
        
        // 2. Creación de Ventana
        hwnd = CreateWindowW(wc.lpszClassName, title, WS_OVERLAPPEDWINDOW, 100, 100, 1280, 800, nullptr, nullptr, wc.hInstance, nullptr);
        if (!hwnd) return false;

        // 3. Inicializar Direct3D 11 usando los miembros de la clase (NO globales)
        DXGI_SWAP_CHAIN_DESC sd;
        ZeroMemory(&sd, sizeof(sd));
        sd.BufferCount = 2;
        sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.OutputWindow = hwnd;
        sd.SampleDesc.Count = 1;
        sd.Windowed = TRUE;
        sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

        D3D_FEATURE_LEVEL featureLevel;
        const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
        
        // Usamos los punteros pd3dDevice de la clase Render.h
        HRESULT hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &pSwapChain, &pd3dDevice, &featureLevel, &pd3dDeviceContext);
        
        if (FAILED(hr)) return false;

        // 4. Crear Render Target
        CreateRenderTarget();

        // 5. Inicializar ImGui
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        
        ImGui_ImplWin32_Init(hwnd);
        ImGui_ImplDX11_Init(pd3dDevice, pd3dDeviceContext);

        ShowWindow(hwnd, SW_SHOWDEFAULT);
        UpdateWindow(hwnd);

        return true;
    }

    void Render::CreateRenderTarget() {
        ID3D11Texture2D* pBackBuffer;
        if (SUCCEEDED(pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer)))) {
            pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &mainRenderTargetView);
            pBackBuffer->Release();
        }
    }

    void Render::CleanupDeviceD3D() {
        if (mainRenderTargetView) { mainRenderTargetView->Release(); mainRenderTargetView = nullptr; }
        if (pSwapChain) { pSwapChain->Release(); pSwapChain = nullptr; }
        if (pd3dDeviceContext) { pd3dDeviceContext->Release(); pd3dDeviceContext = nullptr; }
        if (pd3dDevice) { pd3dDevice->Release(); pd3dDevice = nullptr; }
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

            // Iniciar frame
            ImGui_ImplDX11_NewFrame();
            ImGui_ImplWin32_NewFrame();
            ImGui::NewFrame();

            // Dibujar
            Gui::Menu::Draw(); 

            // Finalizar frame
            ImGui::Render();
            const float clear_color[4] = { 0.02f, 0.02f, 0.02f, 1.0f };
            pd3dDeviceContext->OMSetRenderTargets(1, &mainRenderTargetView, nullptr);
            pd3dDeviceContext->ClearRenderTargetView(mainRenderTargetView, clear_color);
            ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

            pSwapChain->Present(1, 0); 
        }
    }

    LRESULT CALLBACK Render::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
            return true;

        switch (msg) {
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