#########################################################################
# File Name: func.sh
# Author: ma6174
# mail: ma6174@163.com
# Created Time: Thu 24 Jul 2014 06:02:11 AM PDT
#########################################################################
#!/bin/bash
yes_or_no()
{
	echo "Is your name $* ?"
	while true
	do
		echo -n "Enter yes or no: "
		read x
		case "$x" in
			y | yes ) return 0;;
		    n | no)   return 1;;
		    * )    echo "Answer yes or no"
		esac
	done
}
echo "Original parameters are $*"

if yes_or_no "$2"
then
	echo "Hi $1, nice name"
else
	echo "Never mind"
fi

exit 0
