CC = gcc                      # Compiler to use (gcc)
CFLAGS = -Wall -Wextra -std=c99  # Flags: enable warnings, C99 standard
TARGET = memory_simulator     # Name of the output executable

# List of source files (now includes inputs_part1.c)
SRCS = main.c memory_sim.c inputs_part1.c
OBJS = $(SRCS:.c=.o)          # Object files (.o) generated from .c files

all: $(TARGET)                # Default target (builds the executable)

# Rule to link object files into the executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^  # $@ = target name, $^ = all dependencies

# Rule to compile .c files into .o files
%.o: %.c
	$(CC) $(CFLAGS) -c $<     # $< = the input .c file

# Cleanup rule (removes .o files, executable, and output logs)
clean:
	rm -f $(OBJS) $(TARGET) fifo*.out lru*.out

.PHONY: all clean             # Declare 'all' and 'clean' as phony targets