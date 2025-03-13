#ifndef USER_H
#define USER_H
#include "RSAWrapper.h"
#include "AESWrapper.h"

#include <optional>
#include <string>
#include <sstream>
#include <fstream>


class User {
    public:
        User(const std::string& name, const std::string& uuid, const std::string& key);        
        User(const std::string& name);
        void setUUID(const std::string& uuid);
        const std::optional<RSAPrivateWrapper>& getDecryptor() const;                                          // Gets a reference for private key 
        const std::string& getName() const;                                                                          // Gets username
        const std::array<uint8_t, 16>& getUUID() const;                                                                          // Gets UUID
    
    private:
        std::string name;
        std::array<uint8_t, 16> UUID;
        std::optional<RSAPrivateWrapper> decryptor;            // std::string decrypted = decrypter.decrypt(cipher)
};




#endif