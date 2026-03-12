// memobf.hpp - Ofuscación de memoria en caliente
#pragma once
#include <windows.h>
#include <vector>
#include <random>

class MemoryObfuscator {
private:
    struct ObfuscatedBlock {
        void* real_ptr;
        size_t size;
        uint8_t xor_key;
        bool is_encrypted;
    };
    
    std::vector<ObfuscatedBlock> blocks;
    CRITICAL_SECTION cs;
    
public:
    MemoryObfuscator() {
        InitializeCriticalSection(&cs);
    }
    
    ~MemoryObfuscator() {
        DeleteCriticalSection(&cs);
        for (auto& block : blocks) {
            if (block.real_ptr && block.is_encrypted) {
                // Limpiar memoria antes de liberar
                SecureZeroMemory(block.real_ptr, block.size);
                VirtualFree(block.real_ptr, 0, MEM_RELEASE);
            }
        }
    }
    
    // Asigna memoria ofuscada
    void* alloc(size_t size) {
        EnterCriticalSection(&cs);
        
        // Generar clave aleatoria
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dist(1, 255);
        uint8_t key = static_cast<uint8_t>(dist(gen));
        
        // Asignar memoria real
        void* ptr = VirtualAlloc(nullptr, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
        
        if (ptr) {
            ObfuscatedBlock block = {ptr, size, key, false};
            blocks.push_back(block);
            
            // Inicializar con datos aleatorios
            std::vector<uint8_t> randomData(size);
            for (size_t i = 0; i < size; i++) {
                randomData[i] = static_cast<uint8_t>(dist(gen));
            }
            memcpy(ptr, randomData.data(), size);
        }
        
        LeaveCriticalSection(&cs);
        return ptr;
    }
    
    // Escribe datos de forma ofuscada
    void write(void* ptr, const void* data, size_t size) {
        EnterCriticalSection(&cs);
        
        // Buscar el bloque
        for (auto& block : blocks) {
            if (block.real_ptr == ptr && block.size >= size) {
                // Ofuscar los datos antes de escribir
                uint8_t* dest = (uint8_t*)ptr;
                const uint8_t* src = (const uint8_t*)data;
                
                for (size_t i = 0; i < size; i++) {
                    dest[i] = src[i] ^ block.xor_key ^ (i & 0xFF);
                }
                
                block.is_encrypted = true;
                break;
            }
        }
        
        LeaveCriticalSection(&cs);
    }
    
    // Lee datos desofuscándolos
    void read(void* ptr, void* buffer, size_t size) {
        EnterCriticalSection(&cs);
        
        for (auto& block : blocks) {
            if (block.real_ptr == ptr && block.size >= size) {
                uint8_t* src = (uint8_t*)ptr;
                uint8_t* dst = (uint8_t*)buffer;
                
                for (size_t i = 0; i < size; i++) {
                    dst[i] = src[i] ^ block.xor_key ^ (i & 0xFF);
                }
                break;
            }
        }
        
        LeaveCriticalSection(&cs);
    }
    
    // Libera memoria ofuscada
    void free(void* ptr) {
        EnterCriticalSection(&cs);
        
        for (auto it = blocks.begin(); it != blocks.end(); ++it) {
            if (it->real_ptr == ptr) {
                // Sobrescribir con basura antes de liberar
                std::random_device rd;
                std::mt19937 gen(rd());
                std::uniform_int_distribution<> dist(0, 255);
                
                uint8_t* p = (uint8_t*)ptr;
                for (size_t i = 0; i < it->size; i++) {
                    p[i] = static_cast<uint8_t>(dist(gen));
                }
                
                VirtualFree(ptr, 0, MEM_RELEASE);
                blocks.erase(it);
                break;
            }
        }
        
        LeaveCriticalSection(&cs);
    }
};

// Singleton global
inline MemoryObfuscator& g_MemObf() {
    static MemoryObfuscator instance;
    return instance;
}