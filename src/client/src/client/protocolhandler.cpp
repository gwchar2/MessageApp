#include "../../include/ProtocolManager.h"
#include "../../include/Client.h"
#include <boost/endian/conversion.hpp>
#include <boost/asio.hpp>
#include <filesystem>
#include <limits>
#include <fstream>

/* Parses the new request header correctly, while turning the payloadsize and requestop to little endian */
void ProtocolManager::setRequestHeader(std::array<uint8_t,16> clientID, uint8_t version, uint16_t requestOp){
    requestHeader.clientID = clientID;
    requestHeader.version = version;
    if (boost::endian::order::native == boost::endian::order::big)
        requestHeader.requestOp = boost::endian::native_to_little(requestOp);
    else requestHeader.requestOp = requestOp;
}

void ProtocolManager::setPayloadSize(uint32_t payloadSize) {
    if (boost::endian::order::native == boost::endian::order::big)
        requestHeader.payloadSize = boost::endian::native_to_little(payloadSize);
    else requestHeader.payloadSize = payloadSize;
}
/* Parses the resposne header properly, while turning payloadsize and responseop to big endian. */
void ProtocolManager::setResponseHeader(uint8_t version, uint16_t responseOp, uint32_t payloadSize){
    responseHeader.version = version;
    responseHeader.responseOp = boost::endian::little_to_native(responseOp);
    responseHeader.payloadSize = boost::endian::little_to_native(payloadSize);
}

void ProtocolManager::setMessageHeader(std::string target_uuid, uint8_t msg_type, uint32_t content_size){
    payload.clear();
    if (boost::endian::order::native == boost::endian::order::big){
        msg_type = boost::endian::native_to_little(msg_type);
        content_size = boost::endian::native_to_little(content_size);
    }
    payload.insert(payload.end(),target_uuid.begin(), target_uuid.end());
    payload.insert(payload.end(),msg_type);
    payload.insert(payload.end(),content_size);
}

RequestHeader ProtocolManager::getRequestHeader() const {
    return requestHeader;
}

ResponseHeader ProtocolManager::getResponseHeader() const {
    return responseHeader;
}


/* Returns a vector of vectors. First vector includes header + message. 
    If message is greater than MAX_BUFFER size, we split it into more vectors, appended. */
