import socket
import threading
import selectors
import os
import database
import request
import struct
from database import initialize_database 
from request import Request  

sel = selectors.DefaultSelector()


# Read the server port from myport.info
def get_server_info():
    filename = os.path.join(os.path.dirname(__file__), "myport.info")
    with open(filename, "r") as file:
        port = file.readline().strip()
        return int(port)



#Initiates the server & DB
def start_server():
    # Intialize IP (local host) & Database
    HOST = "127.0.0.1"
    PORT = get_server_info()  
    initialize_database()         

    # Connect to sockets & Set blocking to false   
    server_socket = socket.socket()
    server_socket.bind((HOST, PORT))
    server_socket.listen()
    server_socket.setblocking(False)
    sel.register(server_socket, selectors.EVENT_READ, accept_client)
    
    print(f"[LISTENING] Server is listening on Port {PORT}...")
    try:
        while True:
            events = sel.select()
            for key, _ in events:
                callback = key.data
                callback(key.fileobj)
    except KeyboardInterrupt:
        print("\n[INFO] Server shutting down...")
    finally:
        sel.close()

# Accepts clients and handles them in different selector 
def accept_client(server_socket):
    client_socket, client_address = server_socket.accept()
    print(f"[NEW CONNECTION] {client_address} connected.")
    client_socket.setblocking(False)
    sel.register(client_socket, selectors.EVENT_READ, handle_client)

# Main client-server function
def handle_client(client_socket):
    try:
        # Accept a new request 
        request = Request(client_socket)
        if request:
            # If request is valid, print the info and handle it.
            print(request)
            request.handle_request()
        
    except ConnectionResetError as e:
        print(f"[DISCONNECTED] Client lost connection: {e}")
        sel.unregister(client_socket)

    except Exception as e:
        print(f"[DISCONNECTED] Client lost connection.")
        disconnect_client(client_socket)

# Formal disconnection
def disconnect_client(client_socket):
    print(f"[CONNECTION CLOSED] {client_socket.getpeername()} disconnected.")
    sel.unregister(client_socket)
    client_socket.close()

if __name__ == "__main__":
    start_server()
        
