# Makefile for UM
# 
# Jack Burton jburto05
# James Hartley jhartl01

############## Variables ###############

CC = gcc # The compiler being used

# Updating include path to use Comp 40 .h files and CII interfaces
IFLAGS = -I. -I/comp/40/build/include -I/usr/sup/cii40/include/cii

# Compile flags
# Set debugging information, allow the c99 standard,
# max out warnings, and use the updated include path
CFLAGS = -g -std=c99 -Wall -Wextra -Werror -Wfatal-errors -pedantic $(IFLAGS)

# Linking flags
# Set debugging information and update linking path
# to include course binaries and CII implementations
LDFLAGS = -g -L/comp/40/build/lib -L/usr/sup/cii40/lib64


# Libraries needed for linking
# Both programs need cii40 (Hanson binaries) and *may* need -lm (math)
# Only brightness requires the binary for pnmrdr.
LDLIBS = -L/comp/40/build/lib -lbitpack -lcii40 -lm -lrt 

# Collect all .h files in your directory.
# This way, you can never forget to add
# a local .h file in your dependencies.
INCLUDES = $(shell echo *.h)

############### Rules #############
# To get *any* .o file, compile its .c file with the following rule.
%.o: %.c $(INCLUDES)
	$(CC) $(CFLAGS) -c $< -o $@



## Linking step (.o -> executable program)	
um: um.o memory.o memload.o memexec.o ## put other .o's once made
	$(CC) $(LDFLAGS) $^ -o $@ $(LDLIBS)

 
clean:
	rm -f *.o

# Both programs need cii40 (Hanson binaries) and *may* need -lm (math)
# Only brightness requires the binary for pnmrdr.
LDLIBS = -L/comp/40/build/lib -lbitpack -lcii40 -lm -lrt 

# Collect all .h files in your directory.
# This way, you can never forget to add
# a local .h file in your dependencies.
INCLUDES = $(shell echo *.h)

############### Rules #############
# To get *any* .o file, compile its .c file with the following rule.
%.o: %.c $(INCLUDES)
	$(CC) $(CFLAGS) -c $< -o $@



## Linking step (.o -> executable program)	
um: um.o memory.o memload.o memexec.o ## put other .o's once made
	$(CC) $(LDFLAGS) $^ -o $@ $(LDLIBS)

 
clean:
	rm -f *.o

