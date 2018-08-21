#include <zmq.hpp>
#include <time.h>
#include <sys/time.h>
#include <iostream>
#include <cstdlib>
#include <json.hpp>
#include <utility>
#include <thread>
#include <mutex>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/posix_time/posix_time_io.hpp>
#include <boost/date_time.hpp>
#include <sstream>

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
      // return closing price
      T retval = queue[std::max(end-1, 0)];
      start = end;
      return retval;
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
#if defined ALIGN_PORT
  std::string bind_addr = "tcp://localhost:"
    + std::to_string(ALIGN_PORT);
  sender.connect(bind_addr.c_str());
#endif
  while(true) {

    std::pair <long long int, float> closing = cque.exec();
    // TODO: send here
    json j;
    j["T"] = std::chrono::duration_cast<std::chrono::milliseconds>(nextStartTime.time_since_epoch()).count();
    j["p"] = closing.second;
#if defined BINANCE
    j["id"] = 1;
#elif defined BITMEX
    j["id"] = 2;
#endif
#if defined DEBUG
    j["t"] = closing.first;
#endif
    std::string _msg = j.dump();
    int len = _msg.length();
    zmq::message_t msg (len);
    memcpy(msg.data(), _msg.c_str(), len);
    sender.send(msg);

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

#if defined BINANCE
inline float get_price (std::string _pr) {
  return std::stof(_pr);
}
#endif
#if defined BITMEX
long long int get_ts(std::string dt) {
    boost::posix_time::ptime pt;
      { 
          std::istringstream iss(dt);
          auto* f = new boost::posix_time::time_input_facet("%Y-%m-%dT%H:%M:%S%fZ");

          std::locale loc(std::locale(""), f);
          iss.imbue(loc);
          iss >> pt;
      }
    return (pt - boost::posix_time::ptime{{1970,1,1},{}}).total_milliseconds();
}
#endif
int main (int argc, char *argv[])
{
    //  Prepare our context and socket
    std::shared_ptr<zmq::context_t> context = std::make_shared<zmq::context_t>(1);
    zmq::socket_t receiver(*context.get(), ZMQ_PULL);
    int interval, port;

    if (argc != 3) {
      std::cout << "Usage: "
                << argv[0]
                << " agg_interval (in ms)"
                << " port"
                << std::endl;
      return -1;
    } else {
      interval = atoi(argv[1]);
      port = atoi(argv[2]);
    }
    std::string bind_addr = "tcp://*:" + std::to_string(port);
    receiver.bind(bind_addr.c_str());
    zmq::message_t message;
    // receiver.recv(&message);


    CircularQueue < std::pair<long long int, float> > cque(1000);


    std::thread consumer(
        send_every,
        static_cast<void*>(context.get()),
        get_closest_tp(interval),
        std::chrono::milliseconds{interval},
        std::ref(cque)
        );

    // get next time stamp
    

    while(true) {
        receiver.recv(&message);
        std::string smessage(static_cast<char*>(message.data()), message.size());
        // convert to json string
        json jmsg = json::parse(smessage);
#if defined BINANCE
        cque.push(std::make_pair(jmsg["T"], get_price(jmsg["p"])));
#elif defined BITMEX
        for (auto k : jmsg["data"]) {
          cque.push(std::make_pair(get_ts(k["timestamp"]), k["price"]));
        }
#endif
    }
    return 0;
}

