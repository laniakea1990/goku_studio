#########################################################################
# File Name: j1.sh
# Author: ma6174
# mail: ma6174@163.com
# Created Time: Thu 24 Jul 2014 05:38:27 AM PDT
#########################################################################
#!/bin/bash

rm -f file_one

if [ -f file_one ] || echo "hello" || echo " there"
then
	echo "in if"
else
	echo "in else"
fi

exit 0
