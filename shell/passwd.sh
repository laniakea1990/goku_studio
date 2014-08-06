#!/bin/sh

echo "Enter your password:"
read trythis

while [ "$trythis" != "secret" ]; do
   echo "Sorry, try again"
   read trythis
done
exit 0
