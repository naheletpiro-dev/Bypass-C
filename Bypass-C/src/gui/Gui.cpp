#include "../../include/gui/Menu.h"
#include "../../include/auth/ApiClient.h"
#include "../../include/core/Cleaner.h" // Asumiendo que esta es la ruta de tu Cleaner
#include <windows.h>
#include <chrono>
#include <thread>
#include <random>
#include <commdlg.h>
#include <nlohmann/json.hpp>

namespace Gui {

    // ==========================================================
    // CONSTRUCTOR
    // ==========================================================
    Menu::Menu() 
    {
        // Constructor vacío para evitar el crash del contexto de ImGui
    }

    // ==========================================================
    // SISTEMA DE LENGUAJES (I18N)
    // ==========================================================
    void Menu::SetupLanguages() 
    {
        // --- ESPAÑOL ---
        languages["ES"] = {
            {"launch", "INICIAR SISTEMA CORE"}, 
            {"admin_btn", "🛡️ PANEL ADMIN"},
            {"settings", "⚙️ AJUSTES"}, 
            {"nodes", "👤 NÓDULOS ACTIVOS"},
            {"mint", "🔑 GENERAR LICENCIAS"}, 
            {"target", "SELECCIONAR ARCHIVO"},
            {"execute", "EJECUTAR BYPASS NEURAL"}, 
            {"lang_title", "CONFIGURACIÓN"},
            {"back", "← VOLVER"}, 
            {"success_reg", "CUENTA ACTIVADA CON ÉXITO"},
            {"refresh", "RECARGAR"}, 
            {"unlock", "DESBLOQUEAR HWID"},
            {"tier", "RANGO"}, 
            {"amount", "CANTIDAD"}, 
            {"mint_btn", "MINTEAR LLAVES"}
        };

        // --- INGLÉS ---
        languages["EN"] = {
            {"launch", "LAUNCH CORE SYSTEM"}, 
            {"admin_btn", "🛡️ ADMIN PANEL"},
            {"settings", "⚙️ SETTINGS"}, 
            {"nodes", "👤 ACTIVE NODES"},
            {"mint", "🔑 LICENSE MINT"}, 
            {"target", "SELECT TARGET BINARY"},
            {"execute", "EXECUTE NEURAL BYPASS"}, 
            {"lang_title", "SETTINGS"},
            {"back", "← BACK"}, 
            {"success_reg", "ACCOUNT ACTIVATED SUCCESSFULLY"},
            {"refresh", "REFRESH"}, 
            {"unlock", "UNLOCK HWID"},
            {"tier", "TIER"}, 
            {"amount", "AMOUNT"}, 
            {"mint_btn", "MINT KEYS"}
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

    // ==========================================================
    // ESTILOS VISUALES (CYBER NEON THEME)
    // ==========================================================
void Menu::ApplyCyberNeonTheme() {
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    // Fondos ultra oscuros estilo "Void" con un toque de azul medianoche
    colors[ImGuiCol_WindowBg]       = ImVec4(0.03f, 0.03f, 0.05f, 0.98f); 
    colors[ImGuiCol_ChildBg]        = ImVec4(0.06f, 0.06f, 0.09f, 1.00f);
    colors[ImGuiCol_PopupBg]        = ImVec4(0.05f, 0.05f, 0.07f, 0.98f);
    
    // Bordes sutiles y futuristas (Cian oscuro)
    colors[ImGuiCol_Border]         = ImVec4(0.15f, 0.20f, 0.30f, 1.00f);
    colors[ImGuiCol_BorderShadow]   = ImVec4(0.00f, 0.00f, 0.00f, 0.20f);

    // Inputs de texto y frames (Oscuros, se iluminan al interactuar)
    colors[ImGuiCol_FrameBg]        = ImVec4(0.08f, 0.08f, 0.12f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.12f, 0.12f, 0.18f, 1.00f);
    colors[ImGuiCol_FrameBgActive]  = ImVec4(0.15f, 0.15f, 0.22f, 1.00f);

    // Botones dinámicos (Violeta neón transicionando a Cian)
    colors[ImGuiCol_Button]         = ImVec4(0.40f, 0.20f, 0.80f, 0.80f); // Violeta base
    colors[ImGuiCol_ButtonHovered]  = ImVec4(0.50f, 0.30f, 0.95f, 1.00f); // Se ilumina al pasar el mouse
    colors[ImGuiCol_ButtonActive]   = ImVec4(0.20f, 0.80f, 0.90f, 1.00f); // Estalla en cian al hacer clic

    // Textos limpios y de alto contraste
    colors[ImGuiCol_Text]           = ImVec4(0.90f, 0.90f, 0.95f, 1.00f);
    colors[ImGuiCol_TextDisabled]   = ImVec4(0.40f, 0.40f, 0.50f, 1.00f);

    // Barras y pestañas
    colors[ImGuiCol_Tab]            = ImVec4(0.08f, 0.08f, 0.12f, 1.00f);
    colors[ImGuiCol_TabHovered]     = ImVec4(0.40f, 0.20f, 0.80f, 0.80f);
    colors[ImGuiCol_TabActive]      = ImVec4(0.50f, 0.30f, 0.95f, 1.00f);
    
    colors[ImGuiCol_Header]         = ImVec4(0.40f, 0.20f, 0.80f, 0.40f);
    colors[ImGuiCol_HeaderHovered]  = ImVec4(0.50f, 0.30f, 0.95f, 0.80f);
    colors[ImGuiCol_HeaderActive]   = ImVec4(0.20f, 0.80f, 0.90f, 1.00f);

    // Geometría "Cyber 2026" (Redondeos suaves y elegantes)
    style.WindowRounding    = 12.0f; // Ventanas más redondas
    style.ChildRounding     = 8.0f;  // Paneles internos sutiles
    style.FrameRounding     = 6.0f;  // Botones e inputs curvos
    style.PopupRounding     = 8.0f;
    style.ScrollbarRounding = 12.0f;
    style.WindowBorderSize  = 1.0f;  // Borde ultra fino
    style.ChildBorderSize   = 1.0f;
    style.FrameBorderSize   = 0.0f;  // Inputs sin borde rígido, se funden con el fondo
    
    // Espaciado para que respire la interfaz
    style.ItemSpacing       = ImVec2(10, 12);
    style.FramePadding      = ImVec2(15, 10); // Botones más "gorditos" y clickeables
}

// ==========================================================
    // RENDERIZADO DE FONDOS (CYBER 2026 AESTHETIC)
    // ==========================================================
    void Menu::DrawCyberGrid(ImDrawList* drawList, ImVec2 pos, ImVec2 size) 
    {
        // Colores más sutiles para no distraer, estilo "Blueprint" tecnológico
        ImU32 lineColor = IM_COL32(40, 45, 65, 80);        // Líneas finas
        ImU32 thickLineColor = IM_COL32(80, 90, 120, 120); // Líneas maestras

        float step = 45.0f;
        
        // Animación de desplazamiento infinito (Parallax suave)
        static float offset = 0.0f;
        offset += ImGui::GetIO().DeltaTime * 15.0f; 
        if (offset > step) offset -= step;

        // Lineas verticales con patrón de repetición (cada 4 líneas es más gruesa)
        for (float x = fmodf(size.x / 2.0f, step) + offset - step; x < size.x; x += step) 
        {
            bool isThick = (int)(x / step) % 4 == 0;
            drawList->AddLine(ImVec2(pos.x + x, pos.y), ImVec2(pos.x + x, pos.y + size.y), 
                              isThick ? thickLineColor : lineColor, isThick ? 2.0f : 1.0f);
        }
        
        // Lineas horizontales
        for (float y = fmodf(size.y / 2.0f, step) + offset - step; y < size.y; y += step) 
        {
            bool isThick = (int)(y / step) % 4 == 0;
            drawList->AddLine(ImVec2(pos.x, pos.y + y), ImVec2(pos.x + size.x, pos.y + y), 
                              isThick ? thickLineColor : lineColor, isThick ? 2.0f : 1.0f);
        }
    }

    void Menu::DrawCentralGlow(ImDrawList* drawList, ImVec2 center) 
    {
        // Animación de respiración suave basada en el tiempo (más fluido que sumar/restar)
        static float time = 0.0f;
        time += ImGui::GetIO().DeltaTime * 2.0f;
        float pulse = (sin(time) * 0.5f) + 0.5f; // Rango de 0.0 a 1.0

        // Radios dinámicos
        float baseRadius = 160.0f + (pulse * 15.0f);

        // Capas para simular luz volumétrica (Bloom)
        ImU32 coreColor  = IM_COL32(139, 92, 246, 40 + (int)(20 * pulse));  // Centro Violeta intenso
        ImU32 midColor   = IM_COL32(0, 255, 255, 20 + (int)(15 * pulse));   // Borde Cian
        ImU32 outerColor = IM_COL32(139, 92, 246, 5);                       // Corona ultra suave

        drawList->AddCircleFilled(center, baseRadius * 1.5f, outerColor, 64);
        drawList->AddCircleFilled(center, baseRadius * 1.1f, midColor, 64);
        drawList->AddCircleFilled(center, baseRadius * 0.6f, coreColor, 64);

        // Anillos tecnológicos de contorno para darle un toque UI/HUD
        drawList->AddCircle(center, baseRadius * 1.2f, IM_COL32(0, 255, 255, 100), 64, 1.5f);
        drawList->AddCircle(center, baseRadius * 0.85f, IM_COL32(139, 92, 246, 180), 64, 1.0f);
    }

    void Menu::DrawMatrixBackground(ImDrawList* drawList, ImVec2 pos, ImVec2 size) 
    {
        // Usamos HEX Code en lugar de símbolos random para un look más "Reverse Engineering"
        const char chars[] = "01A2B3C4D5E6F789"; 
        int cols = 60;
        float colWidth = size.x / cols;
        float speed = ImGui::GetIO().DeltaTime * 350.0f;
        
        for (int i = 0; i < cols; i++) 
        {
            // Caída asíncrona: unas columnas van más rápido que otras
            matrixDrops[i] += speed + (i % 5) * 40.0f; 
            if (matrixDrops[i] > size.y + 50.0f) 
            {
                matrixDrops[i] = -20.0f; // Reinicia arriba suavemente
            }

            char c = chars[rand() % (sizeof(chars) - 1)];
            std::string s(1, c);
            
            // Colores Cyberpunk: Cian brillante cayendo con estela violeta
            ImU32 headColor  = IM_COL32(0, 255, 255, 220);  // Cabeza Cian
            ImU32 trailColor = IM_COL32(139, 92, 246, 120); // Cuerpo Violeta
            ImU32 fadeColor  = IM_COL32(139, 92, 246, 30);  // Cola casi invisible
            
            // Dibujamos el rastro (Efecto de lluvia con degradado)
            drawList->AddText(ImVec2(pos.x + (i * colWidth), pos.y + matrixDrops[i]), headColor, s.c_str());
            drawList->AddText(ImVec2(pos.x + (i * colWidth), pos.y + matrixDrops[i] - 15), trailColor, "X");
            drawList->AddText(ImVec2(pos.x + (i * colWidth), pos.y + matrixDrops[i] - 30), fadeColor, "1");
        }
    }

// ==========================================================
    // UTILIDADES GENERALES
    // ==========================================================
    std::string Menu::GetKeyName(int vkCode) 
    {
        if (vkCode == 0) return "NONE";
        
        UINT scanCode = MapVirtualKeyA(vkCode, MAPVK_VK_TO_VSC);
        char keyName[128];
        
        if (GetKeyNameTextA(scanCode << 16, keyName, sizeof(keyName))) 
        {
            return std::string(keyName);
        }
        
        return "UNKNOWN";
    }

    void Menu::ShowAlert(const std::string& title, const std::string& msg, bool isError) 
    {
        alertTitle = title;
        alertMessage = msg;
        alertIsError = isError;
        showAlert = true;
    }

    void Menu::DrawCyberAlert() 
    {
        if (!showAlert) return;

        // Oscurecer el fondo de toda la aplicación detrás del modal para darle enfoque
        ImGui::PushStyleColor(ImGuiCol_ModalWindowDimBg, ImVec4(0.01f, 0.01f, 0.02f, 0.85f));
        
        ImGui::OpenPopup("CyberAlertPopup");
        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(450, 230)); // Ligeramente más amplio para respirar

        // Paleta Cyberpunk dependiente del estado
        // ERROR: Rojo Carmesí | INFO: Cian Neón
        ImVec4 accentColor = alertIsError ? ImVec4(1.0f, 0.15f, 0.25f, 1.0f) : ImVec4(0.15f, 0.85f, 0.95f, 1.0f);
        ImVec4 bgPanelColor = ImVec4(0.05f, 0.05f, 0.07f, 1.0f);

        ImGui::PushStyleColor(ImGuiCol_PopupBg, bgPanelColor);
        ImGui::PushStyleColor(ImGuiCol_Border, accentColor);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.5f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 12.0f);

        if (ImGui::BeginPopupModal("CyberAlertPopup", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove)) 
        {
            float windowWidth = ImGui::GetWindowSize().x;
            ImDrawList* drawList = ImGui::GetWindowDrawList();
            ImVec2 pos = ImGui::GetWindowPos();

            // 1. Barra superior decorativa (LED Indicator)
            drawList->AddRectFilled(pos, ImVec2(pos.x + windowWidth, pos.y + 6.0f), 
                                    ImGui::ColorConvertFloat4ToU32(accentColor), 12.0f, ImDrawFlags_RoundCornersTop);

            // 2. Título principal con corchetes de terminal
            ImGui::SetCursorPosY(30);
            std::string formattedTitle = alertIsError ? "[ CRITICAL ERROR ]" : "[ SYSTEM NOTICE ]";
            
            ImGui::PushStyleColor(ImGuiCol_Text, accentColor);
            float textWidth = ImGui::CalcTextSize(formattedTitle.c_str()).x;
            ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
            ImGui::Text(formattedTitle.c_str());
            ImGui::PopStyleColor();

            // Línea separadora ultra fina
            ImGui::SetCursorPosY(55);
            ImGui::SetCursorPosX(30);
            drawList->AddLine(ImVec2(pos.x + 30, pos.y + 55), ImVec2(pos.x + windowWidth - 30, pos.y + 55), IM_COL32(255, 255, 255, 20));

            // 3. Mensaje
            ImGui::SetCursorPosY(75);
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.85f, 0.85f, 0.90f, 1.0f)); // Texto blanco-azulado claro
            ImGui::PushTextWrapPos(windowWidth - 30); // Margen derecho
            ImGui::SetCursorPosX(30);                 // Margen izquierdo
            
