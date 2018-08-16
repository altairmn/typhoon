// testing condition variable
#include <iostream>
#include <mutex>
#include <chrono>
#include <condition_variable>
#include <json.hpp>
#include <zmq.hpp>
#include <cstdio>

using json = nlohmann::json;

struct tick {
  tick() : p1(0.0), p2(0.0), p1_set(0), p2_set(0) {}
  float p1, p2;
  bool p1_set, p2_set;
};

std::map <long long int, tick> mp;

int main(int argc, char* argv[]) {

  for(int i = 0; i < 1000; ++i) {
    int f = rand() % 100;
    mp.emplace(f, tick());
  }

  return 0;
}
