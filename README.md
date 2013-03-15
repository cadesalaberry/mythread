# A User Level Threads Package
-----------------------------------------


## What is this project?

In this project, I developped a user-level threads package. It provides a simplified replacement for Pthreads. My library provides a preemptive multi-threading on Linux. Because it is built as a user library without any kernel support, it does not handle system calls properly. I implemented the thread package completely in C.

Remember that a thread is an independent execution of a program code. The multiple threads of the program share the resources that belong to the parent process.


## Notes on the Proposed Solution

Because this project is a library, I decided to put all the functions in a .h file, the way a library should be developped. I had a lot of trouble at the beginning to find the proper compiler command, but I finally ended up lucky and made a **makefile** out of it.

Also, because testing implied compiling and running the code a lot, I also made a simple [bash file](simulate.sh) to compile and run the provided code. I also provided a sample output in **sample_output**.