            ImGui::TextWrapped("%s", alertMessage.c_str());
            
            ImGui::PopTextWrapPos();
            ImGui::PopStyleColor();

            // 4. Botón de Confirmación "Hacker"
            ImGui::SetCursorPosY(165);
            
            // Estilo del botón (Borde transparente, texto del color del acento)
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.08f, 0.08f, 0.12f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.12f, 0.12f, 0.18f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, accentColor);
            ImGui::PushStyleColor(ImGuiCol_Text, accentColor);
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);
            
            const char* btnText = "A C K N O W L E D G E";
            float btnWidth = 240.0f;
            ImGui::SetCursorPosX((windowWidth - btnWidth) * 0.5f);
            
            if (ImGui::Button(btnText, ImVec2(btnWidth, 40))) 
            {
                showAlert = false;
                ImGui::CloseCurrentPopup();
            }
            
            ImGui::PopStyleVar();
            ImGui::PopStyleColor(4);

            ImGui::EndPopup();
        }
        
        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor(3);
    }

// ==========================================================
    // SPLASH SCREEN (PANTALLA DE CARGA - CYBER 2026)
    // ==========================================================
    void Menu::DrawSplashScreen() 
    {
        static std::vector<std::string> steps = {
            "INITIALIZING NEURAL KERNEL...", 
            "ESTABLISHING SECURE HANDSHAKE...", 
            "DECRYPTING RENDER API TOKENS...", 
            "BYPASS PROTOCOL READY"
        };
        
        static auto startTime = std::chrono::steady_clock::now();
        auto currentTime = std::chrono::steady_clock::now();
        float elapsedTime = std::chrono::duration<float>(currentTime - startTime).count();

        // Control del progreso lógico
        if (elapsedTime > 0.6f && splashStep < steps.size()) 
        {
            splashStep++;
            splashProgress = (float)splashStep / steps.size();
            startTime = currentTime; 
        }

        // Interpolación visual fluida (Para que la barra no "salte" de golpe, sino que se deslice)
        static float visualProgress = 0.0f;
        visualProgress += (splashProgress - visualProgress) * ImGui::GetIO().DeltaTime * 5.0f;

        // Si ya cargó todo visualmente y lógicamente, pasa a la pantalla de Login
        if (splashStep >= steps.size() && visualProgress >= 0.99f) 
        {
            currentState = AppState::LoginScreen;
            return;
        }

        ImVec2 windowSize = ImGui::GetWindowSize();
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        
        // --- Renderizado del Título ---
        ImGui::SetCursorPosY(windowSize.y * 0.40f);
        const char* title = "S C A N N E L E R";
        float titleWidth = ImGui::CalcTextSize(title).x;
        ImGui::SetCursorPosX((windowSize.x - titleWidth) * 0.5f);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.55f, 0.36f, 0.96f, 1.0f)); // Violeta Neón
        ImGui::Text(title); 
        ImGui::PopStyleColor();

        // Subtítulo
        ImGui::SetCursorPosY(windowSize.y * 0.45f);
        const char* subTitle = "S Y S T E M   I N I T I A L I Z A T I O N";
        float subWidth = ImGui::CalcTextSize(subTitle).x;
        ImGui::SetCursorPosX((windowSize.x - subWidth) * 0.5f);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.3f, 0.33f, 0.39f, 1.0f)); // Gris Oscuro
        ImGui::Text(subTitle);
        ImGui::PopStyleColor();

        // --- Renderizado del texto del paso actual y porcentaje ---
        ImGui::SetCursorPosY(windowSize.y * 0.62f);
        std::string currentText = (splashStep < steps.size()) ? steps[splashStep] : "READY";
        
        // Añadimos el porcentaje real calculándolo desde el visualProgress
        char statusBuffer[128];
        snprintf(statusBuffer, sizeof(statusBuffer), "%s  [ %d%% ]", currentText.c_str(), (int)(visualProgress * 100.0f));
        
        float textWidth = ImGui::CalcTextSize(statusBuffer).x;
        ImGui::SetCursorPosX((windowSize.x - textWidth) * 0.5f);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.15f, 0.85f, 0.95f, 1.0f)); // Cian brillante
        ImGui::Text(statusBuffer);
        ImGui::PopStyleColor();

        // --- Renderizado de la Barra de Progreso Custom (Estilo Holográfico) ---
        float barWidth = 450.0f;
        float barHeight = 4.0f; // Muy delgada y elegante
        ImVec2 barPos((windowSize.x - barWidth) * 0.5f, windowSize.y * 0.67f);

        // Fondo de la barra (Gris oscuro)
        drawList->AddRectFilled(barPos, ImVec2(barPos.x + barWidth, barPos.y + barHeight), 
                                IM_COL32(30, 30, 40, 255), 2.0f);

        // Relleno de la barra (Violeta)
        float currentFillWidth = barWidth * visualProgress;
        drawList->AddRectFilled(barPos, ImVec2(barPos.x + currentFillWidth, barPos.y + barHeight), 
                                IM_COL32(139, 92, 246, 255), 2.0f);

        // Borde brillante holográfico (Cian)
        drawList->AddRect(ImVec2(barPos.x - 2, barPos.y - 2), 
                          ImVec2(barPos.x + barWidth + 2, barPos.y + barHeight + 2), 
                          IM_COL32(0, 255, 255, 80), 4.0f, 0, 1.0f);
                          
        // Pequeño acento luminoso en la punta de la barra
        if (visualProgress > 0.0f && visualProgress < 1.0f) 
        {
            drawList->AddRectFilled(ImVec2(barPos.x + currentFillWidth - 10, barPos.y - 1), 
                                    ImVec2(barPos.x + currentFillWidth, barPos.y + barHeight + 1), 
                                    IM_COL32(0, 255, 255, 255), 2.0f);
        }
    }

