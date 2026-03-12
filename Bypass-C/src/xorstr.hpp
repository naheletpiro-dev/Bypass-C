// xorstr.hpp - Compile-time string encryption
#pragma once
#include <cstdint>
#include <utility>
#include <algorithm>

template<typename CharT, size_t N>
class xor_string {
public:
    CharT _data[N] = { 0 };
    static constexpr uint64_t _key = 0x7E5A9C3F1D8B6A4F;

    static constexpr uint8_t _xor_key(size_t index) {
        return ((_key >> (index % 8)) & 0xFF) ^ (index * 0x1F);
    }

public:
    // Constructor de encriptación en tiempo de compilación
    constexpr xor_string(const CharT* str) {
        for (size_t i = 0; i < N - 1; ++i) {
            _data[i] = str[i] ^ _xor_key(i);
        }
        _data[N - 1] = 0;
    }

    // Desencriptación en tiempo de ejecución
    CharT* decrypt() {
        for (size_t i = 0; i < N - 1; ++i) {
            _data[i] = _data[i] ^ _xor_key(i);
        }
        return _data;
    }

    // Re-encriptación (usada por el destructor de scoped_ptr)
    void encrypt() {
        for (size_t i = 0; i < N - 1; ++i) {
            _data[i] = _data[i] ^ _xor_key(i);
        }
    }

    struct scoped_ptr {
    private:
        xor_string* _parent;
    public:
        scoped_ptr(xor_string* parent) : _parent(parent) { _parent->decrypt(); }
        ~scoped_ptr() { _parent->encrypt(); }
        CharT* get() const { return const_cast<CharT*>(_parent->_data); }
    };
};

// MACROS CORREGIDAS: Utilizan Lambdas para evitar errores de tipo y asegurar persistencia
#define XorStr(str) \
    ([]() -> char* { \
        static xor_string<char, sizeof(str)> encrypted(str); \
        return xor_string<char, sizeof(str)>::scoped_ptr(&encrypted).get(); \
    }())

#define XorStrW(str) \
    ([]() -> wchar_t* { \
        static xor_string<wchar_t, sizeof(str)/sizeof(wchar_t)> encrypted(str); \
        return xor_string<wchar_t, sizeof(str)/sizeof(wchar_t)>::scoped_ptr(&encrypted).get(); \
    }())