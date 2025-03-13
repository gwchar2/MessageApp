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

class ResponseHeader:
    FORMAT = "<B H I" 

    def __init__(self, response_code: int, payload_size: int):
        self.version = 2  # Always 2
        self.response_code = response_code  # 2 bytes
        self.payload_size = payload_size  # 4 bytes

    def packResponse(self) -> bytes:
        return struct.pack(self.FORMAT, self.version, self.response_code, self.payload_size)

