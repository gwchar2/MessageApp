#include "../../include/RSAWrapper.h"

/*******************************/
/******* RSA PUBLIC KEY *******/
/*******************************/

/* Public key constructor from an existing public key (from server) */
RSAPublicWrapper::RSAPublicWrapper(const std::string& key){
	CryptoPP::StringSource ss(key, true);
	_publicKey.Load(ss);
}

/* Public key empty constructor */
std::string RSAPublicWrapper::getPublicKey() const{
	std::string key;
	CryptoPP::StringSink ss(key);
	_publicKey.Save(ss);
	return key;
}

/* Encrypts according to a given public key */
std::string RSAPublicWrapper::encrypt(const std::string& plain){
	std::string cipher;
	CryptoPP::RSAES_OAEP_SHA_Encryptor e(_publicKey);
	CryptoPP::StringSource ss(plain, true, new CryptoPP::PK_EncryptorFilter(_rng, e, new CryptoPP::StringSink(cipher)));
	return cipher;
}

/* Deep copy for operator = */
RSAPublicWrapper& RSAPublicWrapper::operator=(const RSAPublicWrapper& rsapublic){
	if (this != &rsapublic)
		this -> _publicKey = rsapublic._publicKey;
	return *this;
}

/*******************************/
/******* RSA PRIVATE KEY *******/
/*******************************/

/* Initializes a new private key */
RSAPrivateWrapper::RSAPrivateWrapper(){
	_privateKey.Initialize(_rng, BITS);
}

/* Private key constructor according to existing key (file) */
RSAPrivateWrapper::RSAPrivateWrapper(const std::string& key){
	CryptoPP::StringSource ss(key, true);
	_privateKey.Load(ss);
}

/* Gets a private key */
std::string RSAPrivateWrapper::getPrivateKey() const{	
	std::string key;
	CryptoPP::StringSink ss(key);
	_privateKey.Save(ss);
	return key;
}

/* Gets a public key according to the existing private key */
std::string RSAPrivateWrapper::getPublicKey() const{
	CryptoPP::RSAFunction publicKey(_privateKey);
	std::string key;
	CryptoPP::StringSink ss(key);
	publicKey.Save(ss);
	return key;
}

/* Decrpyts according to private key */
std::string RSAPrivateWrapper::decrypt(std::string cipher) const{
	std::string decrypted;
	CryptoPP::RSAES_OAEP_SHA_Decryptor d(_privateKey);
	CryptoPP::StringSource ss_cipher(cipher, true, new CryptoPP::PK_DecryptorFilter(_rng, d, new CryptoPP::StringSink(decrypted)));
	return decrypted;
}

/* Deep copy for operator = */
RSAPrivateWrapper& RSAPrivateWrapper::operator=(const RSAPrivateWrapper& rsaprivate){
	if (this != &rsaprivate)
		this -> _privateKey = rsaprivate._privateKey;
	return *this;
}

/********************************/
/******* Base 64 Encoding *******/
/********************************/

/* Encodes a string to base 64 */
std::string Base64Wrapper::encode(const std::string& str){
	std::string encoded;
	CryptoPP::StringSource ss(str, true,
		new CryptoPP::Base64Encoder(
			new CryptoPP::StringSink(encoded)
		) 
	);

	return encoded;
}

/* Decodes a base 64 string */
std::string Base64Wrapper::decode(const std::string& str){
	std::string decoded;
	CryptoPP::StringSource ss(str, true,
		new CryptoPP::Base64Decoder(
			new CryptoPP::StringSink(decoded)
		) 
	);

	return decoded;
}


