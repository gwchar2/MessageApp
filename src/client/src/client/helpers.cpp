#include "../../include/Helpers.h"
#include "../../include/Client.h"
#include "Helpers.h"

/* Reads the server IP and port from server.info */
std::pair<std::string, int> getServerInfo() {
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
std::vector<std::string> getUserInfo() {
    std::ifstream file("me.info");
    std::vector<std::string> user_info;
    if (!file.is_open()) return user_info;

    std::string line;

    for (int i = 0 ; i < 2 ; i ++ ){
        if (!std::getline(file,line)){
            user_info.clear();
            return user_info;
        }
        user_info.push_back(line);
    }

    std::string privateKey;
    while ( std::getline(file,line)){
        if (!privateKey.empty()) privateKey += "\n";
        privateKey += line;
    }
    
    if (!privateKey.empty())
        user_info.push_back(privateKey);

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

int openingMessage(Client* client){
    
    if (client -> getUser().has_value()){
        std::cout << "\nMessageU Client at your service. \n" 
                  << "Welcome back " << client -> getUser().value().getName() + "!" << std::endl;
    }
    else std::cout << "\nMessageU Client at your service." << std::endl;

    std::cout << "Please choose an action, type the number you choose." << std::endl;

    std::cout <<"110)   Register\n" << 
                "120)   Request for clients\n" << 
                "130)   Request for public key\n" << 
                "140)   Request for waiting messages\n" << 
                "150)   Send a text message\n" << 
                "151)   Send a request for symmetric key\n" << 
                "152)   Send your symmetric key\n" <<
                " 0)    Exit Client" << std::endl;
    
    std::cin.clear();
    int choice;
    while (true){
        if (std::cin >> choice) break;
        else {
            std::cout << "\n \033[31m  Invalid input, please enter a valid number!  \033[0m \n" << std::endl;
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
    }

    return choice;
}


std::string receiveUsername(){
    /* Request username */
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');                 // Clear the last input 
    std::cout << "Please enter a username, up to 254 characters long." << std::endl;    // Prompt
    std::string username;                   // Username holder
    std::getline(std::cin, username);       // Support 'spaces'

    /* If username is longer than 254 bytes + 1 for null terminator, we throw error */
    if (username.size() > MAX_USERNAME_SIZE)    throw std::runtime_error("Username to long, please enter again!");

    return username;
}


