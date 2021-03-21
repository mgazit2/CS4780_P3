#MAKEFILE 
#MATAN GAZIT  
CC = gcc 
CFLAGS = -Wall -g -pthread -std=gnu99  

MSTR = monitor 
MOBJ = main.o 
MOUT = $(MSTR)  

SLVE = consumer 
SOBJ = consumer.o 
SOUT = $(SLVE)  

PROD = producer
POBJ = producer.o
POUT = $(PROD)

.SUFFIXES: .c .o 
.PHONY: all clean  

all: 	$(MOUT) $(SOUT) $(POUT) 

$(MSTR): $(MOBJ) 	 
	$(CC) $(CFLAGS) -o $@ $(MOBJ)  

$(SLVE): $(SOBJ) 	 
	$(CC) $(CFLAGS) -o $@ $(SOBJ) -L. -l_monitor 
	
$(PROD): $(POBJ)
	$(CC) $(CFLAGS) -o $@ $(POBJ) -L. -l_monitor

.c.o: 	 
	$(CC) $(CFLAGS) -c $<  


clean: 	 
	/bin/rm -rf *.o logfile $(MSTR) $(SLVE) $(PROD)
