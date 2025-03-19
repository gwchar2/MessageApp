#include "../../include/Helpers.h"
#include "../../include/Client.h"
#include "Helpers.h"

/* Reads the server IP and port from server.info */
std::pair<std::string, int> getServerInfo() {
    std::ifstream file("server.info");
    if (!file.is_open()) {
        throw std::runtime_error(getPlaceHolder(PlaceHolder::CANT_OPEN_SERVER_INFO));
    }

    std::string line;
    std::getline(file, line); // Read the entire line

    std::stringstream ss(line);
    std::string ip;
    std::string port_str;
    int port;

    if (!std::getline(ss, ip, ':') || !std::getline(ss, port_str)) {
        throw std::runtime_error(getPlaceHolder(PlaceHolder::INVALID_SERVER_FORMAT));
    }

    try {
        port = std::stoi(port_str);
    } catch (const std::exception&) {
        throw std::runtime_error(getPlaceHolder(PlaceHolder::INVALID_PORT));
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

/* Prints opening message to user */
int openingMessage(Client* client){
    
    if (client -> getUser().has_value()){
        std::cout << "\nMessageU Client at your service. \n" 
                  << "Welcome back " << YELLOW << client -> getUser().value().getName() + "!" << RESET << std::endl;
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
            std::cout << getPlaceHolder(PlaceHolder::INVALID_INPUT) << std::endl;
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
    }

    return choice;
}

/* Receives username input from user*/
std::string receiveUsername(){
    /* Request username */
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');                 // Clear the last input 
    std::cout << getPlaceHolder(PlaceHolder::ENTER_USRNM) << std::endl;    // Prompt
    std::string username;                   // Username holder
    std::getline(std::cin, username);       // Supports 'spaces'

    /* If username is longer than 254 bytes + 1 for null terminator, we throw error */
    if (username.size() > MAX_USERNAME_SIZE)    throw std::runtime_error(getPlaceHolder(PlaceHolder::USRNM_TO_LONG));

    return username;
}

/* Turns a binary vector to string representation for pretty printing */
std::string binaryToStr(std::vector<unsigned char> data, const size_t size){
    std::ostringstream oss;
    for (auto it = data.begin(); it != data.begin() + size; it++) {
        oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(*it);
    }
    return oss.str();
}