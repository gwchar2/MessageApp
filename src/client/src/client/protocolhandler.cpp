#include "../../include/ProtocolHandler.h"


/* Parses the new request header correctly, while turning the payloadsize and requestop to little endian */
void ProtocolHandler::setRequestHeader(std::array<uint8_t,16> clientID, uint8_t version, uint16_t requestOp, uint32_t payloadSize){
    requestHeader.clientID = clientID;
    requestHeader.version = version;
    requestHeader.requestOp = htons(requestOp);;
    requestHeader.payloadSize = htonl(payloadSize);;
}

/* Parses the resposne header properly, while turning payloadsize and responseop to big endian. */
void ProtocolHandler::setResponseHeader(uint8_t version, uint16_t responseOp, uint32_t payloadSize){
    responseHeader.version = version;
    responseHeader.responseOp = ntohs(responseOp);
    responseHeader.payloadSize = ntohl(payloadSize);
}


RequestHeader ProtocolHandler::getRequestHeader() const {
    return requestHeader;
}

ResponseHeader ProtocolHandler::getResponseHeader() const {
    return responseHeader;
}

/* Returns a vector of vectors. First vector includes header + message. 
    If message is greater than MAX_BUFFER size, we split it into more vectors, appended. */
std::vector<std::vector<unsigned char>> ProtocolHandler::createMessage(std::vector<unsigned char>& payload){
    /* A message chunk vector of vectors */
    std::vector<std::vector<unsigned char>> messageChunks;

    /* Size available in first chunk is not including the header. */
    size_t availableSize = MAX_BUFFER - sizeof(RequestHeader);

    /* The first chunk (first vector) of data to be sent */
    std::vector<unsigned char> firstChunk;
    firstChunk.resize(sizeof(RequestHeader));
    std::memcpy(firstChunk.data(), &requestHeader, sizeof(RequestHeader));

    size_t bytesRemaining = payload.size();
    size_t offset = 0;
    /* If the amount of bytes remaining to be added fits in first chunk, we add it. */
    if (bytesRemaining <= availableSize){
        firstChunk.insert(firstChunk.end(), payload.begin(), payload.end());
        messageChunks.push_back(firstChunk);
    }else {
        /* If the payload is greater than size left, we put only the amount we can. Rest will go in other chunks. */
        firstChunk.insert(firstChunk.end(), payload.begin(), payload.begin() + availableSize);
        messageChunks.push_back(firstChunk);
        offset += availableSize;
        bytesRemaining -= availableSize;

        /* As long as we still have data, we make more chunks. */
        while (bytesRemaining > 0){
            /* Chunk size is the max between 4096 or the amount left */
            size_t chunkSize = std::min(bytesRemaining,static_cast<size_t>(MAX_BUFFER));
            std::vector<unsigned char> chunk(payload.begin()+offset,payload.begin()+offset+chunkSize);
            messageChunks.push_back(chunk);
            offset+=chunkSize;
            bytesRemaining -= chunkSize;
        }
    }

    return messageChunks;
}

std::pair<ResponseHeader,std::vector<unsigned char>> ProtocolHandler::parseResponse(std::vector<unsigned char>& response){
    /* We check to see the response is appropriate */
    if (response.size() < sizeof(ResponseHeader))
        throw std::runtime_error("Buffer size is to small! Response is invalid.");

    /* We make the variables, easier to copy. */
    uint8_t version = response[0];
    uint16_t responseOp;
    uint32_t payloadSize;

    /* We copy into them the required data, we copy as is because little -> big endian is controlled in set func. */
    std::memcpy(&responseOp, response.data()+1, sizeof(responseOp));
    std::memcpy(&payloadSize, response.data()+3, sizeof(payloadSize));

    /* We construct the response header */
    setResponseHeader(version, responseOp, payloadSize);

    /* If the payload is not empty already, we will clear it, and assign the remainder of the data to it. */
    if (!payload.empty())   payload.clear();
    payload.assign(response.begin() + sizeof(ResponseHeader), response.end());
    
    /* We return the payload vector and the response header to continue parsing it. */
    return {responseHeader, payload};

}