# This makefile is on purpose not made with configure, to show how to use the library
# The make file requires that the fann library is installed (see ../README)

# TOOLSET
SHELL = /bin/sh
GCC = /usr/bin/gcc
GCC_OPT = -Wall -g
LIBS = -lreadline

# DIRECTORIES
topdir = .
srcdir = .
bindir = .

# C Program
TARGETS = mysh

# Library
OBJS =	debug.o \
		procgroup.o \
		pidtable.o \
		parser.o \
		sighandler.o 

#Unittests
TEST =	procgroup_test \
		pidtable_test \
		parser_test

### MAKE ###
all: $(OBJS) $(TARGETS) $(TEST)

%: %.c $(OBJS) Makefile include.h
	$(GCC) $(GCC_OPT) $(OBJS) $@.c $(LIBS) -o $@

%.o: %.c %.h Makefile include.h
	$(GCC) $(GCC_OPT) -c $<


# Clean up 
clean:
	rm -f $(TARGETS) $(OBJS) $(TEST)
	rm -f *~ *.obj *.exe *.o

check:
	valgrind ./procgroup_test
	valgrind ./pidtable_test
	valgrind ./parser_test
