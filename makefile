CC = gcc
DEBUGFLAGS = --DDEBUG -g -DNO_OUTPUT
PERFOMANCEFLAGS = -DPERFORMANCE
CFLAGS = -I.
PROCESS_NUM = 1
FILE_NAME = test1.xml
OBJS = parse.o preprocess.o post_process.o psax.o pthread_barriers.o
SERIALFLAGS = -DSERIAL

.PHONY: test
test: testmain psax.a
	./testmain $(PROCESS_NUM) $(FILE_NAME)

testmain: testmain.c psax.a
	$(CC) -o $@ psax.a testmain.c $(CFLAGS) $(DEBUGFLAGS) $(PERFOMANCEFLAGS)

%.o: %.c
	$(CC) -c $< $(CFLAGS) $(DEBUGFLAGS) $(SERIALFLAGS) $(PERFOMANCEFLAGS)

psax.a: $(OBJS)
	ar -r $@ $^
	rm *.o

