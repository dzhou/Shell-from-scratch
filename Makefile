# This makefile is on purpose not made with configure, to show how to use the library
# The make file requires that the fann library is installed (see ../README)

# TOOLSET
SHELL = /bin/sh
GCC = /usr/bin/gcc
GCC_OPT = -Wall -g
LIBS = -lreadline
MAKE  = make 

# DIRECTORIES
topdir = .
srcdir = src
bindir = bin

### MAKE ###
all: 	
	$(MAKE) -C $(srcdir) all

clean: 
	$(MAKE) -C $(srcdir) clean 