// ==========================================================
    // LOGIN FRAME (AUTENTICACIÓN Y CANJE - CYBER 2026)
    // ==========================================================
    void Menu::DrawLoginFrame() 
    {
        ImVec2 windowSize = ImGui::GetWindowSize();
        
        // Dimensiones del panel central (Un poco más ancho para mejor legibilidad)
        float panelW = 440.0f;
        float panelH = 640.0f; 
        ImGui::SetCursorPos(ImVec2((windowSize.x - panelW) * 0.5f, (windowSize.y - panelH) * 0.5f));

        // Estilo visual del panel: Fondo Obsidiana con borde sutil Cian
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.04f, 0.04f, 0.06f, 0.95f)); 
        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 16.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.0f);
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.8f, 0.9f, 0.3f)); // Cian semitransparente

        if (ImGui::BeginChild("LoginPanel", ImVec2(panelW, panelH), true)) 
        {
            // --- Títulos y Branding ---
            ImGui::SetCursorPosY(50);
            
            // Efecto de sombra para el título
            const char* title = "S C A N N E L E R";
            float textW = ImGui::CalcTextSize(title).x;
            
            ImGui::SetCursorPosX((panelW - textW) * 0.5f + 2);
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 0.8f)); 
            ImGui::Text(title); // Sombra
            ImGui::PopStyleColor();
            
            ImGui::SetCursorPos(ImVec2((panelW - textW) * 0.5f, 50));
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9f, 0.9f, 0.95f, 1.0f)); // Blanco brillante
            ImGui::Text(title); // Texto real
            ImGui::PopStyleColor();

            const char* sub = "SECURE ACCESS PROTOCOL v3.0";
            textW = ImGui::CalcTextSize(sub).x;
            ImGui::SetCursorPosX((panelW - textW) * 0.5f);
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.45f, 0.55f, 1.0f)); // Gris azulado
            ImGui::Text(sub);
            ImGui::PopStyleColor();

            // Separador sutil
            ImDrawList* drawList = ImGui::GetWindowDrawList();
            ImVec2 pos = ImGui::GetWindowPos();
            drawList->AddLine(ImVec2(pos.x + 60, pos.y + 110), ImVec2(pos.x + panelW - 60, pos.y + 110), IM_COL32(255,255,255,20));

            // --- Estilos para Inputs de Datos ---
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(15, 12)); // Inputs más altos y cómodos
            ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.08f, 0.08f, 0.12f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.12f, 0.12f, 0.18f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0.1f, 0.1f, 0.15f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.85f, 0.85f, 0.90f, 1.0f)); // Texto del input

            float inputWidth = 340.0f;
            
            // Input de Usuario
            ImGui::SetCursorPosY(160);
            ImGui::SetCursorPosX((panelW - inputWidth) * 0.5f);
            ImGui::PushItemWidth(inputWidth);
            ImGui::InputTextWithHint("##User", "  U S E R N A M E", userBuffer, 256);
            
            // Input de Contraseña
            ImGui::SetCursorPosY(230);
            ImGui::SetCursorPosX((panelW - inputWidth) * 0.5f);
            ImGui::InputTextWithHint("##Pass", "  P A S S W O R D", passBuffer, 256, ImGuiInputTextFlags_Password);

            // Modo Canje (Redeem)
            static bool showRedeem = false;
            if (showRedeem) 
            {
                ImGui::SetCursorPosY(300);
                ImGui::SetCursorPosX((panelW - inputWidth) * 0.5f);
                ImGui::InputTextWithHint("##Key", "  L I C E N S E   K E Y", keyBuffer, 256);
            }
            
            ImGui::PopItemWidth();
            ImGui::PopStyleColor(4); // Limpiamos colores de inputs
            ImGui::PopStyleVar(2);   // Limpiamos paddings de inputs

            // --- Mensaje de Estado / Errores (Estilo Terminal) ---
            float statusY = showRedeem ? 370.0f : 320.0f;
            ImGui::SetCursorPosY(statusY);
            
            std::string consoleMsg = loginStatus.empty() ? "[WAITING FOR INPUT]" : "[>] " + loginStatus;
            textW = ImGui::CalcTextSize(consoleMsg.c_str()).x;
            ImGui::SetCursorPosX((panelW - textW) * 0.5f);
            
            ImVec4 statusColor = isLoginError ? ImVec4(1.0f, 0.3f, 0.3f, 1.0f) : // Rojo Error
                                 (loginStatus.empty() ? ImVec4(0.3f, 0.35f, 0.4f, 1.0f) : // Gris Reposo
                                 ImVec4(0.2f, 0.8f, 0.9f, 1.0f)); // Cian Conectando
                                 
            ImGui::PushStyleColor(ImGuiCol_Text, statusColor);
            ImGui::TextWrapped(consoleMsg.c_str());
            ImGui::PopStyleColor();

            // --- Botón de Ejecución Principal ---
            ImGui::SetCursorPosY(440);
            ImGui::SetCursorPosX((panelW - inputWidth) * 0.5f);

            static bool isConnecting = false;
            
            // Estilos del Botón Principal
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.40f, 0.20f, 0.80f, 0.90f)); // Violeta
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.50f, 0.30f, 0.95f, 1.00f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.20f, 0.80f, 0.90f, 1.00f)); // Cian al hacer clic
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));

            if (isConnecting) 
            {
                ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);
                ImGui::Button("A U T H E N T I C A T I N G . . .", ImVec2(inputWidth, 55));
                ImGui::PopStyleVar();
            } 
            else 
            {
                const char* btnLabel = showRedeem ? "R E D E E M   &   L O G I N" : "I N I T I A L I Z E   L O G I N";
                
                if (ImGui::Button(btnLabel, ImVec2(inputWidth, 55))) 
                {
                    isConnecting = true;
                    isLoginError = false;
                    loginStatus = "NEGOTIATING HANDSHAKE...";

                    if (showRedeem) 
                    {
                        auto reg = Auth::ApiClient::RedeemKey(userBuffer, keyBuffer, passBuffer);
                        if (reg.success) 
                        {
                            auto res = Auth::ApiClient::ValidateLogin(userBuffer, passBuffer);
                            if (res.success) 
                            {
                                isAdmin = (res.role == "admin" || res.role == "super_admin");
                                currentState = AppState::MainScreen;
                            } 
                            else { loginStatus = res.message; isLoginError = true; }
                        } 
                        else { loginStatus = reg.message; isLoginError = true; }
                    } 
                    else 
                    {
                        auto res = Auth::ApiClient::ValidateLogin(userBuffer, passBuffer);
                        if (res.success) 
                        {
                            isAdmin = (res.role == "admin" || res.role == "super_admin");
                            currentState = AppState::MainScreen;
                        } 
                        else { loginStatus = res.message; isLoginError = true; }
                    }
                    isConnecting = false;
                }
            }
            ImGui::PopStyleColor(4); // Limpiar colores del botón
            ImGui::PopStyleVar();    // Limpiar rounding del botón

            // ==========================================================
            // TEXTO INTERACTIVO (CORRECCIÓN DEL BUG DE HITBOX)
            // ==========================================================
            ImGui::SetCursorPosY(520);
            
            const char* toggleText = showRedeem ? "Already registered? Return to Login" : "No account? Activate License Key";
            textW = ImGui::CalcTextSize(toggleText).x;
            ImGui::SetCursorPosX((panelW - textW) * 0.5f);
            
            // Guardamos la posición exacta en pantalla donde se dibujará el texto
            ImVec2 textPos = ImGui::GetCursorScreenPos();
            
            // Dibujamos el texto normal en gris oscuro
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.45f, 0.55f, 1.0f));
            ImGui::Text(toggleText);
            ImGui::PopStyleColor();

            // Detectamos si el mouse está exactamente encima de este texto
            if (ImGui::IsItemHovered()) 
            {
                // Cambiamos el cursor a la "manito"
                ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
                // Dibujamos el texto iluminado encima del anterior
                ImGui::GetWindowDrawList()->AddText(textPos, ImGui::ColorConvertFloat4ToU32(ImVec4(0.2f, 0.8f, 0.9f, 1.0f)), toggleText);
            }

            // Detectamos el clic exacto
            if (ImGui::IsItemClicked()) 
            {
                showRedeem = !showRedeem;
                loginStatus = "";
                isLoginError = false;
            }
        }
        
        ImGui::EndChild();
        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor(2);
    }

