CC=g++
LFLAGS=-lgmp


all:
	$(CC) find_shared_primes.cpp $(LFLAGS) -o find_primes

clean:
	rm find_primes
