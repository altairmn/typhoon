#include <zmq.hpp>
#include <time.h>
#include <sys/time.h>
#include <iostream>
#include <cstdlib>
#include <json.hpp>
#include <utility>
#include <thread>
#include <mutex>

using json = nlohmann::json;

template<typename T>
class CircularQueue {
  public:
    int size, start, end;
    std::mutex m;
    std::vector<T> queue;
    CircularQueue (int _size): size{_size}, start(0), end(0) {
      queue.reserve(size);
    }
    void push(T);
    T exec() { 
      // collect everything from the start to the end
      int till = end;
      // return closing price
      start = end;
      return queue[std::max(till - 1, 0)];
    }
};

template<typename T>
void CircularQueue<T>::push(T elem) {
  if ( (end + 1) % size != start ) {
    queue.push_back(elem);
    end = (end + 1) % size;
  }
}

void send_every(void* context,
    std::chrono::system_clock::time_point nextStartTime,
    std::chrono::milliseconds interval,
    CircularQueue< std::pair<long long int, float> >& cque) 
{
  zmq::socket_t sender(*(zmq::context_t*)context, ZMQ_PUSH);
  zmq::message_t message;
  while(true) {
    std::pair <long long int, float> closing = cque.exec();
    // TODO: send here
    
    nextStartTime += interval; 
    std::this_thread::sleep_until(nextStartTime);
  }
}

inline std::chrono::system_clock::time_point get_closest_tp(int interval) {
    // Get current time with precision of milliseconds
    auto now = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
    // sys_milliseconds is type time_point<system_clock, milliseconds>
    using sys_milliseconds = decltype(now);
    // Convert time_point to signed integral type
    auto integral_duration = now.time_since_epoch().count();
    integral_duration = integral_duration - integral_duration%interval + interval;
    sys_milliseconds dt{std::chrono::milliseconds{integral_duration}};
    return dt;
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


    std::thread consumer(
        send_every,
        static_cast<void*>(context.get()),
        get_closest_tp(interval),
        std::chrono::milliseconds{interval},
        std::ref(cque)
        );

    // get next time stamp
    
    auto get_price = [](std::string _pr) -> float { return std::stof(std::string(_pr.begin() + 1, _pr.end() - 1)); };

    while(true) {
        receiver.recv(&message);
        std::string smessage(static_cast<char*>(message.data()), message.size());
        // convert to json string
        json jmsg = json::parse(smessage);
#if defined BINANCE
        std::cout << "defined binance" << std::endl;
        cque.push(std::make_pair(jmsg["T"], get_price(jmsg["p"])));
#elif defined BITMEX
        std::cout << "defined BITMEX" << std::endl; 
#endif
        std::cout << jmsg.dump() << std::endl;
    }

    // consumer.join();

    //  Start our clock now
    return 0;
}

