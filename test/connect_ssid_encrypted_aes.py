import socket
import random
from Crypto.Cipher import AES
import secrets

def encrypt_with_AES(MSG_TO_SEND):
    nonce_num = pow(2,63) + 1
    nonce_bytes = nonce_num.to_bytes((nonce_num.bit_length()+7)//8, byteorder='big')

    enc_key = pow(2,127) + 1  # 16 bytes = 128 bits

    aes_cipher_obj = AES.new(enc_key, AES.MODE_CTR, nonce=nonce_bytes)
    MSG_STR = MSG_TO_SEND
    msg_utf8 = MSG_STR.encode('utf-8')
    cipher_bytes = aes_cipher_obj.encrypt(msg_utf8)

    all_together_msg = nonce_bytes + cipher_bytes

    return all_together_msg

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
                message_will_be_sent = encrypt_with_AES(message)

                client_socket.send(message_will_be_sent.encode('utf-8'))

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
