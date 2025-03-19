#include "../include/Client.h"
#include "../include/Helpers.h"
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
        
        /* If the server is not connected, and we only notice this after the while, throw a runtime error */
        throw std::runtime_error(getPlaceHolder(PlaceHolder::SERVER_DISCONNECTED));

    } catch (const std::exception& e) {
        std::cerr << getPlaceHolder(PlaceHolder::PH_ERROR) << e.what() << std::endl;
        
    }

    return 0;
}
