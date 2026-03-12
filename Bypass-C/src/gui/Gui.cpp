#include "../../include/gui/Menu.h"
#include "../../include/auth/ApiClient.h"
#include "../../include/core/Cleaner.h"
#include <windows.h>
#include <chrono>
#include <thread>
#include <atomic>
#include <TlHelp32.h>
#include <mutex>
#include "../../include/auth/ApiClient.h"
#include <random>
#include <commdlg.h>
#include <nlohmann/json.hpp>
#include <algorithm>
#define STB_IMAGE_IMPLEMENTATION 
#include "../../include/stb_image.h"
#include "../../include/core/Payload.h"
#include "../../include/core/ManualMapper.h"
#pragma comment(lib, "d3d11.lib")

// ==========================================================
// FIX DEFINITIVO PARA MATEMÁTICAS CSS (ELITE ENGINE)
// ==========================================================

// --- OPERADORES MATEMÁTICOS PARA UI ---
static inline ImVec2 operator+(const ImVec2& lhs, const ImVec2& rhs) { return ImVec2(lhs.x + rhs.x, lhs.y + rhs.y); }
static inline ImVec2 operator-(const ImVec2& lhs, const ImVec2& rhs) { return ImVec2(lhs.x - rhs.x, lhs.y - rhs.y); }

// Multiplicación de colores (Necesaria para la línea 677 del Glow)
static inline ImVec4 operator*(const ImVec4& lhs, const ImVec4& rhs) { 
    return ImVec4(lhs.x * rhs.x, lhs.y * rhs.y, lhs.z * rhs.z, lhs.w * rhs.w); 
}
static inline ImVec4 operator*(const ImVec4& lhs, float rhs) { 
    return ImVec4(lhs.x * rhs, lhs.y * rhs, lhs.z * rhs, lhs.w * rhs); 
}
static inline ImVec4 operator+(const ImVec4& lhs, const ImVec4& rhs) { 
    return ImVec4(lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z, lhs.w + rhs.w); 
}
static inline ImVec4 operator-(const ImVec4& lhs, const ImVec4& rhs) { 
    return ImVec4(lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z, lhs.w - rhs.w); 
}

// Función Lerp genérica corregida para transiciones suaves tipo CSS
template<typename T> 
static inline T ImLerp(T a, T b, float t) { 
    return a + (b - a) * t; 
}

#ifndef ImClamp
#define ImClamp(V, MN, MX) ((V) < (MN) ? (MN) : (V) > (MX) ? (MX) : (V))
#endif



namespace Gui {

    // ==========================================================
    // DEFINICIÓN DE VARIABLES ESTÁTICAS (OBLIGATORIO PARA LINKER)
    // ==========================================================

    // Instancia y Configuración Global
    Menu* Menu::_instance = nullptr;
    AppState Menu::currentState = AppState::SplashScreen;
    std::string Menu::currentLang = "EN";
    std::map<std::string, std::map<std::string, std::string>> Menu::languages;

    // --- RECURSOS VISUALES (RESUELVE LNK2019) ---
    ID3D11ShaderResourceView* Menu::logoTexture = nullptr;

    // Animaciones y Efectos Visuales
    float Menu::matrixDrops[70] = { 0 }; 
    float Menu::scanlineTimer = 0.0f;
    float Menu::btnHoverLerp = 0.0f;
    float Menu::glowWidth = 0.0f;
    float Menu::pulseDirection = 1.0f;
    float Menu::splashProgress = 0.0f;
    int Menu::splashStep = 0;
    std::map<std::string, float> Menu::inputAnims;

    // Alertas
    bool Menu::showAlert = false;
    bool Menu::alertIsError = false;
    std::string Menu::alertTitle = "";
    std::string Menu::alertMessage = "";

    // Sistema de Autenticación
    char Menu::userBuffer[256] = "";
    char Menu::passBuffer[256] = "";
    char Menu::keyBuffer[256] = "";
    std::string Menu::loginStatus = "";
    bool Menu::isLoginError = false;
    bool Menu::isAdmin = false;
    bool Menu::isConnecting = false;

    // Registro y Canje (Redeem)
    char Menu::redeemKey[256] = "";
    char Menu::redeemUser[256] = "";
    char Menu::redeemPass[256] = "";
    char Menu::redeemConf[256] = "";
    std::string Menu::redeemStatus = "";

    // Configuración de Sistema
    bool Menu::showSettings = false;
    bool Menu::deepScanEnabled = false;
    bool Menu::isBindingHotkey = false;
    int Menu::bypassHotkey = 0;

    // Ghost Protocol (Bypass)
    std::wstring Menu::targetPath = L"";
    std::vector<std::string> Menu::consoleLogs;
    std::mutex Menu::logMutex;
    bool Menu::isWiping = false;
    bool Menu::showKamikazeModal = false;

    // Command Center (Admin)
    std::vector<UserNode> Menu::cachedUsers;
    int Menu::keyTierIndex = 0;
    int Menu::keyAmount = 1;
    std::string Menu::generatedKeysOutput = "";
    bool Menu::showHwidResetModal = false;
    char Menu::hwidResetTarget[256] = "";
    bool Menu::showDeleteModal = false;
    std::string Menu::userToDelete = "";

// ==========================================================
// IMPLEMENTACIÓN DE CARGA DE LOGO (VERSIÓN ULTRA-SEGURA)
// ==========================================================
bool Menu::LoadLogoTexture(ID3D11Device* device, const char* filename) {
    // 1. Limpieza preventiva y validación de driver
    Menu::logoTexture = nullptr; 
    if (!device) return false;

    int width, height, channels;
    
    // 2. Intento de carga (Ruta relativa)
    unsigned char* data = stbi_load(filename, &width, &height, &channels, 4);
    
    // 3. SEGUNDO INTENTO (Ruta absoluta de emergencia si la relativa falla)
    if (data == NULL) {
        const char* backupPath = "C:\\Users\\Nahele\\Desktop\\Bypass-C-main\\Bypass-C\\assets\\Scanneler.png";
        data = stbi_load(backupPath, &width, &height, &channels, 4);
    }

    // --- SALIDA SEGURA SI AMBOS FALLAN ---
    if (data == NULL) {
        // No cerramos el programa, solo avisamos al sistema que no hay textura
        OutputDebugStringA("[!] LOGO_ERROR: Archivo Scanneler.png no encontrado. Usando fallback.\n");
        return false; 
    }

    // 4. Configuración de la textura
    D3D11_TEXTURE2D_DESC desc;
    ZeroMemory(&desc, sizeof(desc));
    desc.Width = width;
    desc.Height = height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    ID3D11Texture2D* pTexture = nullptr;
    D3D11_SUBRESOURCE_DATA subResource;
    subResource.pSysMem = data;
    subResource.SysMemPitch = width * 4;
    subResource.SysMemSlicePitch = 0;

    // 5. Creación física en GPU con manejo de errores HRESULT
    HRESULT hr = device->CreateTexture2D(&desc, &subResource, &pTexture);
    
    if (SUCCEEDED(hr) && pTexture) {
        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
        ZeroMemory(&srvDesc, sizeof(srvDesc));
        srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = 1;

        hr = device->CreateShaderResourceView(pTexture, &srvDesc, &Menu::logoTexture);
        
        // Liberamos el recurso intermedio
        pTexture->Release();
    } else {
        OutputDebugStringA("[!] D3D11_ERROR: Fallo al crear la textura en la GPU.\n");
    }

    // 6. Limpieza de memoria RAM (ya está en VRAM)
    stbi_image_free(data);
    
    return SUCCEEDED(hr);
}

    // ==========================================================
    // CONSTRUCTOR
    // ==========================================================
    Menu::Menu() 
    {
        _instance = this;
    }

