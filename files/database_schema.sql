-- Database schema for MessageApp

-- Clients table to store registered users
CREATE TABLE IF NOT EXISTS clients (
    ID BLOB(16) PRIMARY KEY,
    UserName VARCHAR(255) NOT NULL,
    PublicKey BLOB(160) NOT NULL,
    LastSeen DATETIME NOT NULL
);

-- Messages table to store messages exchanged between clients
CREATE TABLE IF NOT EXISTS messages (
    ID INTEGER PRIMARY KEY AUTOINCREMENT,
    ToClient BLOB(16) NOT NULL,
    FromClient BLOB(16) NOT NULL,
    Type TINYINT NOT NULL,
    Content BLOB,
    FOREIGN KEY (ToClient) REFERENCES clients(ID),
    FOREIGN KEY (FromClient) REFERENCES clients(ID)
);
