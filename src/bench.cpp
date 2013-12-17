#include <iostream>
#include <cstdlib>
#include <thread>
#include <map>
#include <chrono>
#include <ctime>

#include "bench.hpp"
#include "concurrent_counter.hpp"
#include "transactional.hpp"
#include "../ext/xsync/include/lock.hpp"

using namespace bench;

double MICROS_TO_NANOS = 1000.0;
double MICROS_TO_MILLIS = 0.001;
double MICROS_TO_SECONDS = 0.000001;
std::string DELIM(",");

template <typename LockType>
void hammerArray(counter::ConcurrentCounter<LockType> *counter, int nthreads, int nwrites, std::string implstr) {
  std::thread threads[nthreads];

  timer::time_point start_time = timer::now();

  // seed prng with time
  srand((unsigned)time(nullptr));

  sync::ThreadState ts;
  spinlock_t ts_lock;

  for (int thread_id = 0; thread_id < nthreads; ++thread_id) {

      threads[thread_id] = std::thread([thread_id, nwrites, counter, &ts, &ts_lock]() {
	int sz = counter->size();
	for (int times = 0; times < nwrites;  ++times) {
	  int idx = rand() % sz;
	  counter->increment(idx);
	}

      });

  }

  for (int i = 0; i < nthreads; ++i) {
    threads[i].join();
  }

  timer::time_point end_time = timer::now();
  auto run_time =  std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count();

  int total_recorded = counter->total();
  int total_expected = nthreads * nwrites;

  // i should really be using the std::chrono time conversions but this code is low priority
  auto avg = run_time / (double)total_recorded;

  int missing = total_recorded - total_expected;
  std::cout << implstr << DELIM; // Implementation Col
  std::cout << counter->size() << DELIM; // NumElements Col
  std::cout << (double)run_time * MICROS_TO_MILLIS << DELIM; // TimeMillis Col
  std::cout << total_recorded / ((double)run_time * MICROS_TO_SECONDS) << DELIM;
  std::cout << avg * MICROS_TO_NANOS  << std::endl; // AvgNanos Col
  if (total_recorded != total_expected) {
      std::cerr << "ERROR: MISSING "<< (total_expected-total_recorded) << " WRITES!" << std::endl;
  }
}

int main(void) {

  std::size_t max_elements = 1024;
  std::size_t granularity = 64;

  int nthreads = 8, nwrites = 1000000; // increase write count for additional accuracy

  // Print CSV headers
  std::cout << "Implementation,NumElements,TimeMillis,ThroughputWPerSec,AvgNanos" << std::endl;

  for (std::size_t num_elements = granularity; num_elements <= max_elements; num_elements += granularity) {
      counter::CoarseConcurrentCounter<xsync::lock_t> coarse(num_elements);
      hammerArray(&coarse, nthreads, nwrites, "coarse");
      counter::FineConcurrentCounter<xsync::lock_t> fine(num_elements);
      hammerArray(&fine, nthreads, nwrites, "fine");
      counter::CoarseConcurrentCounter<xsync::tsx_lock_t> tsx_coarse(num_elements);
      hammerArray(&tsx_coarse, nthreads, nwrites, "tsx_coarse");
      counter::FineConcurrentCounter<xsync::tsx_lock_t> tsx_fine(num_elements);
      hammerArray(&tsx_fine, nthreads, nwrites, "tsx_fine");

  }
}