// ==========================================================
    // MAIN FRAME (PANTALLA PRINCIPAL / HUD DASHBOARD - CYBER 2026)
    // ==========================================================
    void Menu::DrawMainFrame() 
    {
        ImVec2 winSize = ImGui::GetWindowSize();
        
        static bool showSettingsModal = false;
        static bool isWaitingForKey = false;

        float panelW = winSize.x * 0.85f;
        float panelH = winSize.y * 0.85f;
        ImGui::SetCursorPos(ImVec2((winSize.x - panelW) * 0.5f, (winSize.y - panelH) * 0.5f));

        // Empujamos 2 colores y 2 variables de estilo
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.04f, 0.04f, 0.06f, 0.95f)); 
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.15f, 0.20f, 0.30f, 1.0f)); 
        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 16.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.0f);

        if (ImGui::BeginChild("MainPanel", ImVec2(panelW, panelH), true)) 
        {
            // --- 1. HEADER Y DATOS DEL USUARIO ---
            ImGui::SetCursorPos(ImVec2(30, 30));
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.2f, 0.8f, 0.9f, 1.0f)); 
            ImGui::Text("N E X U S   C O R E   //   M A I N   D A S H B O A R D");
            ImGui::PopStyleColor(); // Cierra 1
            
            ImGui::SetCursorPosX(30);
            ImGui::Separator();
            
            ImGui::SetCursorPos(ImVec2(30, 70));
            ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]); // Empuja fuente
            ImGui::Text("WELCOME OPERATIVE,");
            ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.55f, 0.36f, 0.96f, 1.0f)); 
            
            std::string userUpper = userBuffer;
            for(auto & c: userUpper) c = toupper(c);
            ImGui::Text("%s", userUpper.c_str());
            ImGui::PopStyleColor(); // Cierra 1
            ImGui::PopFont();       // CORRECCIÓN: Cierra la fuente

            ImGui::SetCursorPosX(30);
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.45f, 0.55f, 1.0f));
            ImGui::Text("CLEARANCE LEVEL: %s", isAdmin ? "[ ROOT ACCESS ]" : "[ STANDARD ACCESS ]");
            ImGui::PopStyleColor(); // Cierra 1

            ImGui::Spacing(); ImGui::Spacing();

            // --- 2. MODULO DE ESTADO (STATUS CARD) ---
            ImGui::SetCursorPosX(30);
            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.02f, 0.02f, 0.03f, 1.0f)); 
            ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 8.0f);
            
            if (ImGui::BeginChild("StatusCard", ImVec2(panelW - 60, 90), true)) 
            {
                ImGui::SetCursorPos(ImVec2(15, 15));
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.5f, 1.0f)); 
                ImGui::Text("[OK]"); 
                ImGui::SameLine(); 
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.7f, 0.7f, 0.75f, 1.0f));
                ImGui::Text(" HARDWARE ID SYNCED & VERIFIED");
                ImGui::PopStyleColor(2); // Cierra 2

                ImGui::SetCursorPosX(15);
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.5f, 1.0f)); 
                ImGui::Text("[OK]"); 
                ImGui::SameLine(); 
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.7f, 0.7f, 0.75f, 1.0f));
                ImGui::Text(" SECURE CONNECTION TO API: ESTABLISHED");
                ImGui::PopStyleColor(2); // Cierra 2

                ImGui::SetCursorPosX(15);
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.5f, 1.0f)); 
                ImGui::Text("[OK]"); 
                ImGui::SameLine(); 
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.7f, 0.7f, 0.75f, 1.0f));
                ImGui::Text(" KERNEL DRIVERS INJECTED");
                ImGui::PopStyleColor(2); // Cierra 2
            }
            ImGui::EndChild();
            ImGui::PopStyleVar();
            ImGui::PopStyleColor();

            ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing();

            // --- 3. CENTRO DE COMANDOS ---
            ImGui::SetCursorPosX(30);
            
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.40f, 0.20f, 0.80f, 0.90f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.50f, 0.30f, 0.95f, 1.00f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.20f, 0.80f, 0.90f, 1.00f));
            
            if (ImGui::Button("I N I T I A T E   G H O S T   P R O T O C O L", ImVec2(panelW - 60, 65))) 
            {
                currentState = AppState::BypassScreen;
            }
            ImGui::PopStyleColor(3); // Cierra 3

            ImGui::Spacing();

            if (isAdmin) 
            {
                ImGui::SetCursorPosX(30);
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.18f, 0.04f, 0.04f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.25f, 0.05f, 0.05f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.3f, 0.05f, 0.05f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.4f, 0.4f, 1.0f)); 
                
                if (ImGui::Button("A C C E S S   A D M I N   C O N S O L E", ImVec2(panelW - 60, 45))) 
                {
                    currentState = AppState::AdminScreen;
                }
                ImGui::PopStyleColor(4); // Cierra 4
            }

            // --- 4. PANEL INFERIOR (CONFIG & LOGOUT) ---
            ImGui::SetCursorPos(ImVec2(30, panelH - 60)); 
            
            float halfBtnW = (panelW - 70) / 2.0f; 

            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.15f, 0.15f, 0.22f, 1.0f)); 
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.2f, 0.3f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 0.8f, 0.9f, 1.0f)); 
            
            if (ImGui::Button("S Y S   C O N F I G", ImVec2(halfBtnW, 40))) 
            {
                showSettingsModal = true;
            }
            ImGui::PopStyleColor(3); // Cierra 3

            ImGui::SameLine();

            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.08f, 0.08f, 0.12f, 1.0f)); 
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.12f, 0.12f, 0.18f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.6f, 1.0f)); 
            
            if (ImGui::Button("D I S C O N N E C T", ImVec2(halfBtnW, 40))) 
            {
                memset(userBuffer, 0, sizeof(userBuffer));
                memset(passBuffer, 0, sizeof(passBuffer));
                memset(keyBuffer, 0, sizeof(keyBuffer));
                isAdmin = false;
                currentState = AppState::LoginScreen;
            }
            ImGui::PopStyleColor(3); // Cierra 3
            ImGui::PopStyleVar(); // Cierra el FrameRounding de esta sección
        }
        ImGui::EndChild();
        ImGui::PopStyleVar(2);     // Cierra ChildRounding y BorderSize principales
        ImGui::PopStyleColor(2);   // CORRECCIÓN: Cierra el ChildBg y el Border principales

        // ==========================================================
        // MODAL DE CONFIGURACIÓN (SYS CONFIG)
        // ==========================================================
        if (showSettingsModal) 
        {
            ImGui::OpenPopup("SYSTEM CONFIGURATION");
            showSettingsModal = false; 
            isWaitingForKey = false;   
        }

        // Empujamos colores para el Modal
        ImGui::PushStyleColor(ImGuiCol_ModalWindowDimBg, ImVec4(0.01f, 0.01f, 0.02f, 0.85f));
        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(480, 360));

        ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.05f, 0.05f, 0.07f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.15f, 0.85f, 0.95f, 0.5f)); 
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.5f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 12.0f);

        if (ImGui::BeginPopupModal("SYSTEM CONFIGURATION", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove)) 
        {
            float modalW = ImGui::GetWindowSize().x;

            ImGui::SetCursorPos(ImVec2(30, 25));
            ImGui::TextColored(ImVec4(0.15f, 0.85f, 0.95f, 1.0f), "[ GLOBAL PARAMETERS ]");
            ImGui::SetCursorPos(ImVec2(30, 45));
            ImGui::Separator();
            
            ImGui::Spacing(); ImGui::Spacing();

            ImGui::SetCursorPosX(30);
            ImGui::TextColored(ImVec4(0.4f, 0.45f, 0.55f, 1.0f), "SYSTEM LANGUAGE:");
            ImGui::SetCursorPosX(30);
            if (ImGui::RadioButton("ENGLISH (US)", currentLang == "EN")) currentLang = "EN";
            ImGui::SameLine(200);
            if (ImGui::RadioButton("ESPANOL (ES)", currentLang == "ES")) currentLang = "ES";
            
            ImGui::Spacing(); ImGui::Spacing();
            ImGui::SetCursorPosX(30);
            ImGui::Separator();
            ImGui::Spacing(); ImGui::Spacing();

            ImGui::SetCursorPosX(30);
            ImGui::TextColored(ImVec4(0.4f, 0.45f, 0.55f, 1.0f), "CLEANING MODULE:");
            ImGui::SetCursorPosX(30);
            ImGui::Checkbox("ENABLE DEEP REGISTRY SWEEP (Requires Admin)", &deepScanEnabled);

            ImGui::Spacing(); ImGui::Spacing();
            ImGui::SetCursorPosX(30);
            ImGui::Separator();
            ImGui::Spacing(); ImGui::Spacing();

            ImGui::SetCursorPosX(30);
            ImGui::TextColored(ImVec4(0.4f, 0.45f, 0.55f, 1.0f), "GHOST PROTOCOL HOTKEY:");
            
            ImGui::SetCursorPosX(30);
            std::string keyText = isWaitingForKey ? "[ PRESS ANY KEY TO BIND ]" : "[ " + GetKeyName(bypassHotkey) + " ]";
            
            if (isWaitingForKey) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 0.8f));
            else ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.15f, 0.15f, 0.22f, 1.0f));
            
            if (ImGui::Button(keyText.c_str(), ImVec2(250, 35))) 
            {
                isWaitingForKey = true;
            }
            ImGui::PopStyleColor(); // Cierra 1

            if (isWaitingForKey) 
            {
                for (int i = 8; i <= 255; i++) 
                {
                    if (GetAsyncKeyState(i) & 0x8000) 
                    {
                        if (i != VK_LBUTTON && i != VK_RBUTTON && i != VK_MBUTTON) 
                        {
                            bypassHotkey = i;
                            isWaitingForKey = false;
                            break;
                        }
                    }
                }
            }

            ImGui::SetCursorPos(ImVec2(30, 290));
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.40f, 0.20f, 0.80f, 0.80f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.50f, 0.30f, 0.95f, 1.00f));
            
            if (ImGui::Button("S A V E   &   R E T U R N", ImVec2(modalW - 60, 45))) 
            {
                isWaitingForKey = false; 
                ImGui::CloseCurrentPopup();
            }
            
            ImGui::PopStyleColor(2); // Cierra 2
            ImGui::PopStyleVar();    // Cierra 1
            ImGui::EndPopup();
        }
        
        ImGui::PopStyleVar(2);   // Cierra los 2 Var del Modal
        ImGui::PopStyleColor(3); // Cierra los 3 Color del Modal (DimBg, PopupBg, Border)
    }

