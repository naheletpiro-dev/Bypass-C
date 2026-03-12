#pragma once
#include <string>
#include <vector>
#include <map>
#include <mutex>
#include <thread>
#include <d3d11.h> // Necesario para la textura del logo
#include <imgui.h>
#include "../auth/ApiClient.h" 
#include "../core/Cleaner.h"
#include <nlohmann/json.hpp> 

namespace Gui {

    enum class AppState {
        SplashScreen,
        LoginScreen,
        MainScreen,
        BypassScreen,
        AdminScreen
    }; 

    struct UserNode {
        std::string username;
        std::string plan;
        std::string hwid;        
    };

    class Menu {
    public:
        Menu();
        static void Draw();
        
        // --- MÉTODO PARA CARGAR RECURSOS (PNG) ---
        static bool LoadLogoTexture(ID3D11Device* device, const char* filename);

    private:
        // --- INSTANCIA PARA THREADS ---
        static Menu* _instance; 

        // --- RECURSOS VISUALES ---
        static ID3D11ShaderResourceView* logoTexture; // Tu Scanneler.png cargado aquí

        // --- ESTADOS Y CONFIGURACIÓN ---
        static AppState currentState;
        static std::string currentLang;
        static std::map<std::string, std::map<std::string, std::string>> languages;

        // --- ANIMACIONES CSS ---
        static std::map<std::string, float> inputAnims; 
        static float btnHoverLerp;                     
        static float scanlineTimer;                    

        // Splash Screen
        static float splashProgress;
        static int splashStep;
        static float pulseDirection;
        static float glowWidth;

        // Alertas
        static bool showAlert;
        static bool alertIsError;
        static std::string alertTitle;
        static std::string alertMessage;

        // Variables de Login
        static char userBuffer[256];
        static char passBuffer[256];
        static char keyBuffer[256];
        static std::string loginStatus;
        static bool isLoginError;
        static bool isAdmin;
        static bool isConnecting; 

        // Variables de Redeem
        static char redeemKey[256];
        static char redeemUser[256];
        static char redeemPass[256];
        static char redeemConf[256];
        static std::string redeemStatus;

        // Variables de Inicio (Main) / SYS CONFIG
        static bool showSettings;
        static bool deepScanEnabled;
        static float matrixDrops[70]; 
        static bool isBindingHotkey;
        static int bypassHotkey;

        // Variables de Admin (Command Center)
        static std::vector<UserNode> cachedUsers;
        static int keyTierIndex;
        static int keyAmount;
        static std::string generatedKeysOutput;
        static bool showHwidResetModal;
        static char hwidResetTarget[256];
        static bool showDeleteModal;
        static std::string userToDelete;

        // Variables de Bypass (Ghost Protocol)
        static std::wstring targetPath;
        static std::vector<std::string> consoleLogs;
        static std::mutex logMutex; 
        static bool isWiping;
        static bool showKamikazeModal;

        // --- MÉTODOS DE DIBUJO (UI ENGINE) ---
        static void SetupLanguages();
        static void ApplyCyberNeonTheme();
        static void DrawCyberGrid(ImDrawList* drawList, ImVec2 pos, ImVec2 size);
        static void DrawCentralGlow(ImDrawList* drawList, ImVec2 center);
        static void DrawMatrixBackground(ImDrawList* drawList, ImVec2 pos, ImVec2 size);
        
        static void DrawSplashScreen();
        static void DrawLoginFrame();
        static void DrawMainFrame();
        static void DrawAdminFrame(); 
        static void DrawBypassFrame();
        static void DrawSettingsFrame();
        static void DrawCyberAlert();

        // --- UTILIDADES ---
        static void ShowAlert(const std::string& title, const std::string& msg, bool isError = false);
        static std::string GetText(const std::string& key);
        static std::string GetKeyName(int vkCode); 
        static void FetchUsersFromDB(); 
        static void OpenFileDialog();
        
        static void RunBypassThread(); 
    };
}