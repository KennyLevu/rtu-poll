import curses
from curses import wrapper
import string
import random
import socket
import time
import select
from enum import Enum
import errno
import threading

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
        return round(float((self.packets_sent - self.packets_received) / self.packets_sent) * 100.0, 2)
    # poll udp socket
    def poll_udp(self, message, timeout = 5):
        output = "ERROR"
        start_time = time.time()
        end_time = 0
        # update packets sent
        self.packets_sent += 1
        try:
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
                # Receive response
                data, addr = sock.recvfrom(1024)
                output = data.decode()
                # Update packets received
                self.packets_received += 1
            else:
                # Timeout occurred
                output = "TIMEOUT"
                # Update errors
        except Exception as e:
            # pass
            output = "ERROR"
            print(e)
        finally:
            # Close socket
            if sock:
                sock.close()
            end_time = time.time()
            return (output, end_time - start_time, message)
            

    def poll_tcp(self, message, timeout=5):
        output = "ERROR"
        end_time = 0
        # Record timestamp for response time
        start_time = time.time()
        try:
            # Create a TCP socket
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)    
            # Attempt to connect
            sock.connect_ex((self.ip, int(self.tcp)))
            # Wait for socket write readiness
            ready = select.select([], [sock], [], timeout)
            # sock.setblocking(False)  # Set socket to non-blocking
            if ready[1]:
                
                # Update packets sent
                self.packets_sent += 1
                # Send message
                sock.send(message.encode())
                ready = select.select([sock], [], [], timeout)
                if ready[0]:
                    # Receive response
                    data = sock.recv(1024)
                    # Update packets received
                    self.packets_received += 1
                    output = data.decode()
                else:
                    output = "TIMEOUT"
                    self.errors += 1
            else:
                # Timeout occurred
                output = "CONNECTION FAILED"

        except socket.error as e:
            # Handle exceptions
            # if e.errno == errno.WSAECONNRESET: # win error 10054
            #     output = "CONNECTION DROPPED"
            print("Error:", e)
        finally:
            # Close socket
            if sock:
                sock.close()
            # Calculate response time
            end_time = time.time()
            time.sleep(0.05)
            return (output, end_time - start_time, message)
    # poll both udp and tcp sockets
    def poll_both(self, message, timeout=5):
        start_time = time.time()
        # print(start_time)
        udp_response = None
        tcp_response = None

        # Define functions for UDP and TCP polling
        def poll_udp_task():
            nonlocal udp_response
            udp_response = self.poll_udp(message)

        def poll_tcp_task():
            nonlocal tcp_response
            tcp_response = self.poll_tcp(message)

        # Create threads for UDP and TCP polling
        udp_thread = threading.Thread(target=poll_udp_task)
        tcp_thread = threading.Thread(target=poll_tcp_task)

        # Start both threads
        udp_thread.start()
        tcp_thread.start()

        # Wait for both threads to complete or timeout
        udp_thread.join(timeout)
        tcp_thread.join(timeout)


        # Check if threads are still alive (indicating timeout)
        if udp_thread.is_alive():
            # Handle UDP timeout
            self.errors += 1
            print("UDP timeout")
            udp_response = "TIMEOUT"

        if tcp_thread.is_alive():
            # Handle TCP timeout
            self.errors += 1
            print("TCP timeout")
            tcp_response = "TIMEOUT"
        end_time = time.time()
        # print(end_time)
        # print(end_time - start_time)
        # times = end_time - start_time

        return (udp_response[0], tcp_response[0], udp_response[1], message, tcp_response[1])

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
# generate 7 digit ascii string sequentially
def gen_seven():
    number = 0
    while True:
        yield str(number).zfill(7)  # Convert number to string with leading zeros
        number += 1

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

    # Help messages
    help2 = "q: QUIT"
    help3 = "m: MODE"
    help5 = "r: REPORT"

    stdscr.refresh()
    curses.curs_set(0)  # Hide the cursor
    curses.noecho() # hide characters typed
    # Set initial getchar blocking state
    stdscr.nodelay(False) 
     # Initialize polling statistics
    mode = "UDP"
    is_polling = False
    receive  = ("", 0,"")
    receive_both  = ("", "", 0,"",0)
    send_once = False
    debug = False
    gen = gen_seven()
    while True:
        # gen message
        if debug:
            send = rtu + next(gen)
        else:
            send = rtu + gen_message() # prepend rtu #
        # clear screen for updates
        in_win.clear()
        stdscr.clear()
        stdscr.border(0, 0, 0, 0, 0, 0, 0, 0) 
        in_win.border(0, 0, 0, 0, 0, 0, 0, 0)
        # title
        stdscr.addstr(0, curses.COLS // 2 - (len(title) // 2), title, curses.A_BOLD | curses.color_pair(1) ) # set title with cyan color
        # update polling stats and mode
        help1 = f"Enter: POLL ON" if is_polling else f"Enter: POLL OFF"
        help4 = f"d: DEBUG ON" if debug else f"d: DEBUG OFF"
        display_mode = f"Mode  . : {mode}"
        display_sent = f"Packets Sent: {poll.packets_sent}"
        display_received = f"Packets Received: {poll.packets_received}"
        display_errors = f"Error Rate: {poll.get_errors()}%"
        display_msent = f"Message Sent: [{receive[2]}]" if mode != "BOTH" else f"Message Sent: [{receive_both[3]}]"
        display_mreceived = f"Message Rec: [{receive[0]}]" if mode != "BOTH" else f"Message Rec: UDP[{receive_both[0]}] TCP[{receive_both[1]}]"
        display_ms = f"Response Time: {round(receive[1] * 1000)}ms" if mode != "BOTH" else f"UDP:{round(receive_both[2] * 1000)}ms TCP:{round(receive_both[4] * 1000)}ms"
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
        in_win.addstr(2, 1, help3, curses.color_pair(4))

        in_win.addstr(1, 1, help1, curses.color_pair(1))
        in_win.addstr(3, 1, help2, curses.color_pair(3))
        if (not is_polling):
            in_win.addstr(4, 1, "Press SPACEBAR to send one packet", curses.color_pair(2))
            # if (mode != "BOTH"): 
            in_win.addstr(2, 1 + 15 + 5, help5, curses.color_pair(4))
        in_win.addstr(1, 1 + 15 + 5, help4, curses.color_pair(1)) # column aligned by lengthof help1 message

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
        elif key == ord('d'):
            debug = not debug
            if debug:
                gen = gen_seven()

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
        elif key ==  ord(' ') and not is_polling:
            send_once = True
        elif key ==  ord('r') and not is_polling:
            with open("output.txt", "w") as file:
                new_poll = Poll(ip_address, udp, tcp)
                # count 1 - 100 on screen
                max_tcp = 0.0
                min_tcp = 999999
                min_ms = 999999
                errors = 0
                receive_both =("","",0,"",0)
                max_ms = 0.0
                for i in range(1, 101):
                    send = rtu + next(gen) if debug else rtu + gen_message()
                    in_win.clear()
                    stdscr.clear()
                    stdscr.addstr(height // 2, width // 2 - len(str(i)) // 2, str(i))
                    stdscr.refresh()
                    if mode == "UDP":
                        receive = new_poll.poll_udp(send, 1)
                    elif mode == "TCP":
                        receive = new_poll.poll_tcp(send, 3)
                    elif mode == "BOTH":
                        receive_both = new_poll.poll_both(send, 3)
                    if mode != "BOTH":
                        if receive[1] > max_ms and receive[0][0] == receive[2][0]:
                            max_ms = receive[1]
                        elif receive[1] < min_ms and receive[0][0] == receive[2][0]:
                            min_ms = receive[1]
                        if receive[0][0] != receive[2][0]:
                            errors += 1
                    elif mode == "BOTH":
                        if receive_both[2] > max_ms:
                            max_ms = receive_both[2]
                        elif float(receive_both[2]) < float(min_ms):
                            min_ms = receive_both[2]
                        if float(receive_both[4]) > float(max_tcp):
                            max_tcp = receive_both[4]
                        elif receive_both[4] < float(min_tcp):
                            min_tcp = receive_both[4]
                        
                    index = f"{str(i).ljust(4)}"
                    sent_text = f"| Sent: [{receive[2]}]" if mode != "BOTH" else f"| Sent: [{receive_both[3]}]"
                    rec_text = f"Received: <{receive[0]}>" if mode != "BOTH" else f"Received: UDP<{receive_both[0]}> TCP<{receive_both[1]}>"
                    time_text = f"time:{round(receive[1] * 1000)}ms\n"  if mode != "BOTH" else f"UDP:{round(receive_both[2] * 1000)}ms TCP:{round(receive_both[4] * 1000)}ms\n"
                    file.write(f"{index} {sent_text} {rec_text} {time_text}")
                    # time.sleep(0.1)
                file.write(f"max_ms:{round(max_ms * 1000)}ms min_ms:{round(min_ms * 1000)}ms ERRORS:{new_poll.errors}"  if mode != "BOTH" else 
                           f"max_udp:{round(max_ms * 1000)}ms min_udp:{round(min_ms * 1000)}ms  max_tcp:{round(max_tcp * 1000)}ms min_tcp:{round(min_tcp * 1000)}ms ERRORS:{new_poll.errors}")

        # handle polling
        if is_polling or send_once:
            if mode == "UDP":
                receive = poll.poll_udp(send, 1)
            elif mode == "TCP":
                receive = poll.poll_tcp(send, 60)
            elif mode == "BOTH":
                receive_both = poll.poll_both(send, 60)
        send_once = False

wrapper(main)