    // ==========================================================
    // SISTEMA DE LENGUAJES (I18N)
    // ==========================================================
    void Menu::SetupLanguages() 
{
    // Usamos una sintaxis de inicialización limpia
    languages["ES"] = {
        {"launch", "INICIAR SISTEMA CORE"}, 
        {"admin_btn", "🛡️ PANEL ADMIN"},
        {"settings", "⚙️ AJUSTES"}, 
        {"nodes", "👤 NÓDULOS ACTIVOS"},
        {"mint", "🔑 GENERAR LICENCIAS"}, 
        {"target", "OBJETIVO BINARIO"},
        {"execute", "EJECUTAR BYPASS NEURAL"}, 
        {"lang_title", "CONFIGURACIÓN"},
        {"back", "← VOLVER"}, 
        {"success_reg", "CUENTA ACTIVADA"},
        {"refresh", "RECARGAR"}, 
        {"unlock", "DESBLOQUEAR HWID"},
        {"tier", "RANGO"}, 
        {"amount", "CANTIDAD"}, 
        {"mint_btn", "MINTEAR LLAVES"},
        {"status_idle", "SISTEMA EN ESPERA"}
    };

    languages["EN"] = {
        {"launch", "LAUNCH CORE SYSTEM"}, 
        {"admin_btn", "🛡️ ADMIN PANEL"},
        {"settings", "⚙️ SETTINGS"}, 
        {"nodes", "👤 ACTIVE NODES"},
        {"mint", "🔑 LICENSE MINT"}, 
        {"target", "TARGET BINARY"},
        {"execute", "EXECUTE NEURAL BYPASS"}, 
        {"lang_title", "SETTINGS"},
        {"back", "← BACK"}, 
        {"success_reg", "ACCOUNT ACTIVATED"},
        {"refresh", "REFRESH"}, 
        {"unlock", "UNLOCK HWID"},
        {"tier", "TIER"}, 
        {"amount", "AMOUNT"}, 
        {"mint_btn", "MINT KEYS"},
        {"status_idle", "SYSTEM IDLE"}
    };
}

    std::string Menu::GetText(const std::string& key) 
    {
        if (languages.find(currentLang) != languages.end()) 
        {
            if (languages[currentLang].find(key) != languages[currentLang].end()) 
            {
                return languages[currentLang][key];
            }
        }
        return key; // Devuelve la llave original si no encuentra traducción
    }

void Menu::ApplyCyberNeonTheme() {
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    // --- GEOMETRÍA PREMIUM ---
    style.WindowRounding    = 12.0f; 
    style.ChildRounding     = 10.0f;
    style.FrameRounding     = 6.0f;  
    style.PopupRounding     = 10.0f;
    style.ScrollbarRounding = 12.0f;
    style.GrabRounding      = 6.0f;

    style.WindowBorderSize  = 1.0f;
    style.ChildBorderSize   = 1.0f;
    style.FrameBorderSize   = 1.0f; 

    style.WindowPadding     = ImVec2(20, 20);
    style.FramePadding      = ImVec2(12, 8); 
    style.ItemSpacing       = ImVec2(10, 12);
    
    // --- PALETA ELITE (VIOLETA & OBSIDIANA) ---
    
    // Window Bg: Un negro azulado con opacidad alta pero no total
    colors[ImGuiCol_WindowBg]         = ImVec4(0.05f, 0.05f, 0.07f, 0.94f); 
    colors[ImGuiCol_ChildBg]          = ImVec4(0.00f, 0.00f, 0.00f, 0.00f); // Transparente para usar el de abajo
    colors[ImGuiCol_PopupBg]          = ImVec4(0.08f, 0.08f, 0.10f, 0.98f);
    
    // El Borde: El violeta "Scanneler" (Brillo neón sutil)
    colors[ImGuiCol_Border]           = ImVec4(0.55f, 0.36f, 0.96f, 0.35f); 
    
    // Botones (Estilo Glass): Opacidad baja para el botón, alta para el hover
    colors[ImGuiCol_Button]           = ImVec4(0.55f, 0.36f, 0.96f, 0.20f); 
    colors[ImGuiCol_ButtonHovered]    = ImVec4(0.55f, 0.36f, 0.96f, 0.50f);
    colors[ImGuiCol_ButtonActive]     = ImVec4(0.55f, 0.36f, 0.96f, 0.80f);

    // Inputs
    colors[ImGuiCol_FrameBg]          = ImVec4(1.00f, 1.00f, 1.00f, 0.03f); // Muy sutil
    colors[ImGuiCol_FrameBgHovered]   = ImVec4(1.00f, 1.00f, 1.00f, 0.08f);
    colors[ImGuiCol_FrameBgActive]    = ImVec4(0.55f, 0.36f, 0.96f, 0.25f);
}

void Menu::DrawCyberGrid(ImDrawList* drawList, ImVec2 pos, ImVec2 size) 
{
    // --- CONFIGURACIÓN DE COLORES ---
    // Usamos el violeta Scanneler pero con opacidad dinámica
    ImU32 lineColor      = IM_COL32(139, 92, 246, 35); // Violeta base sutil
    ImU32 thickLineColor = IM_COL32(139, 92, 246, 70); // Ejes con más presencia
    
    float step = 60.0f; // Espaciado optimizado para no saturar la vista
    
    // Animación de flujo constante (Scroll infinito)
    static float offset = 0.0f;
    offset += ImGui::GetIO().DeltaTime * 15.0f; 
    if (offset > step) offset -= step;

    // --- RENDERIZADO DE LÍNEAS VERTICALES ---
    for (float x = fmodf(size.x / 2.0f, step) + offset - step; x < size.x; x += step) 
    {
        int gridIdx = (int)((x - offset) / step);
        bool isThick = (gridIdx % 4 == 0);
        
        // Dibujamos la línea con un efecto de degradado de altura (opcional pero elegante)
        drawList->AddLine(
            ImVec2(pos.x + x, pos.y), 
            ImVec2(pos.x + x, pos.y + size.y), 
            isThick ? thickLineColor : lineColor, 
            1.2f
        );
    }
    
    // --- RENDERIZADO DE LÍNEAS HORIZONTALES ---
    for (float y = fmodf(size.y / 2.0f, step) + offset - step; y < size.y; y += step) 
    {
        int gridIdx = (int)((y - offset) / step);
        bool isThick = (gridIdx % 4 == 0);
        
        drawList->AddLine(
            ImVec2(pos.x, pos.y + y), 
            ImVec2(pos.x + size.x, pos.y + y), 
            isThick ? thickLineColor : lineColor, 
            1.2f
        );
    }

    // --- CAPA DE POST-PROCESADO: VIGNETTE & RADIAL GLOW ---
    // 1. Sombra circular periférica (CSS Radial Gradient simulation)
    ImVec2 center = ImVec2(pos.x + size.x * 0.5f, pos.y + size.y * 0.5f);
    
    // Dibujamos capas de círculos huecos con opacidad creciente hacia afuera
    for (int i = 0; i < 12; i++) {
        float radius = (size.x > size.y ? size.x : size.y) * (0.4f + i * 0.05f);
        drawList->AddCircle(center, radius, IM_COL32(0, 0, 0, i * 15), 100, 40.0f);
    }

    // 2. Viñeta en las esquinas (Multicolor)
    drawList->AddRectFilledMultiColor(pos, ImVec2(pos.x + size.x, pos.y + size.y),
        IM_COL32(5, 5, 10, 200), IM_COL32(5, 5, 10, 200), // Esquinas superiores
        IM_COL32(5, 5, 10, 240), IM_COL32(5, 5, 10, 240)); // Esquinas inferiores (más oscuras)
}

void Menu::DrawCyberAlert() 
    {
        if (!showAlert) return;

        // Overlay de fondo (Blur simulado con opacidad alta)
        ImGui::PushStyleColor(ImGuiCol_ModalWindowDimBg, ImVec4(0.02f, 0.02f, 0.04f, 0.85f));
        ImGui::OpenPopup("CyberAlertPopup");

        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(460, 250));

