#!/bin/sh
#Version:  ###
#Modfif :  ###
#Date   :  ###

#Version:  v1.0
#Writer :  Huang_chuan 
#Date   :  9/25/2012
####Set test parameter######
trap 'pkill x86_send && sleep 2 &&  pkill x86_recv' 2 3
rm -rf log/send/*
rm -rf log/recv/*
rm -rf log/test.log
echo -n  "\033[0;32;1mPlease input the serial number:\033[0m" ;read COM_NUM
while  [ -z $COM_NUM ]; do
echo -n "\033[0;31;1mError!,please input serial number:\033[0m"
read COM_NUM
done
echo -n "\033[0;32;1mPlease input the baudrate:\033[0m"
read BAUDRATE
echo -n "\033[0;32;1mPlease input the test time(min):\033[0m"
read TESTTIME

####call test procedure######
export I=0
export Y=0
export YY=0
export ZZ
while [ "$I" -lt "$COM_NUM" ]; do
ZZ=`expr $I % 2`
if [ $ZZ -eq 0 ]; then
#echo $ZZ
gnome-terminal --title=Com${I}_Test_Window -e "./x86_recv /dev/ttyS${I} ${BAUDRATE} 8 0 0 2 1 232" --geometry=60x20+0+$Y &
Y=$(($Y+150))
else
gnome-terminal --title=Com${I}_Test_Window -e "./x86_recv /dev/ttyS${I} ${BAUDRATE} 8 0 0 2 1 232" --geometry=60x20+600+$YY &
#echo $X
YY=$(($YY+150))
fi
sleep 1
./x86_send /dev/ttyS${I} ${BAUDRATE} 8 0 0  2 1 232 >> send_tmp &
I=$(($I+1))
done

sleep ${TESTTIME}m
#sleep ${TESTTIME}
pkill x86_send 
echo "Test result  will be displayed in \033[0;31;1m$COM_NUM\033[0m seconds"
sleep $COM_NUM
pkill x86_recv
mv SCom* log/send
mv RCom* log/recv

####Display Test Result########
cd log
echo "################Test Result"###############
export T=0
while [ "$T" -lt "$COM_NUM" ]; do
if [ -f recv/RCom${T} ]; then
diff  send/SCom${T} recv/RCom${T} &&  echo  "Com${T} test:\033[0;32;1mPass\033[0m" | tee -a test.log ||  echo  "Com${T} test:\033[0;31;1mFail\033[0m" |tee -a test.log
else echo   "Com${T} not recv"
fi
T=$(($T+1))
done
echo  "###############The end.....###############"
