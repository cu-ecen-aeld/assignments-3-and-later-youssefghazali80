#!/bin/bash

if [ $# -ne 2 ];
then 
	echo"You entered a wrong number of parameters"
	exit 1
else 
	dir_path=$(dirname $1)
	if [ ! -d $dir_path ];
	then 
		mkdir -p $dir_path
		if [ $? -ne 0 ];
		then
			echo "we could not create the directory of the file"
			exit 1
		fi	
	fi	
	touch $1
	if [ $? -ne 0 ];
	then
		echo "we could not create the file"
		exit 1
	else	
			echo $2 > $1
	fi	 
fi		
			
