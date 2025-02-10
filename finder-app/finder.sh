#!/bin/bash 

if [ $# -ne 2 ];
then 
	echo "You did not enter the required parameters"
	echo "the correct format is ./finder.sh <directory_path> <text_string>"
	exit 1
		
else
		if [ ! -d $1  ];
		then
			echo "The path you entered is not a directory"
		else
			files_num=$(find $1 -type f | wc -l)
			lines_num=$(grep -r $2 $1 2>/dev/null  | wc -l)
			echo "The number of files are $files_num and the number of matching lines are $lines_num"
		fi	
fi
		
