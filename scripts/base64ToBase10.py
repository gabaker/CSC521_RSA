#!/usr/bin/python

import sys
import base64
import binascii
num_args = len(sys.argv)

if (num_args < 2):
   print("Usage: python base64ToBase10.py file_name")
   sys.exit()

#print "Converting file: " + sys.argv[1]


inFile = open(sys.argv[1], 'r')

for line in inFile:
   lineLen =  len(line) 
   print "" + binascii.b2a_hex(base64.b64decode(line))
   
