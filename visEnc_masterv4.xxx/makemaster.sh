#!/bin/bash

#dirname=$(basename `pwd`)
dirname=visEnc.master
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
ls -l -c ~/upload/visEnc.master
cp $dirname ~/upload/visEnc.master
ls -l -c ~/upload/visEnc.master
echo make success
visu -l $dirname -r /mnt/apps/dm365_encode/visEnc.master -p $dstip
else	#make failure
echo make failed
fi
ctags -R *
