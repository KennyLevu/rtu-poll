import curses
from curses import wrapper
import string
import random
from enum import Enum

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

    # Initialize polling statistics
    mode = "UDP"
    packets_sent = 0
    packets_received = 0
    errors = 0
    is_polling = True
    send = rtu + gen_message() # prepend rtu #
    receive  = ""
   

   

    # Help messages
    help2 = "q: QUIT"
    help3 = "m: MODE"
    stdscr.refresh()
    curses.curs_set(0)  # Hide the cursor

    # Set initial getchar to non-blocking
    stdscr.nodelay(True) 
    while True:
        # clear screen for updates
        in_win.clear()
        stdscr.clear()
        stdscr.border(0, 0, 0, 0, 0, 0, 0, 0) 
        in_win.border(0, 0, 0, 0, 0, 0, 0, 0)
        # title
        stdscr.addstr(0, curses.COLS // 2 - (len(title) // 2), title, curses.A_BOLD | curses.color_pair(1) ) # set title with cyan color
        # update polling stats and mode
        help1 = f"Enter: ping OFF" if is_polling else f"Enter: ping ON"
        display_mode = f"Mode  . : {mode}"
        display_sent = f"Packets Sent: {packets_sent}"
        display_received = f"Packets Received: {packets_received}"
        display_errors = f"Error Rate: {errors}%"
        display_msent = f"Message Sent: <{send}>"
        display_mreceived = f"Message Rec: [{receive}]"
        # dispaly updated values
        stdscr.addstr(5, int(curses.COLS * .7), display_mode, curses.A_BOLD | curses.color_pair(4) ) 
        stdscr.addstr(5, 4, display_sent, curses.A_BOLD | curses.color_pair(2) )
        stdscr.addstr(6, 4, display_received, curses.A_BOLD | curses.color_pair(2) )
        stdscr.addstr(7, 4, display_errors, curses.A_BOLD | curses.color_pair(3) )
        stdscr.addstr(8, 4, display_msent, curses.A_BOLD | curses.color_pair(2) )
        stdscr.addstr(9, 4, display_mreceived, curses.A_BOLD | curses.color_pair(2) )
         # Output Network Config to right side of screen
        stdscr.addstr(6, int(curses.COLS * .7), mes_ip + ip_address, curses.A_BOLD | curses.color_pair(1) ) 
        stdscr.addstr(7, int(curses.COLS * .7), mes_rtu + rtu, curses.A_BOLD | curses.color_pair(1) )
        stdscr.addstr(8, int(curses.COLS * .7), mes_udp + udp, curses.A_BOLD | curses.color_pair(1) )
        stdscr.addstr(9, int(curses.COLS * .7), mes_tcp + tcp, curses.A_BOLD | curses.color_pair(1) )

        # Output helper text
        in_win.addstr(3, 1, help3, curses.color_pair(4))
        in_win.addstr(1, 1, help1, curses.color_pair(1))
        in_win.addstr(2, 1, help2, curses.color_pair(3))

        # update screens
        in_win.refresh()
        stdscr.refresh()
        
        
        # handle polling
        if mode == "UDP":
            pass
        elif mode == "TCP":
            pass
        elif mode == "BOTH":
            pass

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
            packets_sent = 0
            packets_received = 0
            errors = 0
    

wrapper(main)