        // Configuración de color basada en el estado
        ImVec4 accentColor = alertIsError ? ImVec4(1.0f, 0.25f, 0.3f, 1.0f) : ImVec4(0.55f, 0.36f, 0.96f, 1.0f);
        ImU32 accentU32 = ImGui::ColorConvertFloat4ToU32(accentColor);

        ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.08f, 0.08f, 0.12f, 1.00f));
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(accentColor.x, accentColor.y, accentColor.z, 0.45f));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.5f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 12.0f);

        if (ImGui::BeginPopupModal("CyberAlertPopup", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove)) 
        {
            ImDrawList* drawList = ImGui::GetWindowDrawList();
            ImVec2 p = ImGui::GetWindowPos();
            ImVec2 s = ImGui::GetWindowSize();

            // --- 1. EFECTO DE GLOW EXTERIOR (DROP SHADOW NEÓN) ---
            for (int i = 0; i < 10; i++) {
                drawList->AddRect(ImVec2(p.x - i, p.y - i), ImVec2(p.x + s.x + i, p.y + s.y + i), 
                    ImGui::ColorConvertFloat4ToU32(ImVec4(accentColor.x, accentColor.y, accentColor.z, (0.15f - i * 0.015f))), 12.0f, 0, 1.5f);
            }

            // --- 2. INDICADOR SUPERIOR (SCANNING LINE) ---
            float lineGlow = (sin(ImGui::GetTime() * 4.0f) * 0.5f) + 0.5f;
            drawList->AddRectFilledMultiColor(p, ImVec2(p.x + s.x, p.y + 3), 
                accentU32, accentU32, IM_COL32(0,0,0,0), IM_COL32(0,0,0,0));
            
            // --- 3. HEADER & TÍTULO ---
            ImGui::SetCursorPosY(35);
            ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);
            
            const char* headerIcon = alertIsError ? "[!] CRITICAL ERROR" : "[>] SYSTEM NOTIFICATION";
            float tw = ImGui::CalcTextSize(headerIcon).x;
            ImGui::SetCursorPosX((s.x - tw) * 0.5f);
            ImGui::TextColored(accentColor, headerIcon);
            ImGui::PopFont();

            // --- 4. CUERPO DEL MENSAJE (TIPO TERMINAL) ---
            ImGui::SetCursorPos(ImVec2(40, 90));
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.85f, 0.85f, 0.90f, 0.9f));
            ImGui::PushTextWrapPos(s.x - 40);
            ImGui::TextWrapped("%s", alertMessage.c_str());
            ImGui::PopTextWrapPos();
            ImGui::PopStyleColor();

            // --- 5. BOTÓN DE CIERRE (ESTILO CSS ACTION) ---
            ImGui::SetCursorPos(ImVec2(40, s.y - 70));
            
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(accentColor.x, accentColor.y, accentColor.z, 0.12f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(accentColor.x, accentColor.y, accentColor.z, 0.25f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(accentColor.x, accentColor.y, accentColor.z, 0.40f));
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
            
            if (ImGui::Button("D I S M I S S", ImVec2(s.x - 80, 45))) 
            {
                showAlert = false;
                ImGui::CloseCurrentPopup();
            }
            if (ImGui::IsItemHovered()) ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);

            ImGui::PopStyleVar();
            ImGui::PopStyleColor(3);

            ImGui::EndPopup();
        }
        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor(4);
    }

void Menu::DrawSplashScreen() 
    {
        static std::vector<std::string> steps = {
            "BOOTING NEURAL KERNEL...", 
            "ESTABLISHING ENCRYPTED LINK...", 
            "DECRYPTING RENDER TOKENS...", 
            "SYSTEM VITALITY: OPTIMAL"
        };
        
        static auto startTime = std::chrono::steady_clock::now();
        auto currentTime = std::chrono::steady_clock::now();
        float elapsedTime = std::chrono::duration<float>(currentTime - startTime).count();
        float deltaTime = ImGui::GetIO().DeltaTime;

        // --- 1. LÓGICA DE PROGRESO "LIQUID FLOW" ---
        if (elapsedTime > 0.8f && splashStep < (int)steps.size()) 
        {
            splashStep++;
            splashProgress = (float)splashStep / (float)steps.size();
            startTime = currentTime; 
        }

        // Interpolación suavizada (Lerp) para evitar saltos bruscos en la barra
        static float visualProgress = 0.0f;
        visualProgress = ImLerp(visualProgress, splashProgress, deltaTime * 2.8f);

        // Umbral de finalización con Fade-out virtual
        if (splashStep >= (int)steps.size() && visualProgress >= 0.99f) 
        {
            static float fadeOut = 1.0f;
            fadeOut -= deltaTime * 2.0f;
            if (fadeOut <= 0.0f) {
                currentState = AppState::LoginScreen;
                return;
            }
        }

        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImVec2 screenSize = viewport->WorkSize;
        ImDrawList* drawList = ImGui::GetBackgroundDrawList(); 
        ImVec2 center = ImVec2(screenSize.x * 0.5f, screenSize.y * 0.5f);

        // --- 2. RENDERIZADO DEL TÍTULO (CYBER BLOOM) ---
        const char* title = "S C A N N E L E R";
        float titleFontSize = 42.0f; 
        ImVec2 titleSize = ImGui::CalcTextSize(title);
        ImVec2 titlePos = ImVec2(center.x - titleSize.x * 0.5f, center.y - 100.0f);

        // Efecto de aberración cromática sutil (Glow capas)
        drawList->AddText(NULL, titleFontSize, titlePos + ImVec2(2, 0), IM_COL32(255, 0, 80, 40), title); // Red shift
        drawList->AddText(NULL, titleFontSize, titlePos - ImVec2(2, 0), IM_COL32(0, 255, 255, 40), title); // Cyan shift
        
        // Resplandor central (Bloom)
        for (int i = 0; i < 3; i++)
            drawList->AddText(NULL, titleFontSize, titlePos, IM_COL32(139, 92, 246, 50 / (i + 1)), title);
            
        drawList->AddText(NULL, titleFontSize, titlePos, IM_COL32(255, 255, 255, 255), title);

        // --- 3. BARRA DE PROGRESO "NEURAL LINK" ---
        float barWidth = 500.0f;
        if (barWidth > screenSize.x * 0.8f) barWidth = screenSize.x * 0.8f;
        float barHeight = 2.0f;
        ImVec2 barPos(center.x - barWidth * 0.5f, center.y + 20.0f);

        // Carril (Track) con diseño segmentado sutil
        drawList->AddRectFilled(barPos, barPos + ImVec2(barWidth, barHeight), IM_COL32(255, 255, 255, 10), 10.0f);

        // Relleno (Fill) con gradiente dinámico
        float fillWidth = barWidth * visualProgress;
        if (fillWidth > 1.0f) {
            // Gradiente: Violeta -> Cian -> Violeta (Efecto de energía fluyendo)
            drawList->AddRectFilledMultiColor(
                barPos, barPos + ImVec2(fillWidth, barHeight),
                IM_COL32(139, 92, 246, 255), IM_COL32(0, 255, 255, 255), 
                IM_COL32(0, 255, 255, 255), IM_COL32(139, 92, 246, 255)
            );

            // Cabezal de la barra (Flare)
            float flareX = barPos.x + fillWidth;
            drawList->AddCircleFilled(ImVec2(flareX, barPos.y + 1), 4.0f, IM_COL32(255, 255, 255, 255));
            drawList->AddCircleFilled(ImVec2(flareX, barPos.y + 1), 15.0f, IM_COL32(0, 255, 255, 30)); // Glow circular
        }

        // --- 4. STATUS FEED (TEXTO DE TERMINAL) ---
        std::string statusMsg = (splashStep < (int)steps.size()) ? steps[splashStep] : "NEURAL LINK ESTABLISHED";
        
        // Porcentaje a la derecha con opacidad variable
        char perc[16]; snprintf(perc, 16, "%d%%", (int)(visualProgress * 100));
        
        drawList->AddText(barPos + ImVec2(0, 15), IM_COL32(0, 255, 255, 180), statusMsg.c_str());
        
        float percW = ImGui::CalcTextSize(perc).x;
        drawList->AddText(barPos + ImVec2(barWidth - percW, 15), IM_COL32(255, 255, 255, 80), perc);

        // --- 5. DETALLE COSMÉTICO: SCANLINE LOCALIZADO ---
        static float scanlineY = 0.0f;
        scanlineY += deltaTime * 300.0f;
        if (scanlineY > screenSize.y) scanlineY = 0.0f;
        
        drawList->AddLine(ImVec2(0, scanlineY), ImVec2(screenSize.x, scanlineY), IM_COL32(255, 255, 255, 15), 1.0f);
    }

