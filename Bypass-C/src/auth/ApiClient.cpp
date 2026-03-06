#include "../../include/auth/ApiClient.h"
#include "../../include/auth/Hwid.h"
#include <httplib.h>
#include <nlohmann/json.hpp> 
#include <iostream>

using json = nlohmann::json;

namespace Auth {

    // Se mantiene HTTP pero configuraremos la librería para seguir redirecciones si Render obliga a HTTPS
    const std::string ApiClient::BASE_URL = "https://api-bypass-e6ty.onrender.com";

    LoginResponse ApiClient::ValidateLogin(const std::string& username, const std::string& password) {
        try {
            httplib::Client cli(BASE_URL);
            cli.set_connection_timeout(20); 
            cli.set_read_timeout(20);
            
            // ESTA ES LA LÍNEA MÁGICA PARA EL ERROR 307
            cli.set_follow_location(true); 

            std::string hwid = Hwid::GetHWID();
            
            // Enviamos el HWID tanto en Header como en Params por seguridad
            httplib::Headers headers = { {"x-hwid", hwid} };
            
            httplib::Params params = {
                {"username", username},
                {"password", password},
                {"hwid", hwid} 
            };

            auto res = cli.Post("/login", headers, params);

            if (!res) {
                return { false, "Server Unreachable (API Offline)", "usuario" };
            }

            if (res->status == 200) {
                auto data = json::parse(res->body);
                std::string role = data.value("role", "usuario");
                return { true, "Access Granted.", role };
            } 
            else {
                try {
                    auto data = json::parse(res->body);
                    std::string detail = data.value("detail", "Invalid Credentials");
                    return { false, detail, "usuario" };
                } catch (...) {
                    return { false, "Auth Error (Status: " + std::to_string(res->status) + ")", "usuario" };
                }
            }
        } catch (const std::exception& e) {
            return { false, "Connection Error: " + std::string(e.what()), "usuario" };
        } catch (...) {
            return { false, "Critical Network Failure.", "usuario" };
        }
    }

    GenericResponse ApiClient::RedeemKey(const std::string& username, const std::string& key_string, const std::string& password) {
        try {
            httplib::Client cli(BASE_URL);
            cli.set_connection_timeout(15);
            cli.set_follow_location(true); // Permitir redirecciones

            std::string hwid = Hwid::GetHWID();
            httplib::Headers headers = { {"x-hwid", hwid} };

            json payload = {
                {"key_code", key_string},
                {"username", username},
                {"password", password},
                {"hwid", hwid}
            };

            auto res = cli.Post("/keys/redeem", headers, payload.dump(), "application/json");

            if (res && (res->status == 200 || res->status == 201)) {
                return { true, "License Activated Successfully." };
            } 
            else if (res) {
                try {
                    auto data = json::parse(res->body);
                    return { false, data.value("detail", "Invalid License Key.") };
                } catch (...) { return { false, "Error processing server response." }; }
            }
        } catch (...) {}
        return { false, "Server connection failed." };
    }

    GenerateResponse ApiClient::GenerateKey(const std::string& membership, int amount) {
        try {
            httplib::Client cli(BASE_URL);
            cli.set_connection_timeout(15);
            cli.set_follow_location(true);

            int duration = 30; 
            if (membership == "Weekly") duration = 7;
            else if (membership == "Yearly") duration = 365;
            else if (membership == "Lifetime") duration = 9999;

            json payload = {
                {"membresia", membership},
                {"duracion_dias", duration},
                {"cantidad", amount}
            };

            auto res = cli.Post("/keys/generate", payload.dump(), "application/json");

            if (res && res->status == 200) {
                auto data = json::parse(res->body);
                std::vector<std::string> generatedKeys;
                if (data.contains("keys") && data["keys"].is_array()) {
                    for (auto& k : data["keys"]) {
                        generatedKeys.push_back(k.get<std::string>());
                    }
                    return { true, generatedKeys };
                }
            }
        } catch (...) {}
        return { false, {} };
    }

    std::string ApiClient::GetAllUsers() {
        try {
            httplib::Client cli(BASE_URL);
            cli.set_connection_timeout(10);
            cli.set_follow_location(true);
            
            auto res = cli.Get("/admin/users");
            if (res && res->status == 200) {
                return res->body;
            }
        } catch (...) {}
        return "[]";
    }

    GenericResponse ApiClient::ResetHwid(const std::string& username) {
        try {
            httplib::Client cli(BASE_URL);
            cli.set_connection_timeout(10);
            cli.set_follow_location(true);
            
            std::string endpoint = "/users/" + username + "/reset-hwid";
            auto res = cli.Put(endpoint.c_str());

            if (res && res->status == 200) {
                auto data = json::parse(res->body);
                return { true, data.value("message", "HWID Reset Success") };
            }
        } catch (...) {}
        return { false, "Connection error" };
    }

    GenericResponse ApiClient::DeleteUser(const std::string& username) {
        try {
            httplib::Client cli(BASE_URL);
            cli.set_connection_timeout(10);
            cli.set_follow_location(true);
            
            std::string endpoint = "/users/" + username;
            auto res = cli.Delete(endpoint.c_str());

            if (res && res->status == 200) {
                auto data = json::parse(res->body);
                return { true, data.value("message", "User deleted") };
            }
        } catch (...) {}
        return { false, "Delete failed" };
    }

    GenericResponse ApiClient::UpdateMembership(const std::string& username, const std::string& newMembership) {
        try {
            httplib::Client cli(BASE_URL);
            cli.set_connection_timeout(10);
            cli.set_follow_location(true);

            int duration = 30;
            if (newMembership == "Weekly") duration = 7;
            else if (newMembership == "Yearly") duration = 365;
            else if (newMembership == "Lifetime") duration = 9999;

            json payload = {
                {"membresia", newMembership},
                {"duracion_dias", duration}
            };

            std::string endpoint = "/users/" + username;
            auto res = cli.Put(endpoint.c_str(), payload.dump(), "application/json");

            if (res && res->status == 200) {
                return { true, "Plan updated" };
            }
        } catch (...) {}
        return { false, "API connection error" };
    }
}