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
	yes) echo "Good Morning";;
    no ) echo "Good Afternoon";;
    y  ) echo "Good Morning";;
    n  ) echo "Good Afternoon";;
    *  ) echo "Sorry, answer net recognized";;
esac

exit 0
