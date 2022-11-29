###############################################################################################################
# Description: This script runs the QuickLabeling toolchain. 
#              Start options are loading from ql.config in same directory
#
# Author:      dmitriy moskalev (dm.moskalev@gmail.com)
#
# data:        2022.09.17
#
###############################################################################################################
#!/bin/sh
echo "------ORIGINAL GROUP quick labeling tool start v1.0------"
#set -x
echo "|||||||||||||||||||||||||||||||||||||||||||||||||||||||||"
echo "   "
echo " Load config options: "
ac_circle_threshold=$(grep '^[^#]' ./ql.config | grep ac_circle_threshold | cut -f 2 -d =)
ac_circle_min_area=$(grep '^[^#]' ./ql.config | grep ac_circle_min_area | cut -f 2 -d =)
ac_circle_max_area=$(grep '^[^#]' ./ql.config | grep ac_circle_max_area | cut -f 2 -d =)
ac_radius=$(grep '^[^#]' ./ql.config | grep ac_radius | cut -f 2 -d =)
echo " Aluminum can circle detector::"
echo "     threshold: "$ac_circle_threshold", min circle area: "$ac_circle_min_area", max circle area: "$ac_circle_max_area", radius: "$ac_radius

ac_latch_threshold=$(grep '^[^#]' ./ql.config | grep ac_latch_threshold | cut -f 2 -d =)
ac_latch_min_area=$(grep '^[^#]' ./ql.config | grep ac_latch_min_area | cut -f 2 -d =)
ac_latch_max_area=$(grep '^[^#]' ./ql.config | grep ac_latch_max_area | cut -f 2 -d =)
ac_scale=$(grep '^[^#]' ./ql.config | grep ac_scale | cut -f 2 -d =)
echo " Aluminum can latch detector::"
echo "     threshold: "$ac_latch_threshold", min latch area: "$ac_latch_min_area", max latch area: "$ac_latch_max_area", scale: "$ac_scale

bc_circle_threshold=$(grep '^[^#]' ./ql.config | grep bc_circle_threshold | cut -f 2 -d =)
bc_circle_min_area=$(grep '^[^#]' ./ql.config | grep bc_circle_min_area | cut -f 2 -d =)
bc_circle_max_area=$(grep '^[^#]' ./ql.config | grep bc_circle_max_area | cut -f 2 -d =)
bc_radius=$(grep '^[^#]' ./ql.config | grep bc_radius | cut -f 2 -d =)
echo " Bottle cap circle detector::"
echo "     threshold: "$bc_circle_threshold", min circle area: "$bc_circle_min_area", max circle area: "$bc_circle_max_area", radius: "$bc_radius

optShutter=$(grep '^[^#]' ./ql.config | grep optShutter | cut -f 2 -d =)
optGain=$(grep '^[^#]' ./ql.config | grep optGain | cut -f 2 -d =)
optcapcnt=$(grep '^[^#]' ./ql.config | grep optcapcnt | cut -f 2 -d =)
opthflip=$(grep '^[^#]' ./ql.config | grep opthflip | cut -f 2 -d =)
optvflip=$(grep '^[^#]' ./ql.config | grep optvflip | cut -f 2 -d =)
#optTriggerMode=$(grep '^[^#]' ./ql.config | grep optTriggerMode | cut -f 2 -d =)
optSaveMode=$(grep '^[^#]' ./ql.config | grep optSaveMode | cut -f 2 -d =)
echo " camera options::"
echo "     shutter: "$optShutter", gain: "$optGain
echo "     horizontal flip: "$opthflip", vertical flip: "$optvflip
echo "     save mode: "$optSaveMode

