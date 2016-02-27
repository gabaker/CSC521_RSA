#!/usr/bin/python

import sys
import base64
import binascii

num_args = len(sys.argv)

if (num_args < 2):
   print("Usage: python base16ToBase10.py file_name")
   sys.exit()

inFile = open(sys.argv[1], 'r')
outFileName = "" + sys.argv[1] + ".base10"
outFile = open(outFileName, 'w')
print (outFileName)

for line in inFile:
   lineLen =  len(line) 
   outFile.write("" + str(bin(int(str(line), 16))) + "\n")
   #outFile.write("" + str(int(str(line), 16)) + "\n")
