#pragma once
#include <string>

namespace Auth {
    class Hwid {
    public:
        // Obtiene el UUID y devuelve un hash (simplificado o SHA256)
        static std::string GetHWID();
    private:
        static std::string ExecCommand(const char* cmd);
        static std::string HashSHA256(const std::string& input);
    };
}