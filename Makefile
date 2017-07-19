CC=gcc
FLAGS=-O3 -Wall
LIBS=-lm

.PHONY: clean

all: fire8 rotozoomer8 test


test: test.c fb.o
	$(CC) $(FLAGS) fb.o test.c -o test $(LIBS)

fire8: fire8.c fb.o 
	$(CC) $(FLAGS) fb.o fire8.c -o fire8 $(LIBS)

rotozoomer8: rotozoomer8.c fb.o
	$(CC) $(FLAGS) fb.o rotozoomer8.c -o rotozoomer8 $(LIBS)

fb.o: fb.c
	$(CC) $(FLAGS) -c fb.c -o fb.o $(LIBS)

png: fire8 rotozoomer8
	$(FLAGS) = $(FLAGS) -DPNG
	$(LIBS) = $(LIBS) -lpng

clean:
	rm -f *.o fb.o fire8 rotozoomer8
