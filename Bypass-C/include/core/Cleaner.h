#pragma once
#include <string>
#include <vector>
#include <functional>
#include <windows.h>

namespace Core {
    class Cleaner {
    public:
        // Definimos el tipo de función para el logger
        using LoggerFunc = std::function<void(const std::string&)>;

        // ==========================================================
        // FUNCIONES PÚBLICAS (Visibles para Menu.cpp y el resto)
        // ==========================================================
        
        // Anti-Forensics: Manipulación de Tiempo
        static void SetSystemDate(int year, int month, int day, LoggerFunc logger);
        static void RestoreSystemTime(LoggerFunc logger);
        static void TimeStompFile(const std::wstring& filePath, LoggerFunc logger);
        static void CleanSpecificPrefetch(const std::wstring& filePath, LoggerFunc logger);

        // Ejecución Maestra y Autodestrucción
        static void DeepCleanProcess(const std::wstring& targetPath, LoggerFunc logger);
        static void ExecuteKamikazeProtocol(LoggerFunc logger);
        static void DeepRegistrySearchCleaner(const std::wstring& filePath, LoggerFunc logger);

        // Limpieza de Registro Avanzada
        static void CleanUserAssist(const std::wstring& filePath, LoggerFunc logger);
        static void CleanRecentApps(const std::wstring& filePath, LoggerFunc logger);
        static void CleanAppCompatTotal(const std::wstring& filePath, LoggerFunc logger);
        static void CleanAmcache(const std::wstring& filePath, LoggerFunc logger);
        static void CleanMuiCache(const std::wstring& filePath, LoggerFunc logger);
        static void CleanTaskCache(const std::wstring& filePath, LoggerFunc logger);
        static void SelectiveRegistryClean(const std::wstring& filePath, LoggerFunc logger);
        static void CleanGlobalTracesByName(const std::wstring& filePath, LoggerFunc logger);

        // Rastros de Sistema y Red
        static void CleanClipboard(LoggerFunc logger);
        static void CleanConsoleHistory(LoggerFunc logger);
        static void CleanMountPoints(const std::wstring& filePath, LoggerFunc logger);
        static void CleanUSBPaperbin(const std::wstring& filePath, LoggerFunc logger);
        static void CleanJumpLists(LoggerFunc logger);
        static void DeepWipeUSNJournal(LoggerFunc logger);
        static void FlushDNSAndARP(LoggerFunc logger);
        static void CleanEventLogs(LoggerFunc logger);
        static void CleanRecentLNKs(const std::wstring& filePath, LoggerFunc logger);
        static void CleanSpecificShimCache(const std::wstring& filePath, LoggerFunc logger);
        static void CleanShellBags(LoggerFunc logger);
        static void CleanShellExperience(LoggerFunc logger);
        static void CleanEverythingService(LoggerFunc logger);
        static void CleanIconCache(LoggerFunc logger);
        static void CleanBrowserDownloads(const std::wstring& filePath, LoggerFunc logger);
        static void CleanSrumAndDirectX(LoggerFunc logger);
        
        // Destrucción y Ofuscación
        static void RemoveADS(const std::wstring& filePath, LoggerFunc logger);
        static void ShredAndDestroy(const std::wstring& filePath, LoggerFunc logger);
        static void CamouflageMFT(const std::wstring& directoryPath, LoggerFunc logger);
        static void ExtremeSurgicalClean(const std::wstring& filePath, LoggerFunc logger);
        
        // Gestión de Memoria y Sistema
        static void PurgeSelfMemory(LoggerFunc logger);
        static void RestartExplorer(LoggerFunc logger);
        static void ConfigurePagefileWipe(LoggerFunc logger);

    private:
        // ==========================================================
        // FUNCIONES PRIVADAS (Auxiliares requeridas por el .cpp)
        // ==========================================================
        
        // Estos helpers DEBEN estar aquí porque Cleaner.cpp los usa internamente
        static void DeleteRegistryValueIfContains(HKEY hKey, const std::wstring& target, LoggerFunc logger);
        static void RecursiveRegistrySearch(HKEY hRoot, const std::wstring& currentPath, const std::wstring& target, const std::wstring& targetSinExt, int& foundCount, LoggerFunc logger);
        static std::wstring GetHkeyString(HKEY hKey);
        static std::wstring DecodeROT13(std::wstring input);

        // Helpers de Texto (El .cpp los usa para procesar rutas)
        static std::wstring GetFileName(const std::wstring& path);
        static std::wstring GetFileNameWithoutExtension(const std::wstring& path);
        static std::string WStringToString(const std::wstring& wstr);
        static std::wstring ToLower(std::wstring str);
        static std::wstring ToUpper(std::wstring str);

        static void FakeActivityGenerator(LoggerFunc logger);
    };
}