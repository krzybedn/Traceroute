CC = g++
CPPFLAGS = -std=gnu++11 -Wall -Wextra

all: main

main: main.o msg_managment.o

main.o: main.cpp msg_managment.h
msg_managment.o: msg_managment.cpp msg_managment.h

clean:
	rm -f *.o\

distclean:
	rm -f main *.o\

# vim: ts=8 sw=8 noet