#!/usr/bin/python

# Script for removing copying the proto files in their purest form to the
# javascript folder. Avoiding us having to maintain two copies.

import sys


print 'num argv = ', len(sys.argv)
if ( len(sys.argv) < 2 ):
    print "Please give a input file"
    exit(1)

protoIn = sys.argv[1]
if ( not protoIn.endswith(".proto") ):
    print "Please give a proto file as input"
    exit(1)

protoOut = "../js/3rdparty/" + protoIn
output = open(protoOut, 'w')

for line in open(protoIn):
    goodLine = True
    strippedLine = line.strip()
    if (strippedLine == ""):
        goodLine=False
    elif (line.startswith("package")):
        goodLine=False
    elif (line.startswith("option")):
        goodLine=False
    else:
        output.write(line)
output.write("\n")
output.close()
print "Wrote to file: ", protoOut

