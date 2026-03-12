#pragma once
#include <Windows.h>

namespace Core {
    // Función principal de inyección Fileless (Manual Mapping)
    // Recibe el handle del proceso destino (ej. explorer.exe) y los bytes del payload
    bool ManualMap(HANDLE hProc, const BYTE* pSrcData);
}