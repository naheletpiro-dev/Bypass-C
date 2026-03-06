#include "../../include/auth/Hwid.h"
#include <windows.h>
#include <wincrypt.h>
#include <sstream>
#include <iomanip>
#include <vector>

namespace Auth {

    std::string Hwid::ExecCommand(const char* cmd) {
        char buffer[128];
        std::string result = "";
        FILE* pipe = _popen(cmd, "r");
        if (!pipe) throw std::runtime_error("popen() failed!");
        try {
            while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
                result += buffer;
            }
        } catch (...) {
            _pclose(pipe);
            throw;
        }
        _pclose(pipe);
        return result;
    }

    // Hash SHA256 nativo usando la API de criptografía de Windows (sin librerías externas)
    std::string Hwid::HashSHA256(const std::string& input) {
        HCRYPTPROV hProv = 0;
        HCRYPTHASH hHash = 0;
        std::string hashStr = "";

        if (CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_AES, CRYPT_VERIFYCONTEXT)) {
            if (CryptCreateHash(hProv, CALG_SHA_256, 0, 0, &hHash)) {
                if (CryptHashData(hHash, (const BYTE*)input.c_str(), input.length(), 0)) {
                    DWORD cbHashSize = 0, dwCount = sizeof(DWORD);
                    CryptGetHashParam(hHash, HP_HASHSIZE, (BYTE*)&cbHashSize, &dwCount, 0);
                    std::vector<BYTE> buffer(cbHashSize);
                    if (CryptGetHashParam(hHash, HP_HASHVAL, buffer.data(), &cbHashSize, 0)) {
                        std::ostringstream oss;
                        for (std::vector<BYTE>::const_iterator iter = buffer.begin(); iter != buffer.end(); ++iter) {
                            oss << std::hex << std::setw(2) << std::setfill('0') << (int)*iter;
                        }
                        hashStr = oss.str();
                    }
                }
                CryptDestroyHash(hHash);
            }
            CryptReleaseContext(hProv, 0);
        }
        return hashStr.empty() ? "GENERIC-HWID-2026-VOID" : hashStr;
    }

    std::string Hwid::GetHWID() {
        try {
            std::string rawUUID = ExecCommand("wmic csproduct get uuid");
            // Limpiar la salida (quitar "UUID" y saltos de línea)
            size_t pos = rawUUID.find("\n");
            if (pos != std::string::npos) {
                rawUUID = rawUUID.substr(pos + 1);
            }
            // Eliminar espacios en blanco y saltos
            rawUUID.erase(std::remove_if(rawUUID.begin(), rawUUID.end(), ::isspace), rawUUID.end());
            
            return HashSHA256(rawUUID);
        } catch (...) {
            return "GENERIC-HWID-2026-VOID";
        }
    }
}