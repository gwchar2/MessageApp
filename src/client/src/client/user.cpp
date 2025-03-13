#include "../../include/User.h"



/* Creates a user from existing info in my.info 
    We already have a private key, and a public key stored in the database, 
    therefor we only initialize the decryptor. */
User::User(const std::string& name,const std::string& uuid,const std::string& base64Key) 
    : name(name) {
        decryptor.emplace(Base64Wrapper::decode(base64Key));
        for (size_t i = 0 ; i < 16; i ++){
            std::stringstream ss;
            ss << std::hex << uuid.substr(i*2,2);
            int byte;
            ss >> byte;
            UUID[i] = static_cast<uint8_t>(byte);
        }
        // does not initiate encryptor! Since encryptor is going to hold target public key.
    }

/* Creates a new encryptor and decryptor, since we need to store both for new user. */
User::User(const std::string& name)
    : name(name){ 
        RSAPrivateWrapper rsapriv;
        std::string pubkey = rsapriv.getPublicKey();
        decryptor.emplace(rsapriv.getPrivateKey());
        // does not initiate encryptor! Since encryptor is going to hold target public key.
    }

const std::optional<RSAPrivateWrapper>& User::getDecryptor() const {
    return decryptor;
}


const std::string& User::getName() const{
    return name;
}

const std::array<uint8_t, 16>& User::getUUID() const{
    return UUID;
}

void User::setUUID(const std::string& uuid){
    for (size_t i = 0 ; i < 16; i ++){
        std::stringstream ss;
        ss << std::hex << uuid.substr(i*2,2);
        int byte;
        ss >> byte;
        UUID[i] = static_cast<uint8_t>(byte);
    }
}