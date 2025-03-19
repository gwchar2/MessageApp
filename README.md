# MessageApp
A secure client-server messaging application using asymmetric and symmetric encryption.

## Table of Contents
- [Introduction](#introduction)
- [Features](#features)
- [File Structure](#file-structure)
- [Installation](#installation)
- [Usage](#usage)
  - [Server Actions](#server-actions)
  - [Client Actions](#client-actions)
- [Secure Communication Process](#secure-communication-process)
- [Protocol Specifications](#protocol-specifications)
  - [Requests from Client to Server](#requests-from-client-to-server)
  - [Responses from Server](#responses-from-server)
- [Encryption Details](#encryption-details)
- [Database Schema](#database-schema)
- [Diagrams](#diagrams)
- [License](#license)

## Introduction
MessageApp is a client-server-based secure messaging system. It uses **RSA (asymmetric encryption)** for key exchange and **AES-CBC (symmetric encryption)** for encrypting messages. The system supports user registration, message exchange, and secure key exchange.

## Features
- Secure **client-server communication**
- User **registration with UUIDs**
- **Public key exchange** for encrypted messaging
- Secure **symmetric key generation and storage**
- **Message queueing and retrieval**
- Supports **text messages and file transfers**

## File Structure
```
/MessageApp
 -/src 
      -/client 
             -/src
               -/ include 
                  - AESWrapper.h
                  - RSAWrapper.h
                  - Client.h
                  - Helpers.h
                  - ProtocolManager.h
                  - User.h
               -/client
                  - client.cpp
                  - helpers.cpp
                  - protocolhandler.cpp
                  - user.cpp
               -/encryption
                  - AESWrapper.cpp
                  - RSAWrapper.cpp
      -/server
            - database.py
            - logger.py
            - request.py
            - response.py
            - server.py
```

## Installation
### 1. Clone the Repository
```sh
git clone https://github.com/gwchar2/MessageApp.git
cd MessageApp
```

### 2. Ensure Python is Installed and Install Dependencies
- Check if Python is installed:
  ```sh
  python3 --version
  ```
  If Python is not installed, download and install it from [python.org](https://www.python.org/downloads/).

- Install required dependencies:
  ```sh
  python3 -m pip install --upgrade pip
  python3 -m pip install -r requirements.txt
  ```

### 3. Ensure GCC is Installed and Compile the Client & Encryption Modules
- Check if GCC is installed:
  ```sh
  gcc --version
  ```
  If GCC is not installed, install it:
  - **Linux (Debian/Ubuntu):** `sudo apt install build-essential`
  - **MacOS:** `brew install gcc`
  - **Windows:** Install [MinGW](https://osdn.net/projects/mingw/) and add it to the system PATH.

- Compile the modules:
  ```sh
  make
  ```

### 4. Start the Server and Client
- Start the server:
  ```sh
  python src/server/server.py
  ```
- Start the client:
  ```sh
  ./client
  ```
  
## Usage

### Server Actions
1. Reads port from `myport.info`
2. Waits indefinitely for client requests
3. Responds to various client requests:
   - **Sign up**: Creates a new user with a UUID (if username does not exist).
   - **Client list**: Returns a list of registered users.
   - **Send message**: Stores a message in memory for retrieval.
   - **Waiting messages**: Delivers queued messages and deletes them after retrieval.

### Client Actions
1. Reads **server and port** from `server.info`
2. Reads and stores **username, UUID, and encryption key** from `me.info`
3. Displays an interactive **terminal interface** for user actions

### User Terminal Options
- **Register User (Request 110)** - Registers and saves UUID.
- **Request User List (Request 120)** - Fetches all users.
- **Request Public Key (Request 130)** - Fetches a specific user's public key.
- **Request Waiting Messages (Request 140)** - Fetches unread messages.
- **Send Message (Request 150)** - Sends a text message.
- **Request Symmetric Key (Request 151)** - Fetches stored symmetric key.
- **Send Symmetric Key (Request 152)** - Generates and sends a new symmetric key.
- **Send a File (Request 153)** - Generates and sends a new symmetric key.

## Secure Communication Process
1. **Client B requests Client A’s public key from the server.**
2. **Client B sends a request to Client A** (via the server) for a **symmetric encryption key**, encrypted using Client A’s public key.
3. **The server stores the request** for Client A to retrieve later.
4. **Client A pulls the waiting request from the server.**
5. **Client A decrypts the request** using its **private key.**
6. **Client A requests Client B’s public key from the server.**
7. **Client A sends a response** with the **symmetric encryption key**, encrypted with B’s public key.
8. **The server stores the response** for Client B to retrieve.
9. **Client B pulls the response from the server.**
10. **Client B decrypts the symmetric key** using its **private key.**
11. **Both clients can now securely communicate using the shared symmetric key.**

## Protocol Specifications

### Requests from Client to Server
| Request Code | Description |
|-------------|-------------|
| 600 | Sign up |
| 601 | Get members list |
| 602 | Get a specific user's public key |
| 603 | Send a message |
| 604 | Pull waiting messages |

### Responses from Server
| Response Code | Description |
|--------------|-------------|
| 2100 | Sign up successful |
| 2101 | Members list |
| 2102 | Public key |
| 2103 | Message stored |
| 2104 | All waiting messages |
| 9000 | General error |

## Encryption Details
- **Symmetric Encryption**: AES-CBC (128-bit key)
- **Asymmetric Encryption**: RSA (1024-bit key without header, 1280-bit with header)

## Database Schema
For database setup, refer to [database_schema.sql](files/database_schema.sql).
```sql
CREATE TABLE IF NOT EXISTS clients (
    ID BLOB(16) PRIMARY KEY,
    UserName VARCHAR(255) NOT NULL,
    PublicKey BLOB(160) NOT NULL,
    LastSeen DATETIME NOT NULL
);

CREATE TABLE IF NOT EXISTS messages (
    ID INTEGER PRIMARY KEY AUTOINCREMENT,
    ToClient BLOB(16) NOT NULL,
    FromClient BLOB(16) NOT NULL,
    Type TINYINT NOT NULL,
    Content BLOB,
    FOREIGN KEY (ToClient) REFERENCES clients(ID),
    FOREIGN KEY (FromClient) REFERENCES clients(ID)
);
```

## Diagrams
Below are diagrams that illustrate the message exchange process:

### Client A, Client B, and Server Communication Flow
![Client Communication Process](https://raw.githubusercontent.com/gwchar2/MessageApp/refs/heads/main/images/client_communication.JPG)

### End-to-End Encryption Process
![Encryption Flow](https://raw.githubusercontent.com/gwchar2/MessageApp/refs/heads/main/images/encryption_diagram.JPG)

## License
This project is licensed under the MIT License.
