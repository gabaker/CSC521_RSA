CC=mpicxx -m64
LFLAGS=-lgmp -fopenmp -lgmpxx
CFLAGS=-std=c++0x
DFLAGS=-D TREE_GCD
PFILES=find_shared_primes.cpp 
EXE=find_primes

all: tree

tree: $(PFILES)
	$(CC) $(DFLAGS) $(CFLAGS) $(LFLAGS) $(PFILES) -o $(EXE)

basic: $(PFILES)
	$(CC) $(CFLAGS) $(LFLAGS) $(PFILES) -o $(EXE)

clean:
	rm find_primes