// ==========================================================
// LOGIN FRAME (PREMIUM UI - ELITE 2026)
// ==========================================================
void Menu::DrawLoginFrame() 
{
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    float dt = ImGui::GetIO().DeltaTime;
    
    float panelW = 440.0f;
    float panelH = 640.0f; 

    ImGui::SetNextWindowPos(viewport->GetCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(panelW + 60, panelH + 60)); 

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | 
                             ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBackground;

    if (ImGui::Begin("LoginMaster", NULL, flags)) 
    {
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImVec2 p = ImGui::GetWindowPos() + ImVec2(30, 30);

        // --- 1. LÓGICA DE ELEVACIÓN ---
        static float panelHoverScale = 0.0f;
        bool isOverPanel = ImGui::IsMouseHoveringRect(p, p + ImVec2(panelW, panelH));
        panelHoverScale = ImLerp(panelHoverScale, isOverPanel ? 1.0f : 0.0f, dt * 4.0f);

        // --- 2. MULTI-LAYER BOX SHADOW ---
        for (int i = 0; i < 20; i++) {
            float shadowAlpha = (25.0f - i) * (1.0f + panelHoverScale * 0.5f);
            drawList->AddRect(p - ImVec2((float)i, (float)i), p + ImVec2(panelW + i, panelH + i), 
                              IM_COL32(0, 0, 0, (int)shadowAlpha), 20.0f, 0, 1.5f);
        }

        // --- 3. CUERPO GLASSMORPHISM ---
        drawList->AddRectFilled(p, p + ImVec2(panelW, panelH), IM_COL32(12, 12, 18, 253), 18.0f);
        drawList->AddRect(p, p + ImVec2(panelW, panelH), IM_COL32(255, 255, 255, 15), 18.0f, 0, 1.0f);

        // --- 4. BRANDING ---
        drawList->AddText(NULL, 32.0f, p + ImVec2(40, 60), IM_COL32(255, 255, 255, 240), "S C A N N E L E R");
        drawList->AddText(p + ImVec2(40, 100), IM_COL32(100, 110, 140, 255), "CORE_SYSTEM_ACCESS_v4.2");

        // --- 5. INPUTS CUSTOM ---
        auto DrawEliteInput = [&](const char* label, char* buf, bool isPass, float y) 
        {
            ImGui::SetCursorPos(ImVec2(40, y));
            ImVec2 inSize(360, 55);
            ImVec2 curPos = ImGui::GetCursorScreenPos();
            
            drawList->AddRectFilled(curPos, curPos + inSize, IM_COL32(20, 20, 30, 255), 10.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(15, 16));
            ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(0,0,0,0)); 
            ImGui::PushItemWidth(inSize.x);
            
            std::string id = std::string("##") + label;
            if (isPass) ImGui::InputTextWithHint(id.c_str(), label, buf, 256, ImGuiInputTextFlags_Password);
            else ImGui::InputTextWithHint(id.c_str(), label, buf, 256);

            bool active = ImGui::IsItemActive();
            inputAnims[label] = ImLerp(inputAnims[label], active ? 1.0f : 0.0f, dt * 12.0f);
            ImU32 bordCol = ImGui::ColorConvertFloat4ToU32(ImLerp(ImVec4(0.2f, 0.2f, 0.3f, 0.3f), ImVec4(0.55f, 0.36f, 0.96f, 1.0f), inputAnims[label]));
            drawList->AddRect(curPos, curPos + inSize, bordCol, 10.0f, 0, 1.5f);
            
            ImGui::PopItemWidth();
            ImGui::PopStyleColor();
            ImGui::PopStyleVar();
        };

        DrawEliteInput("OPERATIVE_ID", userBuffer, false, 190);
        DrawEliteInput("SECURITY_KEY", passBuffer, true, 265);

        static bool showRedeem = false;
        if (showRedeem) DrawEliteInput("LICENSE_TOKEN", keyBuffer, false, 340);

        // --- 6. BOTÓN DE ACCIÓN (LÓGICA MEJORADA) ---
        float btnY = showRedeem ? 430.0f : 350.0f;
        ImGui::SetCursorPos(ImVec2(40, btnY));
        
        static float btnPulse = 0.0f;
        btnPulse += dt * 2.0f;
        float glowAnim = (sin(btnPulse) * 0.5f) + 0.5f;

        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 10.0f);
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.55f, 0.36f, 0.96f, 0.8f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.60f, 0.40f, 1.0f, 1.0f));
        
        if (ImGui::Button(isConnecting ? "SYNCHRONIZING..." : "E X E C U T E  L O G I N", ImVec2(360, 60)) && !isConnecting) 
        {
            isConnecting = true;
            isLoginError = false;
            loginStatus = "CONTACTING CORE...";

            std::thread([&]() {
                auto response = Auth::ApiClient::ValidateLogin(Menu::userBuffer, Menu::passBuffer);

                if (response.success) {
                    // --- LOG DE DIAGNÓSTICO (Mira tu consola de Visual Studio) ---
                    printf("[AUTH] Raw Role Received: '%s'\n", response.role.c_str());

                    // --- NORMALIZACIÓN AGRESIVA ---
                    std::string r = response.role;
                    // Pasar a minúsculas
                    std::transform(r.begin(), r.end(), r.begin(), ::tolower);
                    // Quitar espacios
                    r.erase(std::remove_if(r.begin(), r.end(), ::isspace), r.end());

                    // --- BÚSQUEDA FLEXIBLE ---
                    // Si el string contiene "admin" o es exactamente "1", activamos Admin
                    if (r.find("admin") != std::string::npos || r == "1") {
                        Menu::isAdmin = true;
                        printf("[AUTH] Admin Privileges: ENABLED\n");
                    } else {
                        Menu::isAdmin = false;
                        printf("[AUTH] Admin Privileges: DISABLED\n");
                    }

                    Menu::loginStatus = "ACCESS GRANTED. INITIALIZING...";
                    std::this_thread::sleep_for(std::chrono::milliseconds(800));
                    Menu::currentState = AppState::MainScreen; 
                } else {
                    Menu::loginStatus = "DENIED: " + response.message;
                    Menu::isLoginError = true;
                }
                Menu::isConnecting = false;
            }).detach();
        }

        if (ImGui::IsItemHovered()) {
            ImVec2 bMin = ImGui::GetItemRectMin();
            ImVec2 bMax = ImGui::GetItemRectMax();
            drawList->AddRect(bMin, bMax, IM_COL32(139, 92, 246, (int)(100 * glowAnim)), 10.0f, 0, 4.0f);
        }
        ImGui::PopStyleColor(2);
        ImGui::PopStyleVar();

        // --- 7. STATUS FEEDBACK ---
        if (!loginStatus.empty()) {
            ImGui::SetCursorPos(ImVec2(40, btnY + 75));
            ImVec4 statusCol = isLoginError ? ImVec4(1.0f, 0.3f, 0.3f, 1.0f) : ImVec4(0.3f, 1.0f, 0.6f, 1.0f);
            ImGui::TextColored(statusCol, "[>] %s", loginStatus.c_str());
        }

        // --- 8. FOOTER ---
        ImGui::SetCursorPos(ImVec2(40, panelH - 50));
        const char* toggleText = showRedeem ? "RETURN TO STANDARD AUTH" : "MINT NEW LICENSE ACCESS";
        ImVec2 tSize = ImGui::CalcTextSize(toggleText);
        
        if (ImGui::Selectable(toggleText, false, 0, tSize)) {
            showRedeem = !showRedeem;
            Menu::loginStatus = "";
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
            ImVec2 min = ImGui::GetItemRectMin();
            drawList->AddLine(min + ImVec2(0, tSize.y), min + tSize, IM_COL32(139, 92, 246, 200));
        }
    }
    ImGui::End();
}

