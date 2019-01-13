CC = gcc
TID = 2
TESTFLAGS = -DTEST_$(TID)
DEBUGFLAGS = -DDEBUG -g
CFLAGS = -I.
PROCESS_NUM = 1
FILE_NAME = test2.xml
OBJS = parse.o preprocess.o sendrecvbuf.o psax.o pthread_barriers.o


.PHONY: test
test: testmain psax.a
	./testmain $(PROCESS_NUM) $(FILE_NAME)

testmain: testmain.c psax.a
	$(CC) -o $@ psax.a testmain.c $(CFLAGS) $(DEBUGFLAGS)

%.o: %.c
	$(CC) -c $< $(CFLAGS) $(DEBUGFLAGS) $(TESTFLAGS)

psax.a: $(OBJS)
	ar -r $@ $^
	rm *.o

