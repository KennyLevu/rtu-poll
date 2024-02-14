import socket
import sys

# CHATGPT
def send_receive_message(ip, port, message, timeout = 3):
    try:
        # Create a UDP socket
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        # Set timeout for the socket
        sock.settimeout(timeout)
        # Send message
        sock.sendto(message.encode(), (ip, port))

        # Receive response
        data, addr = sock.recvfrom(1024)
        print("Received:", data.decode())

    except socket.timeout:
        print("No response received within", timeout, "seconds.")
    except Exception as e:
        print("Error:", e)
    finally:
        # Close socket
        sock.close()
        
    

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
