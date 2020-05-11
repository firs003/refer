function make_release {
	make clean
	make -f $1
	if [ $? -eq 0 ]
	then
		echo "Release $2 Success"
		mv release/libvis_util.a $2
	else
		echo "Release $2 Failure!"
	fi
	echo
}

target="all" #default target
if [ ! -z $1 ]
then
	target=$1
fi
echo target=$target
echo
if [ $target == "all" ]
then
	make_release Makefile.ti release/libvis_util_ti.a
	make_release Makefile.x86 release/libvis_util_x86.a
elif [ $target == "pc" -o $target == "x86" ]
then
	make_release Makefile.x86 release/libvis_util_x86.a
elif [ $target == "ti" -o $target == "arm" ]
then
	make_release Makefile.ti release/libvis_util_ti.a
else
	echo -e "\tUsage: sh makerelease [ti/arm/pc/x86]"
fi

exit 0
