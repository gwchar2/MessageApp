#pragma once

#include <string>
 /* SYMMETRICAL ENCODING 
            generate an aes object with a new key 
            AESWrapper aes(AESWrapper::GenerateKey());
            
			encrypt the message 
            std::string encryptedMessage = aes.encrypt(message);
            std::cout << "Plain: "<<std::endl << message << std::endl;
            
			print the original message hexified 
            hexify(encryptedMessage);
            
			decrypt the message 
            std::string decryptedMessage = aes.decrypt(encryptedMessage);
            
			send the encrypted message (always in blocks of 16)
            client.send_message(encryptedMessage);
            
			receive message from server 
            std::string data = client.receive_message();
            
			decrypted message
            std::cout << "Decrypted again: " << std::endl << aes.decrypt(data) << std::endl;
*/

class AESWrapper{
	public:
		static const unsigned int DEFAULT_KEYLENGTH = 16;					// 128-bit AES key
		AESWrapper();														// Constructor with new key
		AESWrapper(const std::string& key);									// Constructor with existing key

		static std::string GenerateKey();									// Generates a random AES key of specified length and stores it in buffer
		const std::string& getKey() const;									// Retrieves the AES key

		std::string encrypt(const std::string& plain) const;						// Encrypts plaintext and returns the ciphertext
		std::string decrypt(const std::string& cipher) const;						// Decrypts ciphertext and returns the plaintext
		
	private:
		std::string _key;													// AES key storage
};