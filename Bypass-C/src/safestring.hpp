// safestring.hpp - Wrapper para strings seguros
#pragma once
#include <string>
#include <vector>

template<typename T>
class SecureString {
private:
    std::vector<T> _data;
    mutable std::vector<T> _cache;
    uint8_t _key;
    
    void obfuscate() {
        for (size_t i = 0; i < _data.size(); i++) {
            _data[i] ^= (_key + i) & 0xFF;
        }
    }
    
    void deobfuscate() const {
        _cache = _data;
        for (size_t i = 0; i < _cache.size(); i++) {
            _cache[i] ^= (_key + i) & 0xFF;
        }
    }
    
public:
    SecureString(const T* str, size_t len) {
        // Generar clave aleatoria
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dist(0, 255);
        _key = static_cast<uint8_t>(dist(gen));
        
        // Copiar y ofuscar
        _data.resize(len + 1);
        for (size_t i = 0; i < len; i++) {
            _data[i] = str[i];
        }
        _data[len] = 0;
        
        obfuscate();
    }
    
    // Acceso seguro (solo por corta duración)
    class Accessor {
    private:
        const SecureString* _parent;
        std::vector<T> _decrypted;
        
    public:
        Accessor(const SecureString* parent) : _parent(parent) {
            parent->deobfuscate();
            _decrypted = parent->_cache;
        }
        
        ~Accessor() {
            // Limpiar cache
            volatile T* ptr = _decrypted.data();
            for (size_t i = 0; i < _decrypted.size(); i++) {
                const_cast<volatile T&>(ptr[i]) = 0;
            }
        }
        
        const T* c_str() const { return _decrypted.data(); }
        size_t length() const { return _decrypted.size() - 1; }
    };
    
    Accessor access() const { return Accessor(this); }
};

// Macro para crear strings seguros
#define SecureStr(str) SecureString<char>(str, sizeof(str)-1)
#define SecureStrW(str) SecureString<wchar_t>(str, (sizeof(str)/sizeof(wchar_t))-1)