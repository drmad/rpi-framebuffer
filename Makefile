CC=gcc
FLAGS=-O3 -Wall

.PHONY: clean

all: fire8
    
fire8: fire8.c fb.o 
	$(CC) $(FLAGS) fb.o fire8.c -o fire8

fb.o: fb.c
	$(CC) $(FLAGS) -c fb.c -o fb.o

clean:
	rm -f *.o fb.o fire8
