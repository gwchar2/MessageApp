#ifndef USER_H
#define USER_H
#include "RSAWrapper.h"
#include "AESWrapper.h"


#include <string>
#include <sstream>
#include <fstream>


class User {
    public:
        User(const std::string name, const std::string UUID, const std::string& key);                   // Constructor func
        void setAESWrapper(const std::string& key);                                                     // Sets a symmetric key
        AESWrapper getAESWrapper();                                                                     // Gets the symmetric key
        const RSAPrivateWrapper& getRSAPrivateWrapper() const;                                          // Gets a reference for private key 
        std::string getName();                                                                          // Gets username
        std::string getUUID();                                                                          // Gets UUID
    
    private:
        RSAPrivateWrapper RSAPrivate_Wrapper;
        AESWrapper AES_Wrapper;
        std::string name;
        std::string UUID;
};




#endif