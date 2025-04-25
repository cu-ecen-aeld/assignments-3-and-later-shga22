#!/bin/bash

WRITEFILE=$1
WRITESTR=$2

if [ "$#" -eq 2 ]
then
	echo "creating new file"
	mkdir -p "$(dirname $1)" && touch "$1"
		

	
elif [ "$#" -eq 1 ]
then
	echo "only one parameter provided"
	exit 1

else
	echo "parameters not supplied"
	exit 1
fi


echo "$2" | tee $1
