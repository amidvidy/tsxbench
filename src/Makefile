CC=icc
CFLAGS=-Wall -std=c++11 -g -ftls-model=local-exec -O2

all: bench

concurrent_counter.o: concurrent_counter.cpp
	$(CC) $(CFLAGS) -c concurrent_counter.cpp -ltbb

transactional.o: transactional.cpp
	$(CC) $(CFLAGS) -c transactional.cpp

bench.o: bench.cpp 
	$(CC) $(CFLAGS) -c bench.cpp

bench: bench.o transactional.o concurrent_counter.o
	$(CC) bench.o transactional.o concurrent_counter.o -o bench -lpthread -ltbb

run: bench
	./bench

clean:
	rm -rf *o bench



