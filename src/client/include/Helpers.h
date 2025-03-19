#ifndef HELPERS_H
#define HELPERS_H
#include "User.h"
#include "Client.h"
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <base64.h>
#include <iomanip>
#include <string>
#include <optional>

/* Defines just for cool text color */
#define RESET   "\033[0m"
#define RED     "\033[31m"   
#define YELLOW  "\033[33m" 

std::pair<std::string, int> getServerInfo();                                                    // Gets server infro from file
std::vector<std::string> getUserInfo();                                                         // Gets user info from file
int openingMessage(Client* client);                                                             // Opening message for the user
std::string receiveUsername();                                                                  // Receives username from client input
std::string binaryToStr(std::vector<unsigned char> data, const size_t size);                    // Turns binary vectors to string



enum class PlaceHolder{
    /* General errors */
    PH_ERROR,
    GEN_ERROR,
    ENTER_MSG,
    ERR_CHOOSING_OPTION,
    FILE_NOT_FOUND,
    ENTER_FILE_PATH,
    CANT_OPEN_FILE,
    MSG_TO_LONG,
    REQ_PUBLIC_KEY_FIRST,
    DID_NOT_REQ_SYMMETRIC,
    REQ_SYMMETRIC_FIRST ,
    ENTER_USRNM,
    USRNM_TO_LONG,
    INVALID_INPUT,

    /* Encryption Related */
    KEY_LENGTH_ERROR,

    /* Server related */
    USER_SIGNED_IN,
    SERVER_DISCONNECTED,
    INVALID_PAYLOAD,
    NO_MORE_MEMBERS,
    MEMBER_LIST_FIRST,
    CLIENT_LIST_REFRESH,
    PUB_KEY_RECEIVED,
    SENT_MSG_SUCCESS,
    NO_WAITING_MSGS,
    INCMPLT_MSG,
    FROM,
    CANT_OPEN_SERVER_INFO,
    INVALID_SERVER_FORMAT,
    INVALID_PORT,
    NO_SUCH_USER,
    USR_NOT_FOUND,
    CNCTED_ADDR,
    DSCNCTED_FROM_SERVER
};

template<typename... Args>
std::string getPlaceHolder(PlaceHolder holder, Args... args){
    std::stringstream ss; 

    switch (holder) {
        case PlaceHolder::SERVER_DISCONNECTED:
            ss << "Server disconnected. Exiting client.";
            break;
        case PlaceHolder::PH_ERROR:
            ss << RED << "[ERROR] " << RESET;
            break;
        case PlaceHolder::KEY_LENGTH_ERROR:
            ss << "Key length must be 16 bytes (128 bits)";
            break;
        case PlaceHolder::USER_SIGNED_IN:
            ss << YELLOW << "Invalid option, you are already signed in!"<< RESET; 
            break;
        case PlaceHolder::CLIENT_LIST_REFRESH:
            ss << YELLOW << "To refresh client list, please re-log" << RESET;
            break;
        case PlaceHolder::MEMBER_LIST_FIRST:
            ss << YELLOW << "Please request member list first!" << RESET;
            break;
        case PlaceHolder::REQ_PUBLIC_KEY_FIRST:
            ss << YELLOW "Please request a public key for " << RESET;
            ((ss << args << " "), ...);
            break;
        case PlaceHolder::DID_NOT_REQ_SYMMETRIC:
            ss << YELLOW <<  "User";
            ((ss << args << " "), ...);
            ss << " Did not request a symmetric key!" << RESET;
            break;
        case PlaceHolder::REQ_SYMMETRIC_FIRST:
            ss << YELLOW << "Request a symmetrical key first for user " << RESET;
            ((ss << args << " "), ...);
            break;
        case PlaceHolder::ENTER_MSG:
            ss << RED << "Enter message: " << RESET;
            break;
        case PlaceHolder::MSG_TO_LONG:
            ss << RED << "Message is to long! Shorten it." << RESET;
            break;
        case PlaceHolder::FILE_NOT_FOUND:
            ss << RED << "File Not found!" << RESET;
            break;
        case PlaceHolder::ERR_CHOOSING_OPTION:
            ss << RED << "Error choosing option, try again." << RESET;
            break;
        case PlaceHolder::CANT_OPEN_FILE:
            ss << RED << "Could not open the requested file!" << RESET;
            break;
        case PlaceHolder::NO_MORE_MEMBERS:
            ss << YELLOW << "There are no other members!" << RESET;
            break;
        case PlaceHolder::INVALID_PAYLOAD:
            ss << RED << "Invalid payload size, the database is probably corrupted!" << RESET;
            break;
        case PlaceHolder::ENTER_FILE_PATH:
            ss << RED << "Enter complete file path: " << RESET;
            break;
        case PlaceHolder::PUB_KEY_RECEIVED:
            ss << YELLOW << "Public key received for: " << RESET;
            ((ss << args << " "), ...);
            ss << YELLOW << ". You can now send him a symmetric key (If he asked for one)." << RESET;
            break;
        case PlaceHolder::SENT_MSG_SUCCESS:
            ss << YELLOW << "Sent message successfully to " << RESET;
            ((ss << args << " "), ...);
            break;
        case PlaceHolder::NO_WAITING_MSGS:
            ss << YELLOW << "No waiting messages for " << RESET;
            break;
        case PlaceHolder::INCMPLT_MSG:
            ss << YELLOW << "Incomplete message" << RESET;
            break;
        case PlaceHolder::FROM:
            ss << RED << "FROM " << RESET;
            ((ss << args << " "), ...);
            break;
        case PlaceHolder::GEN_ERROR:
            ss << RED << "General Error received, please try again!" << RESET;
            break;
        case PlaceHolder::ENTER_USRNM:
            ss << YELLOW << "Please enter a username, up to 254 characters long - No ending 0's!." << RESET;
            break;
        case PlaceHolder::USRNM_TO_LONG:
            ss << RED << "Username to long, please enter again!" << RESET;
            break;
        case PlaceHolder::INVALID_INPUT:
            ss << RED << "\nInvalid input, please enter a valid number!\n" << RESET;
            break;
        case PlaceHolder::CANT_OPEN_SERVER_INFO:
            ss << RED << "[ERROR] Could not open server.info" << RESET;
            break;
        case PlaceHolder::INVALID_SERVER_FORMAT:
            ss << RED << "[ERROR] Invalid format in server.info" << RESET;
            break;
        case PlaceHolder::INVALID_PORT:
            ss << RED << "[ERROR] Port is not a valid number" << RESET;
            break;
        case PlaceHolder::NO_SUCH_USER:
            ss << YELLOW << "No such user! Please choose again or refresh the list (Request again)." << RESET;
            break;
        case PlaceHolder::USR_NOT_FOUND:
            ss << RED << "User not found" << RESET;
            break;
        case PlaceHolder::CNCTED_ADDR:
            ss << RED << "[CONNECTED] " << RESET<< "to ";
            break;
        case PlaceHolder::DSCNCTED_FROM_SERVER:
            ss << YELLOW << "[DISCONNECTED] from server.";
            break;
        default:  
            break;
    }
  
    return ss.str();
};



#endif