// ==========================================================
    // ADMIN FRAME (COMMAND CENTER - CYBER 2026)
    // ==========================================================
    void Menu::FetchUsersFromDB() 
    {
        cachedUsers.clear();
        std::string rawJson = Auth::ApiClient::GetAllUsers();
        
        try 
        {
            auto j = nlohmann::json::parse(rawJson);
            for (const auto& item : j) 
            {
                UserNode u;
                u.username = item.value("username", "UNK");
                u.plan = item.value("membresia", "N/A");
                u.hwid = item.value("hwid", "NONE");
                cachedUsers.push_back(u);
            }
        } 
        catch (...) 
        {
            ShowAlert("API ERROR", "CRITICAL FAILURE PARSING NODE LIST.", true);
        }
    }

    void Menu::DrawAdminFrame() 
    {
        ImVec2 winSize = ImGui::GetWindowSize();

        float panelW = winSize.x * 0.90f; // Un poco más ancho para las tablas
        float panelH = winSize.y * 0.88f;
        ImGui::SetCursorPos(ImVec2((winSize.x - panelW) * 0.5f, (winSize.y - panelH) * 0.5f));

        // Estilo del panel Admin (Fondo obsidiana con borde rojo oscuro para indicar "Zona Peligrosa")
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.03f, 0.03f, 0.04f, 0.98f));
        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 12.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.0f);
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.5f, 0.1f, 0.1f, 0.5f)); // Borde rojo sutil

        if (ImGui::BeginChild("AdminPanel", ImVec2(panelW, panelH), true)) 
        {
            // --- HEADER ---
            ImGui::SetCursorPos(ImVec2(25, 25));
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f)); // Botón invisible
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.45f, 0.55f, 1.0f));
            if (ImGui::Button("< RETURN TO NEXUS", ImVec2(150, 30))) 
            {
                currentState = AppState::MainScreen;
            }
            ImGui::PopStyleColor(2);

            ImGui::SameLine(panelW - 200);
            ImGui::SetCursorPosY(30);
            ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "ROOT COMMAND CENTER");
            
            ImGui::SetCursorPosX(25);
            ImGui::Separator();

            // --- SISTEMA DE PESTAÑAS CUSTOM ---
            ImGui::SetCursorPos(ImVec2(25, 70));
            
            // Estilizar pestañas para que parezcan flat design
            ImGui::PushStyleColor(ImGuiCol_TabActive, ImVec4(0.85f, 0.27f, 0.94f, 0.8f)); // Magenta
            ImGui::PushStyleColor(ImGuiCol_TabHovered, ImVec4(0.85f, 0.27f, 0.94f, 0.4f));
            ImGui::PushStyleColor(ImGuiCol_Tab, ImVec4(0.08f, 0.08f, 0.12f, 1.0f));
            ImGui::PushStyleVar(ImGuiStyleVar_TabRounding, 4.0f);

            if (ImGui::BeginTabBar("AdminTabs", ImGuiTabBarFlags_None)) 
            {
                // ==========================================
                // PESTAÑA 1: GESTIÓN DE NODOS (Usuarios)
                // ==========================================
                if (ImGui::BeginTabItem("  ACTIVE NODES  ")) 
                {
                    ImGui::Spacing(); ImGui::Spacing();
                    
                    // Controles Superiores (Toolbar)
                    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
                    
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.15f, 0.15f, 0.22f, 1.0f));
                    if (ImGui::Button("REFRESH DATA", ImVec2(150, 35))) FetchUsersFromDB();
                    
                    ImGui::SameLine();
                    if (ImGui::Button("UNLOCK HWID...", ImVec2(150, 35))) 
                    {
                        showHwidResetModal = true;
                        memset(hwidResetTarget, 0, sizeof(hwidResetTarget));
                    }
                    ImGui::PopStyleColor();
                    ImGui::PopStyleVar(); // FrameRounding

                    ImGui::Spacing(); ImGui::Spacing();
                    
                    // Lista Scrolleable de Nodos
                    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.02f, 0.02f, 0.03f, 1.0f));
                    if (ImGui::BeginChild("UsersList", ImVec2(0, panelH - 200), true)) 
                    {
                        if (cachedUsers.empty()) 
                        {
                            ImGui::SetCursorPosY(50);
                            ImGui::SetCursorPosX((panelW - 250) * 0.5f);
                            ImGui::TextColored(ImVec4(0.4f, 0.45f, 0.55f, 1.0f), "NO ACTIVE NODES. CLICK REFRESH.");
                        } 
                        else 
                        {
                            // Animación global para los LEDs de estado
                            static float time = 0.0f;
                            time += ImGui::GetIO().DeltaTime * 3.0f;
                            float ledPulse = (sin(time) * 0.5f) + 0.5f; 

                            for (size_t i = 0; i < cachedUsers.size(); i++) 
                            {
                                auto& u = cachedUsers[i];
                                ImGui::PushID((int)i);
                                
                                // Tarjeta de Nodo Individual
                                ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.06f, 0.06f, 0.08f, 1.0f));
                                ImGui::BeginChild("UserCard", ImVec2(0, 55), true);
                                
                                // 1. Nombre de usuario
                                ImGui::SetCursorPos(ImVec2(15, 18));
                                ImGui::TextColored(ImVec4(0.9f, 0.9f, 0.95f, 1.0f), "%s", u.username.c_str());

                                // 2. Estado de HWID (Indicador LED)
                                ImGui::SameLine(180);
                                ImGui::SetCursorPosY(18);
                                
                                ImVec4 ledColor;
                                const char* statusText;
                                if (u.hwid != "NONE") 
                                {
                                    ledColor = ImVec4(1.0f, 0.2f, 0.2f, 0.5f + (ledPulse * 0.5f)); // Rojo palpitante
                                    statusText = "LOCKED";
                                } 
                                else 
                                {
                                    ledColor = ImVec4(0.0f, 1.0f, 0.5f, 0.8f); // Verde fijo
                                    statusText = "READY";
                                }
                                
                                ImDrawList* drawList = ImGui::GetWindowDrawList();
                                ImVec2 p = ImGui::GetCursorScreenPos();
                                drawList->AddCircleFilled(ImVec2(p.x + 5, p.y + 6), 4.0f, ImGui::ColorConvertFloat4ToU32(ledColor));
                                
                                ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 15);
                                ImGui::TextColored(ledColor, statusText);

                                // 3. Selector de Membresía
                                ImGui::SameLine(320);
                                ImGui::SetCursorPosY(14);
                                const char* plans[] = { "Weekly", "Monthly", "Yearly", "Lifetime" };
                                int currentPlanIndex = 0;
                                for (int p = 0; p < 4; p++) { if (u.plan == plans[p]) currentPlanIndex = p; }
                                
                                ImGui::PushItemWidth(130);
                                ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.03f, 0.03f, 0.04f, 1.0f));
                                if (ImGui::Combo("##plan", &currentPlanIndex, plans, IM_ARRAYSIZE(plans))) 
                                {
                                    auto res = Auth::ApiClient::UpdateMembership(u.username, plans[currentPlanIndex]);
                                    if (res.success) FetchUsersFromDB();
                                    else ShowAlert("ERROR", res.message, true);
                                }
                                ImGui::PopStyleColor();
                                ImGui::PopItemWidth();

                                // 4. Botón de Purga (Eliminar)
                                ImGui::SameLine(ImGui::GetWindowWidth() - 100);
                                ImGui::SetCursorPosY(12);
                                
                                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.05f, 0.05f, 1.0f));
                                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.8f, 0.1f, 0.1f, 1.0f));
                                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1.0f, 0.2f, 0.2f, 1.0f));
                                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.5f, 0.5f, 1.0f));
                                
                                if (ImGui::Button("PURGE", ImVec2(80, 30))) 
                                {
                                    userToDelete = u.username;
                                    showDeleteModal = true;
                                }
                                ImGui::PopStyleColor(4);

                                ImGui::EndChild();
                                ImGui::PopStyleColor(); // ChildBg de la tarjeta
                                ImGui::PopID();
                            }
                        }
                    }
                    ImGui::EndChild();
                    ImGui::PopStyleColor(); // ChildBg de la lista
                    
                    ImGui::EndTabItem();
                }

                // ==========================================
                // PESTAÑA 2: LICENSE FORGE (Generador)
                // ==========================================
                if (ImGui::BeginTabItem("  LICENSE FORGE  ")) 
                {
                    ImGui::Spacing(); ImGui::Spacing();
                    
                    // Contenedor para el generador
                    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.06f, 0.06f, 0.08f, 1.0f));
                    ImGui::BeginChild("MintControl", ImVec2(0, 180), true);
                    
                    ImGui::SetCursorPos(ImVec2(20, 20));
                    ImGui::TextColored(ImVec4(0.85f, 0.27f, 0.94f, 1.0f), "MINTING PROTOCOL");
                    
                    ImGui::SetCursorPos(ImVec2(20, 60));
                    ImGui::Text("TIER SELECTION:");
                    ImGui::SameLine(180);
                    const char* plans[] = { "Weekly", "Monthly", "Yearly", "Lifetime" };
                    ImGui::PushItemWidth(200);
                    ImGui::Combo("##mintTier", &keyTierIndex, plans, IM_ARRAYSIZE(plans));
                    
                    ImGui::SetCursorPos(ImVec2(20, 100));
                    ImGui::Text("QUANTITY:");
                    ImGui::SameLine(180);
                    ImGui::InputInt("##mintAmt", &keyAmount);
                    if (keyAmount < 1) keyAmount = 1;
                    if (keyAmount > 100) keyAmount = 100; 
                    ImGui::PopItemWidth();

                    // Botón Ejecutar Minting
                    ImGui::SetCursorPos(ImVec2(420, 60));
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.1f, 0.8f, 0.8f)); // Magenta oscuro
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.85f, 0.27f, 0.94f, 1.0f));
                    if (ImGui::Button("MINT LICENSES", ImVec2(200, 65))) 
                    {
                        auto res = Auth::ApiClient::GenerateKey(plans[keyTierIndex], keyAmount);
                        if (res.success) 
                        {
                            generatedKeysOutput.clear();
                            for (const auto& k : res.keys) 
                            {
                                generatedKeysOutput += k + "\n";
                            }
                        } 
                        else 
                        {
                            ShowAlert("ERROR", "Minting sequence failed.", true);
                        }
                    }
                    ImGui::PopStyleColor(2);

                    ImGui::EndChild();
                    ImGui::PopStyleColor();

                    ImGui::Spacing();
                    
                    // Caja de salida (Consola de llaves)
                    ImGui::TextColored(ImVec4(0.4f, 0.45f, 0.55f, 1.0f), "OUTPUT BUFFER:");
                    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.02f, 0.02f, 0.03f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.15f, 0.85f, 0.95f, 1.0f)); // Texto cian para las llaves
                    ImGui::InputTextMultiline("##keyOutput", (char*)generatedKeysOutput.c_str(), generatedKeysOutput.capacity() + 1, ImVec2(-1, -1), ImGuiInputTextFlags_ReadOnly);
                    ImGui::PopStyleColor(2);
                    
                    ImGui::EndTabItem();
                }
                
                ImGui::EndTabBar();
            }
            
            // Limpieza de colores de las pestañas
            ImGui::PopStyleVar();
            ImGui::PopStyleColor(3);

            // --- MODALES DEL ADMIN (No requieren cambios estéticos mayores por ahora) ---
            ImVec2 center = ImGui::GetMainViewport()->GetCenter();

            if (showHwidResetModal) ImGui::OpenPopup("HWID Override");
            ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
            if (ImGui::BeginPopupModal("HWID Override", &showHwidResetModal, ImGuiWindowFlags_AlwaysAutoResize)) 
            {
                ImGui::TextColored(ImVec4(0.15f, 0.85f, 0.95f, 1.0f), "TARGET USERNAME:");
                ImGui::InputText("##hwidTarget", hwidResetTarget, IM_ARRAYSIZE(hwidResetTarget));
                ImGui::Spacing();
                
                if (ImGui::Button("FORCE UNLOCK", ImVec2(120, 35))) 
                {
                    auto res = Auth::ApiClient::ResetHwid(hwidResetTarget);
                    if (res.success) FetchUsersFromDB();
                    else ShowAlert("ERROR", res.message, true);
                    showHwidResetModal = false;
                }
                ImGui::SameLine();
                if (ImGui::Button("ABORT", ImVec2(120, 35))) showHwidResetModal = false;
                ImGui::EndPopup();
            }

            if (showDeleteModal) ImGui::OpenPopup("Confirm Purge");
            ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
            if (ImGui::BeginPopupModal("Confirm Purge", &showDeleteModal, ImGuiWindowFlags_AlwaysAutoResize)) 
            {
                ImGui::Text("Permanently erase node: ");
                ImGui::SameLine();
                ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "%s?", userToDelete.c_str());
                ImGui::Spacing();
                
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.1f, 0.1f, 1.0f));
                if (ImGui::Button("EXECUTE PURGE", ImVec2(140, 35))) 
                {
                    auto res = Auth::ApiClient::DeleteUser(userToDelete);
                    if (res.success) FetchUsersFromDB();
                    else ShowAlert("ERROR", res.message, true);
                    showDeleteModal = false;
                }
                ImGui::PopStyleColor();
                
                ImGui::SameLine();
                if (ImGui::Button("ABORT", ImVec2(120, 35))) showDeleteModal = false;
                ImGui::EndPopup();
            }
        }
        
        ImGui::EndChild();
        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor(2);
    }

