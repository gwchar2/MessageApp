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
std::string saveToTemp(std::string data);                                                       // Saves a string of data to a random temp file in %temp%


#endif
