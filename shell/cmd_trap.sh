#########################################################################
# File Name: cmd_trap.sh
# Author: JinRanRan
# mail: jinranran1990910@163.com
# Created Time: Tue 29 Jul 2014 05:35:47 AM PDT
#########################################################################
#!/bin/bash

trap 'rm -f /tmp/my_tmp_file_$$' INT
echo creating file /tmp/my_tmp_file_$$
date > /tmp/my_tmp_file_$$

echo "press interrupt (CTRL-C) to interrupt ...."
while [ -f /tmp/my_tmp_file_$$ ];do
	echo File exists
	sleep 1
done
echo The file no longer exists

trap INT
echo creating file /tmp/my_tmp_file_$$
date > /tmp/my_tmp_file_$$


echo "press interrupt (CTRL-C) to interrupt ...."
while [ -f /tmp/my_tmp_file_$$ ];do
	echo File exists
	sleep 1
done

echo we never get here 
exit 0
