#include "../../include/User.h"



/* Creates a user from existing info in my.info */
User::User(const std::string name,const std::string UUID,const std::string& key) 
    : RSAPrivate_Wrapper(RSAPrivateWrapper(key)), name(name), UUID(UUID) {}

void User::setAESWrapper(const std::string& key){
    AES_Wrapper = AESWrapper(key);
}

AESWrapper User::getAESWrapper(){
    return AES_Wrapper;
}

const RSAPrivateWrapper& User::getRSAPrivateWrapper() const {
    return RSAPrivate_Wrapper;
}

std::string User::getName(){
    return name;
}

std::string User::getUUID(){
    return UUID;
}