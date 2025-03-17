#ifndef CLIENT_H
#define CLIENT_H

#include "ProtocolManager.h"
#include <User.h>
#include <Helpers.h>
#include <boost/asio.hpp>
#include <optional>

#define MAX_USERNAME_SIZE 254        // Max minus null terminator.

struct ClientData {
    std::string uuid;
    std::string username;
    std::optional<AESWrapper> symmetric_key;
    std::optional<RSAPublicWrapper> public_key;

    ClientData(std::string uid, std::string uname) 
        : uuid(std::move(uid)), username(std::move(uname)) {}
    
    void setSymmetric(){
        symmetric_key.emplace();
    }

    void setPublic(std::string key){
        public_key.emplace(key);
    }

    std::string getUUID(){
        return uuid;
    }

    std::string getUsername(){
        return username;
    }

    std::optional<AESWrapper>& getAESWrapper(){
        return symmetric_key;
    }
    
    std::optional<RSAPublicWrapper>& getRSAPublicWrapper(){
        return public_key;
    }
};

class Client {
    public:
        Client(const std::string& server_ip, int server_port);
        void setUser(const std::string& name, const std::string& UUID, const std::string& key);
        void setUser(const std::string& name);
        const std::optional<User>& getUser() const;
        std::vector<ClientData>& getMembers();
        ClientData& getMember();
        void clientService();
        bool isConnected();
        void connectToServer();
        void closeConnection();
        void sendMessage(const std::vector<std::vector<unsigned char>>& message);
        std::vector<unsigned char> receiveMessage(size_t size);

    private:
        std::optional<User> user;
        ProtocolManager protocolManager;
        boost::asio::io_context io_context;
        boost::asio::ip::tcp::socket socket;
        std::vector<ClientData> members;
        std::string server_ip;
        int server_port;
};

#endif 
