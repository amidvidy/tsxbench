#include <chrono>

namespace bench {

  typedef std::chrono::high_resolution_clock timer;

  //typedef std::chrono::duration duration;

  typedef std::chrono::nanoseconds nanos;
  typedef std::chrono::microseconds micros;
  typedef std::chrono::milliseconds millis;

  typedef struct BenchConf {
    
    int nthreads;
    int nwrites;

  } benchconf_t;
  
  enum Impl { Coarse, Fine, RTMAdaptiveCoarse, RTMAdaptiveFine, RTMNaive, NoSync };
  const char* ImplStr[] = { "Coarse", "Fine", "RTMAdaptiveCoarse", "RTMAdaptiveFine", "RTMNaive"};

  class Benchmark {
    
    virtual long doBench(Impl impl, benchconf_t conf) = 0;
    //virtual duration<double, nanos> getElapsed();

  };

} // namespace bench
