#MAKEFILE 
#MATAN GAZIT  
CC = gcc 
CFLAGS = -Wall -g -pthread -std=gnu99  

MSTR = monitor 
MOBJ = main.o 
MOUT = $(MSTR)  

SLVE = child 
SOBJ = child.o 
SOUT = $(SLVE)  

.SUFFIXES: .c .o 
.PHONY: all clean  

all: 	$(MOUT) $(SOUT)  

$(MSTR): $(MOBJ) 	 
	$(CC) $(CFLAGS) -o $@ $(MOBJ)  

$(SLVE): $(SOBJ) 	 
	$(CC) $(CFLAGS) -o $@ $(SOBJ)  
	
.c.o: 	 
	$(CC) $(CFLAGS) -c $<  


clean: 	 
	/bin/rm -rf *.o logfile $(MSTR) $(SLVE)
