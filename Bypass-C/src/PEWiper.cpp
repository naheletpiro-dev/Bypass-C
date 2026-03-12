#include "../security/PEWiper.h"
#include "../include/apihash.hpp"
#include <vector>

void PEWiper::Initialize(int level) {
    if (level == 1) {
        WipeHeaders();
    } else if (level == 2) {
        AggressiveWipe();
    }
}

void PEWiper::WipeHeaders() {
    // Obtenemos la base de nuestra DLL en memoria
    HMODULE hModule = GetModuleHandleA(NULL);
    if (!hModule) return;

    // Resolvemos VirtualProtect por Hash (0x3A6F9C2B es el hash de Jenkins)
    auto pVirtualProtect = API_CALL("kernel32.dll", 0x3A6F9C2B, VirtualProtect);
    
    DWORD oldProtect;
    // Cambiamos a RWX para poder escribir en la cabecera (normalmente es ReadOnly)
    if (pVirtualProtect(hModule, 4096, PAGE_EXECUTE_READWRITE, &oldProtect)) {
        
        // Borramos los primeros 4096 bytes (Cabeceras completas)
        // Esto elimina el "MZ", el "This program cannot be run in DOS mode" y la firma "PE"
        SecureZeroMemory(hModule, 4096);

        // Restauramos la protección original
        pVirtualProtect(hModule, 4096, oldProtect, &oldProtect);
    }
}

void PEWiper::AggressiveWipe() {
    HMODULE hModule = GetModuleHandleA(NULL);
    if (!hModule) return;

    auto pVirtualProtect = API_CALL("kernel32.dll", 0x3A6F9C2B, VirtualProtect);
    
    PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)hModule;
    if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE) return;

    PIMAGE_NT_HEADERS ntHeaders = (PIMAGE_NT_HEADERS)((BYTE*)hModule + dosHeader->e_lfanew);
    
    // Calculamos el tamaño total de los encabezados
    DWORD sizeOfHeaders = ntHeaders->OptionalHeader.SizeOfHeaders;

    DWORD oldProtect;
    if (pVirtualProtect(hModule, sizeOfHeaders, PAGE_EXECUTE_READWRITE, &oldProtect)) {
        
        // Llenamos con basura aleatoria en lugar de ceros para dificultar la reconstrucción
        for (DWORD i = 0; i < sizeOfHeaders; i++) {
            ((BYTE*)hModule)[i] = (BYTE)(rand() % 255);
        }

        pVirtualProtect(hModule, sizeOfHeaders, oldProtect, &oldProtect);
    }
}