#pragma once
#include <string>
#include <vector>

namespace Auth {
    // Estructuras de respuesta para emular el comportamiento de la API
    struct LoginResponse {
        bool success;
        std::string message;
        std::string role;
    };

    struct GenericResponse {
        bool success;
        std::string message;
    };

    struct GenerateResponse {
        bool success;
        std::vector<std::string> keys;
    };

    class ApiClient {
    private:
        // Definida en ApiClient.cpp como http:// para evitar problemas de SSL
        static const std::string BASE_URL;

    public:
        // Métodos estáticos: No necesitan instanciar la clase para ser usados
        static LoginResponse ValidateLogin(const std::string& username, const std::string& password);
        static GenericResponse RedeemKey(const std::string& username, const std::string& key_string, const std::string& password);
        static GenerateResponse GenerateKey(const std::string& membership, int amount);
        
        // Administración
        static std::string GetAllUsers(); 
        static GenericResponse ResetHwid(const std::string& username);
        static GenericResponse DeleteUser(const std::string& username);
        static GenericResponse UpdateMembership(const std::string& username, const std::string& newMembership);
    };
}