// ==========================================================
// ADMIN FRAME (COMMAND CENTER ELITE - ADAPTATIVO 2026)
// ==========================================================
void Menu::DrawAdminFrame() 
{
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImVec2 screenSize = viewport->WorkSize;

    float panelW = screenSize.x * 0.94f; 
    float panelH = screenSize.y * 0.90f;

    ImGui::SetNextWindowPos(viewport->GetCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(panelW + 40, panelH + 40));

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | 
                             ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | 
                             ImGuiWindowFlags_NoBackground;

    if (ImGui::Begin("AdminMasterPanel", NULL, flags)) 
    {
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImVec2 p = ImGui::GetWindowPos() + ImVec2(20, 20);

        // --- 1. AMBIENTE TÁCTICO (GLOW ROJO) ---
        for (int i = 0; i < 15; i++)
            drawList->AddRect(p - ImVec2((float)i, (float)i), p + ImVec2(panelW + i, panelH + i), 
                              IM_COL32(255, 30, 30, 15 - i), 16.0f, 0, 1.5f);

        drawList->AddRectFilled(p, p + ImVec2(panelW, panelH), IM_COL32(10, 10, 14, 252), 16.0f);
        drawList->AddRect(p, p + ImVec2(panelW, panelH), IM_COL32(255, 255, 255, 10), 16.0f, 0, 1.0f);

        // --- 2. HEADER ---
        ImGui::SetCursorPos(ImVec2(40, 40));
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 1, 1, 0.05f));
        if (ImGui::Button("< EXIT_ROOT", ImVec2(120, 35))) currentState = AppState::MainScreen;
        ImGui::PopStyleColor(); // Pop de EXIT_ROOT

        ImGui::SameLine(panelW - 350);
        ImGui::SetCursorPosY(42);
        ImGui::TextColored(ImVec4(1.0f, 0.2f, 0.25f, 1.0f), "[!] ADMINISTRATIVE_OVERRIDE_ACTIVE");

        // --- 3. PESTAÑAS (PUSH DE ESTILOS DE BARRA) ---
        ImGui::SetCursorPos(ImVec2(40, 100));
        ImGui::PushStyleColor(ImGuiCol_TabActive, ImVec4(1.0f, 0.1f, 0.2f, 0.25f));
        ImGui::PushStyleColor(ImGuiCol_TabHovered, ImVec4(1.0f, 0.1f, 0.2f, 0.15f));
        ImGui::PushStyleColor(ImGuiCol_Tab, ImVec4(0, 0, 0, 0));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(25, 0));

        if (ImGui::BeginTabBar("AdminTabs", ImGuiTabBarFlags_NoTooltip)) 
        {
            // --- TAB 1: DATABASE ---
            if (ImGui::BeginTabItem("   DATABASE_NODES   ")) 
            {
                ImGui::SetCursorPosY(160);
                ImGui::SetCursorPosX(40);
                
                if (ImGui::Button("RE-SYNC_DATABASE", ImVec2(180, 42))) FetchUsersFromDB();
                ImGui::SameLine();
                if (ImGui::Button("GLOBAL_HWID_RELEASE", ImVec2(180, 42))) showHwidResetModal = true;

                ImGui::Dummy(ImVec2(0, 15));

                ImGui::SetCursorPosX(40);
                ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(1, 1, 1, 0.01f));
                if (ImGui::BeginChild("ScrollableTable", ImVec2(panelW - 80, panelH - 280), true)) 
                {
                    ImGui::Columns(4, "NodeColumns", false);
                    ImGui::SetColumnWidth(0, 250); 
                    
                    ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.6f, 1.0f), "IDENTIFIER"); ImGui::NextColumn();
                    ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.6f, 1.0f), "HARDWARE"); ImGui::NextColumn();
                    ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.6f, 1.0f), "ACCESS_LEVEL"); ImGui::NextColumn();
                    ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.6f, 1.0f), "OPERATIONS"); ImGui::NextColumn();
                    ImGui::Separator();

                    for (size_t i = 0; i < cachedUsers.size(); i++) 
                    {
                        auto& u = cachedUsers[i];
                        ImGui::PushID((int)i);
                        
                        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10);
                        ImGui::Text("%s", u.username.c_str()); ImGui::NextColumn();

                        bool isLocked = (u.hwid != "NONE");
                        ImVec4 statusCol = isLocked ? ImVec4(1, 0.1f, 0.1f, 1) : ImVec4(0, 1, 0.7f, 1);
                        ImGui::TextColored(statusCol, isLocked ? "LOCKED" : "READY"); ImGui::NextColumn();

                        ImGui::PushItemWidth(140);
                        const char* plans[] = { "Weekly", "Monthly", "Yearly", "Lifetime" };
                        int curIdx = 0; 
                        for(int j=0; j<4; j++) if(u.plan == plans[j]) curIdx = j;
                        
                        if (ImGui::Combo("##p", &curIdx, plans, 4)) 
                            Auth::ApiClient::UpdateMembership(u.username, plans[curIdx]);
                        ImGui::PopItemWidth(); 
                        ImGui::NextColumn();

                        if (ImGui::Button("PURGE", ImVec2(80, 30))) { 
                            userToDelete = u.username; 
                            showDeleteModal = true; 
                        }
                        ImGui::NextColumn();

                        ImGui::Separator();
                        ImGui::PopID();
                    }
                }
                ImGui::EndChild();
                ImGui::PopStyleColor(); // Pop del ChildBg
                ImGui::EndTabItem();
            }

            // --- TAB 2: FORGE ---
            if (ImGui::BeginTabItem("   LICENSE_FORGE   ")) 
            {
                ImGui::SetCursorPos(ImVec2(60, 180));
                ImGui::BeginGroup();
                static int fTier = 0; static int fQty = 1;
                const char* tiers[] = { "Weekly", "Monthly", "Yearly", "Lifetime" };
                ImGui::PushItemWidth(320);
                ImGui::Combo("TIER_TYPE", &fTier, tiers, 4);
                ImGui::InputInt("QUANTITY", &fQty);
                ImGui::PopItemWidth();

                if (ImGui::Button("EXECUTE_MINT_PROTOCOL", ImVec2(320, 60))) {
                    auto res = Auth::ApiClient::GenerateKey(tiers[fTier], fQty);
                    if(res.success) {
                        generatedKeysOutput.clear();
                        for(const auto& k : res.keys) generatedKeysOutput += k + "\n";
                    }
                }
                ImGui::EndGroup();

                ImGui::SameLine(panelW - 540);
                ImGui::BeginGroup();
                ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(5, 5, 10, 200));
                ImGui::InputTextMultiline("##keys", (char*)generatedKeysOutput.c_str(), 8192, ImVec2(500, 350), ImGuiInputTextFlags_ReadOnly);
                ImGui::PopStyleColor(); // Pop del FrameBg Multiline
                ImGui::EndGroup();

                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }

        // --- CIERRE DE STACK DE ESTILOS (Sincronizado con el principio de la función) ---
        ImGui::PopStyleColor(3); // Pop de TabActive, Hovered y Tab
        ImGui::PopStyleVar();    // Pop de ItemSpacing
    }
    ImGui::End(); // Fin de AdminMasterPanel
}

