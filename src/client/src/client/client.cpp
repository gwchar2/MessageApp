#include "../../include/Client.h"

Client::Client(const std::string& server_ip, int server_port)
    : socket(io_context), server_ip(server_ip), server_port(server_port) {}

void Client::connect_to_server() {
    try {
        boost::asio::ip::tcp::resolver resolver(io_context);
        boost::asio::ip::tcp::resolver::results_type endpoints =
            resolver.resolve(server_ip, std::to_string(server_port));

        boost::asio::connect(socket, endpoints);
        std::cout << "[CONNECTED] to " << server_ip << ":" << server_port << std::endl;
    } catch (std::exception& e) {
        std::cerr << "[ERROR] Connection failed: " << e.what() << std::endl;
    }
}

void Client::send_message(const std::string& message) {
    try {
        boost::asio::write(socket, boost::asio::buffer(message));
    } catch (std::exception& e) {
        std::cerr << "[ERROR] Sending failed: " << e.what() << std::endl;
    }
}

std::string Client::receive_message() {
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

void Client::close_connection() {
    socket.close();
    std::cout << "[DISCONNECTED] from server." << std::endl;
}
