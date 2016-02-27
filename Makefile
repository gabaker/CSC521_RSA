CC=g++ -m64
LFLAGS=-lgmp -fopenmp -lgmpxx
CFLAGS=-std=c++0x
DFLAGS=-D TREE_GCD
PFILES=find_shared_primes.cpp 
EXE=find_primes

all:
	$(CC) $(CFLAGS) $(LFLAGS) $(PFILES) -o $(EXE)

tree:
	$(CC) $(DFLAGS) $(CFLAGS) $(LFLAGS) $(PFILES) -o $(EXE)

clean:
	rm find_primes
