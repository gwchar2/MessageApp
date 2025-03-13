import sqlite3

# Initializes the database 
def initialize_database():
    conn = sqlite3.connect("defensive.db");
    cursor = conn.cursor()

    # Makes the client table. ID is primary key, the rest can not be null. 
    cursor.execute("""
            CREATE TABLE IF NOT EXISTS clients (
                ID BLOB(16) PRIMARY KEY,
                UserName VARCHAR(255) NOT NULL,
                PublicKey BLOB(160) NOT NULL,
                LastSeen DATETIME NOT NULL
            )""")
    
    # Makes the messages table, while making sure that ToClient ID and FromClient ID exist in database!
    # Type is TINYINT, we will store only 1 byte, so it will be saved as 1 byte in the memory. Better than saving an INT.
    cursor.execute("""
            CREATE TABLE IF NOT EXISTS messages (
                ID INTEGER PRIMARY KEY AUTOINCREMENT,
                ToClient BLOB(16) NOT NULL,
                FromClient BLOB(16) NOT NULL,
                Type TINYINT NOT NULL,
                Content BLOB NOT NULL,
                FOREIGN KEY (ToClient) REFERENCES clients(ID),
                FOREIGN KEY (FromClient) REFERENCES clients(ID)
            )""")

    conn.commit()
    conn.close()

# Inserts a user while checking if it already exists
def register_user(ID: bytes, username: str, publicKey: bytes, lastSeen: str):
    conn = None
    try:
        conn = sqlite3.connect("defensive.db")
        cursor = conn.cursor()
        cursor.execute("INSERT INTO clients (ID, UserName, PublicKey, LastSeen) VALUES (?, ?, ?, ?)", 
                    (ID, username, publicKey, lastSeen))
        conn.commit()
        conn.close()
        return 0
    except sqlite3.Error as e:
        raise RuntimeError(f"Database error: {e}")

    finally:
        if conn:
            conn.close()
    
# Sends a message to a client
def sendMessageToTarget(ToClient: bytes, FromClient: bytes, Type : bytes, Content : bytes = None):
    conn = None
    try:
        conn = sqlite3.connect("defensive.db")
        cursor = conn.cursor()

        cursor.execute("INSERT INTO messages (ToClient, FromClient, Type, Content) VALUES (?, ?, ?, ?)",
             (ToClient, FromClient, Type, Content))

        if cursor.rowcount <= 0:
            raise RuntimeError("Insertion failed: No rows were inserted.")

        conn.commit()
        return True  

    except sqlite3.Error as e:
        raise RuntimeError(f"Database error: {e}")

    finally:
        if conn:
            conn.close()

def getUUID(Username: str):
    conn = None
    try:
        conn = sqlite3.connect("defensive.db")
        cursor = conn.cursor()
        cursor.execute("SELECT UserName FROM clients WHERE UserName = ?", (UserName,))

        result = cursor.fetchone()
        conn.close()
        return result[0] if result else None
    except sqlite3.Error as e:
        raise RuntimeError(f"Database error: {e}")

    finally:
        if conn:
            conn.close()

# Searches for specific client ID, pulls the public key 
def getPublicKey(ID: bytes):
    conn = None
    try:
        conn = sqlite3.connect("defensive.db")
        cursor = conn.cursor()

        cursor.execute("SELECT PublicKey FROM clients WHERE ID = ?", (ID,))
        result = cursor.fetchone()

        conn.close()
        return result[0] if result else None
    except sqlite3.Error as e:
        raise RuntimeError(f"Database error: {e}")

    finally:
        if conn:
            conn.close()

def getUsername(ID: bytes):
    try:      
        conn = sqlite3.connect("defensive.db")
        cursor = conn.cursor()

        cursor.execute("SELECT UserName FROM clients WHERE ID = ? ", (ID,))
        result = cursor.fetchone()

        conn.close()
        return result[0] if result else None
    except sqlite3.Error as e:
        raise RuntimeError(f"Database error: {e}")

    finally:
        if conn:
            conn.close()
    
# Returns a list of all messages for a certain client ID
def getAllMessages(ID: bytes):
    try:
        conn = sqlite3.connect("defensive.db")
        cursor = conn.cursor()

        cursor.execute("SELECT ID, FromClient, Type, Content FROM messages WHERE ToClient = ?" ,(ID,))
        messages = cursor.fetchall()

        conn.close()
        return messages
    except sqlite3.Error as e:
        raise RuntimeError(f"Database error: {e}")

    finally:
        if conn:
            conn.close()

def getAllUsers(ID: bytes) -> list[tuple[bytes,str]]:
    try:
        conn = sqlite3.connect("defensive.db")
        cursor = conn.cursor()

        cursor.execute("SELECT ID, UserName FROM clients WHERE ID != ?",(ID,))
        users = cursor.fetchall()

        conn.close()
        return users;
    except sqlite3.Error as e:
        raise RuntimeError(f"Database error: {e}")

    finally:
        if conn:
            conn.close()

# We check for ID / Username, so we know what prompt to send to user. 
# If its ID we need to recreate! Else we send general error.
# We dont always check for username, some requests come without!
def userCheck(ID: bytes, UserName : str = None) -> tuple[bool,bool | None]:
    try:
        conn = sqlite3.connect("defensive.db")
        cursor = conn.cursor()

        cursor.execute("SELECT 1 FROM clients WHERE ID = ?", (ID,))
        id_exists = cursor.fetchone() is not None  

        if UserName is not None:
            cursor.execute("SELECT 1 FROM clients WHERE UserName = ?", (UserName,))
            username_exists = cursor.fetchone() is not None  
        else:
            username_exists = None

        conn.close();

        return id_exists,username_exists;

    except sqlite3.Error as e:
        raise RuntimeError(f"Database error: {e}")

    finally:
        if conn:
            conn.close()

