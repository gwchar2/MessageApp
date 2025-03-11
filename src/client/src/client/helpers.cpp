#include "../../include/Helpers.h"



/* Reads the server IP and port from server.info */
std::pair<std::string, int> get_server_info() {
    std::ifstream file("server.info");
    if (!file.is_open()) {
        throw std::runtime_error("[ERROR] Could not open server.info");
    }

    std::string line;
    std::getline(file, line); // Read the entire line

    std::stringstream ss(line);
    std::string ip;
    std::string port_str;
    int port;

    if (!std::getline(ss, ip, ':') || !std::getline(ss, port_str)) {
        throw std::runtime_error("[ERROR] Invalid format in server.info");
    }

    try {
        port = std::stoi(port_str);
    } catch (const std::exception&) {
        throw std::runtime_error("[ERROR] Port is not a valid number");
    }

    return {ip, port};
}


/* Reads user data from my.info */
std::vector<std::string> get_user_info() {
    std::ifstream file("my.info");
    if (!file.is_open()) 
        throw std::runtime_error("[ERROR] Could not open my.info");

    std::string line;
    std::vector<std::string> user_info;

    for (int i = 0 ; i < 3 ; i ++ ){
        if (!std::getline(file,line))
            throw std::runtime_error("[ERROR] File my.info does not contain enough lines, possibly corrupted.");
        user_info.push_back(line);
    }

    return user_info;
}

/* Prints a string in its hexadecimal representation */
void hexify(const std::string& input) {
    std::ios::fmtflags f(std::cout.flags());  // Save current output formatting
    std::cout << std::hex;

    for (size_t i = 0; i < input.size(); i++) {
        std::cout << std::setfill('0') << std::setw(2)
                  << (0xFF & static_cast<unsigned char>(input[i]))  // Ensure correct byte interpretation
                  << (((i + 1) % 16 == 0) ? "\n" : " ");
    }

    std::cout << std::endl;
    std::cout.flags(f);  // Restore original formatting
}


/* Converts std::vector<char> to std::string */
std::string vectorToString(const std::vector<unsigned char>& vec) {
    return std::string(vec.begin(), vec.end());
}

/* Converts std::string to std::vector<char> */
std::vector<unsigned char> stringToVector(const std::string& str) {
    return std::vector<unsigned char>(str.begin(), str.end());
}
