#ifndef RSA_WRAPPER_H
#define RSA_WRAPPER_H

#pragma once

#include <osrng.h>
#include <rsa.h>
#include <string>
#include <base64.h>
#include <iomanip>
#include <iostream>


/* EXAMPLE */
/* 
// User A generates keys
RSAPrivateWrapper userA_private;  
std::string userA_public = userA_private.getPublicKey(); // Extract public key

// User A sends only the public key to User B
RSAPublicWrapper userA_publicKeyWrapper(userA_public);  

// User B encrypts a message using A's public key
std::string encryptedMessage = userA_publicKeyWrapper.encrypt("Hello, User A!");

// User A decrypts it with their private key
std::string decryptedMessage = userA_private.decrypt(encryptedMessage);
std::cout << "Decrypted Message: " << decryptedMessage << std::endl;
*/

class RSAPublicWrapper{
	public:
		static const unsigned int KEYSIZE = 160;
		static const unsigned int BITS = 1024;

		RSAPublicWrapper(const std::string& key);					// Loads an existing public key from a string.
		std::string getPublicKey() const;							// Returns the public key as a string.
		std::string encrypt(const std::string& plain);				// Encrypts plaintext using the public key
		RSAPublicWrapper& operator=(const RSAPublicWrapper& rsaprivate);			// Deep copy for operator = mostly for User class

	private:
		CryptoPP::AutoSeededRandomPool _rng;
		CryptoPP::RSA::PublicKey _publicKey;

};


class RSAPrivateWrapper{
	public:
		static const unsigned int BITS = 1024;
		RSAPrivateWrapper();											// Generates a new RSA key pair (both private and public keys)
		RSAPrivateWrapper(const std::string& key);						// Loads an existing private key
		std::string getPrivateKey() const;								// Returns the private key as a string (should be encoded in Base64 before storing)
		std::string getPublicKey() const;								// Extracts the public key from the private key
		std::string decrypt(const std::string& cipher);					// Decrypts a message encrypted with the corresponding public key


		RSAPrivateWrapper& operator=(const RSAPrivateWrapper& rsaprivate);			// Makes a deep copy of the private wrapper (Mostly for user class)

	private:
		CryptoPP::AutoSeededRandomPool _rng;
		CryptoPP::RSA::PrivateKey _privateKey;

};

class Base64Wrapper{
	public:
		static std::string encode(const std::string& str);			// Encodes to base 64
		static std::string decode(const std::string& str);			// Decodes from base 64
};



#endif 