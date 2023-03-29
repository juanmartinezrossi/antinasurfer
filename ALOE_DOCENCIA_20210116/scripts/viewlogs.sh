#!/bin/bash
	
FILE=* #all files by default
LOGPATH=logs
XTERM=xterm
TAIL='tail -f'

REPOSPATH=$1
APP=$2

if ! test -n "$REPOSPATH" 
then
	echo "Need a repository path as 1st parameter."
	exit
fi

if ! test -n "$APP"
then
	echo "Need waveform name as 2nd parameter."
	exit
fi

DIR="$REPOSPATH/$LOGPATH/$APP/"

echo ""
echo "==== Opening logs at path: $DIR ==== "
echo ""

if ! test -d $DIR
then
	echo "Error directory $DIR does not exist"
	exit
fi

for i in $DIR/$FILE.log
do
	echo "Opening xterm for $i..."
	$XTERM -e $TAIL $i &
done

