#include <iostream>
#include <mutex>
#include <chrono>
#include <condition_variable>
#include <json.hpp>
#include <zmq.hpp>
#include <thread>
#include <cstdio>


using json = nlohmann::json;

/*
 * Have 3 threads
 * One that increments
 * One that locks the increment
 * One that prints the number
 */

/* Program logic:
 * interval puts random intervals on increment and print
 * after the random interval, `print` prints the `count`
 * after the random interval, increment the `count
 */

class ThreadTest {
  private:
    int count, numIter;
    std::mutex m;
  public:
    ThreadTest(int _numIter) : numIter{_numIter}, count{0} {};
    ~ThreadTest() {
      std::cout << "Ending Program" << std::endl;
    }
    void increment() {
      std::chrono::system_clock::time_point nextStartTime { std::chrono::system_clock::now() };
      for (int i = 0; i < numIter; ++i) {
        {
          std::lock_guard<std::mutex> lock(m);
          count++;
        }
        nextStartTime += std::chrono::milliseconds { rand() % 100 };
        std::this_thread::sleep_until(nextStartTime);
      }
    }

    void print() {
      std::chrono::system_clock::time_point nextStartTime { std::chrono::system_clock::now() };
      std::chrono::system_clock::duration delay;
      for (int i = 0; i < numIter; ++i) {
        {
          std::lock_guard<std::mutex> lock(m);
          std::cout << count << std::endl;
        }
        nextStartTime += std::chrono::milliseconds { rand() % 100 };
        std::this_thread::sleep_until(nextStartTime);
      }
    }
};

int main() {
  ThreadTest tt(100);
   // start increment
  // start print
  // std::thread t2([&tt](){
  //     tt.print();
  //     });
  // 
  // tt.increment();

  // t2.join();
    using namespace std::chrono;
    // Get current time with precision of milliseconds
    auto now = time_point_cast<milliseconds>(system_clock::now());
    // sys_milliseconds is type time_point<system_clock, milliseconds>
    using sys_milliseconds = decltype(now);
    // Convert time_point to signed integral type
    auto integral_duration = now.time_since_epoch().count();
    integral_duration = integral_duration - integral_duration%5000 + 5000;
    sys_milliseconds dt{milliseconds{integral_duration}};
    std::cout<< dt.time_since_epoch().count() << std::endl;

  
  return 0;
}