optMode=$(grep '^[^#]' ./ql.config | grep optMode | cut -f 2 -d =)
optGraph=$(grep '^[^#]' ./ql.config | grep optGraph | cut -f 2 -d =)
optPeriod=$(grep '^[^#]' ./ql.config | grep optPeriod | cut -f 2 -d =)
optLoopN=$(grep '^[^#]' ./ql.config | grep optLoopN | cut -f 2 -d =)
optDirectory=$(grep '^[^#]' ./ql.config | grep optDirectory | cut -f 2 -d =)
echo " toolchain options::"
echo "     mode: "$optMode", X-graphic output: "$optGraph
echo "     timeout: "$optPeriod" milliseconds, loop number: "$optLoopN
echo "     gallery directory for modes 0 and 2: "$optDirectory

optIP=$(grep '^[^#]' ./ql.config | grep optIP | cut -f 2 -d =)
optPort=$(grep '^[^#]' ./ql.config | grep optPort | cut -f 2 -d =)
echo " printer options::"
echo "     ip adress: "$optIP", port: "$optPort

echo "|||||||||||||||||||||||||||||||||||||||||||||||||||||||||"
echo "   "
QL_WORKING_DIR=quick_labeling
QL_GRABBER=OV9281_grabber_v1
QL_TRIGGER=imgen_gpio
QL_STRIGGER=imgen_single
QL_TCP_SERVER=tcp_server
QL_TCP_CLIENT=tcp_client

