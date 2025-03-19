#ifndef CLIENT_H
#define CLIENT_H

#include "ProtocolManager.h"
#include <User.h>
#include <Helpers.h>
#include <boost/asio.hpp>
#include <optional>

#define MAX_USERNAME_SIZE 254                                                                   // Max username length, minus null terminator.

/* This class is for the users that are received in the client list from the server. */
class ClientData {
    public:

        ClientData(std::string uid, std::string uname) 
            : uuid(std::move(uid)), username(std::move(uname)), requestedSymmetric(false){}                             // Basic constructor 

        ClientData(ClientData&& other) noexcept                                                                         // After reserving room we need a move constructor
            : uuid(std::move(other.uuid)), username(std::move(other.username)),requestedSymmetric(false) {}

        ClientData& operator=(ClientData&& other) noexcept {                                                            // Copy constructor for move (deep copy)
            if (this != &other) {
                uuid = std::move(other.uuid);
                username = std::move(other.username);
                requestedSymmetric = std::move(other.requestedSymmetric);
            }
            return *this;
        }

        /* Sets a new symmetric key for a specific user */
        void setNewSymmetric(){                                                                                         
            symmetric_key.emplace();
        }
        
        /* Sets a new symmetric key for a specific user according to a received value */
        void setSymmetric(std::string key){
            symmetric_key.emplace(key);
        }

        /* Sets public key according to value received */
        void setPublic(std::string key){
            public_key.emplace(key);
        }

        /* Returns the UUID of a user in an array */
        std::array<uint8_t, 16> getUUID() const{
            std::array<uint8_t, 16> uuidBytes;
            for (size_t i = 0; i < 16; i++) {
                uuidBytes[i] = std::stoul(uuid.substr(i * 2, 2), nullptr, 16);
            }
            return uuidBytes;
        }

        /* Returns the specific client member UUID */
        std::string getUUIDString() const {
            return uuid; 
        }

        /* Returns a string of the members username */
        std::string getUsername() const{
            return username;
        }

        /* Returns the symmetric encrypter */
        std::optional<AESWrapper>& getAESWrapper(){
            return symmetric_key;
        }
        
        /* Returns the public encrypter for specific user */
        std::optional<RSAPublicWrapper>& getRSAPublicWrapper(){
            return public_key;
        }

        /* Sets bool value according to symmetric request */
        void setRequested(){
            requestedSymmetric = !requestedSymmetric;
        }

        /* Returns if symmetric key is requested by this user or not */
        bool getRequested(){
            return requestedSymmetric;
        }

    private:
        std::string uuid;                                   // Member UUID
        std::string username;                               // Member username
        std::optional<AESWrapper> symmetric_key;            // Member symmetric key
        std::optional<RSAPublicWrapper> public_key;         // Member public key
        bool requestedSymmetric;                            // Did he request a symmetric key from us?
};

class Client {
    public:
        Client(const std::string& server_ip, int server_port);                                          // Constructor for client connection
        /* User related */
        void setUser(const std::string& name, const std::string& UUID, const std::string& key);         // Sets a user according to file
        void setUser(const std::string& name);                                                          // Sets a new user after username input 
        const std::optional<User>& getUser() const;                                                     // Returns user object
        void setUserUUID(const std::array<uint8_t, 16>& newUUID);
        void clearUser();                                                                               // Clears the user object (for error on req 110)
        

        /* Member list related */
        void setMembers(const std::string& uuid, const std::string& username);                          // Sets the members list (req 120) after response from server
        std::vector<ClientData>& getMembers();                                                          // Returns the members list (req 120)
        ClientData& getMember();                                                                        // Returns a specific member from the list
        ClientData& findUser(std::string& useruid);
        
        /* Connection related */
        void clientService();                                                                           // The client request/response handler
        bool isConnected();                                                                             // Checks for active connection
        void connectToServer();                                                                         // Connects to the server 
        void closeConnection();                                                                         // Closes the connection
        void sendMessage(const std::vector<std::vector<unsigned char>>& message);                       // Sends a message to the server
        std::vector<unsigned char> receiveMessage(size_t size);                                         // Receives a message from the server

    private:    
        std::optional<User> user;                                                   // Client-user information
        ProtocolManager protocolManager;                                            // Handles the protocol
        boost::asio::io_context io_context;                                         // Connection context
        boost::asio::ip::tcp::socket socket;                                        // Connection socket
        std::vector<ClientData> members;                                            // Members on the server
        std::string server_ip;                                                      // Server IP
        int server_port;                                                            // Server PORT
};

#endif 

