# Makefile for compiling code with SDCC and flashing using STCGAL

# Compiler and linker settings
SDCC = sdcc
STCGAL = stcgal

# SDCC compiler flags
SDCC_FLAGS = --model-small --stack-auto
# SDCC_FLAGS = --model-small


# STCGAL settings
STCGAL_PORT = COM8
STCGAL_PARAMS = -P stc89 -o cpu_6t_enabled=true

# Source files and target
SRC = main.c serial.c wiz.c
TARGET = main

# Header files
HEADERS = serial.h wiz.h periph.h constant.h

# OBJ macro serves as a template that matches every file in SRC using a substitution reference $(...), 
# it replaces SRC: by substituing every '.c' with a .rel file  .c=.rel
# OBJ = main.rel serial.rel wiz.rel
OBJ = $(SRC:.c=.rel)

# Default target builds main.ihx
all:	$(TARGET).ihx
		make flash

# Compile source files and relevant includes, target main.ihx depends on macro OBJ being updated before build process
# compiles and links object files
$(TARGET).ihx:	$(OBJ)
	$(SDCC)		$(SDCC_FLAGS)	$(OBJ)

# Compile header files generating %.rel object file for every .c file '%.rel: %.c'
# pattern rule % matches any string, $() substitution reference matches dependency on header files (good practice) for any updates on .h
# compiles object files with -c 
# the $< pattern is an automatic variable, $< represents the first prerequeisite required to create output file, in this case it will expand to each c file main.c, serial.c, wiz.c
%.rel:	%.c	$(HEADERS)
	$(SDCC)	$(SDCC_FLAGS)	-c	$<

# Flash the compiled binary using STCGAL
flash: $(TARGET).ihx
	$(STCGAL) -p $(STCGAL_PORT) $(STCGAL_PARAMS) $(TARGET).ihx

# Clean up intermediate files
clean:
	rm $(TARGET).lk

# Run Python script with UDP arguments
udp:
	py poll.py $(IP) $(PORT)	$(MODE) "$(MESSAGE)"