// ==========================================================|
// MAIN FRAME (CORE CONTROL HUB - FUTURISTA 2026)            |   
// ==========================================================|
void Menu::DrawMainFrame() 
{
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImVec2 center = viewport->GetCenter();
    float dt = ImGui::GetIO().DeltaTime;

    // --- LINEA DE DEBUG (F9) ---
    if (ImGui::IsKeyPressed(ImGuiKey_F9)) Menu::isAdmin = !Menu::isAdmin;

    // Dimensiones para layout horizontal
    float panelW = 720.0f; 
    float panelH = 460.0f; 

    ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(panelW + 40, panelH + 40));

    if (ImGui::Begin("MainSelector", NULL, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground)) 
    {
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImVec2 p = ImGui::GetWindowPos() + ImVec2(20, 20);

        // --- 1. FONDO Y BORDE ---
        drawList->AddRectFilled(p, p + ImVec2(panelW, panelH), IM_COL32(10, 10, 15, 252), 12.0f);
        ImU32 accentCol = Menu::isAdmin ? IM_COL32(255, 50, 50, 200) : IM_COL32(139, 92, 246, 120);
        drawList->AddRect(p, p + ImVec2(panelW, panelH), accentCol, 12.0f, 0, 2.0f);

        // --- 2. LOGO PNG CENTRADO ---
        float imgSize = 160.0f;
        // Calculamos el centro exacto del panel para la imagen
        ImVec2 imgPos = ImVec2(p.x + (panelW / 2.0f) - (imgSize / 2.0f), p.y + 90);

        if (Menu::logoTexture != nullptr) {
            // Dibujamos el Scanneler.png real
            drawList->AddImage((void*)Menu::logoTexture, imgPos, imgPos + ImVec2(imgSize, imgSize));
        } else {
            // Placeholder si la imagen no carga (Círculo estético)
            drawList->AddCircle(imgPos + ImVec2(imgSize/2, imgSize/2), imgSize/2, accentCol, 100, 1.5f);
            drawList->AddText(NULL, 18.0f, imgPos + ImVec2(imgSize/4, imgSize/2.2f), IM_COL32(255, 255, 255, 50), "NO_LOGO");
        }

        // --- 3. HEADER TITULO ---
        const char* title = "S C A N N E L E R   C O R E";
        ImVec2 titleSize = ImGui::CalcTextSize(title);
        ImGui::SetCursorPos(ImVec2((panelW - titleSize.x) / 2.0f + 20, 50));
        ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);
        ImGui::TextColored(ImVec4(1, 1, 1, 0.8f), title);
        ImGui::PopFont();
        
        // Separador sutil bajo el logo
        drawList->AddLine(p + ImVec2(panelW * 0.2f, 270), p + ImVec2(panelW * 0.8f, 270), IM_COL32(255, 255, 255, 15));

        // --- 4. BOTONES HORIZONTALES (BARRA INFERIOR) ---
        float btnW = 190.0f;
        float btnH = 50.0f;
        float spacing = 20.0f;
        
        // Cálculo de centrado dinámico para los botones
        int totalBtns = Menu::isAdmin ? 3 : 2;
        float totalWidth = (btnW * totalBtns) + (spacing * (totalBtns - 1));
        float startX = (panelW - totalWidth) / 2.0f + 20;

        ImGui::SetCursorPos(ImVec2(startX, panelH - 90));

        // [BOTÓN 1: GHOST]
        if (ImGui::Button("GHOST PROTOCOL", ImVec2(btnW, btnH))) {
            currentState = AppState::BypassScreen;
        }

        ImGui::SameLine(0, spacing);

        // [BOTÓN 2: ADMIN]
        if (Menu::isAdmin) 
        {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.1f, 0.15f, 0.2f));
            if (ImGui::Button("ADMIN PANEL", ImVec2(btnW, btnH))) {
                FetchUsersFromDB(); 
                currentState = AppState::AdminScreen;
            }
            ImGui::PopStyleColor();
            ImGui::SameLine(0, spacing);
        }

        // [BOTÓN 3: SETTINGS]
        if (ImGui::Button("SETTINGS", ImVec2(btnW, btnH))) {
            showSettings = true;
        }

        // --- 5. LOGOUT (ESTILO TERMINAL ABAJO) ---
        ImGui::SetCursorPos(ImVec2(20, panelH - 25));
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
        if (ImGui::Selectable("SYS_LOGOUT", false, 0, ImGui::CalcTextSize("SYS_LOGOUT"))) {
            Menu::isAdmin = false; 
            Menu::currentState = AppState::LoginScreen;
        }
        ImGui::PopStyleColor();
        
        if (ImGui::IsItemHovered()) ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
    }
    ImGui::End();

    if (showSettings) DrawSettingsFrame();
}

