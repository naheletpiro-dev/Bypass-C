#include "../../include/core/ManualMapper.h"
#include <iostream>
#include <TlHelp32.h>

namespace Core {

    // Estructura que pasamos al proceso remoto para que se "auto-repare"
    struct LoaderData {
        LPVOID ImageBase;
        PIMAGE_NT_HEADERS NtHeaders;
        PIMAGE_BASE_RELOCATION BaseReloc;
        PIMAGE_IMPORT_DESCRIPTOR ImportDirectory;
        
        // Punteros a funciones vitales de Windows
        using fnLoadLibraryA = HMODULE(WINAPI*)(LPCSTR);
        using fnGetProcAddress = FARPROC(WINAPI*)(HMODULE, LPCSTR);
        fnLoadLibraryA pLoadLibraryA;
        fnGetProcAddress pGetProcAddress;
    };

    // --- EL SHELLCODE (Se ejecuta DENTRO del proceso objetivo) ---
    DWORD __stdcall LibraryLoader(LoaderData* pData) {
        auto pBase = (PBYTE)pData->ImageBase;
        auto pReloc = pData->BaseReloc;
        auto pImport = pData->ImportDirectory;

        // 1. Corregir Relocalizaciones (Delta fix)
        auto delta = pBase - (PBYTE)pData->NtHeaders->OptionalHeader.ImageBase;
        while (pReloc->VirtualAddress) {
            if (pReloc->SizeOfBlock >= sizeof(IMAGE_BASE_RELOCATION)) {
                int count = (pReloc->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(WORD);
                auto list = (PWORD)(pReloc + 1);
                for (int i = 0; i < count; i++) {
                    if (list[i]) {
                        auto ptr = (PDWORD)(pBase + (pReloc->VirtualAddress + (list[i] & 0xFFF)));
                        *ptr += (DWORD)delta;
                    }
                }
            }
            pReloc = (PIMAGE_BASE_RELOCATION)((PBYTE)pReloc + pReloc->SizeOfBlock);
        }

        // 2. Resolver Importaciones (IAT)
        while (pImport->Characteristics) {
            auto szLib = (PSTR)(pBase + pImport->Name);
            auto hLib = pData->pLoadLibraryA(szLib);
            auto pThunk = (PIMAGE_THUNK_DATA)(pBase + pImport->FirstThunk);
            auto pOriginalThunk = (PIMAGE_THUNK_DATA)(pBase + pImport->OriginalFirstThunk);

            while (pOriginalThunk->u1.AddressOfData) {
                if (IMAGE_SNAP_BY_ORDINAL(pOriginalThunk->u1.Ordinal)) {
                    *reinterpret_cast<FARPROC*>(&pThunk->u1.Function) = pData->pGetProcAddress(hLib, (LPCSTR)IMAGE_ORDINAL(pOriginalThunk->u1.Ordinal));
                } else {
                    auto pImportData = (PIMAGE_IMPORT_BY_NAME)(pBase + pOriginalThunk->u1.AddressOfData);
                    *reinterpret_cast<FARPROC*>(&pThunk->u1.Function) = pData->pGetProcAddress(hLib, (LPCSTR)pImportData->Name);
                }
                pThunk++;
                pOriginalThunk++;
            }
            pImport++;
        }

        // 3. Ejecutar DllMain
        if (pData->NtHeaders->OptionalHeader.AddressOfEntryPoint) {
            auto pEntryPoint = (BOOL(WINAPI*)(HINSTANCE, DWORD, LPVOID))(pBase + pData->NtHeaders->OptionalHeader.AddressOfEntryPoint);
            return pEntryPoint((HINSTANCE)pBase, DLL_PROCESS_ATTACH, nullptr);
        }
        return 1;
    }

    // Marcador para saber dónde termina el shellcode
    void __stdcall Stub() {}

    // --- FUNCIÓN PRINCIPAL DE INYECCIÓN ---
    bool ManualMap(HANDLE hProc, const BYTE* pSrcData) {
        auto pOldDosHeader = (PIMAGE_DOS_HEADER)pSrcData;
        auto pOldNtHeader = (PIMAGE_NT_HEADERS)(pSrcData + pOldDosHeader->e_lfanew);

        // Reservar memoria en proceso remoto
        auto pTargetBase = (PBYTE)VirtualAllocEx(hProc, nullptr, pOldNtHeader->OptionalHeader.SizeOfImage, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
        if (!pTargetBase) return false;

        // Mapear Secciones
        auto pSectionHeader = IMAGE_FIRST_SECTION(pOldNtHeader);
        for (int i = 0; i < pOldNtHeader->FileHeader.NumberOfSections; i++, pSectionHeader++) {
            if (pSectionHeader->SizeOfRawData) {
                WriteProcessMemory(hProc, pTargetBase + pSectionHeader->VirtualAddress, pSrcData + pSectionHeader->PointerToRawData, pSectionHeader->SizeOfRawData, nullptr);
            }
        }

        // Preparar Datos del Loader
        LoaderData loaderParams = { 0 };
        loaderParams.ImageBase = pTargetBase;
        loaderParams.NtHeaders = (PIMAGE_NT_HEADERS)(pTargetBase + pOldDosHeader->e_lfanew);
        loaderParams.BaseReloc = (PIMAGE_BASE_RELOCATION)(pTargetBase + pOldNtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress);
        loaderParams.ImportDirectory = (PIMAGE_IMPORT_DESCRIPTOR)(pTargetBase + pOldNtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);
        loaderParams.pLoadLibraryA = LoadLibraryA;
        loaderParams.pGetProcAddress = GetProcAddress;

        // Escribir Parámetros y Shellcode
        auto pParamsRemoto = (PVOID)VirtualAllocEx(hProc, nullptr, sizeof(LoaderData), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
        WriteProcessMemory(hProc, pParamsRemoto, &loaderParams, sizeof(LoaderData), nullptr);

        auto pLoaderRemoto = (PVOID)VirtualAllocEx(hProc, nullptr, (PBYTE)Stub - (PBYTE)LibraryLoader, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
        WriteProcessMemory(hProc, pLoaderRemoto, (PVOID)LibraryLoader, (PBYTE)Stub - (PBYTE)LibraryLoader, nullptr);

        // Ejecutar el hilo remoto
        HANDLE hThread = CreateRemoteThread(hProc, nullptr, 0, (LPTHREAD_START_ROUTINE)pLoaderRemoto, pParamsRemoto, 0, nullptr);
        if (!hThread) {
            VirtualFreeEx(hProc, pTargetBase, 0, MEM_RELEASE);
            return false;
        }

        WaitForSingleObject(hThread, INFINITE);
        CloseHandle(hThread);

        // Opcional: Liberar la memoria del loader (limpieza de huellas de inyección)
        VirtualFreeEx(hProc, pLoaderRemoto, 0, MEM_RELEASE);
        VirtualFreeEx(hProc, pParamsRemoto, 0, MEM_RELEASE);

        return true;
    }
}