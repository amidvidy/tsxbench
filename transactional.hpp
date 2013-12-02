#include "tbb/spin_rw_mutex.h"
#include <iostream>
#include "lock.hpp"

namespace bench {
namespace sync {

// Forward declarations  
class TransactionalScope;
class ThreadState;

// lightweight thread-local state padded to the length of a cache line.
struct threadstate_t {
public:
  /* How many times this thread has entered transactional scope. */
  int txCount = 0;

  /* How many operations have been executed in the current transaction. */
  int curTxLen = 0;

  /* The maximum amount of operations to coalesce into a single transaction. */
  int maxTxLen = 1;

  /* the total number of transactional commits that have occurred during execution for this thread. */
  int totalCommits = 0;

  /* The total number of transactional aborts that have occurred during execution for this thread. */
  int totalAborts = 0;
  
  /* The number of successive aborts that have occurred. */
  int successiveAborts = 0;

  /* The maximum number of aborts that can occur before the lock is acquired. */
  int maxAborts = 10;

  /* How many times the fallback was taken. */
  int fallbackTaken = 0;

} __attribute__((aligned(64)));

class ThreadState : public threadstate_t {
public:
  ThreadState(const threadstate_t &state) : threadstate_t(state) {
    //std::cout << state.txCount << std::endl;
  }
  
  ThreadState() { 
    txCount = 0;
    curTxLen = 0;
    maxTxLen = 0;
    totalCommits = 0;
    totalAborts = 0;
    maxAborts = 0;
    fallbackTaken = 0;
  }

  ThreadState operator+(const ThreadState &other) {
    ThreadState ts;
    ts.txCount += other.txCount;
    ts.curTxLen += other.curTxLen;
    ts.maxTxLen += other.maxTxLen;
    ts.totalCommits += other.totalCommits;
    ts.totalAborts += other.totalAborts;
    ts.maxAborts += other.maxAborts;
    ts.fallbackTaken += other.fallbackTaken;
    return ts;
  }

  ThreadState& operator+=(const ThreadState &other) {
    txCount += other.txCount;
    curTxLen += other.curTxLen;
    maxTxLen += other.maxTxLen;
    totalCommits += other.totalCommits;
    totalAborts += other.totalAborts;
    maxAborts += other.maxAborts;
    fallbackTaken += other.fallbackTaken;
    return *this;
  }

};

class TransactionalScope {
public:
  TransactionalScope(spinlock_t &fallback, bool writeAccess = false);
  ~TransactionalScope();
  static void printStats();
  static void printStats(ThreadState &ts);
  static ThreadState getStats() { return ThreadState(tstate); }
private:
  static threadstate_t state();
  static __thread threadstate_t tstate;
  static __thread bool tstate_init;
  spinlock_t &spinlock;
  int lockstate;
};

}  // namespace sync
} // namespace bench
