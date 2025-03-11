#ifndef CLIENT_H
#define CLIENT_H

#include <boost/asio.hpp>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>


class Client {
    public:
        Client(const std::string& server_ip, int server_port);
        void connect_to_server();
        void close_connection();
        void send_message(const std::string& message);
        std::string receive_message();


    private:
        
        boost::asio::io_context io_context;
        boost::asio::ip::tcp::socket socket;
        std::string server_ip;
        int server_port;
};

std::pair<std::string, int> get_server_info();

#endif 
