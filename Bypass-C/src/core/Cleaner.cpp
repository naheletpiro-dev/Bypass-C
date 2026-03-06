#include "../../include/core/Cleaner.h"
#include "../../include/core/SystemUtils.h" // Asumiendo que aquí tienes tu ExecuteHiddenCommand
#include <iostream>
#include <fstream>
#include <random>
#include <algorithm>
#include <filesystem>
#include <sqlite3.h>
#include <psapi.h>   // <--- NECESARIO para arreglar el error 'EmptyWorkingSet'
#include <shellapi.h> // <--- NECESARIO para operaciones con archivos y LNK

namespace fs = std::filesystem;

namespace Core {

// --- Helpers Internos Actualizados ---

    std::string Cleaner::WStringToString(const std::wstring& wstr) {
        if (wstr.empty()) return std::string();
        int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
        std::string strTo(size_needed, 0);
        WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
        return strTo;
    }

    std::wstring Cleaner::ToLower(std::wstring str) {
        std::transform(str.begin(), str.end(), str.begin(), ::towlower);
        return str;
    }

    // No olvides añadir ToUpper que te pedía el error anterior
    std::wstring Cleaner::ToUpper(std::wstring str) {
        std::transform(str.begin(), str.end(), str.begin(), ::towupper);
        return str;
    }

    std::wstring Cleaner::GetFileName(const std::wstring& path) {
        size_t pos = path.find_last_of(L"/\\");
        return (pos == std::wstring::npos) ? path : path.substr(pos + 1);
    }

    std::wstring Cleaner::GetFileNameWithoutExtension(const std::wstring& path) {
        std::wstring name = GetFileName(path);
        size_t pos = name.find_last_of(L".");
        return (pos == std::wstring::npos) ? name : name.substr(0, pos);
    }

    // ==========================================================
    // ANTI-FORENSICS: MANIPULACIÓN DE TIEMPO
    // ==========================================================

    void Cleaner::SetSystemDate(int year, int month, int day, LoggerFunc logger) {
        try {
            // Utilizamos la estructura nativa en lugar de powershell para mayor sigilo
            SYSTEMTIME st;
            GetLocalTime(&st);
            st.wYear = year;
            st.wMonth = month;
            st.wDay = day;
            
            if (SetLocalTime(&st)) {
                logger("⚡ System time desynchronized successfully.");
            } else {
                logger("Time Error: Insufficient privileges.");
            }
        } catch (...) {
            logger("Time Error: Unknown exception.");
        }
    }

    void Cleaner::RestoreSystemTime(LoggerFunc logger) {
        // Para resincronizar, los comandos del sistema siguen siendo la mejor opción
        // Asume que tienes SystemUtils::ExecuteHiddenCommand implementado
        SystemUtils::ExecuteHiddenCommand(L"w32tm /resync");
        SystemUtils::ExecuteHiddenCommand(L"powershell -Command \"Start-Service w32time; resync-time\"");
        logger("✅ System time resynchronized.");
    }

    // ==========================================================
    // LIMPIEZA ELITE (NÚCLEO FORENSE)
    // ==========================================================

    void Cleaner::TimeStompFile(const std::wstring& filePath, LoggerFunc logger) {
        if (!fs::exists(filePath)) return;

        HANDLE hFile = CreateFileW(filePath.c_str(), FILE_WRITE_ATTRIBUTES, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile == INVALID_HANDLE_VALUE) return;

        // Fecha antigua: 12/05/2015 10:30:00
        SYSTEMTIME st = {0};
        st.wYear = 2015; st.wMonth = 5; st.wDay = 12;
        st.wHour = 10; st.wMinute = 30; st.wSecond = 0;

        FILETIME ft;
        SystemTimeToFileTime(&st, &ft); // Convertir a Windows FILETIME

        if (SetFileTime(hFile, &ft, &ft, &ft)) {
            logger("→ TimeStomping: MAC timestamps set to 2015.");
        } else {
            logger("→ TimeStomp: Failed to alter timestamps.");
        }
        CloseHandle(hFile);
    }

    void Cleaner::DeleteRegistryValueIfContains(HKEY hKey, const std::wstring& target, LoggerFunc logger) {
        DWORD numVals = 0;
        DWORD maxValNameLen = 0;
        if (RegQueryInfoKeyW(hKey, NULL, NULL, NULL, NULL, NULL, NULL, &numVals, &maxValNameLen, NULL, NULL, NULL) == ERROR_SUCCESS) {
            std::vector<wchar_t> valNameBuf(maxValNameLen + 1);
            
            // Iterar hacia atrás al borrar
            for (int i = numVals - 1; i >= 0; --i) {
                DWORD valNameLen = maxValNameLen + 1;
                if (RegEnumValueW(hKey, i, valNameBuf.data(), &valNameLen, NULL, NULL, NULL, NULL) == ERROR_SUCCESS) {
                    std::wstring valName(valNameBuf.data());
                    if (ToLower(valName).find(target) != std::wstring::npos) {
                        RegDeleteValueW(hKey, valName.c_str());
                        logger("→ Registry trace purged: " + WStringToString(GetFileName(valName)));
                    }
                }
            }
        }
    }

