import sys

fileName=sys.argv[1]

keyFile=open(fileName)

for line in keyFile:
   if (line[0] == 'n'):
      print line[3:-1]
