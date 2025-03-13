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

void hexify(const std::string& input);		                                        // Prints a buffer of length, to base 16 representation

std::pair<std::string, int> getServerInfo();                                      // Gets server infro from file
std::vector<std::string> getUserInfo();                                           // Gets user info from file
std::string vectorToString(const std::vector<unsigned char>& vec);                  // Vectors to string  
std::vector<unsigned char> stringToVector(const std::string& str);                  // Strings to vector 
int openingMessage(Client* client);
std::string receiveUsername();
#endif