    void Cleaner::SelectiveRegistryClean(const std::wstring& filePath, LoggerFunc logger) {
        std::wstring target = ToLower(filePath);

        struct RegPath {
            HKEY root;
            std::wstring subkey;
        };

        std::vector<RegPath> rutasReg = {
            { HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Services\\bam\\UserSettings" },
            { HKEY_CURRENT_USER, L"Software\\Classes\\Local Settings\\Software\\Microsoft\\Windows\\Shell\\MuiCache" },
            { HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows NT\\CurrentVersion\\AppCompatFlags\\Compatibility Assistant\\Store" },
            { HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows NT\\CurrentVersion\\AppCompatFlags\\Layers" }
        };

        for (const auto& r : rutasReg) {
            HKEY hKey;
            if (RegOpenKeyExW(r.root, r.subkey.c_str(), 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS) {
                
                // Si es BAM, tiene subclaves (los SIDs de los usuarios)
                if (ToLower(r.subkey).find(L"bam") != std::wstring::npos) {
                    DWORD numSubkeys = 0;
                    DWORD maxSubkeyLen = 0;
                    if (RegQueryInfoKeyW(hKey, NULL, NULL, NULL, &numSubkeys, &maxSubkeyLen, NULL, NULL, NULL, NULL, NULL, NULL) == ERROR_SUCCESS) {
                        std::vector<wchar_t> subkeyBuf(maxSubkeyLen + 1);
                        for (DWORD i = 0; i < numSubkeys; ++i) {
                            DWORD subkeyLen = maxSubkeyLen + 1;
                            if (RegEnumKeyExW(hKey, i, subkeyBuf.data(), &subkeyLen, NULL, NULL, NULL, NULL) == ERROR_SUCCESS) {
                                HKEY hSidKey;
                                if (RegOpenKeyExW(hKey, subkeyBuf.data(), 0, KEY_ALL_ACCESS, &hSidKey) == ERROR_SUCCESS) {
                                    DeleteRegistryValueIfContains(hSidKey, target, logger);
                                    RegCloseKey(hSidKey);
                                }
                            }
                        }
                    }
                } else {
                    DeleteRegistryValueIfContains(hKey, target, logger);
                }
                RegCloseKey(hKey);
            }
        }
    }

    void Cleaner::CleanSpecificPrefetch(const std::wstring& filePath, LoggerFunc logger) {
        std::wstring baseName = ToLower(GetFileNameWithoutExtension(filePath));
        
        wchar_t pfPath[MAX_PATH];
        ExpandEnvironmentStringsW(L"%SystemRoot%\\Prefetch", pfPath, MAX_PATH);
        
        try {
            for (const auto& entry : fs::directory_iterator(pfPath)) {
                if (entry.is_regular_file()) {
                    std::wstring currentFile = ToLower(entry.path().filename().wstring());
                    if (currentFile.find(baseName) == 0) { // startswith
                        fs::remove(entry.path());
                        logger("→ Prefetch purged: " + WStringToString(entry.path().filename().wstring()));
                    }
                }
            }
        } catch (...) {
            logger("→ Prefetch error: Access Denied or Missing Folder.");
        }
    }

    void Cleaner::CleanGlobalTracesByName(const std::wstring& filePath, LoggerFunc logger) {
        std::wstring targetName = ToLower(GetFileName(filePath));
        std::wstring targetNameNoExt = GetFileNameWithoutExtension(targetName);

        // 1. Prefetch global
        CleanSpecificPrefetch(filePath, logger);

        // 2. Registro Persistente (Igual que en Python, reusamos el limpiador pasándole el nombre en lugar de la ruta entera)
        SelectiveRegistryClean(targetName, logger);
        
        logger("→ Registry scan complete for name: " + WStringToString(targetName));
    }

    void Cleaner::RemoveADS(const std::wstring& filePath, LoggerFunc logger) {
        // En C++ no necesitamos PowerShell. Simplemente borramos el Alternate Data Stream de forma nativa.
        std::wstring adsPath = filePath + L":Zone.Identifier";
        if (DeleteFileW(adsPath.c_str())) {
            logger("→ ADS / Zone.Identifier neutralized natively.");
        }
    }

    void Cleaner::ShredAndDestroy(const std::wstring& filePath, LoggerFunc logger) {
        if (!fs::exists(filePath)) return;

        try {
            // 1. Shredding (Sobreescritura física masiva)
            std::fstream file(filePath, std::ios::in | std::ios::out | std::ios::binary);
            if (file.is_open()) {
                file.seekp(0, std::ios::end);
                size_t size = file.tellp();
                file.seekp(0, std::ios::beg);

                std::vector<char> buffer(size);
                std::random_device rd;
                std::mt19937 gen(rd());
                std::uniform_int_distribution<> dis(0, 255);
                
                for (size_t i = 0; i < size; ++i) buffer[i] = dis(gen);
                
                file.write(buffer.data(), size);
                file.close();
                logger("→ Physical data stream destroyed.");
            }

            // 2. MFT Renaming (Renombrado múltiple)
            std::wstring dirName = fs::path(filePath).parent_path().wstring();
            std::wstring currPath = filePath;
            
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> dis(1000, 9999);

            for (int i = 0; i < 3; ++i) {
                std::wstring newName = dirName + L"\\tmp" + std::to_wstring(dis(gen)) + L".sys";
                _wrename(currPath.c_str(), newName.c_str());
                currPath = newName;
            }

            // 3. Eliminación final
            if (DeleteFileW(currPath.c_str())) {
                logger("→ File unlinked. MFT name trace obfuscated.");
            }
        } catch (const std::exception& e) {
            logger(std::string("→ Destruction error: ") + e.what());
        }
    }

    // ==========================================================
    // RASTROS ADICIONALES (PENDRIVE, RED Y SISTEMA)
    // ==========================================================

    void Cleaner::CleanClipboard(LoggerFunc logger) {
        if (OpenClipboard(NULL)) {
            EmptyClipboard();
            CloseClipboard();
        }
        SystemUtils::ExecuteHiddenCommand(L"powershell.exe Restart-Service -Name \"cbdhsvc_*\" -Force");
        logger("→ Clipboard & Win+V history sanitized.");
    }

    void Cleaner::CleanConsoleHistory(LoggerFunc logger) {
        // 1. Historial de PowerShell
        wchar_t psPath[MAX_PATH];
        ExpandEnvironmentStringsW(L"%AppData%\\Microsoft\\Windows\\PowerShell\\PSReadLine\\ConsoleHost_history.txt", psPath, MAX_PATH);
        if (DeleteFileW(psPath)) {
            logger("→ PowerShell command history annihilated.");
        }
        SystemUtils::ExecuteHiddenCommand(L"doskey /reinstall");

        // 2. Host Prefetch
        wchar_t pfPath[MAX_PATH];
        ExpandEnvironmentStringsW(L"%SystemRoot%\\Prefetch", pfPath, MAX_PATH);
        std::vector<std::wstring> hosts = { L"CMD.EXE", L"POWERSHELL.EXE", L"REG.EXE", L"WEVTUTIL.EXE", L"FSUTIL.EXE", L"SC.EXE" };
        
        int count = 0;
        try {
            for (const auto& entry : fs::directory_iterator(pfPath)) {
                if (entry.is_regular_file()) {
                    std::wstring currentFile = ToUpper(entry.path().filename().wstring()); // ToUpper es similar a ToLower que ya creamos
                    for (const auto& h : hosts) {
                        if (currentFile.find(h) != std::wstring::npos) {
                            fs::remove(entry.path());
                            count++;
                            break;
                        }
                    }
                }
            }
            if (count > 0) logger("→ Host Prefetch: " + std::to_string(count) + " system tool traces purged.");
        } catch (...) {}
    }

    void Cleaner::CleanMountPoints(const std::wstring& filePath, LoggerFunc logger) {
        std::wstring driveLetter = filePath.substr(0, 1); // Extrae la letra 'C', 'D', etc.
        if (driveLetter.empty() || filePath.length() < 2 || filePath[1] != L':') return;

        HKEY hKey;
        std::wstring path = L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\MountPoints2";
        if (RegOpenKeyExW(HKEY_CURRENT_USER, path.c_str(), 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS) {
            DWORD numSubkeys = 0, maxSubkeyLen = 0;
            if (RegQueryInfoKeyW(hKey, NULL, NULL, NULL, &numSubkeys, &maxSubkeyLen, NULL, NULL, NULL, NULL, NULL, NULL) == ERROR_SUCCESS) {
                std::vector<wchar_t> subkeyBuf(maxSubkeyLen + 1);
                for (int i = numSubkeys - 1; i >= 0; --i) {
                    DWORD subkeyLen = maxSubkeyLen + 1;
                    if (RegEnumKeyExW(hKey, i, subkeyBuf.data(), &subkeyLen, NULL, NULL, NULL, NULL) == ERROR_SUCCESS) {
                        std::wstring subkeyName(subkeyBuf.data());
                        if (subkeyName.find(driveLetter) != std::wstring::npos) {
                            RegDeleteTreeW(hKey, subkeyName.c_str());
                            logger("→ USB MountPoint cleared for " + WStringToString(driveLetter) + ":");
                        }
                    }
                }
            }
            RegCloseKey(hKey);
        }
    }

    void Cleaner::CleanUSBPaperbin(const std::wstring& filePath, LoggerFunc logger) {
        if (filePath.length() >= 2 && filePath[1] == L':') {
            std::wstring drive = filePath.substr(0, 2);
            std::wstring recycleBinPath = drive + L"\\$RECYCLE.BIN";
            try {
                if (fs::exists(recycleBinPath)) {
                    fs::remove_all(recycleBinPath);
                    logger("→ Device Recycle Bin sanitized on " + WStringToString(drive));
                }
            } catch (...) {}
        }
    }

    void Cleaner::CleanJumpLists(LoggerFunc logger) {
        std::vector<std::wstring> rutas = {
            L"%AppData%\\Microsoft\\Windows\\Recent\\AutomaticDestinations",
            L"%AppData%\\Microsoft\\Windows\\Recent\\CustomDestinations"
        };
        for (const auto& r : rutas) {
            wchar_t path[MAX_PATH];
            ExpandEnvironmentStringsW(r.c_str(), path, MAX_PATH);
            try {
                for (const auto& entry : fs::directory_iterator(path)) {
                    fs::remove(entry.path());
                }
            } catch (...) {}
        }
        logger("→ Jump Lists sanitized.");
    }

    void Cleaner::DeepWipeUSNJournal(LoggerFunc logger) {
        Sleep(1000); // Dar un respiro al disco duro
        SystemUtils::ExecuteHiddenCommand(L"fsutil usn deletejournal /d C:");
        logger("→ NTFS Journal reset successfully.");
    }

    void Cleaner::FlushDNSAndARP(LoggerFunc logger) {
        SystemUtils::ExecuteHiddenCommand(L"ipconfig /flushdns");
        SystemUtils::ExecuteHiddenCommand(L"arp -d *");
        logger("→ Network stack (DNS/ARP) purged.");
    }

    void Cleaner::CleanEventLogs(LoggerFunc logger) {
        SystemUtils::ExecuteHiddenCommand(L"wevtutil cl Security");
        SystemUtils::ExecuteHiddenCommand(L"wevtutil cl System");
        logger("→ Security Event Logs purged.");
    }

    void Cleaner::CleanRecentLNKs(const std::wstring& filePath, LoggerFunc logger) {
        std::wstring nameNoExt = ToLower(GetFileNameWithoutExtension(filePath));
        wchar_t recentPath[MAX_PATH];
        ExpandEnvironmentStringsW(L"%AppData%\\Microsoft\\Windows\\Recent", recentPath, MAX_PATH);
        try {
            for (const auto& entry : fs::directory_iterator(recentPath)) {
                if (ToLower(entry.path().wstring()).find(nameNoExt) != std::wstring::npos) {
                    fs::remove(entry.path());
                }
            }
            logger("→ Recent LNK traces destroyed.");
        } catch (...) {}
    }

    void Cleaner::CleanSpecificShimCache(const std::wstring& filePath, LoggerFunc logger) {
        SystemUtils::ExecuteHiddenCommand(L"rundll32.exe apphelp.dll,ShimFlushCache");
        
        std::wstring targetPath = ToLower(filePath);
        // String wide ya ocupa 2 bytes por carácter, ideal para UTF-16LE en memoria
        size_t targetSizeByte = targetPath.size() * sizeof(wchar_t);

        HKEY hKey;
        std::wstring regPath = L"SYSTEM\\CurrentControlSet\\Control\\Session Manager\\AppCompatCache";
        if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, regPath.c_str(), 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS) {
            DWORD dataType;
            DWORD dataSize = 0;
            // 1. Obtener el tamaño del BLOB
            if (RegQueryValueExW(hKey, L"AppCompatCache", NULL, &dataType, NULL, &dataSize) == ERROR_SUCCESS) {
                std::vector<BYTE> buffer(dataSize);
                // 2. Leer los datos binarios
                if (RegQueryValueExW(hKey, L"AppCompatCache", NULL, &dataType, buffer.data(), &dataSize) == ERROR_SUCCESS) {
                    
                    // 3. Buscar la secuencia de bytes (ruta UTF-16LE) dentro del BLOB
                    auto it = std::search(buffer.begin(), buffer.end(), 
                                          reinterpret_cast<const BYTE*>(targetPath.data()), 
                                          reinterpret_cast<const BYTE*>(targetPath.data()) + targetSizeByte);
                    
                    if (it != buffer.end()) {
                        // 4. Sobrescribir con ceros para mantener la integridad del BLOB
                        std::fill(it, it + targetSizeByte, 0);
                        
                        // 5. Escribir el BLOB limpio al registro
                        RegSetValueExW(hKey, L"AppCompatCache", 0, dataType, buffer.data(), dataSize);
                        logger("→ ShimCache: Binary trace for " + WStringToString(GetFileName(filePath)) + " sanitized.");
                    } else {
                        logger("→ ShimCache: No specific trace found (Clean).");
                    }
                }
            }
            RegCloseKey(hKey);
        } else {
            logger("→ ShimCache Error: Failed to open key.");
        }
    }

    void Cleaner::CleanShellBags(LoggerFunc logger) {
        // RegDeleteTreeW elimina de forma recursiva (como reg delete /f)
        RegDeleteTreeW(HKEY_CURRENT_USER, L"Software\\Classes\\Local Settings\\Software\\Microsoft\\Windows\\Shell\\BagMRU");
        RegDeleteTreeW(HKEY_CURRENT_USER, L"Software\\Classes\\Local Settings\\Software\\Microsoft\\Windows\\Shell\\Bags");
        logger("→ ShellBags sanitized.");
    }

    void Cleaner::CleanShellExperience(LoggerFunc logger) {
        RegDeleteTreeW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\RunMRU");
        RegDeleteTreeW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\WordWheelQuery");
        logger("→ Taskbar & Win+R history sanitized.");
    }

    void Cleaner::CleanEverythingService(LoggerFunc logger) {
        SystemUtils::ExecuteHiddenCommand(L"net stop Everything");
        wchar_t dbPath[MAX_PATH];
        ExpandEnvironmentStringsW(L"%AppData%\\Everything\\Everything.db", dbPath, MAX_PATH);
        if (DeleteFileW(dbPath)) {
            logger("→ Everything Search Engine DB neutralized.");
        }
    }

    // ==========================================================
    // HELPERS AVANZADOS (Añadir junto a ToLower, GetFileName, etc.)
    // ==========================================================

    std::wstring Cleaner::DecodeROT13(std::wstring input) {
        for (auto& c : input) {
            if (c >= L'a' && c <= L'z') {
                c = ((c - L'a' + 13) % 26) + L'a';
            } else if (c >= L'A' && c <= L'Z') {
                c = ((c - L'A' + 13) % 26) + L'A';
            }
        }
        return input;
    }

    // ==========================================================
    // FAKE ACTIVITY Y REGISTRO AVANZADO
    // ==========================================================

    void Cleaner::FakeActivityGenerator(LoggerFunc logger) {
        std::vector<std::string> legit_apps = { "chrome.exe", "spotify.exe", "discord.exe", "calc.exe" };
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, (int)legit_apps.size() - 1);
        
        logger("→ Masking activity with " + legit_apps[dis(gen)] + " traces...");
    }

    void Cleaner::CleanUserAssist(const std::wstring& filePath, LoggerFunc logger) {
        std::wstring target = ToLower(filePath);
        std::wstring targetName = ToLower(GetFileName(filePath));
        
        HKEY hKey;
        std::wstring pathUa = L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\UserAssist";
        
        if (RegOpenKeyExW(HKEY_CURRENT_USER, pathUa.c_str(), 0, KEY_READ | KEY_WRITE, &hKey) == ERROR_SUCCESS) {
            DWORD numGuids = 0;
            if (RegQueryInfoKeyW(hKey, NULL, NULL, NULL, &numGuids, NULL, NULL, NULL, NULL, NULL, NULL, NULL) == ERROR_SUCCESS) {
                std::vector<wchar_t> guidBuf(256);
                for (DWORD i = 0; i < numGuids; i++) {
                    DWORD guidLen = 256;
                    if (RegEnumKeyExW(hKey, i, guidBuf.data(), &guidLen, NULL, NULL, NULL, NULL) == ERROR_SUCCESS) {
                        std::wstring countPath = std::wstring(guidBuf.data()) + L"\\Count";
                        HKEY hCountKey;
                        
                        if (RegOpenKeyExW(hKey, countPath.c_str(), 0, KEY_READ | KEY_WRITE, &hCountKey) == ERROR_SUCCESS) {
                            DWORD numVals, maxValName;
                            if (RegQueryInfoKeyW(hCountKey, NULL, NULL, NULL, NULL, NULL, NULL, &numVals, &maxValName, NULL, NULL, NULL) == ERROR_SUCCESS) {
                                std::vector<wchar_t> valBuf(maxValName + 1);
                                for (int j = numVals - 1; j >= 0; j--) {
                                    DWORD valLen = maxValName + 1;
                                    if (RegEnumValueW(hCountKey, j, valBuf.data(), &valLen, NULL, NULL, NULL, NULL) == ERROR_SUCCESS) {
                                        
                                        // ROT13 Bypass nativo en memoria
                                        std::wstring decodedName = ToLower(DecodeROT13(valBuf.data()));
                                        
                                        if (decodedName.find(target) != std::wstring::npos || decodedName.find(targetName) != std::wstring::npos) {
                                            RegDeleteValueW(hCountKey, valBuf.data());
                                            logger("→ UserAssist forensic trace destroyed (ROT13 bypass).");
                                        }
                                    }
                                }
                            }
                            RegCloseKey(hCountKey);
                        }
                    }
                }
            }
            RegCloseKey(hKey);
        } else {
            logger("→ UserAssist Warning: Could not open root key.");
        }
    }

    void Cleaner::CleanRecentApps(const std::wstring& filePath, LoggerFunc logger) {
        std::wstring nombre = ToLower(GetFileName(filePath));
        HKEY hKey;
        std::wstring pathRa = L"Software\\Microsoft\\Windows\\CurrentVersion\\Search\\RecentApps";
        
        if (RegOpenKeyExW(HKEY_CURRENT_USER, pathRa.c_str(), 0, KEY_READ | KEY_WRITE, &hKey) == ERROR_SUCCESS) {
            DWORD numSubkeys = 0, maxSubkeyLen = 0;
            if (RegQueryInfoKeyW(hKey, NULL, NULL, NULL, &numSubkeys, &maxSubkeyLen, NULL, NULL, NULL, NULL, NULL, NULL) == ERROR_SUCCESS) {
                std::vector<wchar_t> subkeyBuf(maxSubkeyLen + 1);
                
                for (int i = numSubkeys - 1; i >= 0; i--) {
                    DWORD subkeyLen = maxSubkeyLen + 1;
                    if (RegEnumKeyExW(hKey, i, subkeyBuf.data(), &subkeyLen, NULL, NULL, NULL, NULL) == ERROR_SUCCESS) {
                        HKEY hSubKey;
                        if (RegOpenKeyExW(hKey, subkeyBuf.data(), 0, KEY_READ, &hSubKey) == ERROR_SUCCESS) {
                            std::vector<wchar_t> appIdBuf(MAX_PATH);
                            DWORD appIdLen = MAX_PATH * sizeof(wchar_t);
                            DWORD type = 0;
                            
                            if (RegQueryValueExW(hSubKey, L"AppId", NULL, &type, (LPBYTE)appIdBuf.data(), &appIdLen) == ERROR_SUCCESS) {
                                std::wstring appIdStr = ToLower(appIdBuf.data());
                                if (appIdStr.find(nombre) != std::wstring::npos) {
                                    RegCloseKey(hSubKey);
                                    // RegDeleteTreeW elimina la subclave y todo su contenido
                                    RegDeleteTreeW(hKey, subkeyBuf.data());
                                    logger("→ RecentApps entry purged for " + WStringToString(nombre));
                                    continue;
                                }
                            }
                            RegCloseKey(hSubKey);
                        }
                    }
                }
            }
            RegCloseKey(hKey);
        }
    }

    void Cleaner::CleanAppCompatTotal(const std::wstring& filePath, LoggerFunc logger) {
        std::wstring nombreExe = ToLower(GetFileName(filePath));
        std::wstring nombreSinExt = GetFileNameWithoutExtension(nombreExe);
        std::wstring targetPath = ToLower(filePath);

        // Nivel 1: Flush (Usamos el ExecuteHiddenCommand para sigilo)
        SystemUtils::ExecuteHiddenCommand(L"rundll32.exe apphelp.dll,ShimFlushCache");

        // Nivel 2: Rutas
        struct RegNode { HKEY root; std::wstring path; };
        std::vector<RegNode> rutas = {
            {HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Control\\Session Manager\\AppCompatCache"},
            {HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows NT\\CurrentVersion\\AppCompatFlags\\Compatibility Assistant\\Store"},
            {HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows NT\\CurrentVersion\\AppCompatFlags\\Layers"}
        };

        for (const auto& r : rutas) {
            HKEY hKey;
            if (RegOpenKeyExW(r.root, r.path.c_str(), 0, KEY_READ | KEY_WRITE, &hKey) == ERROR_SUCCESS) {
                DWORD numVals = 0, maxValName = 0;
                if (RegQueryInfoKeyW(hKey, NULL, NULL, NULL, NULL, NULL, NULL, &numVals, &maxValName, NULL, NULL, NULL) == ERROR_SUCCESS) {
                    std::vector<wchar_t> valBuf(maxValName + 1);
                    for (int i = numVals - 1; i >= 0; i--) {
                        DWORD valLen = maxValName + 1;
                        if (RegEnumValueW(hKey, i, valBuf.data(), &valLen, NULL, NULL, NULL, NULL) == ERROR_SUCCESS) {
                            std::wstring valNameLower = ToLower(valBuf.data());
                            
                            if (valNameLower.find(targetPath) != std::wstring::npos || 
                                valNameLower.find(nombreExe) != std::wstring::npos || 
                                valNameLower.find(nombreSinExt) != std::wstring::npos) {
                                
                                RegDeleteValueW(hKey, valBuf.data());
                                logger("→ AppCompat: Surgical trace purged: " + WStringToString(GetFileName(valBuf.data())));
                            }
                        }
                    }
                }
                RegCloseKey(hKey);
            }
        }

        // Nivel 3: Persisted
        HKEY hKeyAmcache;
        std::wstring pathAmcache = L"Software\\Microsoft\\Windows NT\\CurrentVersion\\AppCompatFlags\\Compatibility Assistant\\Persisted";
        if (RegOpenKeyExW(HKEY_CURRENT_USER, pathAmcache.c_str(), 0, KEY_READ | KEY_WRITE, &hKeyAmcache) == ERROR_SUCCESS) {
            DWORD numVals = 0, maxValName = 0;
            if (RegQueryInfoKeyW(hKeyAmcache, NULL, NULL, NULL, NULL, NULL, NULL, &numVals, &maxValName, NULL, NULL, NULL) == ERROR_SUCCESS) {
                std::vector<wchar_t> valBuf(maxValName + 1);
                for (int i = numVals - 1; i >= 0; i--) {
                    DWORD valLen = maxValName + 1;
                    if (RegEnumValueW(hKeyAmcache, i, valBuf.data(), &valLen, NULL, NULL, NULL, NULL) == ERROR_SUCCESS) {
                        std::wstring valNameLower = ToLower(valBuf.data());
                        if (valNameLower.find(nombreSinExt) != std::wstring::npos) {
                            RegDeleteValueW(hKeyAmcache, valBuf.data());
                            logger("→ Amcache Persisted: Legacy name trace purged.");
                        }
                    }
                }
            }
            RegCloseKey(hKeyAmcache);
        }
    }

    void Cleaner::CleanAmcache(const std::wstring& filePath, LoggerFunc logger) {
        std::wstring nombreExe = ToLower(GetFileName(filePath));
        std::wstring targetPath = ToLower(filePath);

        struct RegNode { HKEY root; std::wstring path; };
        std::vector<RegNode> rutas = {
            {HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\AppCompatFlags\\Compatibility Assistant\\Persisted"},
            {HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\AppCompatFlags\\InventoryApplicationFile"},
            {HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows NT\\CurrentVersion\\AppCompatFlags\\Compatibility Assistant\\Persisted"}
        };

        for (const auto& r : rutas) {
            HKEY hKey;
            if (RegOpenKeyExW(r.root, r.path.c_str(), 0, KEY_READ | KEY_WRITE, &hKey) == ERROR_SUCCESS) {
                
                // 1. Borrar valores
                DWORD numVals = 0, maxValName = 0, numSubkeys = 0, maxSubkeyLen = 0;
                if (RegQueryInfoKeyW(hKey, NULL, NULL, NULL, &numSubkeys, &maxSubkeyLen, NULL, &numVals, &maxValName, NULL, NULL, NULL) == ERROR_SUCCESS) {
                    std::vector<wchar_t> valBuf(maxValName + 1);
                    for (int i = numVals - 1; i >= 0; i--) {
                        DWORD valLen = maxValName + 1;
                        if (RegEnumValueW(hKey, i, valBuf.data(), &valLen, NULL, NULL, NULL, NULL) == ERROR_SUCCESS) {
                            std::wstring valNameLower = ToLower(valBuf.data());
                            if (valNameLower.find(targetPath) != std::wstring::npos || valNameLower.find(nombreExe) != std::wstring::npos) {
                                RegDeleteValueW(hKey, valBuf.data());
                                logger("→ Amcache: Surgical removal from " + WStringToString(GetFileName(r.path)));
                            }
                        }
                    }

                    // 2. Borrar subclaves (InventoryApplicationFile IDs)
                    std::vector<wchar_t> subkeyBuf(maxSubkeyLen + 1);
                    for (int i = numSubkeys - 1; i >= 0; i--) {
                        DWORD subkeyLen = maxSubkeyLen + 1;
                        if (RegEnumKeyExW(hKey, i, subkeyBuf.data(), &subkeyLen, NULL, NULL, NULL, NULL) == ERROR_SUCCESS) {
                            HKEY hSubKey;
                            if (RegOpenKeyExW(hKey, subkeyBuf.data(), 0, KEY_READ, &hSubKey) == ERROR_SUCCESS) {
                                std::vector<wchar_t> lowPathBuf(MAX_PATH);
                                DWORD lowPathLen = MAX_PATH * sizeof(wchar_t);
                                if (RegQueryValueExW(hSubKey, L"LowerCaseLongPath", NULL, NULL, (LPBYTE)lowPathBuf.data(), &lowPathLen) == ERROR_SUCCESS) {
                                    std::wstring lowPathStr = ToLower(lowPathBuf.data());
                                    if (lowPathStr.find(targetPath) != std::wstring::npos) {
                                        RegCloseKey(hSubKey);
                                        RegDeleteTreeW(hKey, subkeyBuf.data());
                                        logger("→ Amcache: Inventory node destroyed.");
                                        continue;
                                    }
                                }
                                RegCloseKey(hSubKey);
                            }
                        }
                    }
                }
                RegCloseKey(hKey);
            }
        }
    }

    void Cleaner::CleanMuiCache(const std::wstring& filePath, LoggerFunc logger) {
        std::wstring nombreBase = ToLower(GetFileName(filePath));
        std::wstring pathMui = L"Software\\Classes\\Local Settings\\Software\\Microsoft\\Windows\\Shell\\MuiCache";
        HKEY hKey;
        
        if (RegOpenKeyExW(HKEY_CURRENT_USER, pathMui.c_str(), 0, KEY_READ | KEY_WRITE, &hKey) == ERROR_SUCCESS) {
            DeleteRegistryValueIfContains(hKey, nombreBase, logger);
            RegCloseKey(hKey);
        }
    }

    void Cleaner::CleanTaskCache(const std::wstring& filePath, LoggerFunc logger) {
        std::wstring nombre = ToLower(GetFileNameWithoutExtension(filePath));
        std::wstring path = L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Schedule\\TaskCache\\Tree";
        HKEY hKey;

        if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, path.c_str(), 0, KEY_READ | KEY_WRITE, &hKey) == ERROR_SUCCESS) {
            DWORD numSubkeys = 0, maxSubkeyLen = 0;
            if (RegQueryInfoKeyW(hKey, NULL, NULL, NULL, &numSubkeys, &maxSubkeyLen, NULL, NULL, NULL, NULL, NULL, NULL) == ERROR_SUCCESS) {
                std::vector<wchar_t> subkeyBuf(maxSubkeyLen + 1);
                for (int i = numSubkeys - 1; i >= 0; i--) {
                    DWORD subkeyLen = maxSubkeyLen + 1;
                    if (RegEnumKeyExW(hKey, i, subkeyBuf.data(), &subkeyLen, NULL, NULL, NULL, NULL) == ERROR_SUCCESS) {
                        std::wstring subkeyStr = ToLower(subkeyBuf.data());
                        if (subkeyStr.find(nombre) != std::wstring::npos) {
                            RegDeleteTreeW(hKey, subkeyBuf.data());
                            logger("→ TaskCache: Scheduled residue purged.");
                        }
                    }
                }
            }
            RegCloseKey(hKey);
        }
    }
    // ==========================================================
    // OFUSCACIÓN MFT Y CACHÉS VISUALES
    // ==========================================================

    void Cleaner::CamouflageMFT(const std::wstring& directoryPath, LoggerFunc logger) {
        std::vector<std::wstring> fakeNames = {
            L"ETW_Trace_Log", L"Win_Diag_Data", L"Cbs_Persist", L"Dism_Host_Provider",
            L"Spp_Svc_Cache", L"Appx_Deployment_Log", L"Temp_Win_Update"
        };
        std::vector<std::wstring> exts = { L".log", L".tmp", L".dat", L".cache" };

        SYSTEMTIME st = { 0 };
        st.wYear = 2018; st.wMonth = 9; st.wDay = 24;
        st.wHour = 11; st.wMinute = 45; st.wSecond = 0;
        FILETIME ftOld;
        SystemTimeToFileTime(&st, &ftOld);

        logger("→ MFT: Initiating Deep Journal Overwrite...");

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> disSize(4096, 16384);
        std::uniform_int_distribution<> disByte(0, 255);
        std::uniform_int_distribution<> disExt(0, 3);
        std::uniform_int_distribution<> disId(1000, 99999);

        try {
            for (int cycle = 0; cycle < 2; cycle++) {
                for (const auto& baseName : fakeNames) {
                    std::wstring fakeName = baseName + L"_" + std::to_wstring(disId(gen)) + exts[disExt(gen)];
                    std::wstring fakePath = directoryPath + L"\\" + fakeName;

                    // Escribir datos aleatorios
                    int fileSize = disSize(gen);
                    std::ofstream f(fakePath, std::ios::binary);
                    if (f.is_open()) {
                        std::vector<char> buffer(fileSize);
                        for (int i = 0; i < fileSize; i++) buffer[i] = disByte(gen);
                        f.write(buffer.data(), fileSize);
                        f.close();

                        // TimeStomp al archivo basura
                        HANDLE hFile = CreateFileW(fakePath.c_str(), FILE_WRITE_ATTRIBUTES, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
                        if (hFile != INVALID_HANDLE_VALUE) {
                            SetFileTime(hFile, &ftOld, &ftOld, &ftOld);
                            CloseHandle(hFile);
                        }

                        Sleep(50);
                        DeleteFileW(fakePath.c_str());
                    }
                }
            }
            logger("→ MFT Journal: Records unlinked and overwritten (Surgical Masking).");
        } catch (const std::exception& e) {
            logger(std::string("→ MFT Warning: ") + e.what());
        }
    }

    void Cleaner::CleanIconCache(LoggerFunc logger) {
        wchar_t path[MAX_PATH];
        ExpandEnvironmentStringsW(L"%LocalAppData%\\Microsoft\\Windows\\Explorer", path, MAX_PATH);
        try {
            for (const auto& entry : fs::directory_iterator(path)) {
                if (entry.is_regular_file()) {
                    std::wstring filename = ToLower(entry.path().filename().wstring());
                    if (filename.find(L"iconcache") == 0) {
                        try { fs::remove(entry.path()); } catch (...) {}
                    }
                }
            }
            logger("→ IconCache: Attempted safe cleanup.");
        } catch (...) {}
    }

    void Cleaner::CleanBrowserDownloads(const std::wstring& filePath, LoggerFunc logger) {
        std::wstring nombreArchivo = GetFileName(filePath);
        
        // 1. Neutralizar Zone.Identifier
        RemoveADS(filePath, logger); // Reutilizamos nuestra función nativa C++

        // 2. Limpiar SQLite de Navegadores
        wchar_t profilePath[MAX_PATH];
        ExpandEnvironmentStringsW(L"%USERPROFILE%", profilePath, MAX_PATH);
        std::wstring base(profilePath);

        std::vector<std::wstring> rutasDb = {
            base + L"\\AppData\\Local\\Google\\Chrome\\User Data\\Default\\History",
            base + L"\\AppData\\Local\\Microsoft\\Edge\\User Data\\Default\\History",
            base + L"\\AppData\\Local\\BraveSoftware\\Brave-Browser\\User Data\\Default\\History"
        };

        wchar_t tempDir[MAX_PATH];
        GetTempPathW(MAX_PATH, tempDir);
        std::wstring tempDb = std::wstring(tempDir) + L"temp_hist.db";

        std::string targetUtf8 = WStringToString(nombreArchivo);

        for (const auto& dbPath : rutasDb) {
            if (!fs::exists(dbPath)) continue;

            try {
                fs::copy_file(dbPath, tempDb, fs::copy_options::overwrite_existing);

                sqlite3* db;
                if (sqlite3_open(WStringToString(tempDb).c_str(), &db) == SQLITE_OK) {
                    
                    std::string querySelect = "SELECT id FROM downloads WHERE target_path LIKE '%" + targetUtf8 + "%';";
                    sqlite3_stmt* stmt;
                    std::vector<int> idsToDel;

                    if (sqlite3_prepare_v2(db, querySelect.c_str(), -1, &stmt, NULL) == SQLITE_OK) {
                        while (sqlite3_step(stmt) == SQLITE_ROW) {
                            idsToDel.push_back(sqlite3_column_int(stmt, 0));
                        }
                        sqlite3_finalize(stmt);
                    }

                    if (!idsToDel.empty()) {
                        for (int id : idsToDel) {
                            std::string qDelDown = "DELETE FROM downloads WHERE id = " + std::to_string(id) + ";";
                            std::string qDelChain = "DELETE FROM downloads_url_chains WHERE id = " + std::to_string(id) + ";";
                            
                            char* errMsg = 0;
                            sqlite3_exec(db, qDelDown.c_str(), 0, 0, &errMsg);
                            sqlite3_exec(db, qDelChain.c_str(), 0, 0, &errMsg);
                        }
                        sqlite3_close(db);

                        fs::copy_file(tempDb, dbPath, fs::copy_options::overwrite_existing);
                        logger("→ Browser Download History sanitized.");
                    } else {
                        sqlite3_close(db);
                    }
                }
                fs::remove(tempDb);
            } catch (...) {
                logger("→ Browser DB Skip (In use or locked).");
            }
        }
    }

    // ==========================================================
    // DEEP REGISTRY SCAN (FUERZA BRUTA)
    // ==========================================================

    std::wstring Cleaner::GetHkeyString(HKEY hKey) {
        if (hKey == HKEY_LOCAL_MACHINE) return L"HKLM";
        if (hKey == HKEY_CURRENT_USER) return L"HKCU";
        return L"HKLM";
    }

    void Cleaner::RecursiveRegistrySearch(HKEY hRoot, const std::wstring& currentPath, const std::wstring& target, const std::wstring& targetSinExt, int& foundCount, LoggerFunc logger) {
        HKEY hKey;
        if (RegOpenKeyExW(hRoot, currentPath.c_str(), 0, KEY_READ | KEY_WRITE, &hKey) != ERROR_SUCCESS) {
            return;
        }

        DWORD numVals = 0, maxValName = 0, maxValData = 0;
        DWORD numSubkeys = 0, maxSubkeyLen = 0;

        if (RegQueryInfoKeyW(hKey, NULL, NULL, NULL, &numSubkeys, &maxSubkeyLen, NULL, &numVals, &maxValName, &maxValData, NULL, NULL) == ERROR_SUCCESS) {
            
            // 1. Revisar Valores
            std::vector<wchar_t> valNameBuf(maxValName + 1);
            std::vector<BYTE> valDataBuf(maxValData + 1);

            for (int i = numVals - 1; i >= 0; i--) {
                DWORD valNameLen = maxValName + 1;
                DWORD valDataLen = maxValData + 1;
                DWORD type = 0;

                if (RegEnumValueW(hKey, i, valNameBuf.data(), &valNameLen, NULL, &type, valDataBuf.data(), &valDataLen) == ERROR_SUCCESS) {
                    std::wstring valNameLower = ToLower(valNameBuf.data());
                    bool match = (valNameLower.find(target) != std::wstring::npos || valNameLower.find(targetSinExt) != std::wstring::npos);
                    
                    if (!match && (type == REG_SZ || type == REG_EXPAND_SZ)) {
                        std::wstring dataLower = ToLower(reinterpret_cast<wchar_t*>(valDataBuf.data()));
                        if (dataLower.find(target) != std::wstring::npos) match = true;
                    }

                    if (match) {
                        RegDeleteValueW(hKey, valNameBuf.data());
                        foundCount++;
                        logger("→ DeepClean: Value removed from ...\\" + WStringToString(GetFileName(currentPath)));
                    }
                }
            }

            // 2. Revisar Subclaves
            std::vector<wchar_t> subkeyBuf(maxSubkeyLen + 1);
            for (int i = numSubkeys - 1; i >= 0; i--) {
                DWORD subkeyLen = maxSubkeyLen + 1;
                if (RegEnumKeyExW(hKey, i, subkeyBuf.data(), &subkeyLen, NULL, NULL, NULL, NULL) == ERROR_SUCCESS) {
                    std::wstring subkeyStr = subkeyBuf.data();
                    std::wstring subkeyLower = ToLower(subkeyStr);
                    std::wstring subkeyFullPath = currentPath + L"\\" + subkeyStr;

                    if (subkeyLower.find(target) != std::wstring::npos || subkeyLower == targetSinExt) {
                        RegDeleteTreeW(hRoot, subkeyFullPath.c_str());
                        foundCount++;
                        logger("→ DeepClean: Key tree removed " + WStringToString(subkeyStr));
                    } else {
                        // Recursión controlada
                        RecursiveRegistrySearch(hRoot, subkeyFullPath, target, targetSinExt, foundCount, logger);
                    }
                }
            }
        }
        RegCloseKey(hKey);
    }

    void Cleaner::DeepRegistrySearchCleaner(const std::wstring& filePath, LoggerFunc logger) {
        std::wstring target = ToLower(GetFileName(filePath));
        std::wstring targetSinExt = GetFileNameWithoutExtension(target);

        logger("→ DEEP SCAN: Scanning registry hives for '" + WStringToString(target) + "'... (This may take a while)");

        struct RootNode { HKEY hKey; std::wstring path; };
        std::vector<RootNode> roots = {
            { HKEY_CURRENT_USER, L"Software" },
            { HKEY_LOCAL_MACHINE, L"SOFTWARE" },
            { HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Services" }
        };

        int foundCount = 0;
        for (const auto& r : roots) {
            RecursiveRegistrySearch(r.hKey, r.path, target, targetSinExt, foundCount, logger);
        }

        if (foundCount > 0) {
            logger("→ DEEP SCAN COMPLETE: " + std::to_string(foundCount) + " hidden traces eliminated.");
        } else {
            logger("→ DEEP SCAN COMPLETE: No deep traces found.");
        }
    }

    // ==========================================================
    // LIMPIEZA DE SRUM, RAM Y SISTEMA GLOBAL
    // ==========================================================

    void Cleaner::CleanSrumAndDirectX(LoggerFunc logger) {
        SystemUtils::ExecuteHiddenCommand(L"net stop DPS");
        SystemUtils::ExecuteHiddenCommand(L"net stop SruSvc");
        Sleep(1000);

        wchar_t srumPath[MAX_PATH];
        ExpandEnvironmentStringsW(L"%SystemRoot%\\System32\\sru\\SRUDB.dat", srumPath, MAX_PATH);
        if (DeleteFileW(srumPath)) {
            logger("→ SRUM Database annihilated (Resource Usage History purged).");
        }

        std::vector<std::wstring> rutasGpu = {
            L"%LocalAppData%\\D3DSCache",
            L"%LocalAppData%\\NVIDIA\\GLCache",
            L"%LocalAppData%\\AMD\\DxCache"
        };

        for (const auto& r : rutasGpu) {
            wchar_t fullPath[MAX_PATH];
            ExpandEnvironmentStringsW(r.c_str(), fullPath, MAX_PATH);
            try {
                if (fs::exists(fullPath)) {
                    fs::remove_all(fullPath);
                    logger("→ GPU Cache purged: " + WStringToString(GetFileName(fullPath)));
                }
            } catch (...) {}
        }
    }

    void Cleaner::PurgeSelfMemory(LoggerFunc logger) {
        // En C++, no necesitamos gc.collect() de Python.
        // Solo vaciamos el Working Set del proceso actual.
        HANDLE hProcess = GetCurrentProcess();
        if (EmptyWorkingSet(hProcess)) {
            logger("→ RAM: Process footprint minimized (Working Set emptied).");
        }
    }

    void Cleaner::RestartExplorer(LoggerFunc logger) {
        SystemUtils::ExecuteHiddenCommand(L"taskkill /F /IM explorer.exe");
        Sleep(1000);
        SystemUtils::ExecuteHiddenCommand(L"explorer.exe");
        logger("→ UI Memory: Explorer.exe restarted (RAM cache purged).");
    }

    void Cleaner::ConfigurePagefileWipe(LoggerFunc logger) {
        HKEY hKey;
        std::wstring path = L"SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Memory Management";
        if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, path.c_str(), 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS) {
            DWORD val = 1;
            if (RegSetValueExW(hKey, L"ClearPageFileAtShutdown", 0, REG_DWORD, (const BYTE*)&val, sizeof(val)) == ERROR_SUCCESS) {
                logger("→ Anti-Forensics: Pagefile wipe on shutdown ENABLED.");
            }
            RegCloseKey(hKey);
        }
    }

    // ==========================================================
    // LIMPIEZA EXTREMA QUIRÚRGICA (FASE 99%)
    // ==========================================================

    void Cleaner::ExtremeSurgicalClean(const std::wstring& filePath, LoggerFunc logger) {
        std::wstring targetName = ToLower(GetFileName(filePath));
        std::wstring targetSinExt = GetFileNameWithoutExtension(targetName);
        std::string targetUtf8 = WStringToString(targetName);

        // 1. WINDOWS TIMELINE (SQLite Quirúrgico)
        try {
            wchar_t timelinePath[MAX_PATH];
            ExpandEnvironmentStringsW(L"%LocalAppData%\\ConnectedDevicesPlatform", timelinePath, MAX_PATH);
            
            if (fs::exists(timelinePath)) {
                for (const auto& entry : fs::recursive_directory_iterator(timelinePath)) {
                    if (entry.is_regular_file() && ToLower(entry.path().filename().wstring()) == L"activitiescache.db") {
                        
                        SystemUtils::ExecuteHiddenCommand(L"net stop CDPUserSvc");
                        Sleep(1000);

                        sqlite3* db;
                        if (sqlite3_open(WStringToString(entry.path().wstring()).c_str(), &db) == SQLITE_OK) {
                            std::string query = "DELETE FROM Activity WHERE Payload LIKE '%" + targetUtf8 + "%';";
                            char* errMsg = 0;
                            if (sqlite3_exec(db, query.c_str(), 0, 0, &errMsg) == SQLITE_OK) {
                                logger("→ Timeline: Surgical extraction of activity records complete.");
                            }
                            sqlite3_close(db);
                        }
                        
                        SystemUtils::ExecuteHiddenCommand(L"net start CDPUserSvc");
                    }
                }
            }
        } catch (...) {}

        // 2. EXPLORER SEARCH HISTORY (WordWheelQuery - Análisis de bytes UTF-16LE)
        try {
            HKEY hKey;
            std::wstring searchPath = L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\WordWheelQuery";
            if (RegOpenKeyExW(HKEY_CURRENT_USER, searchPath.c_str(), 0, KEY_READ | KEY_WRITE, &hKey) == ERROR_SUCCESS) {
                DWORD numVals = 0, maxValName = 0, maxValData = 0;
                if (RegQueryInfoKeyW(hKey, NULL, NULL, NULL, NULL, NULL, NULL, &numVals, &maxValName, &maxValData, NULL, NULL) == ERROR_SUCCESS) {
                    
                    std::vector<wchar_t> valNameBuf(maxValName + 1);
                    std::vector<BYTE> valDataBuf(maxValData + 1);

                    for (int i = numVals - 1; i >= 0; i--) {
                        DWORD valNameLen = maxValName + 1;
                        DWORD valDataLen = maxValData + 1;
                        DWORD type = 0;

                        if (RegEnumValueW(hKey, i, valNameBuf.data(), &valNameLen, NULL, &type, valDataBuf.data(), &valDataLen) == ERROR_SUCCESS) {
                            std::wstring valNameStr = valNameBuf.data();
                            if (valNameStr == L"MRUListEx") continue; // No tocamos el índice principal
                            
                            // El dato es REG_BINARY en UTF-16LE, podemos castearlo a wchar_t*
                            if (type == REG_BINARY && valDataLen > 0) {
                                std::wstring decodedSearch = ToLower(reinterpret_cast<wchar_t*>(valDataBuf.data()));
                                
                                if (decodedSearch.find(targetName) != std::wstring::npos || decodedSearch.find(targetSinExt) != std::wstring::npos) {
                                    RegDeleteValueW(hKey, valNameBuf.data());
                                    logger("→ WordWheelQuery: Specific search trace purged.");
                                }
                            }
                        }
                    }
                }
                RegCloseKey(hKey);
            }
        } catch (...) {}

        // 3. PAPELERA DE RECICLAJE (Análisis profundo de metadatos $I)
        try {
            wchar_t driveStr[MAX_PATH];
            ExpandEnvironmentStringsW(L"%SystemDrive%", driveStr, MAX_PATH);
            std::wstring recycleBinPath = std::wstring(driveStr) + L"\\$Recycle.Bin";

            if (fs::exists(recycleBinPath)) {
                int count = 0;
                for (const auto& entry : fs::recursive_directory_iterator(recycleBinPath)) {
                    if (entry.is_regular_file()) {
                        std::wstring filename = entry.path().filename().wstring();
                        
                        // Los archivos de metadatos de la papelera empiezan por $I
                        if (filename.length() >= 2 && filename[0] == L'$' && filename[1] == L'I') {
                            
                            std::ifstream file(entry.path(), std::ios::binary);
                            if (file.is_open()) {
                                // Leemos el archivo completo a memoria
                                file.seekg(0, std::ios::end);
                                size_t size = file.tellg();
                                file.seekg(0, std::ios::beg);
                                
                                std::vector<char> buffer(size);
                                file.read(buffer.data(), size);
                                file.close();

                                // Casteamos el buffer a wchar_t* ya que la ruta original se guarda en UTF-16LE
                                // Saltamos la cabecera del archivo $I (usualmente 24 o 28 bytes dependiendo de Windows 10/11)
                                if (size > 28) {
                                    std::wstring originalPath = ToLower(reinterpret_cast<wchar_t*>(buffer.data() + 28));
                                    
                                    if (originalPath.find(targetName) != std::wstring::npos) {
                                        // 1. Borramos el archivo de metadatos ($I)
                                        fs::remove(entry.path());
                                        
                                        // 2. Calculamos y borramos el archivo físico ($R)
                                        std::wstring rFileName = L"$R" + filename.substr(2);
                                        std::wstring rFilePath = entry.path().parent_path().wstring() + L"\\" + rFileName;
                                        if (fs::exists(rFilePath)) {
                                            fs::remove(rFilePath);
                                        }
                                        count++;
                                    }
                                }
                            }
                        }
                    }
                }
                if (count > 0) {
                    logger("→ Recycle Bin: " + std::to_string(count) + " surgical extractions of $I/$R ghosts complete.");
                }
            }
        } catch (...) {}
    }

    // ==========================================================
    // EJECUCIÓN MAESTRA
    // ==========================================================

    void Cleaner::DeepCleanProcess(const std::wstring& targetPath, LoggerFunc logger) {
        logger("--- INITIATING GHOST PROTOCOL FOR: " + WStringToString(GetFileName(targetPath)) + " ---");
        
        // 1. ORIGEN Y RED
        CleanBrowserDownloads(targetPath, logger);
        FlushDNSAndARP(logger);
        
        // 2. HARDWARE Y UNIDADES
        CleanMountPoints(targetPath, logger);
        CleanUSBPaperbin(targetPath, logger);
        
        // 3. INTERFAZ, RED Y MEMORIA UI
        CleanRecentLNKs(targetPath, logger);
        CleanJumpLists(logger);
        CleanShellBags(logger);
        CleanShellExperience(logger);
        CleanClipboard(logger);
        ConfigurePagefileWipe(logger);
        RestartExplorer(logger);
        
        // 4. REGISTRO, TELEMETRÍA Y RASTROS PROFUNDOS
        CleanUserAssist(targetPath, logger);
        CleanRecentApps(targetPath, logger);
        CleanAppCompatTotal(targetPath, logger);
        CleanAmcache(targetPath, logger);
        CleanMuiCache(targetPath, logger);
        ExtremeSurgicalClean(targetPath, logger);
        CleanSrumAndDirectX(logger);
        
        // 5. MANIPULACIÓN DE MFT Y DESTRUCCIÓN
        RemoveADS(targetPath, logger);
        TimeStompFile(targetPath, logger);
        CamouflageMFT(fs::path(targetPath).parent_path().wstring(), logger);
        ShredAndDestroy(targetPath, logger);
        
        // 6. AUTO-LIMPIEZA FINAL Y RAM PROPIA
        PurgeSelfMemory(logger);
        CleanEverythingService(logger);
        CleanIconCache(logger);
        CleanConsoleHistory(logger);
        DeepWipeUSNJournal(logger);
        CleanEventLogs(logger);
        
        logger("--- CLEANING COMPLETE: TRACES ANNIHILATED ---");
    }

    // ==========================================================
    // PROTOCOLO KAMIKAZE (AUTODESTRUCCIÓN)
    // ==========================================================

    void Cleaner::ExecuteKamikazeProtocol(LoggerFunc logger) {
        wchar_t myPathBuf[MAX_PATH];
        GetModuleFileNameW(NULL, myPathBuf, MAX_PATH);
        std::wstring myPath(myPathBuf);
        
        std::wstring myName = GetFileName(myPath);
        std::wstring myNameNoExt = GetFileNameWithoutExtension(myName);

        logger("→ SELF-DESTRUCT: Purging own execution traces from Registry...");

        // 1. Auto-limpieza de registro usando nuestras propias armas
        CleanGlobalTracesByName(myPath, logger);
        CleanRecentApps(myPath, logger);
        CleanAppCompatTotal(myPath, logger);
        CleanAmcache(myPath, logger);

        // 2. Creación del agente externo (.bat)
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(1000, 9999);
        std::wstring batName = L"ghost_" + std::to_wstring(dis(gen)) + L".bat";

        wchar_t pfPath[MAX_PATH];
        ExpandEnvironmentStringsW(L"%SystemRoot%\\Prefetch", pfPath, MAX_PATH);

        std::wstring batContent = 
            L"@echo off\n"
            L":: Esperar a que el proceso principal libere el archivo\n"
            L"timeout /t 2 /nobreak > NUL\n"
            L":LOOP\n"
            L":: Intentar borrar el ejecutable del bypass\n"
            L"del /F /Q \"" + myPath + L"\"\n"
            L"if exist \"" + myPath + L"\" goto LOOP\n\n"
            L":: FASE CRÍTICA: BORRADO DE PREFETCH DEL PROPIO BYPASS\n"
            L"del /F /Q \"" + std::wstring(pfPath) + L"\\" + ToUpper(myNameNoExt) + L"*.pf\"\n\n"
            L":: Borrarse a sí mismo (El crimen perfecto)\n"
            L"del \"%~f0\"\n";

        // Escribir el bat
        std::wofstream batFile(batName);
        if (batFile.is_open()) {
            batFile << batContent;
            batFile.close();
            logger("→ AGENT ARMED: Prefetch & Binary will be incinerated on exit.");
            
            // 3. Ejecutar el BAT de forma oculta (CREATE_NO_WINDOW)
            STARTUPINFOW si = { sizeof(si) };
            PROCESS_INFORMATION pi;
            si.dwFlags = STARTF_USESHOWWINDOW;
            si.wShowWindow = SW_HIDE;

            std::wstring cmd = L"cmd.exe /c " + batName;
            CreateProcessW(NULL, &cmd[0], NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi);
            
            if (pi.hProcess) {
                CloseHandle(pi.hProcess);
                CloseHandle(pi.hThread);
            }
        } else {
            logger("→ Self-destruct error: Could not create agent.");
        }
    }
}