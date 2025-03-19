#include "../../include/ProtocolManager.h"
#include "../../include/Client.h"
#include "../../include/Helpers.h"
#include <boost/endian/conversion.hpp>
#include <boost/asio.hpp>
#include <filesystem>
#include <limits>
#include <fstream>

/* Parses the new request header correctly, while turning the payloadsize and requestop to little endian */
void ProtocolManager::setRequestHeader(std::array<uint8_t,16> clientID, uint8_t version, uint16_t requestOp){
    requestHeader.clientID = clientID;
    requestHeader.version = version;
    if(boost::endian::order::native == boost::endian::order::little)
        requestHeader.requestOp = requestOp;
    else 
        requestHeader.requestOp = boost::endian::native_to_little(requestOp);
}

/* Sets the payload size field */
void ProtocolManager::setPayloadSize(uint32_t payloadSize) {
    if(boost::endian::order::native == boost::endian::order::little)
        requestHeader.payloadSize = payloadSize;
    else requestHeader.payloadSize = boost::endian::native_to_little(payloadSize);
}

/* Sets the message header (Secondary header upon sending a message)*/
void ProtocolManager::setMessageHeader(std::array<uint8_t, 16> target_uuid, uint8_t msg_type, uint32_t content_size) {
    payload.clear();

    if (boost::endian::order::native != boost::endian::order::little) {
        msg_type = boost::endian::native_to_little(msg_type);
        content_size = boost::endian::native_to_little(content_size);
    }

    payload.insert(payload.end(), target_uuid.begin(), target_uuid.end());
    payload.push_back(msg_type);  
    /* We need to make sure content size is added as 4 bytes */
    uint8_t size_bytes[4];  
    std::memcpy(size_bytes, &content_size, sizeof(content_size));  
    payload.insert(payload.end(), size_bytes, size_bytes + 4);
}

/* Gets the request header */
RequestHeader ProtocolManager::getRequestHeader() const {
    return requestHeader;
}

/* Gets the response header */
ResponseHeader ProtocolManager::getResponseHeader() const {
    return responseHeader;
}

