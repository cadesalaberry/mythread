if [ -e ./mythread ]
	then make clean #> /dev/null
fi
make #tester #> /dev/null
./mythread
#make clean #> /dev/null
