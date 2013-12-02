#include "transactional.hpp"
#include <thread>
#include <iostream>
#include <emmintrin.h>
#include <immintrin.h>
#include "lock.hpp"

namespace bench {
namespace sync {

  __thread threadstate_t TransactionalScope::tstate = threadstate_t();

  TransactionalScope::TransactionalScope(spinlock_t &fallback_mutex, bool writeAccess) : 
    // initializaer list
    spinlock(fallback_mutex) 
  {
  unsigned int xact_status;
  threadstate_t &ts = tstate;
  
  ts.txCount++;

  // we are already executing transactionally, continue.
  if (_xtest()) return;

  do {
    xact_status = _xbegin();

    if (xact_status == _XBEGIN_STARTED) {
      
      if ( *(reinterpret_cast<int*>(&fallback_mutex)) == 0 ) { 
	return;
      } else { 
	_xabort(0xFF); 
      }
    
    } else { 
      /** We have aborted. */
      ++ts.totalAborts;
      ++ts.successiveAborts;

      // if we xaborted because the lock was held, acquire the lock
      if ((xact_status & _XABORT_EXPLICIT) && _XABORT_CODE(xact_status) == 0xFF) {
	ts.maxAborts = 1;
	ts.maxTxLen = 1;
	break;
      }

      //if xabort:retry or xabort:conflict is set retry
      if (xact_status & (_XABORT_RETRY | _XABORT_CONFLICT)) {
	ts.maxTxLen = 1;
      }

      // // if we used too much buffer space inside the transaction half the max transaction length
      if ((xact_status & _XABORT_CAPACITY)) {
	ts.maxTxLen = 1;
      }
      _mm_pause();
    }
  } while (ts.successiveAborts < ts.maxAborts);

  ts.fallbackTaken++;

  // Fallback to lock
  if (writeAccess) { 
    spinlock.lock(); 
  } else { 
    spinlock.lock_read();
  }
}

TransactionalScope::~TransactionalScope() {
  threadstate_t &ts = tstate;
  if (_xtest()) {
     if (ts.curTxLen < ts.maxTxLen) {
       ++ts.curTxLen;
       return;
     }
    _xend();

     if (ts.successiveAborts > 0) {
       ts.maxTxLen = 1;
       ts.maxAborts = 8;
    } else {
       ts.maxTxLen += 1;
       ts.maxAborts += 1;
     }

     ++ts.totalCommits;

  } else {
    spinlock.unlock();
    ts.maxTxLen = 1;
    ts.maxAborts = 8;
 }
  ts.successiveAborts = 0;
}

void TransactionalScope::printStats() {
  threadstate_t &ts = tstate;
  std::cout << "Thread Id: " << std::this_thread::get_id() << std::endl;
  std::cout << "txCount: " << ts.txCount << std::endl;
  std::cout << "totalCommits: " << ts.totalCommits << std::endl;
  std::cout << "totalAborts: " << ts.totalAborts << std::endl;
  std::cout << "fallbackTaken: " << ts.fallbackTaken << std::endl;
}

void TransactionalScope::printStats(ThreadState &ts) {
  std::cout << "Thread Id: " << std::this_thread::get_id() << std::endl;
  std::cout << "txCount: " << ts.txCount << std::endl;
  std::cout << "totalCommits: " << ts.totalCommits << std::endl;
  std::cout << "totalAborts: " << ts.totalAborts << std::endl;
  std::cout << "fallbackTaken: " << ts.fallbackTaken << std::endl;
}

}  // namespace sync {
}  // namespace bench {

