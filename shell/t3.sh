#########################################################################
# File Name: t3.sh
# Author: ma6174
# mail: ma6174@163.com
# Created Time: Thu 24 Jul 2014 01:33:52 AM PDT
#########################################################################
#!/bin/bash

until who | grep "$1" > /dev/null
do
	sleep 60
done

#now ring the bell and announce the expected user

echo -e '\a'
echo "**** $1 has just logged in ****"

exit 0
