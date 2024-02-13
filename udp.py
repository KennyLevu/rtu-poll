import socket
import sys

# CHATGPT
def send_receive_message(ip, port, message):
    try:
        # Create a UDP socket
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

        # Send message
        sock.sendto(message.encode(), (ip, port))

        # Receive response
        data, addr = sock.recvfrom(1024)
        print("Received:", data.decode())

        # Close socket
        sock.close()
    except Exception as e:
        print("Error:", e)

if __name__ == "__main__":
    # Check if IP, port, and message are provided as command-line arguments
    if len(sys.argv) != 4:
        print("Usage: python script.py <IP> <port> <message>")
        sys.exit(1)

    # Extract IP, port, and message from command-line arguments
    ip = sys.argv[1]
    port = int(sys.argv[2])
    # Extract message from command-line arguments
    message = ' '.join(sys.argv[3:])
    # message = sys.argv[3]

    # Send and receive message
    send_receive_message(ip, port, message)
