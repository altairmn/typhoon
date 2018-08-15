// testing condition variable
#include <iostream>
#include <mutex>
#include <chrono>

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

class CircularTaskQueue() {
private:
    int size, start, end, till;
    std::condition_variable cv;
    std::mutex lock;
    float* queue;
    zmq::context_t context;
    zmq::socket_t sender;
    zmq::message_t message;
public:
  CircularTaskQueue(int _size): 
    size(_size), start(0), end(0),
    context(1), receiver(context, ZMQ_PULL)
  {
    queue = new float[size]; 
    receiver.bind("tcp://*:5558");
  }

  ~CircularTaskQueue() {
    delete[] queue;
  }
  void push(float elem) {

    while(true) {
      receiver.recv(&message);
      receiver.recv(&message);
      std::string smessage(static_cast<char*>(message.data()), message.size());
      json jmsg = json::parse(smessage);
      this_ts = jmsg["T"];
      if (next_ts < this_ts) {
        long long int get_ts = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        printf("Delay: %lld\n", get_ts - this_ts);
        next_ts += interval;
      }
      prices[next_ts].push_back(1);
    }
    // this thread intends to modify the variable
    std::unique_lock<std::mutex> l(lock);
    aggregate = true;
    // check if full
    // when the other thread is consuming, then can't write to this.
    // other thread knows of the range in which these elements exist
    // check if the thread is full
    // otherwise, reexecute the whole thing
    
    // rockstar
    ev.notify_one();
    queue[end++] = elem;
  }

  void exec() {
    // acquire the lock
    std::unique_lock<std::mutex> l(lock);
    cv.wait(l, [] { return aggregate }); // wait until aggregate == True

    float sum = 0; 
    while (start < till) {
      sum += prices[start];
      start++;
    }

    aggregate = false;
    l.unlock();
  }
};

int main(int argc, char* argv[]) {

  return 0;
}
