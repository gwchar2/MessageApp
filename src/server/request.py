import struct
import socket
import logger
import uuid
import database
import pickle
from datetime import datetime
from enum import IntEnum

MAX_BLOCK_SIZE = 2048

class RequestOp(IntEnum):
    REQ_REGISTER = 600
    REQ_USER_LIST = 601
    REQ_PUBLIC_KEY = 602
    REQ_MESSAGE_TO_USER = 603
    REQ_AWAITING_MESSAGES = 604

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

    # Parses all the payload
    def parse_payload(self):
        if self.payload is None and self.payload_size > 0:
            data = bytearray()
            while len(data) < self.payload_size:
                try:
                    packet = self.socket.recv(self.payload_size - len(data))
                    if not packet:
                        raise ConnectionError("Connection closed prematurely by the server.")
                    data.extend(packet)
                    print(f"Received {len(data)}/{self.payload_size} Bytes")
                except ConnectionError as e:
                    print(f"Error: {e}")
                    return;
            self.payload = data 

    def get_payload(self):
        return self.payload
    
    def reset_payload(self):
        self.payload = None

    def get_UUID(self):
        return self.UUID
    
    def getPayloadSize(self):
        return self.payload_size
    
    def getOp(self):
        return self.requestOp
    
    # Prints the self.data field
    def print_payload(self):
        try:
            decoded_data = self.get_payload().decode('utf-8') if self.get_payload() is not None else "No data"
            return decoded_data
        except UnicodeDecodeError:
            return f"Raw data (hex): {self.get_payload().hex()}"

    # Prints the self.UUID field
    def print_UUID(self):
        try:
            decoded_data = self.get_UUID().decode('utf-8') if self.get_UUID() is not None else "No Name"
            return decoded_data
        except UnicodeDecodeError:
            return f"Raw data (hex): {self.get_UUID().hex()}"

    def __str__(self):
        # Parse the header
        self.parse_header()
        header = (f"Request Received: \n"
                f"{self.hex_data}\n"
                f"UUID: {logger.format_hex(self.UUID)}\n"
                f"Version: {self.version}\n"
                f"Request: {self.OpCode}")
        
        #self.parse_payload()
        #payload_received = self.print_payload()

        # Construct the full response string and return it
        return (f"{header}\n"
                f"Data Size: {self.payload_size} Bytes")
                #f"{payload_received}\n")

    # Receives OpCode, but messagetype can be none
    def handle_request(self):
        match self.OpCode:
            # req 600
            case RequestOp.REQ_REGISTER:
                self.registerRequest()
            # req 601
            case RequestOp.REQ_USER_LIST:
                print("Fetching user list")
                self.userlistRequest()
            # req 602
            case RequestOp.REQ_PUBLIC_KEY:
                print("Fetching public key")
                self.publicKeyRequest()
            # req 603
            case RequestOp.REQ_MESSAGE_TO_USER:
                # Need to parse message header
                # (page 8)
                #Need to fill here func to get message type!!!
                #Parse message type from here on!!
                if self.messageType is None:
                    print("Error: message_type required for REQ_MESSAGE_TO_USER")
                    return
                
                match self.messageType:
                    # type 1
                    case MessageType.REQ_SYMMETRIC_KEY:
                        print("Handling request for symmetric key")
                    # type 2
                    case MessageType.SEND_SYMMETRIC_KEY:
                        print("Handling sending of symmetric key")
                    # type 3
                    case MessageType.SEND_TEXT_MSG:
                        print("Handling sending of text message")
                    # type 4
                    case MessageType.SEND_FILE:
                        print("Handling file transfer")
                    #default
                    case _:
                        print("Unknown message type")

            # req 604
            case RequestOp.REQ_AWAITING_MESSAGES:
                self.collectMsgsRequest()
                print("Checking awaiting messages")

            case _:
                print("Unknown request operation")


    def registerRequest(self):
        # Grabbing username
        username = self.receive_all(255).decode('utf-8')
        readable_name = username.rstrip('\x00').rstrip('0')
        
        #Creating random UUID
        rndUUID = uuid.uuid4().bytes
        #Checking that the UUID and username are unique
        id_exists, username_exists = database.userCheck(rndUUID,username)

        # While the ID exists, we keep creating a new one.
        while (id_exists):
            rndUUID = uuid.uuid4().bytes
            id_exists, username_exists = userCheck(rndUUID,username)

        #if (username_exists)
            #send general error

        # Grabbing public key and registering 
        publicKey = self.receive_all(160)
        database.register_user(rndUUID,username,publicKey,datetime.now().isoformat())
        
        print(f"Registering:\n"
              f"Username: {readable_name}\n"
              f"UUID: {logger.format_hex(rndUUID)}\n"
              f"Public Key: {logger.format_hex(publicKey)}\n")

    def userlistRequest(self):
        id_exists = database.userCheck(self.UUID)
        if (id_exists):
            print(f"Sending user list to {logger.format_hex(self.UUID)}")
        #else:
            #send error

        user_list = database.getAllUsers(self.UUID)
        user_dump = pickle.dumps(user_list)
        for uuid, user in user_list:
            print(f"{[(logger.format_hex(uuid),user.rstrip('0'))]}")
      
        # need to send user list 

    def publicKeyRequest(self):
        target_uuid = self.receive_all(16)
        publicKey = database.getPublicKey(target_uuid)
        printf(f"Sending public key of:\n"  
            f"  Target: {logger.format_hex(target_uuid)}\n"    
            f"  Asker: {logger.format_hex(self.UUID)}\n"
            f"  Target Public Key: {logger.format_hex(publicKey)}")


    def messageToUserRequest(self):
        PAYLOAD_HEADER_FORMAT = '16s B I'
        PAYLOAD_HEADER_SIZE = struct.calcsize(PAYLOAD_HEADER_FORMAT)

        payload_header_data = self.receive_all(PAYLOAD_HEADER_SIZE)
        target_UUID, msg_type, content_size = struct.unpack(PAYLOAD_HEADER_FORMAT, payload_header_data)
        content = None
        print(f"Message Header:\n{logger.format_hex(payload_header_data, 1)}")

        # Handle sending of symmetric key
        if msg_type != MessageType.REQ_SYMMETRIC_KEY:
            content = self.receive_all(content_size)
        
        try:
            database.sendMessageToTarget(target_UUID,self.UUID,msg_type,content)
        except RuntimeError:
            #Send general error
            print(f"Send general error")
    
    def collectMsgsRequest(self):
        messages = database.getAllMessage(self.UUID)
        print(f"Messages Retreived: \n"
            f"{logger.format_hex(messages)}")


