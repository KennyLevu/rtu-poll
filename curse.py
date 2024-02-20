import curses
from curses import wrapper
import string
import random
import socket
import time
import select
from enum import Enum


from enum import Enum
class Poll():
    # Initialize polling statistics
    packets_sent = 0
    packets_received = 0
    errors = 0

    def __init__(self, ip, udp, tcp) -> None:
        self.ip = ip
        self.udp = udp
        self.tcp = tcp
    # get error rate
    def get_errors(self):
        if self.packets_sent == 0:
            return 0
        return float(self.errors / self.packets_sent) * 100.0
    # poll udp socket
    def poll_udp(self, message, timeout = 5):
        output = "ERROR"
        try:
            # update packets sent
            self.packets_sent += 1
            # Create a UDP socket
            sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
            # Set socket to non-blocking
            sock.setblocking(False)
            # Set timeout for the socket
            # sock.settimeout(timeout)
            # Record timestamp for response time
            start_time = time.time()
            # Send message
            sock.sendto(message.encode(), (self.ip, int(self.udp)))
            # Wait for socket readiness
            ready = select.select([sock], [], [], timeout)
            if ready[0]:
                # Receive response and calculate responsetime
                data, addr = sock.recvfrom(1024)
                end_time = time.time()
                output = data.decode()
                # Update packets received
                self.packets_received += 1
            else:
                # Timeout occurred
                end_time = 0
                output = "TIMEOUT"
                # Update errors
                self.errors += 1
            # # Receive response
            # data, addr = sock.recvfrom(1024)
            # end_time = time.time()
            # # print("UDP Received:", data.decode())
        # except socket.timeout:
        #     # pass
        #     # Update errors
        #     self.errors += 1
        #     output = "TIMEOUT"
        #     end_time = 0
        except Exception as e:
            # pass
            output = "ERROR"
            print(e)
            self.errors += 1
            end_time = 0
        # else:
        #     # Update packets received
        #     self.packets_received += 1
        #     output = data.decode()
        finally:
            # Close socket
            if sock:
                sock.close()
            return (output, end_time - start_time, message)
            

    def poll_tcp(self, message, timeout = 5):
        output = "ERROR"
        try:
            # update packets sent
            self.packets_sent += 1
            # Create a TCP SOCKET
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            # Set socket to non-blocking
            sock.setblocking(False)
            # Set timeout for the socket
            # sock.settimeout(timeout)
            # Record timestamp for response time
            start_time = time.time()
            # Connect to server
            sock.connect((self.ip,  int(self.tcp)))
            # Send message
            sock.send(message.encode())
             # Wait for socket readiness
            ready = select.select([sock], [], [], timeout)
            if ready[0]:
                # Receive response and calculate response time
                data, addr = sock.recvfrom(1024)
                end_time = time.time()
                output = data.decode()
                # Update packets received
                self.packets_received += 1
            else:
                # Timeout occurred
                end_time = 0
                output = "TIMEOUT"
                # Update errors
                self.errors += 1
            # # Receive response
            # data, addr = sock.recvfrom(1024)
            # end_time = time.time()
            
        # except socket.timeout as e:
        #     print(e)
        #     # Update errors
        #     self.errors += 1
        #     output = "TIMEOUT"
        #     end_time = 0
        except Exception as e:
            self.errors += 1
            output = "ERROR"
            print(e)
            end_time = 0
        else:
            # Update packets received
            self.packets_received += 1
            output = data.decode()
        finally:
            # Close socket
            if sock:
                sock.close()
            return (output, end_time - start_time, message)

class Protocol(Enum):
    UDP = (1, "UDP")
    TCP = (2, "TCP")
    BOTH = (3, "BOTH")

def get_protocol(value):
    for protocol in Protocol:
        if protocol.value[0] == value:
            return protocol

# takes string prompt and returns user input as string from prompt
def get_input (stdscr, prompt, y, x):
    # stdscr.addstr(int(curses.LINES * .8), curses.COLS // 2 - (len(prompt) // 2), prompt) # print prompt to screen at the center of input window
    stdscr.addstr(y, x, prompt, curses.A_BOLD | curses.color_pair(2) )
    curses.echo() # enable echo, user can see input
    input_str = stdscr.getstr().decode('utf-8') # get user input and terminate on enter key
    curses.noecho() #disable echo
    return input_str

# generate random string with lower case letters and len 8-16
def gen_message():
    len = random.randint(8,16) # get length of string
    gen_char = [random.choice(string.ascii_lowercase) for _letter in range(len)] #list comprehension fill array up to len from ascii lowercase
    msg = ''.join(gen_char)
    return msg

