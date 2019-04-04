CC = gcc
DEBUGFLAGS = -g -O3 -DDEBUG
PERFOMANCEFLAGS = -DPERFORMANCE
CFLAGS = -I.
PROCESS_NUM = 8
FILE_NAME = test1.xml
OBJS = parse.o preprocess.o post_process.o psax.o pthread_barriers.o
SERIALFLAGS = -DPARALLEL

.PHONY: test serial parallel

test: testmain psax.a
	./testmain $(PROCESS_NUM) $(FILE_NAME)

serial:
	make SERIALFLAGS=-DSERIAL

testmain: testmain.c psax.a
	$(CC) -o $@ psax.a testmain.c $(CFLAGS) $(DEBUGFLAGS) $(PERFOMANCEFLAGS)

%.o: %.c
	$(CC) -c $< $(CFLAGS) $(DEBUGFLAGS) $(SERIALFLAGS) $(PERFOMANCEFLAGS)

psax.a: $(OBJS)
	ar -r $@ $^
	rm *.o

