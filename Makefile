# Makefile for compiling code with SDCC and flashing using STCGAL

# Compiler and linker settings
SDCC = sdcc
STCGAL = stcgal

# SDCC compiler flags
SDCC_FLAGS = --model-small --stack-auto

# STCGAL settings
STCGAL_PORT = COM8
STCGAL_PARAMS = -P stc89 -o cpu_6t_enabled=true

# Source files and target
SRC = main.c serial.c wiz.c
TARGET = main

# Header files
HEADERS = serial.h wiz.h periph.h constant.h

# Object files
OBJ = $(SRC:.c=.rel)

# Default target
all:	$(TARGET).ihx
		make flash

# Compile source files
$(TARGET).ihx:	$(OBJ)
	$(SDCC)		$(SDCC_FLAGS)	$(OBJ)

# Compile header files
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
	py udp.py $(IP) $(PORT) "$(MESSAGE)"