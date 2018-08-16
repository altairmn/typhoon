// testing condition variable
#include <iostream>
#include <mutex>
#include <chrono>
#include <condition_variable>
#include <json.hpp>
#include <zmq.hpp>

using json = nlohmann::json;
// have a circular queue with a pointer.
// when the circular queue is full, no more is put into it
// and then a condition variable is used
// to call the aggregation function
// call the aggregation, whenever next timestamp is found

/*
 * `push` will be executed in one thread
 * The other thread will execute `exec`
 * capacity == full if end + 1 == start
 * capacity == empty if end == start
 * there'll always be one empty slot
 * all elements are [start, start+1 %mod, start+2 %mod, ..., end-1]
 * a function that is called periodically, does the work 
 */

/*
 * same code is used for alignment except the calling of exec is conditional
 * and not periodic
 */

class CircularTaskQueue {
private:
    int size, start, end, till, interval;
    std::condition_variable cv;
    std::mutex lock;
    long long int next_ts, till_ts;
    float* queue;
    zmq::context_t context;
    zmq::socket_t receiver;
    zmq::message_t message;
    bool aggregate;
public:
  CircularTaskQueue(int _size, int _interval): 
    size(_size), start(0), end(0), interval(_interval),
    context(1), receiver(context, ZMQ_PULL)
  {
    queue = new float[size]; 
    receiver.bind("tcp://*:5558");
  }

  ~CircularTaskQueue() {
    delete[] queue;
  }
  void push() {
    while(true) {
      /* receive messages here */
      receiver.recv(&message);
      std::string smessage(static_cast<char*>(message.data()), message.size());

      json jmsg = json::parse(smessage);
      std::cout << jmsg.dump() << std::endl;

      long long int this_ts = jmsg["T"];

      // queue[end] = jmsg["p"];
      end = (end + 1) % size;

      if (next_ts < this_ts) {
        std::unique_lock<std::mutex> l(lock);
        aggregate = true;
        till = end;
        till_ts = this_ts;
        next_ts += interval;
        cv.notify_one();
      }
    }
  }

  void exec() {
    std::unique_lock<std::mutex> l(lock);
    cv.wait(l, [=] { return aggregate; } ); // wait until aggregate == True

    float sum = 0; 
    while (start != till) {
      sum += queue[start];
      start = (start + 1) % size;
    }
    long long int get_ts = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    printf("[%f] at %lld delay", sum, till_ts - get_ts);
    aggregate = false;
  }
};

int main(int argc, char* argv[]) {
  CircularTaskQueue * cqueue = new CircularTaskQueue(1000, 2000);
  cqueue->push();

  return 0;
}
