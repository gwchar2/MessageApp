import socket
import threading
import os

# Read the server port from myport.info
def get_server_info():
    filename = os.path.join(os.path.dirname(__file__), "myport.info")
    with open(filename, "r") as file:
        port = file.readline().strip()
        return int(port)

# Handle client requests
def handle_client(client_socket, client_address):
    print(f"[NEW CONNECTION] {client_address} connected.")

    while True:
        try:            # Receive data from the client
            data = client_socket.recv(1024)
            if not data:
                break  # Client disconnected

            print(f"[RECEIVED] From {client_address}:\n {data}")

            # Send a response back
            client_socket.sendall(data)
            
            print(f"Sent : {data}")
        except ConnectionResetError:
            print(f"[DISCONNECTED] {client_address} lost connection.")
            break

    client_socket.close()
    print(f"[CONNECTION CLOSED] {client_address} disconnected.")

# Start the server
def start_server():
    port = get_server_info()
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_socket.bind(("0.0.0.0", port))
    server_socket.listen()

    print(f"[LISTENING] Server is listening on port {port}...")

    while True:
        client_socket, client_address = server_socket.accept()
        client_thread = threading.Thread(target=handle_client, args=(client_socket, client_address))
        client_thread.start()

        print(f"[ACTIVE CONNECTIONS] {threading.active_count() - 1}")

if __name__ == "__main__":
    start_server()
