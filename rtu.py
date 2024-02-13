import socket

UDP_IP = "127.10.218.163"
UDP_PORT = 8888

RT_IP = "127.0.0.1"
RT_PORT = 6500
MESSAGE = b"Hello, World!"

print("UDP target IP: %s" % UDP_IP)
print("UDP target port: %s" % UDP_PORT)
print("message: %s " % MESSAGE)

sock = socket.socket(socket.AF_INET, #internet
                            socket.SOCK_DGRAM) #UDP
sock.sendto(MESSAGE, (UDP_IP, UDP_PORT))                        

sock.bind((RT_IP, RT_PORT))

while True:
    data, addr = sock.recvfrom(1024) # buffer size 1024 bytse
    print("received message: %s" % data)