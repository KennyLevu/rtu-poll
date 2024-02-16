import socket
import sys


def poll_udp(ip, port, message, timeout = 3):
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
        

def poll_tcp(ip, port, message, timeout = 5):
    try:
        # Create a TCP SOCKET
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        # Set timeout for the socket
        sock.settimeout(timeout)
        # Connect to server
        sock.connect((ip, port))
        # Send message
        sock.send(message)
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
    if len(sys.argv) != 5:
        print("Usage: python script.py <IP> <port> <mode> <message>")
        sys.exit(1)

    # Extract IP, port, mode and message from command-line arguments
    ip = sys.argv[1]
    port = int(sys.argv[2])
    mode = sys.argv[3]

    # Extract message from command-line arguments
    message = ' '.join(sys.argv[4:])

    # Send and receive message
    if mode == 'UDP':
        poll_udp(ip, port, message)
    if mode == 'TCP':
        poll_tcp(ip, port, message)
    # if mode == 'BOTH':
    #     poll_both(ip, port, message)

