#ifndef PROTOCOL_HANDLER_H
#define PROTOCOL_HANDLER_H

#include <cstdint>
#include <iostream>
#include <vector>
#include <string>
#include <cstdint>
#include <array>
#include <cstring>
#include <WinSock2.h>

#define MAX_BUFFER 2048

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
    REQ_MESSAGE_TO_USER = 603,
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
struct ResponseHeader {
    uint8_t version;       
    uint16_t responseOp;
    uint32_t payloadSize;  
};

class ProtocolHandler{
    public: 
        /* Default constructor, we will combine the pieces one by one */
        ProtocolHandler() = default;
        
        /* Makes a new header upon request */
        void setRequestHeader(std::array<uint8_t,16> clientID, uint8_t version, uint16_t requestOp, uint32_t payloadSize);      
        void setResponseHeader(uint8_t version, uint16_t responseOp, uint32_t payloadSize);
        
        /* Gets a header */
        RequestHeader getRequestHeader() const;
        ResponseHeader getResponseHeader() const;

        /* Creates messages or parses responses from server according to data */
        std::vector<std::vector<unsigned char>> createMessage(std::vector<unsigned char>& payload);
        std::pair<ResponseHeader,std::vector<unsigned char>> parseResponse(std::vector<unsigned char>& response);

        /* Controls the messages and responses sent/received */
        void messageHandler(RequestOp op);                                                                    // switch(message op)
        void responseHandler(std::pair<ResponseHeader,std::vector<unsigned char>> response);                  // switch(response op)

    private:
        RequestHeader requestHeader;
        ResponseHeader responseHeader;
        std::vector<unsigned char> payload;
        
};

#endif
