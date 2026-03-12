#pragma once
#include <windows.h>
#include <stdint.h> // <--- ESTO SOLUCIONA LOS ERRORES uint32_t

class APIResolver {
private:
    // Hash de Jenkins one-at-a-time corregido
    static uint32_t hash_string(const char* str) {
        uint32_t hash = 0;
        while (*str) {
            hash += (uint8_t)*str++;
            hash += (hash << 10);
            hash ^= (hash >> 6);
        }
        hash += (hash << 3);
        hash ^= (hash >> 11);
        hash += (hash << 15);
        return hash;
    }

    static FARPROC get_proc_by_hash(HMODULE hModule, uint32_t targetHash) {
        if (!hModule) return nullptr;

        PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)hModule;
        PIMAGE_NT_HEADERS ntHeaders = (PIMAGE_NT_HEADERS)((BYTE*)hModule + dosHeader->e_lfanew);
        
        DWORD exportAddr = ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
        if (exportAddr == 0) return nullptr;

        PIMAGE_EXPORT_DIRECTORY exportDir = (PIMAGE_EXPORT_DIRECTORY)((BYTE*)hModule + exportAddr);
        
        DWORD* names = (DWORD*)((BYTE*)hModule + exportDir->AddressOfNames);
        WORD* ordinals = (WORD*)((BYTE*)hModule + exportDir->AddressOfNameOrdinals);
        DWORD* functions = (DWORD*)((BYTE*)hModule + exportDir->AddressOfFunctions);

        for (DWORD i = 0; i < exportDir->NumberOfNames; i++) {
            const char* funcName = (const char*)((BYTE*)hModule + names[i]);
            if (hash_string(funcName) == targetHash) {
                return (FARPROC)((BYTE*)hModule + functions[ordinals[i]]);
            }
        }
        return nullptr;
    }

public:
    template<typename T>
    static T get_api(const char* moduleName, uint32_t apiHash) {
        HMODULE hModule = GetModuleHandleA(moduleName);
        if (!hModule) hModule = LoadLibraryA(moduleName);
        if (!hModule) return nullptr;

        // Cache para no repetir la búsqueda (Mejorado)
        static struct CacheEntry {
            uint32_t hash;
            FARPROC proc;
        } cache[64];

        for (int i = 0; i < 64; i++) {
            if (cache[i].hash == apiHash) return (T)cache[i].proc;
            if (cache[i].hash == 0) {
                cache[i].hash = apiHash;
                cache[i].proc = get_proc_by_hash(hModule, apiHash);
                return (T)cache[i].proc;
            }
        }
        return (T)get_proc_by_hash(hModule, apiHash);
    }
};

// Macro simplificada para evitar errores de argumentos
#define API_CALL(module, hash, func) \
    APIResolver::get_api<decltype(&func)>(module, hash)