// ==========================================================
// SETTINGS FRAME (MODAL OVERLAY)
// ==========================================================
void Menu::DrawSettingsFrame() 
{
    ImGui::OpenPopup("SystemSettings");
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(450, 400));

    ImGui::PushStyleColor(ImGuiCol_PopupBg, IM_COL32(15, 15, 20, 255));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 12.0f);

    if (ImGui::BeginPopupModal("SystemSettings", &showSettings, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize)) 
    {
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImVec2 p = ImGui::GetWindowPos();
        ImVec2 s = ImGui::GetWindowSize();

        drawList->AddText(NULL, 22.0f, p + ImVec2(30, 25), IM_COL32(139, 92, 246, 255), "CONFIG_SYSTEM");
        ImGui::SetCursorPosY(70);

        // --- SECCIÓN: IDIOMA ---
        ImGui::Indent(20);
        ImGui::TextColored(ImVec4(1, 1, 1, 0.5f), "LANGUAGE_REGION");
        const char* langs[] = { "ENGLISH", "ESPAÑOL" };
        int curLangIdx = (currentLang == "EN") ? 0 : 1;
        
        ImGui::PushItemWidth(370);
        if (ImGui::Combo("##lang", &curLangIdx, langs, 2)) {
            currentLang = (curLangIdx == 0) ? "EN" : "ES";
        }
        ImGui::PopItemWidth();

        ImGui::Dummy(ImVec2(0, 20));

        // --- SECCIÓN: HOTKEYS ---
        ImGui::TextColored(ImVec4(1, 1, 1, 0.5f), "EMERGENCY_BYPASS_HOTKEY");
        
        std::string btnText = isBindingHotkey ? "< PRESS ANY KEY >" : GetKeyName(bypassHotkey);
        if (ImGui::Button(btnText.c_str(), ImVec2(370, 45))) {
            isBindingHotkey = true;
        }

        // Lógica para capturar la tecla
        if (isBindingHotkey) {
            for (int i = 0; i < 512; i++) {
                if (GetAsyncKeyState(i) & 0x8000) {
                    if (i != VK_LBUTTON && i != VK_RBUTTON) {
                        bypassHotkey = i;
                        isBindingHotkey = false;
                        break;
                    }
                }
            }
        }

        ImGui::Dummy(ImVec2(0, 20));

        // --- SECCIÓN: OPCIONES DE ESCANEO ---
        ImGui::Checkbox("ENABLE_DEEP_KERNEL_CLEAN", &deepScanEnabled);
        ImGui::TextWrapped("Deep scan removes registry keys that standard methods might miss.");

        // Botón de Cierre
        ImGui::SetCursorPos(ImVec2(30, s.y - 65));
        if (ImGui::Button("SAVE & EXIT", ImVec2(390, 45))) {
            showSettings = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
}



// ==========================================================
    // BYPASS FRAME (GHOST PROTOCOL ELITE - ADAPTATIVO 2026)
    // ==========================================================
    void Menu::DrawBypassFrame() 
    {
        // FIX DE CONTEXTO: Aseguramos la persistencia de la instancia
        
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImVec2 screenSize = viewport->WorkSize;
        float dt = ImGui::GetIO().DeltaTime;
        
        float panelW = screenSize.x * 0.94f;
        float panelH = screenSize.y * 0.90f;

        ImGui::SetNextWindowPos(viewport->GetCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(panelW + 40, panelH + 40));

        if (ImGui::Begin("BypassMasterPanel", NULL, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBackground)) 
        {
            ImDrawList* drawList = ImGui::GetWindowDrawList();
            ImVec2 p = ImGui::GetWindowPos() + ImVec2(20, 20);

            // --- 1. FONDO TÁCTICO CON GLOW ---
            drawList->AddRectFilled(p, p + ImVec2(panelW, panelH), IM_COL32(10, 10, 15, 252), 12.0f);
            drawList->AddRect(p, p + ImVec2(panelW, panelH), IM_COL32(139, 92, 246, 60), 12.0f, 0, 1.5f);

            // --- 2. HEADER DINÁMICO ---
            ImGui::SetCursorPos(ImVec2(40, 40));
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 1, 1, 0.05f));
            if (ImGui::Button("< ABORT", ImVec2(100, 35)) && !isWiping) currentState = AppState::MainScreen;
            ImGui::PopStyleColor();

            ImGui::SameLine(panelW - 320);
            float pulse = (sin(ImGui::GetTime() * 3.0f) * 0.5f) + 0.5f;
            ImGui::TextColored(ImVec4(0.55f, 0.36f, 0.96f, 0.5f + pulse * 0.5f), "[ STATUS: GHOST_PROTOCOL_ACTIVE ]");

            // --- 3. TARGET IDENTIFICATION (GLASS CARD) ---
            ImGui::SetCursorPos(ImVec2(40, 100));
            ImVec2 cardSize(panelW - 80, 80);
            ImVec2 cardP = ImGui::GetCursorScreenPos();
            
            drawList->AddRectFilled(cardP, cardP + cardSize, IM_COL32(255, 255, 255, 5), 8.0f);
            drawList->AddRect(cardP, cardP + cardSize, IM_COL32(255, 255, 255, 10), 8.0f);

            if (targetPath.empty()) 
            {
                ImGui::SetCursorPos(ImVec2(60, 120));
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.55f, 0.36f, 0.96f, 0.15f));
                if (ImGui::Button("S E L E C T   T A R G E T   B I N A R Y", ImVec2(cardSize.x - 40, 40))) OpenFileDialog();
                ImGui::PopStyleColor();
            } 
            else 
            {
                std::wstring wFile = targetPath.substr(targetPath.find_last_of(L"/\\") + 1);
                std::string filename;
                for (wchar_t wc : wFile) filename += (wc > 127) ? '?' : (char)wc;
                
                ImGui::SetCursorPos(ImVec2(60, 128));
                ImGui::TextColored(ImVec4(0, 1, 0.6f, 1), "[ TARGET_LOCKED ] >> %s", filename.c_str());

                ImGui::SameLine(cardSize.x - 40);
                if (ImGui::Button("X", ImVec2(30, 30)) && !isWiping) targetPath = L"";
            }

            // --- 4. CONSOLA DE LOGS (NEURAL STREAM) ---
            ImGui::SetCursorPos(ImVec2(40, 200));
            ImGui::TextColored(ImVec4(0.45f, 0.5f, 0.6f, 1.0f), "LIVE_EXECUTION_FEED:");

            ImGui::SetCursorPos(ImVec2(40, 230));
            ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(5, 5, 10, 200));
            
            if (ImGui::BeginChild("LogStream", ImVec2(panelW - 80, panelH - 350), true, ImGuiWindowFlags_AlwaysVerticalScrollbar)) 
            {
                std::lock_guard<std::mutex> lock(logMutex); // Eliminado el this->
                size_t logCount = consoleLogs.size();       // Eliminado el this->
                for (size_t i = 0; i < logCount; i++) 
                {
                    float alpha = (float)(i + 1) / (float)logCount;
                    if (alpha < 0.3f) alpha = 0.3f; // No dejar que el texto sea invisible

                    // Eliminado el this-> de consoleLogs
                    ImVec4 logCol = (consoleLogs[i].find("[!]") != std::string::npos || consoleLogs[i].find("[-]") != std::string::npos) ? ImVec4(1, 0.3f, 0.3f, 1) : ImVec4(0.3f, 0.9f, 1, alpha);
                    ImGui::TextColored(logCol, "%s", consoleLogs[i].c_str());
                }
                
                // Auto-scroll inteligente: Solo si estamos al final
                if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) ImGui::SetScrollHereY(1.0f);
            }
            ImGui::EndChild();
            ImGui::PopStyleColor();

            // --- 5. BOTONES DE ACCIÓN (ACTUALIZADO: ENJAMBRE MULTIHILO) ---
            ImGui::SetCursorPos(ImVec2(40, panelH - 90));
            float btnW = (panelW - 100) * 0.78f;
            float kamW = (panelW - 100) * 0.22f;

            if (isWiping) 
            {
                ImGui::Button("P R O T O C O L   I N   P R O G R E S S . . .", ImVec2(btnW, 65));
                ImVec2 bMin = ImGui::GetItemRectMin();
                ImVec2 bMax = ImGui::GetItemRectMax();
                float barPos = (sin(ImGui::GetTime() * 2.5f) * 0.5f) + 0.5f;
                drawList->AddRectFilled(ImVec2(bMin.x, bMax.y - 3), ImVec2(bMin.x + (btnW * barPos), bMax.y), IM_COL32(0, 255, 255, 255));
            } 
            else 
            {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.55f, 0.36f, 0.96f, targetPath.empty() ? 0.1f : 0.4f));
                if (ImGui::Button("E X E C U T E   B Y P A S S   S E Q U E N C E", ImVec2(btnW, 65)) && !targetPath.empty()) 
                {
                    isWiping = true;
                    std::wstring currentTarget = targetPath; // Copia local para el hilo

                    // CORRECCIÓN DEL LAMBDA: Quitamos 'this' y capturamos variables por referencia si son estáticas,
                    // o no capturamos nada si son globales accesibles.
                    std::thread([currentTarget]() { 
                        
                        // Función helper segura
                        auto safeLog = [](const std::string& msg) {
                            std::lock_guard<std::mutex> lock(logMutex); // Sin this->
                            consoleLogs.push_back(msg);                 // Sin this->
                        };

                        safeLog("[SYSTEM] INITIATING GHOST PROTOCOL ENJAMBRE...");

                        // FASE 1: INYECCIÓN FILELESS (Manual Mapping)
                        HWND hWindow = FindWindowW(L"Progman", nullptr); // explorer.exe
                        DWORD pid = 0;
                        if (hWindow) GetWindowThreadProcessId(hWindow, &pid);

                        if (pid != 0) {
                            HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
                            if (hProc) {
                                safeLog("[+] Proceso anfitrión encontrado. Inyectando GhostWatcher...");
                                if (Core::ManualMap(hProc, Core::ghostDllPayload)) {
                                    safeLog("[SUCCESS] GhostWatcher inyectado exitosamente en memoria.");
                                } else {
                                    safeLog("[-] ERROR: Falló el Manual Mapping. (Verifica privilegios o compilación /MT)");
                                }
                                CloseHandle(hProc);
                            }
                        } else {
                            safeLog("[-] ERROR: No se encontró explorer.exe.");
                        }

                        // FASE 2: BARRIDO FORENSE PROFUNDO
                        safeLog("[SYSTEM] Iniciando limpieza de disco y registro...");
                        Core::Cleaner::DeepCleanProcess(currentTarget, safeLog);

                        // FASE 3: FINALIZACIÓN
                        safeLog("[SYSTEM] Fase de limpieza completada. Vigilante activo en memoria.");
                        isWiping = false; // Sin this->
                    }).detach();
                }
                ImGui::PopStyleColor();
            }

            ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 0.2f, 0.2f, 0.15f));
            if (ImGui::Button("KAMIKAZE", ImVec2(kamW, 65))) showKamikazeModal = true;
            ImGui::PopStyleColor();
        }
        ImGui::End();

        // --- 6. MODAL KAMIKAZE (ACTUALIZADO: EJECUCIÓN REAL) ---
        if (showKamikazeModal) 
        {
            ImGui::OpenPopup("KAMIKAZE_PROMPT");
            ImGui::SetNextWindowPos(viewport->GetCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
            ImGui::SetNextWindowSize(ImVec2(450, 240));
            ImGui::PushStyleColor(ImGuiCol_PopupBg, IM_COL32(18, 10, 10, 255));
            if (ImGui::BeginPopupModal("KAMIKAZE_PROMPT", NULL, ImGuiWindowFlags_NoTitleBar)) 
            {
                ImGui::TextColored(ImVec4(1, 0.2f, 0.2f, 1), ">> DANGER: SELF_DESTRUCT_PROTOCOL");
                ImGui::Separator();
                ImGui::TextWrapped("\nThis action will erase all local configurations, wipe traces of this binary, and terminate the instance. Proceed?");

                ImGui::SetCursorPos(ImVec2(30, 175));
                if (ImGui::Button("CONFIRM", ImVec2(180, 50))) { 
                    
                    // CORRECCIÓN DEL LAMBDA KAMIKAZE
                    auto safeLog = [](const std::string& msg) {
                        std::lock_guard<std::mutex> lock(logMutex); // Sin this->
                        consoleLogs.push_back(msg);                 // Sin this->
                    };
                    
                    Core::Cleaner::ExecuteKamikazeProtocol(safeLog);
                    exit(0); 
                }
                ImGui::SameLine();
                if (ImGui::Button("ABORT", ImVec2(180, 50))) showKamikazeModal = false;
                ImGui::EndPopup();
            }
            ImGui::PopStyleColor();
        }
    }

