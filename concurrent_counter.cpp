#include <emmintrin.h>
#include "concurrent_counter.hpp"
#include "transactional.hpp"

namespace bench {
namespace counter {

void IncorrectConcurrentCounter::increment(std::size_t index) {
  ++storage[index];
}

void IncorrectConcurrentCounter::incrementRange(std::size_t index, std::size_t len) {
  for (std::size_t idx = 0; idx < len; ++idx) {
    ++storage[index+idx];
  }
}

int IncorrectConcurrentCounter::get(std::size_t index) {
  return storage[index];
}

ConcurrentCounter::ConcurrentCounter(std::size_t size) {
  sz = size;
  // parens ensure array elements are initialized to 0
  storage = new int[size]();
}

ConcurrentCounter::~ConcurrentCounter() {
  delete [] storage;
}

std::size_t ConcurrentCounter::size() {
  return sz;
}

int ConcurrentCounter::total() {
  int total = 0;
  for (int i = 0; i < sz; ++i) {
    total += storage[i];
  }
  return total;
}

void CoarseConcurrentCounter::increment(std::size_t index) {
  spinlock_t::scoped_lock acquire(spinlock, true);
  ++storage[index];
}

void CoarseConcurrentCounter::incrementRange(std::size_t index, std::size_t len) {
  spinlock_t::scoped_lock acquire(spinlock, true);
  for (std::size_t idx = 0; idx < len; ++idx) {
    ++storage[index+idx];
  }
}

int CoarseConcurrentCounter::get(std::size_t index) {
  spinlock_t::scoped_lock acquire(spinlock);
  return storage[index];
}

void FineConcurrentCounter::increment(std::size_t index) {
  spinlock_t::scoped_lock acquire(spinlocks[index], true);
  int current = storage[index];
  storage[index] = current + 1;
}

void FineConcurrentCounter::incrementRange(std::size_t index, std::size_t len) {
  for (std::size_t idx = 0; idx < len; ++idx) {
    std::size_t cur = index + idx;
    spinlocks[cur].lock();
    ++storage[cur];
    spinlocks[cur].unlock();
  }
}

int FineConcurrentCounter::get(std::size_t index) {
  spinlock_t::scoped_lock acquire(spinlocks[index]);
  return storage[index];
}

void RTMCoarseConcurrentCounter::increment(std::size_t index) {
  sync::TransactionalScope xact(spinlock, true);
  int current = storage[index];
  storage[index] = current + 1;
}

void RTMCoarseConcurrentCounter::incrementRange(std::size_t index, std::size_t len) {
  sync::TransactionalScope xact(spinlock, true);
  for (std::size_t idx = 0; idx < len; ++idx) {
    ++storage[index + idx];
  }
}

int RTMCoarseConcurrentCounter::get(std::size_t index) {
  sync::TransactionalScope xact(spinlock);
  return storage[index];
}

void RTMFineConcurrentCounter::increment(std::size_t index) {
  sync::TransactionalScope xact(spinlocks[index], true);
  int current = storage[index];
  storage[index] = current + 1;
}

void RTMFineConcurrentCounter::incrementRange(std::size_t index, std::size_t len) {
  sync::TransactionalScope xact(spinlocks[index], true);
  for (std::size_t idx = 0; idx < len; ++idx) {
    ++storage[index + idx];
  }
}


int RTMFineConcurrentCounter::get(std::size_t index) {
  sync::TransactionalScope xact(spinlocks[index]);
  return storage[index];
}

/* For some reason, optimizations result in lost writes here. Probably a compiler bug. */
#pragma optimize("", off)
void RTMConcurrentCounter::increment(std::size_t index) {
  do { 
    _xbegin();
  } while (!_xtest());
  int current = storage[index];
  storage[index] = current + 1;
  _xend();
}

#pragma optimize("", off)
void RTMConcurrentCounter::incrementRange(std::size_t index, std::size_t len) {
  do { 
    _xbegin();
  } while (!_xtest());
  
  for (std::size_t idx = 0; idx < len; ++idx) {
    ++storage[index + idx];
  }
  _xend();
}

/* For some reason, optimizations result in lost writes here. Probably a compiler bug. */
#pragma optimize("", off)
int RTMConcurrentCounter::get(std::size_t index) {
  int ret = -1;
  do {
    _xbegin();
  } while (!_xtest());
  ret = storage[index];
  _xend();
  return ret;
}

} // namespace counter {
} // namespace bench {
