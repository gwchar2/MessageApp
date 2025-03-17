#include "../include/Client.h"



/* Launches the client-server interaction */
int main() {
    try {
        /* Grab server info from file, create a new client object and connect to server */
        auto [server_ip, server_port] = getServerInfo();
        Client client(server_ip, server_port);

        /* initiate a client without a user */
        client.connectToServer();

        /* Grab the user information from file, if it exists, make one. */
        std::vector<std::string> user_info = getUserInfo();

        if(!user_info.empty())
                client.setUser(user_info[0],user_info[1],user_info[2]);
        
        /* Client Service Function */
        while (client.isConnected()) {
            client.clientService();
        }
        
        throw std::runtime_error("Server disconnected. Exiting client.");

    } catch (const std::exception& e) {
        std::cerr << "[ERROR] " << e.what() << std::endl;
        
    }

    return 0;
}