def main(stdscr):
    # clear and init screen
    stdscr = curses.initscr()
    stdscr.clear()


    # define colors
    curses.init_pair(1, curses.COLOR_CYAN, curses.COLOR_BLACK)
    curses.init_pair(2, curses.COLOR_WHITE, curses.COLOR_BLACK)
    curses.init_pair(3, curses.COLOR_RED, curses.COLOR_BLACK)
    curses.init_pair(4, curses.COLOR_GREEN, curses.COLOR_BLACK)



    # Define dimensions for the inner window
    height, width = stdscr.getmaxyx()
    inner_height = 6                       # set window height
    inner_width = width - 4                # set window width
    inner_y = height - inner_height - 1    # set window y towards the bottom of the screen
    inner_x = (width - inner_width) // 2   # set window x towards the center
    in_win = stdscr.subwin(inner_height, inner_width, inner_y, inner_x)

    # adds border around term and inner window
    stdscr.border(0, 0, 0, 0, 0, 0, 0, 0) 
    in_win.border(0, 0, 0, 0, 0, 0, 0, 0)

    # Set title
    title = "Polling Statistics"
    stdscr.addstr(0, curses.COLS // 2 - (len(title) // 2), title, curses.A_BOLD | curses.color_pair(1) ) # set title with cyan color

    # Get initial network config from user
    mes_ip  = "IP  . . : "      # store prompt vars
    mes_rtu = "RTU . . : "
    mes_udp = "UDP . . : "
    mes_tcp = "TCP . . : "
    ip_address = get_input(in_win, mes_ip, inner_height // 2, (inner_width // 3) ) 
    in_win.clear()
    in_win.border(0, 0, 0, 0, 0, 0, 0, 0)
    rtu = get_input(in_win, mes_rtu, inner_height // 2, (inner_width // 3))
    in_win.clear()
    in_win.border(0, 0, 0, 0, 0, 0, 0, 0)
    udp = get_input(in_win, mes_udp, inner_height // 2, (inner_width // 3))
    in_win.clear()
    in_win.border(0, 0, 0, 0, 0, 0, 0, 0)
    tcp = get_input(in_win, mes_tcp, inner_height // 2, (inner_width // 3))
    in_win.clear()
    in_win.border(0, 0, 0, 0, 0, 0, 0, 0)



    # Initialize Class Poll object has class values sent, received, errors
    if (ip_address == ""):
        ip_address = "126.10.210.10"
    if (udp == ""):
        udp = "5000"
    if (tcp == ""):
        tcp = "6000"
    if (rtu == ""):
        rtu = "0"
    poll = Poll(ip_address, udp, tcp)

    # Initialize polling statistics
    mode = "UDP"
    is_polling = True
    receive  = ("", 0,"")

    # Help messages
    help2 = "q: QUIT"
    help3 = "m: MODE"
    stdscr.refresh()
    curses.curs_set(0)  # Hide the cursor

    # Set initial getchar to non blocking
    stdscr.nodelay(True) 
    while True:

        # gen message
        send = rtu + gen_message() # prepend rtu #
        # clear screen for updates
        in_win.clear()
        stdscr.clear()
        stdscr.border(0, 0, 0, 0, 0, 0, 0, 0) 
        in_win.border(0, 0, 0, 0, 0, 0, 0, 0)
        # title
        stdscr.addstr(0, curses.COLS // 2 - (len(title) // 2), title, curses.A_BOLD | curses.color_pair(1) ) # set title with cyan color
        # update polling stats and mode
        help1 = f"Enter: ON" if is_polling else f"Enter: OFF"
        display_mode = f"Mode  . : {mode}"
        display_sent = f"Packets Sent: {poll.packets_sent}"
        display_received = f"Packets Received: {poll.packets_received}"
        display_errors = f"Error Rate: {poll.get_errors()}%"
        display_msent = f"Message Sent: <{receive[2]}>"
        display_mreceived = f"Message Rec: [{receive[0]}]"
        display_ms = f"Response Time: {round(receive[1] * 1000)}ms"
        # dispaly updated values
        stdscr.addstr(5, int(curses.COLS * .7), display_mode, curses.A_BOLD | curses.color_pair(4) ) 
        stdscr.addstr(5, 4, display_sent, curses.A_BOLD | curses.color_pair(2) )
        stdscr.addstr(6, 4, display_received, curses.A_BOLD | curses.color_pair(2) )
        stdscr.addstr(7, 4, display_errors, curses.A_BOLD | curses.color_pair(3) )
        stdscr.addstr(8, 4, display_msent, curses.A_BOLD | curses.color_pair(2) )
        stdscr.addstr(9, 4, display_mreceived, curses.A_BOLD | curses.color_pair(2) )
        stdscr.addstr(10, 4, display_ms, curses.A_BOLD | curses.color_pair(2) )
         # Output Network Config to right side of screen
        stdscr.addstr(6, int(curses.COLS * .7), mes_ip + ip_address, curses.A_BOLD | curses.color_pair(1) ) 
        stdscr.addstr(7, int(curses.COLS * .7), mes_rtu + rtu, curses.A_BOLD | curses.color_pair(1) )
        stdscr.addstr(8, int(curses.COLS * .7), mes_udp + udp, curses.A_BOLD | curses.color_pair(1) )
        stdscr.addstr(9, int(curses.COLS * .7), mes_tcp + tcp, curses.A_BOLD | curses.color_pair(1) )

        # Output helper text
        in_win.addstr(3, 1, help3, curses.color_pair(4))
        in_win.addstr(1, 1, help1, curses.color_pair(1))
        in_win.addstr(2, 1, help2, curses.color_pair(3))
        if (not is_polling):
            in_win.addstr(4, 1, "Press any key to send one packet", curses.color_pair(2))

        # update screens
        in_win.refresh()
        stdscr.refresh()
        
        
        # Listen for key press
        key = stdscr.getch()

        if key == curses.KEY_ENTER or key in [10,13]:
            if (is_polling):
                stdscr.nodelay(False) # make getch() blocking to turn off polling
            elif (not is_polling):
                stdscr.nodelay(True) # make getch() non-blocking to turn on polling
            is_polling = not is_polling
        elif key == ord('q'):
            curses.nocbreak()
            curses.echo()
            curses.endwin()
            return
        elif key == ord('m'):
            if mode == "UDP":
                mode = "TCP"
            elif mode == "TCP":
                mode = "BOTH"
            else:
                mode = "UDP"
            # reset polling statistics
            poll.packets_sent = 0
            poll.packets_received = 0
            poll.errors = 0

        # handle polling
        if is_polling:
            if mode == "UDP":
                receive = poll.poll_udp(send, 1)
            elif mode == "TCP":
                receive = poll.poll_tcp(send, 1)
            elif mode == "BOTH":
                pass

wrapper(main)