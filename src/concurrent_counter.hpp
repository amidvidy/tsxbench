#include <array>
#include <mutex>

namespace bench {
namespace counter {

template <typename LockType>
class ConcurrentCounter {
public:
  ConcurrentCounter(std::size_t size): sz_(size)  {
      storage_ = new int[size]();
  }

  ~ConcurrentCounter() {
      delete [] storage_;
      storage_ = nullptr;
  };

  void increment(std::size_t index) {
    typename LockType::scoped_lock lock(lock_for(index));
    ++storage_[index];
  }

  int get(std::size_t index) {
    typename LockType::scoped_lock lock(lock_for(index));
    return storage_[index];
  }

  size_t size() { return sz_; }

  int total() {
      int sum = 0;
      for (int i = 0; i < sz_; ++i) {
          sum += storage_[i];
      }
      return sum;
  } // default implementation not threadsafe
protected:
  // Get the lock for a certain element
  virtual LockType& lock_for(std::size_t index) = 0;
  int* storage_;
  std::size_t sz_;
};

template <typename LockType>
class CoarseConcurrentCounter : public ConcurrentCounter<LockType> {
public:
  CoarseConcurrentCounter(std::size_t size) : ConcurrentCounter<LockType>(size) {}
protected:
  LockType& lock_for(std::size_t index) {
    return lock_;
  }
  LockType lock_;
};

template <typename LockType>
class FineConcurrentCounter : public ConcurrentCounter<LockType> {
public:
    FineConcurrentCounter(std::size_t size, std::size_t lock_count = 64) : lock_count_(lock_count), ConcurrentCounter<LockType>(size) {
    locks_ = new LockType[lock_count];
  }
  ~FineConcurrentCounter() {
    delete [] locks_;
    locks_ = nullptr;
  }
protected:
  LockType& lock_for(std::size_t index) {
    return locks_[index % lock_count_];
  }
  std::size_t lock_count_;
  LockType *locks_;
};

} // namespace counter
} // namespace bench
