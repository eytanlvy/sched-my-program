qCC = gcc -Wall -g -O3 -pthread

.PHONY: clean

main: quicksort

quicksort: quicksort.o sched.o
	$(CC) -o quicksort quicksort.o sched.o

quicksort.o: src/quicksort.c src/sched.h
	$(CC) -c src/quicksort.c

sched.o: src/sched.c src/sched.h
	$(CC) -c src/sched.c

clean:
	rm -f quicksort *.o
