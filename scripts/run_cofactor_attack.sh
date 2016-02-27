#!/bin/sh

keyFile=$1
make tree
./scripts/rm_key_spacing.sh $1
python ./scripts/base16ToBase10.py ${1}.no_delim
./find_primes ${1}.no_delim.base10 10
