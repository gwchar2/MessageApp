#ifndef PROTOCOL_MANAGER_H
#define PROTOCOL_MANAGER_H
#include "User.h"
#include <cstdint>
#include <iostream>
#include <vector>
#include <string>
#include <cstdint>
#include <array>
#include <cstring>
#include <optional>

#define MAX_BUFFER 4096

class Client;

/* Message type deffinitions */
enum class MessageType : uint8_t {
    REQ_SYMMETRIC_KEY = 1,
    SEND_SYMMETRIC_KEY = 2,
    SEND_TEXT_MSG = 3,
    SEND_FILE = 4
};

/* Request definitions */
enum class RequestOp : uint16_t {
    REQ_REGISTER = 600,
    REQ_USER_LIST = 601,
    REQ_PUBLIC_KEY = 602,
    REQ_SEND_MSG_TO_USR = 603,
    REQ_AWAITING_MESSAGES = 604
};

/* Response status definitions */
enum class ResponseOp : uint16_t {
    RESP_REGISTER_SUCCESSFULL = 2100,
    RESP_USER_LIST = 2101,
    RESP_PUBLIC_KEY = 2102,
    RESP_MSG_SENT_TO_USER = 2103,
    RESP_AWAITING_MESSAGES = 2104,    
    RESP_GENERAL_ERROR = 9000
};

/* A request header */
struct RequestHeader {
    std::array<uint8_t, 16> clientID; 
    uint8_t version;                  
    uint16_t requestOp;             
    uint32_t payloadSize;               
};

/* A response header */
/* Pragma so that it will be true value */
#pragma pack(1)
struct ResponseHeader {
    uint8_t version;       
    uint16_t responseOp;
    uint32_t payloadSize;  
};
#pragma pack()

class ProtocolManager{
    public: 
        ProtocolManager() = default;                                                                            // A defualt constructor so that we can initiate without values
        
        
        void setRequestHeader(std::array<uint8_t,16> clientID, uint8_t version, uint16_t requestOp);            // Sets a new Request header 
        void setMessageHeader(std::array<uint8_t, 16> target_uuid, uint8_t msg_type, uint32_t content_size);    // Sets a new Message header
        void setPayloadSize(uint32_t payloadSize);                                                              // Sets the payload size value
        
        RequestHeader getRequestHeader() const;                                                                 // Returns the request header
        ResponseHeader getResponseHeader() const;                                                               // Returns the response header

        std::vector<std::vector<unsigned char>> createMessage();                                                // Creates messages or parses responses from server according to data

        void messageHandler(int choice,Client* client);                                                         // Controls the messages sent
        void responseHandler(Client* client);                                                                   // Controls the responses received
        void printResponseHeader();                                                                             // Prints the response header (mainly for debugging)

    private:
        RequestHeader requestHeader;                                            // Request header
        ResponseHeader responseHeader;                                          // Response header
        std::vector<unsigned char> payload;                                     // Holds the payload data
        
};

#endif
