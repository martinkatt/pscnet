CC = g++
OBJ = netbase.o
CFLAGS = -c -Wall 

netbase: $(OBJ)
	$(CC) -I. $(OBJ) -o $@

netbase.o: net/netbase.cpp $(HEADER)
	$(CC) -I. $(CFLAGS) $< -o $@

clean: 
	rm -rf *o hello
