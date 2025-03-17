import struct
from enum import IntEnum

MAX_BLOCK_SIZE = 2048

class ResponseOp(IntEnum):
    RESP_REGISTER_SUCCESSFULL = 2100
    RESP_USER_LIST = 2101
    RESP_PUBLIC_KEY = 2102
    RESP_MSG_SENT_TO_USER = 2103
    RESP_AWAITING_MESSAGES = 2104
    RESP_GENERAL_ERROR = 9000

class Response:
    RESPONSE_HEADER = "<B H I" 

    def __init__(self, responseOp : ResponseOp, payloadSize, clientID = None, clientName = None, publicKey = None): 
        self.version = 2                        # Always 2
        self.op = responseOp                    # 2 bytes
        self.payload_size = payloadSize         # 4 bytes
        self.clientID = clientID
        self.username = clientName
        self.public_key = publicKey
        

    def pack_header(self) -> bytes:
        return struct.pack(self.RESPONSE_HEADER, self.version, self.op.value, self.payload_size)

    def build_message(self, payload=None):
        header = self.pack_header()
        try:
            if self.op == ResponseOp.RESP_REGISTER_SUCCESSFULL and self.clientID:
                return header + self.clientID
            
            elif self.op == ResponseOp.RESP_USER_LIST and payload:
                return header + payload
            
            elif self.op == ResponseOp.RESP_PUBLIC_KEY and self.clientID and self.public_key:
                return header + self.clientID + self.public_key
            
            elif self.op == ResponseOp.RESP_MSG_SENT_TO_USER and self.clientID:
                return header + self.clientID
            
            elif self.op == ResponseOp.RESP_AWAITING_MESSAGES and payload:
                return header + payload
            
            elif self.op == ResponseOp.RESP_GENERAL_ERROR:
                return header
            
        except Exception as e:
            print(f"Error occurred while creating a message: {e}")
            return header
    
    def __str__(self):
       return ( f"Version: {self.version}\n"
                f"Op: {self.op}\n"
                f"Payload Size: {self.payload_size}\n"
                f"Total Size: {self.payload_size}\n"
                f"Target ID: {self.clientID}\n"
                f"Target Username: {self.username}\n"
                f"Public Key: {self.public_key}")