// ==========================================================
    // BYPASS FRAME (GHOST PROTOCOL - CYBER 2026)
    // ==========================================================
    void Menu::OpenFileDialog() 
    {
        OPENFILENAMEW ofn;
        wchar_t szFile[260] = { 0 };

        ZeroMemory(&ofn, sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = NULL;
        ofn.lpstrFile = szFile;
        ofn.nMaxFile = sizeof(szFile);
        ofn.lpstrFilter = L"Executables (*.exe)\0*.exe\0DLLs (*.dll)\0*.dll\0All Files (*.*)\0*.*\0";
        ofn.nFilterIndex = 1;
        ofn.lpstrFileTitle = NULL;
        ofn.nMaxFileTitle = 0;
        ofn.lpstrInitialDir = NULL;
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

        if (GetOpenFileNameW(&ofn) == TRUE) 
        {
            targetPath = ofn.lpstrFile;
            
            std::lock_guard<std::mutex> lock(logMutex);
            std::wstring filename = targetPath.substr(targetPath.find_last_of(L"/\\") + 1);
            
            int size_needed = WideCharToMultiByte(CP_UTF8, 0, &filename[0], (int)filename.size(), NULL, 0, NULL, NULL);
            std::string strTo(size_needed, 0);
            WideCharToMultiByte(CP_UTF8, 0, &filename[0], (int)filename.size(), &strTo[0], size_needed, NULL, NULL);
            
            consoleLogs.push_back("[>] SYS_LINK ESTABLISHED WITH: " + strTo);
            consoleLogs.push_back("[>] AWAITING EXECUTION COMMAND...");
        }
    }

    void Menu::RunBypassThread() 
    {
        auto logger = [](const std::string& msg) 
        {
            std::lock_guard<std::mutex> lock(logMutex); 
            consoleLogs.push_back("[>] " + msg);        
        };

        logger("INITIATING ULTRA-GOD-TIER ANTI-SS PROTOCOL...");

        // Llamada al motor maestro
        Core::Cleaner::DeepCleanProcess(targetPath, logger);
        
        // Si el Deep Scan estaba activado en Ajustes, lo lanzamos ahora
        if (deepScanEnabled) 
        {
            logger("DEPLOYING DEEP REGISTRY SCANNERS...");
            Core::Cleaner::DeepRegistrySearchCleaner(targetPath, logger);
        }

        logger("=========================================");
        logger("SYSTEM SANITIZED FOR SS. TARGET ANNIHILATED.");
        logger("=========================================");
        
        isWiping = false;
        targetPath = L""; 
    }

    void Menu::DrawBypassFrame() 
    {
        ImVec2 winSize = ImGui::GetWindowSize();
        
        // Panel expansivo (ocupa más espacio para dar protagonismo a la terminal)
        float panelW = winSize.x * 0.90f;
        float panelH = winSize.y * 0.90f;
        ImGui::SetCursorPos(ImVec2((winSize.x - panelW) * 0.5f, (winSize.y - panelH) * 0.5f));

        // Estilo visual del panel: Fondo Obsidiana con borde Violeta neón
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.04f, 0.04f, 0.06f, 0.95f)); 
        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 12.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.0f);
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.55f, 0.36f, 0.96f, 0.5f));

        if (ImGui::BeginChild("BypassPanel", ImVec2(panelW, panelH), true)) 
        {
            // --- HEADER ---
            ImGui::SetCursorPos(ImVec2(25, 25));
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f)); 
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.45f, 0.55f, 1.0f));
            
            if (ImGui::Button("< ABORT PROTOCOL", ImVec2(150, 30)) && !isWiping) 
            {
                consoleLogs.clear();
                targetPath = L"";
                currentState = AppState::MainScreen; 
            }
            ImGui::PopStyleColor(2);

            ImGui::SameLine(panelW - 220);
            ImGui::SetCursorPosY(30);
            ImGui::TextColored(ImVec4(0.85f, 0.27f, 0.94f, 1.0f), "GHOST PROTOCOL v3.0"); // Magenta
            
            ImGui::SetCursorPosX(25);
            ImGui::Separator();

            // --- SECCIÓN 1: SELECCIÓN DEL OBJETIVO ---
            ImGui::SetCursorPosY(70);
            ImGui::SetCursorPosX(30);
            ImGui::TextColored(ImVec4(0.4f, 0.45f, 0.55f, 1.0f), "TARGET IDENTIFICATION:");

            ImGui::SetCursorPosY(95);
            ImGui::SetCursorPosX(30);
            
            // Si no hay archivo, mostramos el botón. Si hay, mostramos la ruta.
            if (targetPath.empty()) 
            {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.08f, 0.08f, 0.12f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.55f, 0.36f, 0.96f, 1.0f));
                ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
                ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
                
                if (ImGui::Button("BROWSE FOR TARGET BINARY (.EXE / .DLL)", ImVec2(panelW - 60, 45)) && !isWiping) 
                {
                    OpenFileDialog(); 
                }
                
                ImGui::PopStyleVar(2);
                ImGui::PopStyleColor(2);
            } 
            else 
            {
                // Caja que muestra el archivo cargado
                ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.02f, 0.02f, 0.03f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 1.0f, 0.5f, 0.6f)); // Borde verde
                ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 4.0f);
                ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.0f);
                
                if (ImGui::BeginChild("TargetLockedBox", ImVec2(panelW - 60, 45), true))
                {
                    ImGui::SetCursorPos(ImVec2(15, 14));
                    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.5f, 1.0f), "[LOCKED]");
                    
                    std::wstring wFilename = targetPath.substr(targetPath.find_last_of(L"/\\") + 1);
                    int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wFilename[0], (int)wFilename.size(), NULL, 0, NULL, NULL);
                    std::string strFilename(size_needed, 0);
                    WideCharToMultiByte(CP_UTF8, 0, &wFilename[0], (int)wFilename.size(), &strFilename[0], size_needed, NULL, NULL);

                    ImGui::SameLine();
                    ImGui::Text(" %s", strFilename.c_str());

                    // Botón para cancelar la selección
                    ImGui::SameLine(ImGui::GetWindowWidth() - 40);
                    ImGui::SetCursorPosY(12);
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.2f, 0.2f, 1.0f));
                    if (ImGui::Button("X", ImVec2(25, 25)) && !isWiping) 
                    {
                        targetPath = L"";
                        consoleLogs.push_back("[!] TARGET DISENGAGED.");
                    }
                    ImGui::PopStyleColor();
                }
                ImGui::EndChild();
                ImGui::PopStyleVar(2);
                ImGui::PopStyleColor(2);
            }

            // --- SECCIÓN 2: CONSOLA DE DIAGNÓSTICO TÁCTICO ---
            ImGui::SetCursorPosY(160);
            ImGui::SetCursorPosX(30);
            ImGui::TextColored(ImVec4(0.4f, 0.45f, 0.55f, 1.0f), "EXECUTION LOGS:");

            ImGui::SetCursorPosY(185);
            ImGui::SetCursorPosX(30);
            
            // Fondo de la consola (Negro puro con ligero tinte azul)
            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.01f, 0.01f, 0.015f, 1.0f));
            // Texto de la consola (Cian Matrix)
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.15f, 0.85f, 0.95f, 1.0f));
            ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 4.0f);
            
            if (ImGui::BeginChild("LogConsole", ImVec2(panelW - 60, panelH - 310), true, ImGuiWindowFlags_AlwaysVerticalScrollbar)) 
            {
                ImGui::Spacing();
                std::lock_guard<std::mutex> lock(logMutex);
                for (const auto& log : consoleLogs) 
                {
                    ImGui::SetCursorPosX(15);
                    // Colorear errores o advertencias en rojo
                    if (log.find("[!]") != std::string::npos || log.find("ERROR") != std::string::npos) {
                        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));
                        ImGui::TextWrapped("%s", log.c_str());
                        ImGui::PopStyleColor();
                    } else {
                        ImGui::TextWrapped("%s", log.c_str());
                    }
                }
                
                // Auto-scroll automático cuando se añaden líneas
                if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) 
                {
                    ImGui::SetScrollHereY(1.0f);
                }

                ImGui::EndChild();
            }
            ImGui::PopStyleVar();
            ImGui::PopStyleColor(2);

            // --- SECCIÓN 3: BOTÓN DE EJECUCIÓN (WIPE) ---
            ImGui::SetCursorPosY(panelH - 100);
            ImGui::SetCursorPosX(30);
            
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);

            if (isWiping) 
            {
                // Estado en progreso: Botón oscurecido
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.1f, 0.3f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.6f, 0.7f, 1.0f));
                ImGui::Button("W I P I N G   I N   P R O G R E S S . . .", ImVec2(panelW - 60, 70));
                ImGui::PopStyleColor(2);
            } 
            else 
            {
                // Estado listo: Botón Rojo Peligro si hay objetivo, Gris si no hay
                if (targetPath.empty()) 
                {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f, 0.1f, 0.15f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.15f, 0.15f, 0.22f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f, 0.1f, 0.15f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.45f, 0.55f, 1.0f));
                } 
                else 
                {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.1f, 0.2f, 0.9f)); // Rojo intenso
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.2f, 0.3f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.6f, 0.05f, 0.1f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
                }

                if (ImGui::Button("E X E C U T E   N E U R A L   B Y P A S S", ImVec2(panelW - 60, 70))) 
                {
                    if (targetPath.empty()) 
                    {
                        ShowAlert("SYSTEM ERROR", "TARGET BINARY NOT SELECTED. PLEASE BROWSE FOR A FILE FIRST.", true);
                    } 
                    else 
                    {
                        isWiping = true;
                        // No limpiamos los logs anteriores para mantener la historia, solo añadimos separador
                        consoleLogs.push_back("");
                        std::thread wipeThread(RunBypassThread); 
                        wipeThread.detach(); 
                    }
                }
                
                ImGui::PopStyleColor(4);
            }
            
            ImGui::PopStyleVar(); // FrameRounding
        }
        
        ImGui::EndChild();
        ImGui::PopStyleColor(2);
        ImGui::PopStyleVar(2);
    }

