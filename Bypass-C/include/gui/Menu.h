#pragma once
#include <string>
#include <vector>
#include <map>
#include <mutex>
#include <thread>
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

    // Estructura para almacenar el caché de la DB
    struct UserNode {
        std::string username;
        std::string plan;
        std::string hwid;        
    };

    class Menu {
    public:
        Menu();
        static void Draw();

    private:
        // --- TODAS LAS VARIABLES SON STATIC (Declaraciones puras sin '=') ---
        // El valor real se asigna en main.cpp para evitar el error 'No current context'
        
        static AppState currentState;
        static std::string currentLang;
        static std::map<std::string, std::map<std::string, std::string>> languages;

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

        // Variables de Redeem (Canjear)
        static char redeemKey[256];
        static char redeemUser[256];
        static char redeemPass[256];
        static char redeemConf[256];
        static std::string redeemStatus;

        // Variables de Inicio (Main) / SYS CONFIG
        static bool showSettings;
        static bool deepScanEnabled;
        static float matrixDrops[50];
        static bool isBindingHotkey;
        static int bypassHotkey; // <--- AQUÍ ESTÁ LA NUEVA VARIABLE DEL HOTKEY

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

        // --- MÉTODOS DE DIBUJO ---
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
        static void DrawCyberAlert();
        static void ShowAlert(const std::string& title, const std::string& msg, bool isError = false);
        static std::string GetText(const std::string& key);
        static std::string GetKeyName(int vkCode); 

        // --- MÉTODOS AUXILIARES ---
        static void FetchUsersFromDB(); 
        static void OpenFileDialog();
        static void RunBypassThread();
    };
}