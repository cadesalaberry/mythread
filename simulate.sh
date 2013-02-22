if [ -e ./*.o ]
	then make clean
fi
make > /dev/null
./mythread
make clean > /dev/null
