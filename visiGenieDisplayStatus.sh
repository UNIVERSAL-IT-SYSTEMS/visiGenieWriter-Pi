#!/bin/bash
while true ; do
	# CPU % to gauge
	#./visiGenieWrite -x -ocool_gauge -i0 -v$(top -bn 1 | awk 'NR>7{s+=$9} END {print s}')
	top -bn 1 | awk 'NR>7{s+=$9} END {print s}' | ./visiGenieWrite -x -ocool_gauge -i0
	# Mem % to slider
	#./visiGenieWrite -x -oslider -i0 -v$(free -m | grep Mem | awk {'printf("%1.f", $3 / $2 *100)'})
	free -m | grep Mem | awk {'printf("%1.f", $3 / $2 *100)'} | ./visiGenieWrite -x -oslider -i0
	# IP address to string
	#./visiGenieWrite -x -s -i0 -v$(hostname -I)
	hostname -I | ./visiGenieWrite -x -s -i0
	# Process count to LED
	#./visiGenieWrite -x -oled_digits -i0 -v$(ps -e | wc -l)
	# Board temperature to LED
	#./visiGenieWrite -x -oled_digits -i0 -v$(vcgencmd measure_temp | sed 's/temp=//' | sed "s/'C//")
	vcgencmd measure_temp | sed 's/temp=//' | sed "s/'C//" | ./visiGenieWrite -x -oled_digits -i0
	sleep 1
done
