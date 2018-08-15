#include <zmq.hpp>
#include <time.h>
#include <sys/time.h>
#include <iostream>
#include <cstdlib>
#include <json.hpp>
#include <utility>

using json = nlohmann::json;

template<typename T>
class CircularQueue {
  int size, start, end;
  std::mutex m;
  std::vector<T> queue;
  public:
    CircularQueue (int _size): size{_size}, start(0), end(0) {
      queue.reserve(size);
    }
    void push(T);
    T exec() { 
      // collect everything from the start to the end
      int till = end;
      // return closing price
      start = till;
      return queue[till];
    }
};

void send_every(void* context,
    std::chrono::sytem_clock::time_point nextStartTime,
    std::chrono::milliseconds interval,
    CircularQueue& cque) {
  zmq::socket_t sender(*(zmq::context_t*)context, ZMQ_PUSH);
  zmq::message_t message;
  while(true) {
    std::pair <long long int, float> closing = cque.exec();
    nextStartTime += interval; 
    std::this_thread::sleep_until(nextStartTime);
  }
}

/* have another sender connection
 * that sends the result of stat
 * every x seconds. the x second
 * wakeup is initiated by a thread
 * which wakes up every x seconds
 */

template<typename T>
void CircularQueue<T>::push(T elem) {
  if ( (end + 1) % size != start ) {
    queue.push_back(elem);
    end = (end + 1) % size;
  }
}

int main (int argc, char *argv[])
{
    //  Prepare our context and socket
    std::shared_ptr<zmq::context_t> context = std::make_shared<zmq::context_t>(1);
    zmq::socket_t receiver(*context.get(), ZMQ_PULL);
    receiver.bind("tcp://*:5558");

    zmq::message_t message;
    // receiver.recv(&message);

    int interval;

    if (argc != 2) {
      std::cout << "Usage: " << argv[0] << " agg_interval (in ms)" << std::endl;
      return -1;
    } else {
      interval = atoi(argv[1]);
    }

    CircularQueue < std::pair<long long int, float> > cque(1000);

    // get next time stamp
    long long int next_ts = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    next_ts = next_ts - next_ts % interval + interval;
    long long int this_ts;
    
    auto get_price = [](std::string _pr) -> float { return std::stof(std::string(_pr.begin() + 1, _pr.end() - 1)); };

    while(true) {
        receiver.recv(&message);
        std::string smessage(static_cast<char*>(message.data()), message.size());
        // convert to json string
        json jmsg = json::parse(smessage);
        cque.push(std::make_pair(jmsg["T"], get_price(jmsg["p"])));
      
        // if (next_ts < this_ts) {
        //   long long int get_ts = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        //   printf("Delay: %lld\n", get_ts - this_ts);
        //   next_ts += interval;
        // }
    }

    //  Start our clock now
    return 0;
}

