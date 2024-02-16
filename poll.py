import socket
import sys
import threading



def poll_udp(ip, port, message, timeout = 5):
    try:
        # Create a UDP socket
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        # Set timeout for the socket
        sock.settimeout(timeout)
        # Send message
        sock.sendto(message.encode(), (ip, port))

        # Receive response
        data, addr = sock.recvfrom(1024)
        print("UDP Received:", data.decode())

    except socket.timeout:
        print("UDP No response received within", timeout, "seconds.")
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
        sock.send(message.encode())
        # Receive response
        data, addr = sock.recvfrom(1024)
        print("TCP Received:", data.decode())
    except socket.timeout:
        print("TCP No response received within", timeout, "seconds.")
    except Exception as e:
        print("Error:", e)
    finally:
        # Close socket
        sock.close()

# def poll_both(ip, port1, port2, message, timeout = 5):

#     return

if __name__ == "__main__":
    # Check if IP, port, and message are provided as command-line arguments
    if len(sys.argv) != 6:
        print("Usage: python script.py <IP> <port_udp> <port_tcp> <mode> <message> OR python script.py <IP> <port1> <port2> BOTH <message>")

        sys.exit(1)

    # Extract IP, port, mode and message from command-line arguments
    ip = sys.argv[1]
    port_udp = int(sys.argv[2])
    port_tcp = int(sys.argv[3])
    mode = sys.argv[4]

    # Extract message from command-line arguments
    message = ' '.join(sys.argv[5:])

    # Send and receive message
    if mode == 'UDP':
        poll_udp(ip, port_udp, message)
    if mode == 'TCP':
        # print("test")
        poll_tcp(ip, port_tcp, message)

    if mode == 'BOTH':
    # Create threads for UDP/TCP
        udp_thread = threading.Thread(target=poll_udp, args=(ip, port_udp, message))
        tcp_thread = threading.Thread(target=poll_tcp, args=(ip, port_tcp, message))

        # Start both threads
        udp_thread.start()
        tcp_thread.start()

        # Wait for both threads to finish
        udp_thread.join()
        tcp_thread.join()
