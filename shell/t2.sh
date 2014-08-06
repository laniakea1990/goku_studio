#########################################################################
# File Name: t2.sh
# Author: ma6174
# mail: ma6174@163.com
# Created Time: Thu 24 Jul 2014 01:26:10 AM PDT
#########################################################################
#!/bin/bash

echo "Enter password"
read trythis

while [ "$trythis" != "secret" ]; do
	echo "Sorry, try again"
	read trythis
done
echo "Congratulations!"
exit 0
