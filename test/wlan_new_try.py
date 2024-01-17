import socket
import threading

# Set the server address and port
server_address = ('', 8888)

# Create a socket for the server
server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server_socket.bind(server_address)
server_socket.listen(5)

# Get the server's IP address
server_ip = socket.gethostbyname(socket.gethostname())
print(f"Server listening on {server_ip}:{server_address[1]}")

# Function to handle client connections
def handle_client(client_socket, client_address):
    print(f"Accepted connection from {client_address}")

    try:
        while True:
            data = client_socket.recv(1024)
            if not data:
                break
            print(f"Received from {client_address}: {data.decode()}")

            # Send a response back to the client
            response = "Message received by the server"
            client_socket.sendall(response.encode())

    except Exception as e:
        print(f"Error handling client {client_address}: {e}")
    finally:
        client_socket.close()
        print(f"Connection with {client_address} closed")

# Function to listen for incoming connections
def listen_for_clients():
    while True:
        client_socket, client_address = server_socket.accept()
        client_handler = threading.Thread(target=handle_client, args=(client_socket, client_address))
        client_handler.start()

if __name__ == "__main__":
    try:
        listen_for_clients()
    except KeyboardInterrupt:
        print("\nServer script terminated.")
    finally:
        server_socket.close()
