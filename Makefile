CC=g++
LFLAGS=-lgmp -fopenmp -lgmpxx
CFLAGS=-std=c++0x
PFILES=find_shared_primes.cpp 
EXE=find_primes

all:
	$(CC) $(CFLAGS) $(LFLAGS) $(PFILES) -o $(EXE)

clean:
	rm find_primes
