#!/usr/bin/python

import sys
import base64
import binascii
num_args = len(sys.argv)

if (num_args < 2):
   print("Usage: python base64ToBase10.py file_name")
   sys.exit()

inFile = open(sys.argv[1], 'r')
outFileName = "" + sys.argv[1] + ".hex"
outFile = open(outFileName, 'w')
print outFileName

for line in inFile:
   lineLen =  len(line) 
   outFile.write("" + str(bin(int(binascii.hexlify(base64.b64decode(line)), 16))) + "\n")
   #base64.b64decode(line)) + "\n")
   #binascii.b2a_hex(

