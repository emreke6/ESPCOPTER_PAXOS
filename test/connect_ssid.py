import socket

def wait_for_connection():
    server_ip = '172.20.10.2'  # Change this to the desired IP address
    server_port = 8080

    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_socket.bind((server_ip, server_port))
    server_socket.listen(1)

    print(f"Server is running on {server_ip}:{server_port}")
    print(f"Waiting for ESP8266 to connect...")

    while True:
        try:
            client_socket, client_address = server_socket.accept()
            print(f"ESP8266 connected from {client_address}")

            esp_ip = client_address[0]
            print(f"ESP8266 IP address: {esp_ip}")
            print(f"Server IP address: {server_ip}")

            # Receive and print messages from ESP8266
            while True:
                message = str(input("Write what you want to send: "))
                client_socket.send(message.encode('utf-8'))
                if not message:
                    break  # No more data, close the connection
                
            print("Connection closed by ESP8266.")
            client_socket.close()
            break
        except KeyboardInterrupt:
            print("Server stopped by user.")
            break
        except Exception as e:
            print(f"Error: {e}")
            client_socket.close()
            break

    server_socket.close()

if __name__ == "__main__":
    wait_for_connection()
