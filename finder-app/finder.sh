#!/bin/sh

FILESDIR=$1
SEARCHSTR=$2

if [ "$#" -eq 2 ]
then
	if  [ -d $1 ]
	then
		echo " argument $1 is a directory"
	else 
		echo " argument $1 is not a directory"
		exit 1
	fi
	
elif [ "$#" -eq 1 ]
then
	echo "only one parameter provided"
	exit 1

else
	echo "parameters not supplied"
	exit 1
fi

X="$(grep -rnwl $FILESDIR -e $SEARCHSTR | wc -l)"
Y="$(grep -rnw $FILESDIR -e $SEARCHSTR | wc -l)"


echo "The number of files are $X and the number of matching lines are $Y"
