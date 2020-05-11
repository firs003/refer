#!/bin/bash

dirname=$(basename `pwd`)
echo $dirname
if [ -z $1 ]
then
dstip="192.168.18.223"
else
dstip=$1
fi
echo dest ip is $dstip
make
if [ $? = 0 ] #make success
then
ls -l -c ~/upload/hdmi_enc/visEnc.slave
cp ${dirname} ~/upload/hdmi_enc/visEnc.slave
ls -l -c ~/upload/hdmi_enc/visEnc.slave
echo make success
visu -l ${dirname} -r /mnt/apps/dm365_encode/visEnc.slave -p $dstip
else	#make failure
echo make failed
fi
ctags *
