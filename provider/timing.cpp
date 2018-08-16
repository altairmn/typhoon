#include <chrono>
#include <iostream>
#include <thread>
#include <vector>
#include <numeric>

const double microPerClkTic{
  1.0E6 * std::chrono::system_clock::period::num / std::chrono::system_clock::period::den
};

const std::chrono::milliseconds intervalPeriodMillis { 100 };

int32_t cycles { 300 };
void periodicTask() {

  std::cout << "system_clock precision = "
            << microPerClkTic
            << " microseconds/tic"
            << std::endl
            << "Desired wakeup period = "
            << intervalPeriodMillis.count()
            << " milliseconds"
            << std::endl
            << "Num Wakeups = "
            << cycles - 1
            << std::endl;


  // Initialize the chrono timepoint and duration objects
  std::chrono::system_clock::time_point currentStartTime { std::chrono::system_clock::now() };
  std::chrono::system_clock::time_point nextStartTime { currentStartTime };

  std::vector <std::chrono::system_clock::duration> wakeupErrors;
  wakeupErrors.reserve(cycles);

  int32_t loopNum{};

  while (loopNum < cycles) {
    currentStartTime = std::chrono::system_clock::now();

    wakeupErrors.push_back(currentStartTime - nextStartTime);

    nextStartTime = currentStartTime + intervalPeriodMillis;


    std::this_thread::sleep_until(nextStartTime);

    ++loopNum;
  }


  // Print the average wakeup error
  std::chrono::system_clock::duration sum{};
  sum = std::accumulate(std::begin(wakeupErrors) + 1,
                        std::end(wakeupErrors),
                        sum);

  std::cout << "\nAverage wakeup error = "
            << std::chrono::duration_cast<std::chrono::microseconds>(sum).count() / (cycles - 1)
            << " microseconds\n";

}

int main() {
  periodicTask();

  return 0;
}