/* Prints the response header */
void ProtocolManager::printResponseHeader() {   
    std::cout << YELLOW << "Raw Header Bytes: " << RESET;
    for (size_t i = 0; i < payload.size(); ++i)
        std::cout << std::hex << std::setfill('0') << std::setw(2) << (int)payload[i] << " ";
    std::cout << std::dec << std::endl;
    std::cout << YELLOW <<"Version: " << RESET <<(int)responseHeader.version << std::endl;
    std::cout << YELLOW <<"ResponseOp: " << RESET << responseHeader.responseOp << std::endl;
    std::cout << YELLOW <<"Payload Size: " << RESET <<responseHeader.payloadSize << std::endl;
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

/* Handles the sending messages interaction according to user choice*/
void ProtocolManager::messageHandler(int choice, Client* client){
    switch (choice){
        /* Register request */
        case 110:{
            /* We make sure user is not signed in, else he cant re-register, and we set the op. */
            if (client -> getUser().has_value())    throw std::runtime_error(getPlaceHolder(PlaceHolder::USER_SIGNED_IN));
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
            if (!(client -> getUser().has_value()))    throw std::runtime_error(getPlaceHolder(PlaceHolder::USER_SIGNED_IN));
            if (!(client -> getMembers()).empty())  throw std::runtime_error(getPlaceHolder(PlaceHolder::CLIENT_LIST_REFRESH));
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
            if (!(client -> getUser().has_value()))    throw std::runtime_error(getPlaceHolder(PlaceHolder::USER_SIGNED_IN));
            uint16_t op = static_cast<uint16_t>(RequestOp::REQ_PUBLIC_KEY);
            
            
            /* Get target username from client & message */
            ClientData& it = client -> getMember();
            /* Set the header */
            setRequestHeader(client -> getUser().value().getUUID(),2,op);
            setPayloadSize(it.getUUID().size());
            
            /* Set the payload */
            payload.clear();
            std::array<uint8_t, 16> uuidBytes = it.getUUID();  
            payload.insert(payload.end(), uuidBytes.begin(), uuidBytes.end());  
            std::cout << payload.size() << std::endl;
            break;
        }
        /* Pull Awaiting Messages request */
        case 140:{
            if (!(client -> getUser().has_value()))    throw std::runtime_error(getPlaceHolder(PlaceHolder::USER_SIGNED_IN));
            if ((client -> getMembers()).empty())   throw std::runtime_error(getPlaceHolder(PlaceHolder::MEMBER_LIST_FIRST));
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
            if (!(client -> getUser().has_value()))    throw std::runtime_error(getPlaceHolder(PlaceHolder::USER_SIGNED_IN));
            uint16_t op = static_cast<uint16_t>(RequestOp::REQ_SEND_MSG_TO_USR);
            uint8_t type = static_cast<uint8_t>(MessageType::REQ_SYMMETRIC_KEY);

            /* Get target username from client & message */
            ClientData& it = client -> getMember();
            if (!it.getRSAPublicWrapper().has_value())   throw std::runtime_error(getPlaceHolder(PlaceHolder::REQ_PUBLIC_KEY_FIRST,it.getUsername()));

            /* Fix header */
            setRequestHeader(client -> getUser().value().getUUID(),2,op);
            setMessageHeader(it.getUUID(),static_cast<uint8_t>(type), static_cast<uint32_t>(0));
            setPayloadSize(payload.size());
            std::cout << payload.size() << std::endl;
            break;
        }
        /* Sending Symmetric Key (type 2)  */
        case 152:{
            if (!(client -> getUser().has_value()))    throw std::runtime_error(getPlaceHolder(PlaceHolder::USER_SIGNED_IN));
            uint16_t op = static_cast<uint16_t>(RequestOp::REQ_SEND_MSG_TO_USR);
            uint8_t type = static_cast<uint8_t>(MessageType::SEND_SYMMETRIC_KEY);

            /* Finds user in list, checks for public key, saves a symmetric. */
            ClientData& it = client -> getMember();
            if (!it.getRSAPublicWrapper().has_value()) throw std::runtime_error(getPlaceHolder(PlaceHolder::REQ_PUBLIC_KEY_FIRST,it.getUsername()));
            if (!it.getRequested()) throw std::runtime_error(getPlaceHolder(PlaceHolder::DID_NOT_REQ_SYMMETRIC,it.getUsername()));
            it.setNewSymmetric();

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
            if (!(client -> getUser().has_value()))    throw std::runtime_error(getPlaceHolder(PlaceHolder::USER_SIGNED_IN));
            uint16_t op = static_cast<uint16_t>(RequestOp::REQ_SEND_MSG_TO_USR);
            uint8_t type = static_cast<uint8_t>(MessageType::SEND_TEXT_MSG);
            
            /* Get target username from client & message */
            ClientData& it = client -> getMember();
            if (!it.getAESWrapper().has_value())   throw std::runtime_error(getPlaceHolder(PlaceHolder::REQ_SYMMETRIC_FIRST,it.getUsername()));

            std::string message;
            std::cout << getPlaceHolder(PlaceHolder::ENTER_MSG) << std::endl;
            std::getline(std::cin, message);
            if (message.size() >= std::numeric_limits<uint32_t>::max()-21)  throw std::runtime_error(getPlaceHolder(PlaceHolder::MSG_TO_LONG));
            
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
            if (!(client -> getUser().has_value()))    throw std::runtime_error(getPlaceHolder(PlaceHolder::USER_SIGNED_IN));
            uint16_t op = static_cast<uint16_t>(RequestOp::REQ_SEND_MSG_TO_USR);
            uint8_t type = static_cast<uint8_t>(MessageType::SEND_FILE);

            /* Get target username from client */
            ClientData& it = client -> getMember();
            if (!it.getAESWrapper().has_value())  if (!it.getAESWrapper().has_value())   throw std::runtime_error("Request a symmetrical key first for user "+it.getUsername());

            /* Get file path from user */
            std::cout << getPlaceHolder(PlaceHolder::ENTER_FILE_PATH) << std::endl;
            std::string file_path;
            std::getline(std::cin, file_path);
            std::ifstream file(file_path,std::ios::binary);

            /* Check the path & the size of the file! */
            if (!file)  throw std::runtime_error(getPlaceHolder(PlaceHolder::FILE_NOT_FOUND));
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
            throw std::runtime_error(getPlaceHolder(PlaceHolder::ERR_CHOOSING_OPTION));
    }

}

/* Handles the responses */
void ProtocolManager::responseHandler(Client* client){
    /* We start by clearing the payload buffer */
    payload.clear();

    /* Catch the header & Copy it to response header */
    payload = client -> receiveMessage(sizeof(ResponseHeader));
    memcpy(&responseHeader,payload.data(),sizeof(ResponseHeader));

    /* Print the header received, this is mostly for debugging. */
    printResponseHeader();

    /* We get the remainder of the payload from the socket */
    payload = client -> receiveMessage(responseHeader.payloadSize);
       
    
    switch(static_cast<ResponseOp>(responseHeader.responseOp)){
        /* Makes a new me.info file. Sets the correct UUID / User for the client. */
        case ResponseOp::RESP_REGISTER_SUCCESSFULL:{
            /* Open the me.info file */
            std::ofstream file("me.info");
            if (!file)  throw std::runtime_error(getPlaceHolder(PlaceHolder::CANT_OPEN_FILE));

            /* First row: username*/
            file << client -> getUser().value().getName() << std::endl;
            
            /* Second row: UUID*/
            /* We set a new UUID to the user */
            std::array<uint8_t, 16> newUUID;
            std::copy_n(payload.begin(), 16, newUUID.begin());
            client -> setUserUUID(newUUID);
            
            /* We parse the payload to the file */
            for (unsigned char c : payload)
                file << std::hex << std::setw(2) << std::setfill('0') << (int)c << "";
            file << std::endl;

            /* Third row: Private key in base 64*/
            std::string base64 = Base64Wrapper::encode(client -> getUser().value().getDecryptor().value().getPrivateKey());
            file << base64 << std::endl;
            file.close();

            break;
        }
        /* Saves the member list in client -> members. ClientData(username, uuid) 
            Prints the member list to the user */
        case ResponseOp::RESP_USER_LIST:{
        /* First we make constant sizes for parsing the data */
            constexpr size_t UUID_SIZE = 16;
            constexpr size_t USERNAME_SIZE = 255;
            constexpr size_t INFO_SIZE = UUID_SIZE + USERNAME_SIZE;

            /* If the payload size does not divide by 255+16 without a remainder, it means we got to much or to less data,
             and the data is missing or extra, therefor the database is corrupted!! (For ex: Username without UUID!!)*/

            if (payload.size() == 0)    throw std::runtime_error(getPlaceHolder(PlaceHolder::NO_MORE_MEMBERS));
            else if (payload.size() % INFO_SIZE != 0)
                throw std::runtime_error(getPlaceHolder(PlaceHolder::INVALID_PAYLOAD));
            
            /* We reserve number of users amount of room in the members vector & clear old data  */
            size_t numberOfUsers = payload.size() / INFO_SIZE;
            client -> getMembers().clear();

            /* For every member we extract the data and place a new object (ClientData) in the vector of members */
            /* We now pretty print the uuid / username for the client to see. */
            std::cout << YELLOW << "MEMBERS LIST" << RESET << std::endl;
            for (size_t i = 0 ; i < numberOfUsers; i++){
                size_t offset = i * INFO_SIZE;

                std::string UUID = binaryToStr(std::vector<uint8_t>(payload.begin() + offset, payload.begin() + offset + UUID_SIZE),UUID_SIZE); 
                
                std::string username(reinterpret_cast<const char*>(&payload[offset + UUID_SIZE]), USERNAME_SIZE);

                client->setMembers(UUID, username.erase(username.find_last_not_of('0') + 1));
                std::cout << YELLOW << "UUID: " << RESET << UUID
                        << YELLOW << " | Username: " << RESET << username << std::endl;
            }
            break;
        }
        /* Saves the information in client -> members . setPublicKey(key) */
        case ResponseOp::RESP_PUBLIC_KEY:{
            /* Constant sizes */
            constexpr size_t UUID_SIZE = 16;

            /* We 'grab' the string value of the UID so we can search the member list according to it */
            std::string UUID = binaryToStr(payload, UUID_SIZE);

            /* We grab the member with the same UUID */
            ClientData& user = client -> findUser(UUID);
            
            /* We store the public key for the member. When we need to send a message, we will use it. */
            std::string pubKey(payload.begin()+UUID_SIZE, payload.end());
            user.setPublic(pubKey);

            std::cout << getPlaceHolder(PlaceHolder::PUB_KEY_RECEIVED,user.getUsername()) << std::endl;
            break;
        }
        /* Just prints message to user regarding success */
        case ResponseOp::RESP_MSG_SENT_TO_USER:{
            constexpr size_t UUID_SIZE = 16;
            std::string UUID = binaryToStr(payload,UUID_SIZE);

            std::cout << getPlaceHolder(PlaceHolder::SENT_MSG_SUCCESS,(client -> findUser(UUID)).getUsername()) << std::endl;
            break;
        }
        /* Handles receiving awaiting messages list, including prompting user & parsing data */
        case ResponseOp::RESP_AWAITING_MESSAGES: {
            if (payload.size() == 0) {
                std::cout << getPlaceHolder(PlaceHolder::NO_WAITING_MSGS,client -> getUser().value().getName()) << std::endl;
                break;
            }
            constexpr size_t UUID_SIZE = 16;
            constexpr size_t MSG_ID = sizeof(uint32_t) ;
            constexpr size_t MSG_TYPE = sizeof(uint8_t);
            constexpr size_t MSG_SIZE = sizeof(uint32_t) ;
            size_t offset = 0;
            size_t totalSize = payload.size();
            while (offset < totalSize){
                if (offset + UUID_SIZE + MSG_ID + MSG_TYPE + MSG_SIZE > totalSize) {
                    std::cerr << getPlaceHolder(PlaceHolder::INCMPLT_MSG) << std::endl;
                    break;
                }  

                std::vector<unsigned char> clientID(payload.begin() + offset, payload.begin() + offset + UUID_SIZE);
                std::string stringID = binaryToStr(clientID, UUID_SIZE);
                offset += UUID_SIZE;

                std::vector<unsigned char> msgID(payload.begin() + offset, payload.begin() + offset + MSG_ID);
                offset += MSG_ID;

                uint8_t msgType = payload[offset];
                offset += MSG_TYPE;
                

                std::vector<unsigned char> msgSizeBytes(payload.begin() + offset, payload.begin() + offset + MSG_SIZE);
                offset += MSG_SIZE;
                /* We need to convert it to an int */
                uint32_t msgSize;
                std::memcpy(&msgSize, msgSizeBytes.data(), sizeof(msgSize)); 

                std::vector<unsigned char> content(payload.begin() + offset, payload.begin() + offset + msgSize);
                std::string stringcontent = binaryToStr(content, msgSize);
                offset += msgSize;
                
                std::cout << getPlaceHolder(PlaceHolder::FROM,(client -> findUser(stringID)).getUsername()) << std::endl;

                switch(msgType){
                    /* Request for symmetric key */
                    case 1:{
                        /* Finding the target user */
                        ClientData& user = client -> findUser(stringID);
                        /* Mark that he asked a symmetric (If it wasnt previuosly marked) */
                        if (!user.getRequested()) user.setRequested();
                        stringcontent = "Request for symmetric key.";
                        break;
                    }
                    /* Receiving a symmetric key */
                    case 2:{
                        /* Finding the target user */
                        ClientData& user = client -> findUser(stringID);
                        /* Decrypting the encrypted key */
                        std::string decrpytedkey = client -> getUser().value().getDecryptor().value().decrypt(std::string(content.begin(), content.end()));
                        /* Saving it for specific user */
                        user.setSymmetric(decrpytedkey);
                        /* Print response */
                        stringcontent = "Received symmetric key.";
                        break;}
                    /*Text msg */
                    case 3:{
                        /* Finding the target user */
                        ClientData& user = client -> findUser(stringID);
                        if (!user.getAESWrapper().has_value())  stringcontent="Can't decrypt message.";
                        /* Decrypting the encrypted key */
                        else stringcontent = user.getAESWrapper().value().decrypt(std::string(content.begin(), content.end()));
                        break;}
                    /* File received */
                    case 4:
                        break;
                    default:
                        break;
                }

                std::cout << "CONTENT: " << stringcontent << std::endl;
                std::cout << "----------------------------------------------------------" << std::endl;
            }
            break;
        }
         
        //case ResponseOp::RESP_GENERAL_ERROR : Handled in default!
        default:{
        /* If we get an error, and the request was to register, we need to clear the username field so we can request it again */
            if (requestHeader.requestOp == static_cast<uint16_t>(RequestOp::REQ_REGISTER) && client -> getUser().has_value())
                client -> clearUser();
            else throw std::runtime_error(getPlaceHolder(PlaceHolder::GEN_ERROR));}
    
    }


}
