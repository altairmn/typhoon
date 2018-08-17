#include <iostream>
#include <zmq.hpp>
#include <json.hpp>

using json = nlohmann::json;

/* use to and from JSON
 * push in one, and pop in the other
 * use single thread
 */
/*
 * what needs to be done in the other thread
 * serialize, remove from the map
 */

struct tick {
  tick() : p1(0.0), p2(0.0), p1_set(0), p2_set(0), t1(0), t2(0) {}
  float p1, p2;
  long long int t1, t2;
  bool p1_set, p2_set;
};
std::map<long long int, tick> tickMap;
int main() {
  // need a shared map
  std::shared_ptr<zmq::context_t> context = std::make_shared<zmq::context_t>(1);
  zmq::socket_t receiver(*context.get(), ZMQ_PULL);
  receiver.bind("tcp://*:5559");

  zmq::message_t message;

// receive message
// decode json

  while (true) {
      receiver.recv(&message);
      std::string smessage(static_cast<char*>(message.data()), message.size());
      // convert to json string
      json jmsg = json::parse(smessage);
      long long int ts = jmsg["T"];

      tickMap.emplace(ts, tick());

      if (jmsg["id"]  == 1) {
        tickMap[ts].p1 = jmsg["p"];
        tickMap[ts].p1_set = true;
        tickMap[ts].t1 = jmsg["tr"];
      }
      else {
        tickMap[ts].p2 = jmsg["p"];
        tickMap[ts].p2_set = true;
        tickMap[ts].t2 = jmsg["tr"];
      }

      if(tickMap[ts].p1_set && tickMap[ts].p2_set) {
        auto it = tickMap.find(ts);
        // send it
        // std::cout << "[" << ts << "]: " << it->second.p1 << ", " << it->second.p2 << std::endl;
        long long int get_ts = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        std::cout << get_ts - std::max(it->second.t1, it->second.t2) << std::endl;
        // erase it
        tickMap.erase(it);
      }
  }
  return 0;
}
