import socket

# ESP8266 server details
esp8266_ip = "172.20.10.6"  # Replace with the actual IP address of your ESP8266
esp8266_port = 8888  # Replace with the port number your ESP8266 is listening on

# Create a socket
client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

# Connect to the ESP8266 server
client_socket.connect((esp8266_ip, esp8266_port))
print("Connected to ESP8266 server")

try:
    while True:
        # Send data to ESP8266
        message = "Hello from Python!"
        client_socket.sendall(message.encode())

        # Receive data from ESP8266
        data = client_socket.recv(1024)
        if not data:
            break
        print("Received:", data.decode())

except KeyboardInterrupt:
    pass

finally:
    # Close the socket
    client_socket.close()
    print("Socket closed")
