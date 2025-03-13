#include "../../include/Client.h"


/* Guest mode user (Until sign up)*/
Client::Client(const std::string& server_ip, int server_port)
    : socket(io_context), server_ip(server_ip), server_port(server_port)  {}

/* Sets a new user according to an existing file information */
void Client::setUser(const std::string& name, const std::string& UUID, const std::string& key){
    user.emplace(name,UUID,key);
}

void Client::setUser(const std::string& name){
    user.emplace(name);
}

const std::optional<User>& Client::getUser() const {
    return user;
}
std::vector<ClientData>& Client::getMembers() {
    return members;
}

ClientData& Client::getMember() {
    /* Grab the client ID from list. If List is empty, we need to send request for client list first. */
    if (members.empty())     throw std::runtime_error("Please request member list first!!");
            
    /* We prompt user for target username from client, and check if it exists in the list. */
    std::string member = receiveUsername();
    auto it = std::find_if(members.begin(), members.end(),
                [&](const ClientData& data) { return data.username == member; });

    /* If no such user exists, we throw an error */
    if (it == members.end()) {
        throw std::runtime_error("No such user! Please choose again or refresh the list (Request again).");
    }

    return *it;
            
}

bool Client::isConnected() {
    if (!socket.is_open()){
        return false;
    }
    return true;
}

void Client::connectToServer() {
    
    boost::asio::ip::tcp::resolver resolver(io_context);
    boost::asio::ip::tcp::resolver::results_type endpoints =
        resolver.resolve(server_ip, std::to_string(server_port));

    boost::asio::connect(socket, endpoints);
    std::cout << "[CONNECTED] to " << server_ip << ":" << server_port << std::endl;
    
}

void Client::sendMessage(const std::vector<std::vector<unsigned char>>& messages) {
    for (const auto& message : messages)    
        boost::asio::write(socket, boost::asio::buffer(message.data(), message.size()));
}

std::string Client::recieveMessage() {
    try {
        std::string buffer(128, '\0'); // Allocate buffer for up to 128 bytes -> NEED TO RESIZE LATER WHEN EDITTING PROTOCOL

        // Read data into the buffer
        size_t bytesRead = socket.read_some(boost::asio::buffer(buffer));

        // Resize string to actual received size
        buffer.resize(bytesRead);

        std::cout << "[RECEIVED] " << bytesRead << " bytes of encrypted data" << std::endl;

        return buffer;
    } catch (std::exception& e) {
        std::cerr << "[ERROR] Receiving failed: " << e.what() << std::endl;
        return "";
    }
}

void Client::closeConnection() {
    socket.close();
    std::cout << "[DISCONNECTED] from server." << std::endl;
}

void Client::clientService(){
    if (!isConnected()) return;
    /* Printing entry message */
    int choice = openingMessage(this);
    
    try {
        protocolManager.messageHandler(choice, this);
        sendMessage(protocolManager.createMessage());
    } catch (const std::exception & e){
        std::cerr << e.what() << std::endl;
    }
    
}


