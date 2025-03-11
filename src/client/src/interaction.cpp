#include "../include/Client.h"
#include "../include/AESWrapper.h"
#include "../include/RSAWrapper.h"
#include "../include/Helpers.h"



/* Launches the client-server interaction */
int main() {
    try {
        /* Grab server info from file, create a new client object and connect to server */
        auto [server_ip, server_port] = get_server_info();
        Client client(server_ip, server_port);
        client.connect_to_server();

        /* Holds our message & encrypted message */
        std::string message;
        while (true) {
            std::cout << "[SEND] Enter message: ";
            std::getline(std::cin, message);

            if (message == "exit") 
                break;   
            
        }

        client.close_connection();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}
