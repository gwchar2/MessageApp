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
    return socket.is_open();
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

std::vector<unsigned char> Client::receiveMessage(size_t size) {
    /* Create a buffer of size size*/
    std::vector<unsigned char> buffer(size);
    size_t total_bytes_read = 0;

    /* Set the total bytes read to check */
    //
    /* As long as we dont receive the bytes of amount size, we keep going.
        If we don't read anything, we return a runtime error since the connection probably disconnected. */
    while (total_bytes_read < size) {
        
        size_t bytes_read = socket.read_some(boost::asio::buffer(buffer.data() + total_bytes_read, size - total_bytes_read));

        if (bytes_read == 0) 
            throw std::runtime_error("Server disconnected");
        total_bytes_read += bytes_read;
    }

    /* Let the user know how many bytes received, we have private functions for printing each part. */
    std::cout << "\n[RECEIVED] " << total_bytes_read << " bytes of data: " << std::endl;
    for (size_t i = 0; i < buffer.size(); ++i)
        std::cout << std::hex << std::setfill('0') << std::setw(2) << (int)buffer[i] << " ";
    std::cout << std::dec << std::endl;
    return buffer;
}


void Client::closeConnection() {
    socket.close();
    std::cout << "[DISCONNECTED] from server." << std::endl;
}

void Client::clientService(){
    
    /* Printing entry message & UI */
    int choice = openingMessage(this);
    
    /* Attempting to handle building and sending the message */
    try {
        protocolManager.messageHandler(choice, this);
        sendMessage(protocolManager.createMessage());
        /* Process received response from server */
        protocolManager.responseHandler(this);
        
    } catch (const std::exception & e){
        std::cerr << e.what() << std::endl;
    }

    
}


