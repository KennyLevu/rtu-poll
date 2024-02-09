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

# Default target
all:	$(TARGET).ihx
		make flash

# Compile source files
$(TARGET).ihx:  $(SRC)
	$(SDCC)		$(SDCC_FLAGS)	$(SRC)

# Flash the compiled binary using STCGAL
flash: $(TARGET).ihx
	$(STCGAL) -p $(STCGAL_PORT) $(STCGAL_PARAMS) $(TARGET).ihx

# Clean up intermediate files
clean:
	rm $(TARGET).lk