std::vector<std::vector<unsigned char>> ProtocolManager::createMessage(){
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

std::pair<ResponseHeader,std::vector<unsigned char>> ProtocolManager::parseResponse(std::vector<unsigned char>& response){
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

void ProtocolManager::messageHandler(int choice, Client* client){
    switch (choice){
        /* Register request */
        case 110:{
            /* We make sure user is not signed in, else he cant re-register, and we set the op. */
            if (client -> getUser().has_value())    throw std::runtime_error("Invalid option, you are already signed in!");
            uint16_t op = static_cast<uint16_t>(RequestOp::REQ_REGISTER);
            // If want to send to new func, can do newFunc(*client) , and than void newFunc(Client& client), and we will use client.getUser() for example.
            
            /* We get a new publickey, and the username requested */
            std::string username = receiveUsername();
            client -> setUser(username);                // Set with unpadded name
            username.resize(255,'0');                   // Pad name
            std::string publicKey = client -> getUser().value().getDecryptor().value().getPublicKey();
            
            /* Combine the message header and payload, consisting of publickey and username */
            setRequestHeader(std::array<unsigned char,16>{0},2,op);
            setPayloadSize(static_cast<uint32_t>(publicKey.size() + username.size()));

            /* We clear the payload and put the new data inside */
            payload.clear();
            payload.insert(payload.end(), username.begin(), username.end());
            payload.insert(payload.end(),publicKey.begin(), publicKey.end());
            break;
        }
        /* User List Request */
        case 120:{
            
            if (!(client -> getUser().has_value()))    throw std::runtime_error("Please register before sending messages!");
            uint16_t op = static_cast<uint16_t>(RequestOp::REQ_USER_LIST);

            /* make a header, there is no payload */
            setRequestHeader(client -> getUser().value().getUUID(),2,op);
            setPayloadSize(0);

            /* Clear the payload just incase */
            payload.clear();
            break;
        }
        /* Public Key Request */
        case 130:{
            if (!(client -> getUser().has_value()))    throw std::runtime_error("Please register before sending messages!");
            uint16_t op = static_cast<uint16_t>(RequestOp::REQ_PUBLIC_KEY);
            
            /* Get target username from client & message */
            ClientData& it = client -> getMember();
            
            /* Set the header */
            setRequestHeader(client -> getUser().value().getUUID(),2,op);
            setPayloadSize(it.getUUID().size());

            /* Set the payload */
            payload.clear();
            payload.insert(payload.end(),it.getUUID().begin(), it.getUUID().end());
            break;
        }
        /* Pull Awaiting Messages request */
        case 140:{
            if (!(client -> getUser().has_value()))    throw std::runtime_error("Please register before sending messages!");
            uint16_t op = static_cast<uint16_t>(RequestOp::REQ_AWAITING_MESSAGES);

            /* Make the header */
            setRequestHeader(client -> getUser().value().getUUID(),2,op);
            setPayloadSize(0);

            /* Clear the payload */
            payload.clear();
            break;
        }
        /* Requesting Symmetric Key (type 1) */
        case 151:{
            if (!(client -> getUser().has_value()))    throw std::runtime_error("Please register before sending messages!");
            uint16_t op = static_cast<uint16_t>(RequestOp::REQ_SEND_MSG_TO_USR);
            uint8_t type = static_cast<uint8_t>(MessageType::REQ_SYMMETRIC_KEY);

            /* Get target username from client & message */
            ClientData& it = client -> getMember();
            if (!it.getRSAPublicWrapper().has_value())   throw std::runtime_error("Please request a public key for "+it.getUsername());

            /* Fix header */
            setRequestHeader(client -> getUser().value().getUUID(),2,op);
            
            /* Fix payload */
            payload.clear();
            payload.insert(payload.end(), it.getUUID().begin(), it.getUUID().end());            // Target Client UUID
            payload.insert(payload.end(), static_cast<uint8_t>(type));                                                // Message type
            payload.insert(payload.end(), static_cast<uint8_t>(0));                             // Content Size (=0)

            break;
        }
        /* Sending Symmetric Key (type 2)  */
        case 152:{
            if (!(client -> getUser().has_value()))    throw std::runtime_error("Please register before sending messages!");
            uint16_t op = static_cast<uint16_t>(RequestOp::REQ_SEND_MSG_TO_USR);
            uint8_t type = static_cast<uint8_t>(MessageType::SEND_SYMMETRIC_KEY);

            /* Finds user in list, checks for public key, saves a symmetric. */
            ClientData& it = client -> getMember();
            if (!it.getRSAPublicWrapper().has_value()) throw std::runtime_error("Please request a public key for "+it.getUsername());
            it.setSymmetric();

            /* Encrypt the key */
            std::string encryptedSymmetric = it.getRSAPublicWrapper().value().encrypt(it.getAESWrapper().value().getKey());

            /* Construct header, payload header and content */
            setRequestHeader(client -> getUser().value().getUUID(),2,op);
            setMessageHeader(it.getUUID(),type,encryptedSymmetric.size());
            payload.insert(payload.end(),encryptedSymmetric.begin(), encryptedSymmetric.end());     // Content
            setPayloadSize(payload.size());

            break;
        }
        /* Sending text message (type 3) */
        case 150:{
            if (!(client -> getUser().has_value()))    throw std::runtime_error("Please register before sending messages!");
            uint16_t op = static_cast<uint16_t>(RequestOp::REQ_SEND_MSG_TO_USR);
            uint8_t type = static_cast<uint8_t>(MessageType::SEND_TEXT_MSG);
            
            /* Get target username from client & message */
            ClientData& it = client -> getMember();
            if (!it.getAESWrapper().has_value())   throw std::runtime_error("Request a symmetrical key first for user "+it.getUsername());

            std::string message;
            std::cout << "Enter message: " << std::endl;
            std::getline(std::cin, message);
            if (message.size() >= std::numeric_limits<uint32_t>::max()-21)  throw std::runtime_error("Message is to long! Shorten it.");
            
            /* Encrypt the message */
            std::string encryptedMsg = it.getAESWrapper().value().encrypt(message);

            /* Construct request header, payload header and content */
            setRequestHeader(client -> getUser().value().getUUID(),2,op);
            setMessageHeader(it.getUUID(),type,message.size());
            payload.insert(payload.end(),encryptedMsg.begin(), encryptedMsg.end());             // Content
            setPayloadSize(payload.size());
            break;
        }
        /* Sending File  (type 4) */
        case 153:{
            if (!(client -> getUser().has_value()))    throw std::runtime_error("Please register before sending messages!");
            uint16_t op = static_cast<uint16_t>(RequestOp::REQ_SEND_MSG_TO_USR);
            uint8_t type = static_cast<uint8_t>(MessageType::SEND_FILE);

            /* Get target username from client */
            ClientData& it = client -> getMember();
            if (!it.getAESWrapper().has_value())  if (!it.getAESWrapper().has_value())   throw std::runtime_error("Request a symmetrical key first for user "+it.getUsername());

            /* Get file path from user */
            std::cout << "Enter complete file path: " << std::endl;
            std::string file_path;
            std::getline(std::cin, file_path);
            std::ifstream file(file_path,std::ios::binary);

            /* Check the path & the size of the file! */
            if (!file)  throw std::runtime_error("File Not found!");
            if (std::filesystem::file_size(file_path) >= std::numeric_limits<uint32_t>::max()-21) throw std::runtime_error("File is to big! Please choose a different file.");
            
            /* Parse the data to a string and encrypt */
            std::string data(std::filesystem::file_size(file_path),'\0');
            file.read(data.data(),std::filesystem::file_size(file_path));
            file.close();
            std::string encryptedData = it.getAESWrapper().value().encrypt(data);

            /* Create the headers */
            setRequestHeader(client -> getUser().value().getUUID(),2,op);
            setMessageHeader(it.getUUID(),type,std::filesystem::file_size(file_path));
            payload.insert(payload.end(), encryptedData.begin(), encryptedData.end());
            setPayloadSize(payload.size());
            break;
        }
        /* Exit client */
        case 0:
            client -> closeConnection();
            exit(1);
            break;
        default:
            throw std::runtime_error("Error choosing option, try again.");
    }

}