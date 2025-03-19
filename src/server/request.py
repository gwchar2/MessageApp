import struct
import socket
import logger
import uuid
import database
import pickle
from response import Response, ResponseOp  
from datetime import datetime
from enum import IntEnum


MAX_BLOCK_SIZE = 2048
# Request Op Code
class RequestOp(IntEnum):
    REQ_REGISTER = 600
    REQ_USER_LIST = 601
    REQ_PUBLIC_KEY = 602
    REQ_MESSAGE_TO_USER = 603
    REQ_AWAITING_MESSAGES = 604
# Message Type
class MessageType(IntEnum):
    REQ_SYMMETRIC_KEY = 1
    SEND_SYMMETRIC_KEY = 2
    SEND_TEXT_MSG = 3
    SEND_FILE = 4
    
class Request:
    HEADER_FORMAT = '16s B H I'
    HEADER_SIZE = struct.calcsize(HEADER_FORMAT)

    def __init__(self,socket):
        self.socket = socket
        self.username = None
        self.UUID = None
        self.version = None
        self.OpCode = None
        self.messageType = None
        self.payload_size = None
        self.payload = None
        self.hex_data = None    
    
    # Parses the header message and inputs the data to respected holders
    def parse_header(self):
        header_data = self.receive_all(self.HEADER_SIZE)
        if not header_data:
            self.payload_size = 0
            raise ConnectionResetError("Failed to receive header data. Connection may have been closed or packet failed to arrive\n") 
        try:
            self.hex_data = logger.format_hex(header_data,1)
            self.UUID, self.version, self.OpCode ,self.payload_size, = struct.unpack(self.HEADER_FORMAT,header_data)
        except struct.error as e:
            print(f"Error: {e}")
            self.payload_size = 0

    # Helper function to receive exactly 'size' bytes
    def receive_all(self, size):
        data = bytearray()
        while len(data) < size:
            try:
                packet = self.socket.recv(size - len(data))
                if not packet:
                    raise ConnectionError("Connection closed prematurely by the server.")
                data.extend(packet)
                print(f"Received {len(data)}/{size} Bytes")
            except ConnectionError as e:
                print(f"Error: {e}")
                return;
        return data 

    def __str__(self):
        # Parse the header
        self.parse_header()
        header = (f"Request Received: \n"
                f"{self.hex_data}\n"
                f"UUID: {logger.format_hex(self.UUID)}\n"
                f"Version: {self.version}\n"
                f"Request: {self.OpCode}")

        # Construct the full response string and return it
        return (f"{header}\n"
                f"Data Size: {self.payload_size} Bytes")
                #f"{payload_received}\n")

    # Receives OpCode, but messagetype can be none
    def handle_request(self):
        try:    # We handle all errors from all requests in this try-catch, sending general error for everything.
            # Update the database each time we print the header. 
            match self.OpCode:
                case RequestOp.REQ_REGISTER:
                    self.registerRequest()
                case RequestOp.REQ_USER_LIST:
                    self.userlistRequest()
                case RequestOp.REQ_PUBLIC_KEY:
                    self.publicKeyRequest()
                case RequestOp.REQ_MESSAGE_TO_USER:
                    self.messageToUserRequest()
                case RequestOp.REQ_AWAITING_MESSAGES:
                    self.collectMsgsRequest()

        # If we have an error from any case, we parse it for debugging & Send general error to user
        except Exception as e:
            print(f"[Error] parsing request {self.OpCode}: {e}")  
            response = Response(ResponseOp.RESP_GENERAL_ERROR, 0)
            self.socket.recv(4096) # We 'flush' the socket
            self.socket.sendall(response.build_message())
            print(response)
            
    # Handles the registration of a new user 
    def registerRequest(self):
        # Grabbing username
        username = self.receive_all(255).decode('utf-8')
        readable_name = username.rstrip('\x00').rstrip('0')
        
        #Creating random UUID
        rndUUID = uuid.uuid4().bytes
        print(rndUUID)
        #Checking that the UUID and username are unique
        id_exists, username_exists = database.userCheck(rndUUID,username)

        # if the ID exists, we keep creating a new one.
        if id_exists:
            rndUUID = uuid.uuid4().bytes
            id_exists, username_exists = userCheck(rndUUID,username)

        # If username exists, we raise error to prompt for a new one!
        if username_exists:
            raise ValueError("Username already exists!! Send general error")

        # Grabbing public key and registering 
        publicKey = self.receive_all(160)
        database.register_user(rndUUID,username,publicKey,datetime.now().isoformat())
        
        # Building and sending the response
        response = Response(
            responseOp=ResponseOp.RESP_REGISTER_SUCCESSFULL,
            payloadSize=len(rndUUID),
            clientID=rndUUID
        )
        message = response.build_message()
        self.socket.sendall(message)
        print(f"Registering:\n"
              f"Username: {readable_name}\n"
              f"UUID: {logger.format_hex(rndUUID)}\n"
              f"Public Key: {logger.format_hex(publicKey)}\n")

    # Handles the request for the full user list 
    def userlistRequest(self):
        # We check that our UUID exists, else we won't have a proper response! Raise error if it doesnt.
        id_exists = database.userCheck(self.UUID)
        if id_exists is None:
            raise ValueError(f"Such UUID {logger.format_hex(self.UUID)} Does not exist!")
        print(f"Sending user list to {logger.format_hex(self.UUID)}")
        database.updateLastSeen(self.UUID)

        # We grab the user list from the database.
        user_list = database.getAllUsers(self.UUID)
        for uuid, user in user_list:
            print(f"{[(logger.format_hex(uuid),user.rstrip('0'))]}")
        
        # Building and sending the response, if there are no members we send None
        user_dump = b""
        if user_list:
            user_dump = b"".join(
                uuid + user.encode('utf-8').ljust(255, b'\x00')
                for uuid, user in user_list)

        response = Response(
            responseOp=ResponseOp.RESP_USER_LIST,
            payloadSize=len(user_dump))
        message = response.build_message(user_dump)
        self.socket.sendall(message)

    # Handles the request for public key
    def publicKeyRequest(self):
        target_uuid = self.receive_all(self.payload_size)
        # We check that target UUID exists, else we won't have a proper response! Raise error if it doesnt.
        id_exists = database.userCheck(target_uuid)
        if id_exists is None:
            raise ValueError(f"Such UUID {logger.format_hex(target_uuid)} Does not exist!")
        # Update last seen!
        database.updateLastSeen(self.UUID)

        # We grab the public key of target UUID
        publicKey = database.getPublicKey(target_uuid)
        print(f"Sending public key of:\n"  
            f"  Target: {target_uuid}\n"    
            f"  Asker: {logger.format_hex(self.UUID)}\n"
            f"  Target Public Key: {logger.format_hex(publicKey)}")
        
        # Building and sending the response
        response = Response(
            responseOp=ResponseOp.RESP_PUBLIC_KEY,
            payloadSize=len(publicKey) + len(target_uuid),
            clientID=target_uuid,
            publicKey=publicKey
        )
        message = response.build_message()
        self.socket.sendall(message)

    # Handles sending a message to a user 
    def messageToUserRequest(self):
        # We get the "header" of the payload, and unpack the data of it.
        PAYLOAD_HEADER_FORMAT = '=16s B I'
        PAYLOAD_HEADER_SIZE = struct.calcsize(PAYLOAD_HEADER_FORMAT)
        payload_header_data = self.receive_all(PAYLOAD_HEADER_SIZE)
        target_UUID, msg_type, content_size = struct.unpack(PAYLOAD_HEADER_FORMAT, payload_header_data)
        content = None

        # We check that target UUID exists, else we won't have a proper response! Raise error if it doesnt.
        id_exists = database.userCheck(target_UUID)
        if id_exists is None:
            raise ValueError(f"No such UUID {logger.format_hex(target_UUID)}")
        # Update last seen!
        database.updateLastSeen(self.UUID)

        print(f"Message Header:\n{logger.format_hex(payload_header_data, 1)}")
        print(f"Sending message to user {logger.format_hex(target_UUID)}")

        # Handle sending of symmetric key
        if msg_type != MessageType.REQ_SYMMETRIC_KEY:
            content = self.receive_all(content_size)
        
        # We send a message to target UUID, and for confirmation we get the specific ID from table.
        messageID = database.sendMessageToTarget(target_UUID,self.UUID,msg_type,content)
        # Building and sending the response
        response = Response(
            responseOp=ResponseOp.RESP_MSG_SENT_TO_USER,
            payloadSize=len(target_UUID) + len(messageID),
            clientID=target_UUID,
            messageID=messageID
        )
        message = response.build_message()
        self.socket.sendall(message)
    
    # Collets all the messages on the server for a specific user 
    def collectMsgsRequest(self):
        # We check that our UUID exists, else we won't have a proper response! Raise error if it doesnt.
        id_exists = database.userCheck(self.UUID)
        if id_exists is None:
            raise ValueError(f"No such UUID {logger.format_hex(self.UUID)}")
        # Update last seen!
        database.updateLastSeen(self.UUID)
        
        messages = database.getAllMessages(self.UUID)
        print(f"Messages Retreived: \n"
              f"{messages}")
            
        if not messages:
            byte_msg = b''
        else:
            # We parse into bytes
            for msg_id, from_client, msg_type, content in messages:
                content = content if content else b'' 
                #We make sure the content is binary.
                # We parse the messages according to header, since its different than the return from the database.
                # We pay attention that we do not need to send as little endian, since this is payload. 
                byte_msg = (
                    from_client +                          
                    struct.pack("I", msg_id) +            
                    struct.pack("B", msg_type) +         
                    struct.pack("I", len(content)) +      
                    content)
        
        # Building and sending the response
        response = Response(
            responseOp=ResponseOp.RESP_AWAITING_MESSAGES,
            payloadSize=len(byte_msg))
        message = response.build_message(byte_msg)
        print(f"Message: \n{message}")
        self.socket.sendall(message)


