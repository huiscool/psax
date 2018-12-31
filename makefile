CC = gcc
TID = 1
TESTFLAGS = -DTEST_$(TID)
DEBUGFLAGS = -DDEBUG
CFLAGS = -I.

testmain: testmain.c psax.a
	gcc -o $@ psax.a testmain.c $(CFLAGS)

psax.a: psax.c psax.h makefile 
	gcc -c psax.c $(TESTFLAGS) $(DEBUGFLAGS)
	ar -r $@ psax.o
	rm *.o

