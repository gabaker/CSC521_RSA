#!/bin/sh

keyFile=$1
make tree
./scripts/rm_key_spacing.sh $1
python ./scripts/base64ToBase10.py ${1}.del
./find_primes ${1}.del.hex 2