// ==========================================================
    // MÉTODO DRAW PRINCIPAL (EL ENRUTADOR - CYBER 2026)
    // ==========================================================
    void Menu::Draw() 
    {
        // Inicialización única (Lazy Loading)
        static bool initialized = false;
        
        if (!initialized) 
        {
            SetupLanguages();      
            ApplyCyberNeonTheme();
            
            // Limpieza de seguridad al arrancar
            memset(userBuffer, 0, sizeof(userBuffer));
            memset(passBuffer, 0, sizeof(passBuffer));
            memset(keyBuffer, 0, sizeof(keyBuffer));
            memset(hwidResetTarget, 0, sizeof(hwidResetTarget));
            
            initialized = true;    
        }

        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);

        // Banderas estrictas para la ventana maestra (Fondo invisible para dibujar nosotros mismos)
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | 
                                        ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBackground | 
                                        ImGuiWindowFlags_NoBringToFrontOnFocus;

        // Forzamos un fondo negro puro "Void"
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.01f, 0.01f, 0.01f, 1.0f));

        if (ImGui::Begin("ScannelerMain", NULL, window_flags)) 
        {
            ImDrawList* drawList = ImGui::GetWindowDrawList();
            ImVec2 pos = ImGui::GetCursorScreenPos();
            ImVec2 size = ImGui::GetContentRegionAvail();
            
            // 1. Renderizado de fondos base según la pantalla activa
            if (currentState == AppState::MainScreen || currentState == AppState::BypassScreen) 
            {
                DrawMatrixBackground(drawList, pos, size);
            } 
            else 
            {
                DrawCyberGrid(drawList, pos, size);
                DrawCentralGlow(drawList, ImVec2(pos.x + size.x / 2, pos.y + size.y / 2));
            }

            // 2. EFECTO CRT (Scanlines Globales) - El toque premium
            ImU32 scanlineColor = IM_COL32(0, 0, 0, 40); // Líneas negras semitransparentes
            for (float y = 0; y < size.y; y += 4.0f) 
            {
                drawList->AddLine(ImVec2(pos.x, pos.y + y), ImVec2(pos.x + size.x, pos.y + y), scanlineColor);
            }

            // 3. Manejador central de estados (Enrutamiento de Paneles)
            switch (currentState) 
            {
                case AppState::SplashScreen: 
                    DrawSplashScreen(); 
                    break;
                case AppState::LoginScreen:  
                    DrawLoginFrame();   
                    break;
                case AppState::MainScreen:   
                    DrawMainFrame();    
                    break;
                case AppState::BypassScreen: 
                    DrawBypassFrame();  
                    break;
                case AppState::AdminScreen:  
                    DrawAdminFrame();   
                    break;
            }

            // 4. Capa superior: Alertas del sistema
            DrawCyberAlert();
        }
        
        ImGui::End();
        ImGui::PopStyleColor(); // Restauramos el color de ventana
    }

} // FIN DEL NAMESPACE GUI