#ifndef HELPERS_H
#define HELPERS_H

#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <base64.h>
#include <iomanip>

void hexify(const std::string& input);		                                        // Prints a buffer of length, to base 16 representation

std::pair<std::string, int> get_server_info();                                      // Gets server infro from file
std::vector<std::string> get_user_info();                                           // Gets user info from file
std::string vectorToString(const std::vector<unsigned char>& vec);                  // Vectors to string  
std::vector<unsigned char> stringToVector(const std::string& str);                  // Strings to vector 







#endif
