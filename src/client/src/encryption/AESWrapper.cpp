#include "../../include/AESWrapper.h"
#include "../../include/Helpers.h"
#include <modes.h>
#include <aes.h>
#include <filters.h>
#include <stdexcept>
#include <immintrin.h>	// _rdrand32_step


/*
 * Default constructor: Generates a new random 128-bit AES key.
 */
AESWrapper::AESWrapper() : _key(GenerateKey()) {}

/*
 * Constructor: Initializes AES with an existing key.
 * Throws an error if the key size is not exactly 16 bytes.
 */
AESWrapper::AESWrapper(const std::string& key) {
    if (key.size() != DEFAULT_KEYLENGTH) 
        throw std::length_error(getPlaceHolder(PlaceHolder::KEY_LENGTH_ERROR));
    _key = key;
}


/*
 * Generates a 128-bit AES key using hardware random number generator (RDRAND).
 * Returns a std::string containing 16 bytes of random data.
 */
std::string AESWrapper::GenerateKey() {
    std::string key(DEFAULT_KEYLENGTH, '\0'); // Initialize with null bytes
    for (size_t i = 0; i < DEFAULT_KEYLENGTH; i += sizeof(unsigned int)) 
        _rdrand32_step(reinterpret_cast<unsigned int*>(&key[i]));
    return key;
}

/*
 * Returns the stored AES key as a binary std::string.
 */
const std::string& AESWrapper::getKey() const { return _key;}

/*
 * Encrypts a plaintext message using AES-128 in CBC mode with a fixed IV (not secure for real use).
 * Returns the encrypted ciphertext as a std::string.
 */
std::string AESWrapper::encrypt(const std::string& plain) const{
    CryptoPP::byte iv[CryptoPP::AES::BLOCKSIZE] = {0}; // Fixed IV (unsafe in real applications)
    
    CryptoPP::AES::Encryption aesEncryption(reinterpret_cast<const CryptoPP::byte*>(_key.data()), DEFAULT_KEYLENGTH);
    CryptoPP::CBC_Mode_ExternalCipher::Encryption cbcEncryption(aesEncryption, iv);

    std::string cipher;
    CryptoPP::StreamTransformationFilter stfEncryptor(cbcEncryption, new CryptoPP::StringSink(cipher));
    stfEncryptor.Put(reinterpret_cast<const CryptoPP::byte*>(plain.data()), plain.size());
    stfEncryptor.MessageEnd();

    return cipher;
}

/*
 * Decrypts a ciphertext using AES-128 in CBC mode with a fixed IV.
 * Returns the decrypted plaintext as a std::string.
 */
std::string AESWrapper::decrypt(const std::string& cipher) const{
    CryptoPP::byte iv[CryptoPP::AES::BLOCKSIZE] = {0}; // Fixed IV (unsafe in real applications)
    
    CryptoPP::AES::Decryption aesDecryption(reinterpret_cast<const CryptoPP::byte*>(_key.data()), DEFAULT_KEYLENGTH);
    CryptoPP::CBC_Mode_ExternalCipher::Decryption cbcDecryption(aesDecryption, iv);

    std::string decrypted;
    CryptoPP::StreamTransformationFilter stfDecryptor(cbcDecryption, new CryptoPP::StringSink(decrypted));
    stfDecryptor.Put(reinterpret_cast<const CryptoPP::byte*>(cipher.data()), cipher.size());
    stfDecryptor.MessageEnd();

    return decrypted;
}
