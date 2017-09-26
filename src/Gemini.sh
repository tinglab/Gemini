#!/bin/bash

infile1=""
infile2=""
thresh=0.01

if [ $# -lt 2 ]; then
	echo "must give tow input .ge file"
	exit
else
	infile1=$1
	infile2=$2
fi

if [ $# -eq 3 ]; then
	thresh=$3
fi

echo "start compute similarity matrix......"
g++ getmat.cpp -o getmat.exe
./getmat.exe ${infile1} ${infile2} ${thresh}
python p_value.py