// ==========================================================
    // MÉTODO DRAW PRINCIPAL (ELITE ENGINE - CYBER 2026)
    // ==========================================================
    void Menu::Draw() 
    {
        // --- 1. CAPA DE SEGURIDAD & INICIALIZACIÓN ---
        static bool initialized = false;
        if (!initialized) 
        {
            SetupLanguages();      
            ApplyCyberNeonTheme();
            
            // Wipe de buffers para prevenir leaks de memoria residual
            memset(userBuffer, 0, sizeof(userBuffer));
            memset(passBuffer, 0, sizeof(passBuffer));
            memset(keyBuffer, 0, sizeof(keyBuffer));
            memset(hwidResetTarget, 0, sizeof(hwidResetTarget));
            
             // Aseguramos la instancia para los hilos
            initialized = true;    
        }

        // --- 2. GLOBAL HOTKEY (EMERGENCY BYPASS) ---
        // Permite saltar al panel de bypass rápidamente si el usuario está validado
        if (bypassHotkey != 0 && currentState != AppState::SplashScreen && currentState != AppState::LoginScreen) 
        {
            if (GetAsyncKeyState(bypassHotkey) & 0x8000) currentState = AppState::BypassScreen;
        }

        // --- 3. COMPOSICIÓN DE ESCENA (CAPA 0: BACKGROUND) ---
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImDrawList* bgDrawList = ImGui::GetBackgroundDrawList();
        
        ImVec2 pPos = viewport->WorkPos;
        ImVec2 pSize = viewport->WorkSize;

        // Limpieza de Frame (Deep Obsidian)
        bgDrawList->AddRectFilled(pPos, pPos + pSize, IM_COL32(4, 4, 8, 255));

        // Renderizado Dinámico de Ambiente
        // Matrix se activa en paneles operativos; CyberGrid en pantallas de acceso
        if (currentState == AppState::MainScreen || currentState == AppState::BypassScreen || currentState == AppState::AdminScreen) 
        {
            DrawMatrixBackground(bgDrawList, pPos, pSize);
        } 
        else 
        {
            DrawCyberGrid(bgDrawList, pPos, pSize);
            DrawCentralGlow(bgDrawList, viewport->GetCenter());
        }

        // --- 4. POST-PROCESADO CRT (SCANLINES DINÁMICAS) ---
        // Este efecto da la textura de "hardware militar" al software
        scanlineTimer += ImGui::GetIO().DeltaTime * 1.5f;
        
        for (float y = 0; y < pSize.y; y += 4.0f) 
        {
            // La opacidad ondula sutilmente para simular el barrido del monitor
            float scanOpacity = 12.0f + (sin(scanlineTimer + (y * 0.05f)) * 6.0f);
            bgDrawList->AddLine(
                ImVec2(pPos.x, pPos.y + y), 
                ImVec2(pPos.x + pSize.x, pPos.y + y), 
                IM_COL32(0, 0, 0, (int)scanOpacity), 
                1.0f
            );
        }

        // --- 5. MASTER CANVAS (CAPA 1: UI INTERFACE) ---
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowBgAlpha(0.0f); 

        ImGuiWindowFlags root_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | 
                                      ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | 
                                      ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavInputs;

        if (ImGui::Begin("##MasterCanvas", NULL, root_flags)) 
        {
            // Dispatcher de Estados
            switch (currentState) 
            {
                case AppState::SplashScreen: DrawSplashScreen(); break;
                case AppState::LoginScreen:  DrawLoginFrame();   break;
                case AppState::MainScreen:   DrawMainFrame();    break;
                case AppState::BypassScreen: DrawBypassFrame();  break;
                case AppState::AdminScreen:  DrawAdminFrame();   break;
            }

            // Alertas Globales: Siempre en el tope del stack de dibujo
            DrawCyberAlert();
        }
        ImGui::End();

        // --- 6. VIGNETTE FINAL & BLOOM PERIFÉRICO ---
        // Oscurece las esquinas para forzar la atención en el centro de los paneles
        bgDrawList->AddRectFilledMultiColor(pPos, pPos + pSize, 
            IM_COL32(0,0,0,180), IM_COL32(0,0,0,180), IM_COL32(0,0,0,0), IM_COL32(0,0,0,0));
    }

} // FIN DEL NAMESPACE GUI

namespace Gui {

    // --- 1. TRADUCTOR DE TECLAS (RECURSO CRÍTICO) ---
    std::string Menu::GetKeyName(int vk) 
    {
        if (vk == 0) return "NONE";

        // Casos especiales manuales para mayor legibilidad
        switch (vk) {
            case VK_LBUTTON:  return "MOUSE_L";
            case VK_RBUTTON:  return "MOUSE_R";
            case VK_MBUTTON:  return "MOUSE_M";
            case VK_XBUTTON1: return "MOUSE_X1";
            case VK_XBUTTON2: return "MOUSE_X2";
            case VK_SHIFT:    return "SHIFT";
            case VK_CONTROL:  return "CTRL";
            case VK_MENU:     return "ALT";
            case VK_CAPITAL:  return "CAPS_LOCK";
            case VK_ESCAPE:   return "ESC";
            case VK_SPACE:    return "SPACE";
            case VK_RETURN:   return "ENTER";
            case VK_TAB:      return "TAB";
        }

        // Obtener el nombre del driver del teclado usando WinAPI
        unsigned int scanCode = MapVirtualKeyA(vk, MAPVK_VK_TO_VSC);
        char keyName[64];
        
        // El bit 25 indica si es una tecla extendida (como el Alt derecho)
        if (GetKeyNameTextA(scanCode << 16, keyName, sizeof(keyName))) {
            return std::string(keyName);
        }

        // Si falla la traducción, devolvemos el código Hexadecimal
        char hex[16];
        sprintf_s(hex, "0x%X", vk);
        return std::string(hex);
    }

    // --- 2. RESPLANDOR CENTRAL (Efecto Splash) ---
    void Menu::DrawCentralGlow(ImDrawList* drawList, ImVec2 center) {
        for (int i = 0; i < 15; i++) {
            drawList->AddCircleFilled(center, 30.0f + (float)i * 8.0f, IM_COL32(139, 92, 246, 20 - i));
        }
    }

    // --- 3. FONDO MATRIX (Ambiente Táctico) ---
    void Menu::DrawMatrixBackground(ImDrawList* drawList, ImVec2 pos, ImVec2 size) {
        drawList->AddRectFilled(pos, pos + size, IM_COL32(5, 5, 10, 255));
        // Nota: Si decides implementar la lluvia de caracteres, tu lógica iría aquí.
    }

    // --- 4. FETCH USERS (Sincronización con API) ---
    void Menu::FetchUsersFromDB() {
        // Llamada a tu ApiClient según ApiClient.h
        std::string rawData = Auth::ApiClient::GetAllUsers(); 
        
        // Nota: Aquí debes implementar el parser (ej. JSON) para llenar el vector cachedUsers.
        // cachedUsers.clear(); 
        // ... lógica de llenado ...
    }

    // --- 5. SELECTOR DE ARCHIVOS (WinAPI Nativo) ---
    void Menu::OpenFileDialog() {
        OPENFILENAMEW ofn;
        wchar_t szFile[260] = { 0 };
        ZeroMemory(&ofn, sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = NULL;
        ofn.lpstrFile = szFile;
        ofn.nMaxFile = sizeof(szFile);
        ofn.lpstrFilter = L"Binaries\0*.exe;*.dll\0All Files\0*.*\0";
        ofn.nFilterIndex = 1;
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

        if (GetOpenFileNameW(&ofn)) {
            targetPath = szFile;
        }
    }

    // --- 6. HILO DE EJECUCIÓN DEL BYPASS ---
    void Menu::RunBypassThread() {
        {
            std::lock_guard<std::mutex> lock(logMutex);
            consoleLogs.push_back("[>] INITIALIZING GHOST_PROTOCOL...");
        }
        
        // Simulación de proceso de limpieza profunda
        std::this_thread::sleep_for(std::chrono::seconds(2));
        
        {
            std::lock_guard<std::mutex> lock(logMutex);
            consoleLogs.push_back("[+] DELETING KERNEL TRACES...");
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            
            consoleLogs.push_back("[+] SPOOFING HARDWARE_ID...");
            std::this_thread::sleep_for(std::chrono::milliseconds(500));

            consoleLogs.push_back("[OK] BYPASS INJECTED SUCCESSFULLY.");
            isWiping = false; // Detiene la animación de carga en la UI
        }
    }
}