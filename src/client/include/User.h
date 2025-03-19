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
        User(const std::string& name, const std::string& uuid, const std::string& key);                         // Constructor for existing me.info file
        User(const std::string& name);                                                                          // Constructor for new user
        void setUUID(const std::array<uint8_t, 16>& newUUID);                                                   // Sets the UUID for a new user 
        const std::optional<RSAPrivateWrapper>& getDecryptor() const;                                           // Gets a reference for private key 
        const std::string& getName() const;                                                                     // Gets username
        const std::array<uint8_t, 16>& getUUID() const;                                                         // Gets UUID

    private:
        std::string name;                                       // Name of user 
        std::array<uint8_t, 16> UUID;                           // UUID of user
        std::optional<RSAPrivateWrapper> decryptor;            // std::string decrypted = decrypter.decrypt(cipher)
};



#endif