CC=gcc
JC=javac
CFLAGS=-std=c11 -Wall -Wextra -O3 -g -pthread
LDLIBS=-lm -pthread
EXECS=CreaGrafo.class cammini.out

all: $(EXECS)

# Compilazione Java
CreaGrafo.class: CreaGrafo.java Attore.java
	$(JC) -d ./ $^

# Compilazione C
cammini.out: cammini.o functionality.o xerrori.o
	$(CC) $(CFLAGS) $^ $(LDLIBS) -o $@

cammini.o: cammini.c functionality.h xerrori.h
	$(CC) $(CFLAGS) -c cammini.c -o $@

functionality.o: functionality.c functionality.h xerrori.h
	$(CC) $(CFLAGS) -c functionality.c -o $@

xerrori.o: xerrori.c xerrori.h
	$(CC) $(CFLAGS) -c xerrori.c -o $@

cleanC:
	rm -f *.o cammini.out

cleanJ:
	rm -f *.class

clean:
	rm -f *.o *.out *.class
