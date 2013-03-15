if [ -e ./mythread ]
	then make clean > /dev/null
fi
make > /dev/null
./mythread
make clean > /dev/null
