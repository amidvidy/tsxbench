CC=icc
TBB_LIB=/home/amidvidy/experiment/tbb/build/linux_intel64_icc_cc4.6_libc2.15_kernel3.2.0_release
TBB_INCLUDE=/home/amidvidy/experiment/tbb/include
CFLAGS=-Wall -std=c++11 -g2 -ftls-model=local-exec

all: bench

bench:
	$(CC) $(CFLAGS) bench.cpp -I$(TBB_INCLUDE) -o bench -lpthread -Wl,-rpath,$(TBB_LIB) -L$(TBB_LIB) -ltbb

run: bench
	./bench

clean:
	rm -rf *o bench
