#include <iostream>
#include <zmq.hpp>

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
  tick() : p1(0.0), p2(0.0), p1_set(0), p2_set(0) {}
  float p1, p2;
  bool p1_set, p2_set;
}
std::map<long long int, tick> tickMap;
int main() {
  // need a shared map
  std::shared_ptr<zmq::context_t> context = std::make_shared<zmq::context_t>(1);
  zmq::socket_t receiver(*context.get(), ZMQ_PULL);
  receiver.bind("tcp://*:5558");

  zmq::message_t message;

// receive message
// decode json

  while (true) {
      receiver.recv(&message);
      std::string smessage(static_cast<char*>(message.data()), message.size());
      // convert to json string
      json jmsg = json::parse(smessage);
      long long int ts = jsmg["T"];

      tickMap.emplace(ts, tick());

      if (jmsg["id"]  == 1) {
        tickMap[ts].p1 = jmsg["p"];
        tickMap[ts].p1_set = true;
      }
      else {
        tickMap[ts].p2 = jmsg["p"];
        tickMap[ts].p2_set = true;
      }

      if(tickMap[ts].p1_set && tickMap[ts].p2_set) {
        auto it = tickMap.find(ts);
        // send it
        std::cout << "[" << ts << "]: " << it->p1 << ", " << it->p2 << std::endl;
        // erase it
        tickMap.erase(it);
      }
  }
  return 0;
}
