#########################################################################
# File Name: t4.sh
# Author: ma6174
# mail: ma6174@163.com
# Created Time: Thu 24 Jul 2014 01:44:45 AM PDT
#########################################################################
#!/bin/bash

echo "Is it morning? Please answer yes or no"
read timeofday

case "$timeofday" in
	yes | y | Yes | YES)
		echo "Good Morning"
		echo "Up bright and early this morning"
		;;
	[nN]* ) echo "Good Afternoon"
		;;
    *  )
		echo "Sorry, answer net recognized"
		echo "Please answer yes or no"
		exit 1
		;;
esac

exit 0
