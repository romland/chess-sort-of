#!/bin/sh
PID = "`ps fax | grep "./dgd/bin/driver ./dgd/mud.dgd" | grep -v grep | cut -c1-5`"
echo "'$PID'"
if [ -n $PID ]; then
	echo "Shutting down DGD/MMOCG [$PID]"
	kill $PID
fi
echo 'Starting DGD/MMOCG...'
./dgd/bin/driver ./dgd/mud.dgd >& ./dgd/mud/driver.log &
tail -f ./dgd/mud/driver.log

