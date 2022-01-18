## This is a simple Makefile with lots of comments 
## Check Unix Programming Tools handout for more info.

# Define what compiler to use and the flags.
CC=cc
CXX=CC
CCFLAGS = -std=gnu99 -pthread -lrt -D_POSIX_C_SOURCE=199309L -g -Wall -Werror

all: candykids

# Compile all .c files into .o files
# % matches all (like * in a command)
# $< is the source file (.c file)
%.o : %.c
	$(CC) -c $(CCFLAGS) $<



candykids: candykids.o bbuff.o stats.o 
	$(CC) -o candykids candykids.o bbuff.o stats.o $(CCFLAGS)

clean:
	rm -f core *.o candykids
