#!/bin/sh

for file in $(ls j*.sh)
do
  lpr $file
done
exit 0
