CC = gcc
CFLAGS = -Wall 
all:x86_send x86_recv
RM = rm -f

x86_send:x86_send.c
	$(CC) $(CFLAGS) -o $@  x86_send.c 

x86_recv:x86_recv.c
	$(CC) $(CFLAGS) -o $@ x86_recv.c

#*.o:*.c
#	$(CC) $(CFLAGS) -o $@  *.c
clean:
	$(RM) x86_send x86_recv
#	$(RM) *.o
