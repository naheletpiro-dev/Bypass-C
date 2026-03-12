#pragma once
#include <windows.h>

// Aseguramos la definición de NTSTATUS ya que no siempre está en windows.h
#ifndef NTSTATUS
typedef LONG NTSTATUS;
#endif

// Aseguramos la definición de NTAPI (convención __stdcall)
#ifndef NTAPI
#define NTAPI __stdcall
#endif

class AntiDebug {
public:
    /**
     * @brief Detecta si Process Hacker o System Informer están activos.
     * Utiliza API Hashing para ocultar el uso de Toolhelp32.
     */
    static bool IsProcessHackerPresent();
    
    /**
     * @brief Verifica si el proceso está siendo depurado.
     * Resuelve la función internamente vía hash para evitar detecciones estáticas.
     */
    static bool IsDebuggerPresent();
    
    /**
     * @brief Desconecta el hilo actual del puerto de depuración.
     * Técnica: ThreadHideFromDebugger (0x11) vía ntdll!NtSetInformationThread.
     */
    static void HideThread();
    
    /**
     * @brief Aplica todas las protecciones y termina el proceso si se detecta análisis.
     */
    static void Protect();
};