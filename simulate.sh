if [ -e ./*a.o ]
	then make clean
fi
make tester
./mythread
#make clean > /dev/null
