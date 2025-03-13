import socket
import threading
import selectors
import os
import database
import request
import struct
from database import initialize_database 
from request import Request  

# Read the server port from myport.info
def get_server_info():
    filename = os.path.join(os.path.dirname(__file__), "myport.info")
    with open(filename, "r") as file:
        port = file.readline().strip()
        return int(port)


sel = selectors.DefaultSelector()


def start_server():
    HOST = "0.0.0.0"
    PORT = get_server_info()  
    initialize_database()  
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


def accept_client(server_socket):
    client_socket, client_address = server_socket.accept()
    print(f"[NEW CONNECTION] {client_address} connected.")
    client_socket.setblocking(False)
    sel.register(client_socket, selectors.EVENT_READ, handle_client)


def handle_client(client_socket):
    try:
        request = Request(client_socket)
        if request is not None:
            print(request)
            request.handle_request()
        
        
        # Send response back (for example purposes, echoing back)
        #client_socket.sendall(data)
    except ConnectionResetError as e:
        print(f"[DISCONNECTED] Client lost connection: {e}")
        sel.unregister(client_socket)

    except Exception as e:
        print(f"[DISCONNECTED] Client lost connection.")
        disconnect_client(client_socket)


def disconnect_client(client_socket):
    sel.unregister(client_socket)
    client_socket.close()
    print(f"[CONNECTION CLOSED] {client_socket.getpeername()} disconnected.")

if __name__ == "__main__":
    start_server()
        