case $optMode in
	0)
		echo "SH: Aluminum can Gallery mode selected"
		echo "   "
		cd $(find /home/ -name $QL_WORKING_DIR 2>eaccess | head -1)
		cd ql/build/				
		./ql --mode $optMode --gain $optGraph -n $optLoopN --period $optPeriod --dir $optDirectory \
		--ct $ac_circle_threshold --cmin $ac_circle_min_area --cmax $ac_circle_max_area \
		--lt $ac_latch_threshold --lmin $ac_latch_min_area --lmax $ac_latch_max_area &
		P1=$!
		wait $P1
	;; 
	2)
		echo "SH: Bottle cap Gallery mode selected"
		echo "   "
		cd $(find /home/ -name $QL_WORKING_DIR 2>eaccess | head -1)
		cd ql/build/				
		./ql --mode $optMode --gain $optGraph -n $optLoopN --period $optPeriod --dir $optDirectory \
		--ct $bc_circle_threshold --cmin $bc_circle_min_area --cmax $bc_circle_max_area &
		P1=$!
		wait $P1
	;; 
	1)
		echo "SH: Aluminum can conveyor mode selected--------------------------------"
		echo "   "
		echo "SH: Check printer accessibility "
		cd $(find /home/ -name $QL_TCP_SERVER 2>eaccess | head -1)
		cd build/
		./tcp_server  &#start printer tcp server emulator 

		cd $(find /home/ -name $QL_TCP_CLIENT 2>eaccess | head -1)
		cd build/
		./tcp_client  -a $optIP -p $optPort -n 3 -t 1 #start ql tcp client in test mode 10 loops

		if [ $? -eq 0 ]
		then
			echo "SH: Printer is ready"	
		else
			echo "SH: Printer is not accessible" >&2
			exit 1
		fi
		
		echo "SH: Run trigger generator only as test option----------"
		echo "   "
		# please comment below 3 rows in production mode
		cd $(find /home/ -name $QL_TRIGGER 2>eaccess | head -1)
		cd build/
		./imgen -p $optPeriod -n $((optLoopN+150)) -t 15000 -v 0 &
		P3=$!	
		
		
		echo "SH: Run quick labeling tool chain---------------------"
		echo "   "
		cd $(find /home/ -name $QL_WORKING_DIR 2>eaccess | head -1)
		cd ql/build/		
		./ql --mode $optMode --gain $optGraph -n $optLoopN --period $optPeriod --dir $optDirectory --ct $ac_circle_threshold \
		--cr $ac_radius --cmin $ac_circle_min_area --cmax $ac_circle_max_area --lt $ac_latch_threshold \
		--lmin $ac_latch_min_area --lmax $ac_latch_max_area --ls $ac_scale &
		P1=$!

		echo "SH: Sleeping for 3 seconds…"
		sleep 3
		echo "SH: Completed"
		echo "   "
		echo "SH: Start printer server emulator "
		cd $(find /home/ -name $QL_TCP_SERVER 2>eaccess | head -1)
		cd build/
		./tcp_server &
		P4=$!
		echo "SH: Start tcp client in working mode "
		cd $(find /home/ -name $QL_TCP_CLIENT 2>eaccess | head -1)
		cd build/
		./tcp_client  -a $optIP -p $optPort -n $optLoopN -t 0 & #start tcp client in work mode
		P5=$!

		echo "SH: Run camera grabber--------------------------------"
		echo "   "
		cd $(find /home/ -name $QL_GRABBER 2>eaccess | head -1)
		cd build/
		./grabber -s $optShutter -g $optGain -h $opthflip -v $optvflip -c $optLoopN -w $optSaveMode &
		P2=$!
				
		wait $P1 $P2 $P3 $P4 $P5
		exit 0		
		
	;; 
	3)	echo "SH: Bottle cap conveyor mode selected-------------"
		echo "   "
		echo "SH: Check printer accessibility "
		#please comment below 3 rows in production mode
		cd $(find /home/ -name $QL_TCP_SERVER 2>eaccess | head -1)
		cd build/
		./tcp_server  &#start printer tcp server emulator 

		cd $(find /home/ -name $QL_TCP_CLIENT 2>eaccess | head -1)
		cd build/
		./tcp_client  -a $optIP -p $optPort -n 3 -t 1 #start ql tcp client in test mode 10 loops

		if [ $? -eq 0 ]
		then
			echo "SH: Printer is ready"	
		else
			echo "SH: Printer is not accessible" >&2
			exit 1
		fi
		
		
		# please comment below 6 rows in production mode
		echo "SH: Run trigger generator only as test option----------"
		echo "   "
		cd $(find /home/ -name $QL_TRIGGER 2>eaccess | head -1)
		cd build/
		./imgen -p $optPeriod -n $((optLoopN+150)) -t 15000 -v 0 &
		P3=$!	

		
		
		echo "SH: Run ACQ single impulse generator only as test option----------"
		echo "   "
		cd $(find /home/ -name $QL_STRIGGER 2>eaccess | head -1)
		cd build/
		./simgen -n $((optLoopN)) -t 1000 -v 0 &
		P6=$!
		
		
		echo "SH: Run quick labeling tool chain---------------------"
		echo "   "
		cd $(find /home/ -name $QL_WORKING_DIR 2>eaccess | head -1)
		cd ql/build/		
		./ql --mode $optMode --gain $optGraph -n $optLoopN --period $optPeriod --dir $optDirectory \
		--ct $bc_circle_threshold --cr $bc_radius --cmin $bc_circle_min_area --cmax $bc_circle_max_area &
		P1=$!

		echo "SH: Sleeping for 3 seconds…"
		sleep 3
		echo "SH: Completed"
		echo "   "
		#please comment below 5 rows in production mode
		echo "SH: Start printer server emulator "
		cd $(find /home/ -name $QL_TCP_SERVER 2>eaccess | head -1)
		cd build/
		./tcp_server &
		P4=$!
		echo "SH: Start tcp client in working mode "
		cd $(find /home/ -name $QL_TCP_CLIENT 2>eaccess | head -1)
		cd build/
		./tcp_client  -a $optIP -p $optPort -n $optLoopN -t 0 & #start tcp client in work mode
		P5=$!

		echo "SH: Run camera grabber--------------------------------"
		echo "   "
		cd $(find /home/ -name $QL_GRABBER 2>eaccess | head -1)
		cd build/
		./grabber -s $optShutter -g $optGain -h $opthflip -v $optvflip -c $optLoopN -w $optSaveMode &
		P2=$!
				
		wait $P1 $P2 $P5 $P6 $P3 $P4
		exit 0
	;; 
	*)
		echo "SH: the selected mode is out of range 0-3, check [optMode] option value in ql.config"
		exit 1
	;;
esac


