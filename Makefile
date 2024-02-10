# Makefile for compiling code with SDCC and flashing using STCGAL

# Compiler and linker settings
SDCC = sdcc
STCGAL = stcgal

# SDCC compiler flags
SDCC_FLAGS = --model-small

# STCGAL settings
STCGAL_PORT = COM8
STCGAL_PARAMS = -P stc89 -o cpu_6t_enabled=true

# Source files and target
SRC = main.c
TARGET = main

# Header files
HEADERS = periph.h constant.h serial.h wiz.h

# Object files for headers
HEADER_OBJS = $(HEADERS:.h=.rel)

# Default target
all:	$(TARGET).ihx
		make flash

# Compile source files
$(TARGET).ihx:  $(SRC)	$(HEADER_OBJS)
	$(SDCC)		$(SDCC_FLAGS)	$(SRC)

# Compile header files
%.rel:	%.c
	$(SDCC)	$(SDCC_FLAGS)	-c	$<

# Flash the compiled binary using STCGAL
flash: $(TARGET).ihx
	$(STCGAL) -p $(STCGAL_PORT) $(STCGAL_PARAMS) $(TARGET).ihx

# Clean up intermediate files
clean:
	rm $(TARGET).lk
