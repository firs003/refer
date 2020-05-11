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
}

target="all" #default target
if [ -n $1 ] 
then
	target=$1
fi
if [ $target = "all" ]
then
	make_release Makefile.ti release/libvis_util_ti.a
	make_release Makefile.x86 release/libvis_util_x86.a
fi

exit 0
