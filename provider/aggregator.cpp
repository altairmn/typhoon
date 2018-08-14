//
//  Task sink in C++
//  Binds PULL socket to tcp://localhost:5558
//  Collects results from workers via that socket
//
//  Olivier Chamoux <olivier.chamoux@fr.thalesgroup.com>
//
#include <zmq.hpp>
#include <time.h>
#include <sys/time.h>
#include <iostream>
#include <cstdlib>
#include <json.hpp>

using json = nlohmann::json;

int main (int argc, char *argv[])
{
    //  Prepare our context and socket
    zmq::context_t context(1);
    zmq::socket_t receiver(context, ZMQ_PULL);
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


    // get next time stamp
    long long int next_ts = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    next_ts = next_ts - next_ts % interval + interval;
    
    auto get_price = [](std::string _pr) -> float { return std::stof(std::string(_pr.begin() + 1, _pr.end() - 1)); };
    long long int this_ts;
    std::map <long long int, std::vector<float> > prices;

    // always running
    while(true) {
      // assume binance
        receiver.recv(&message);
        std::string smessage(static_cast<char*>(message.data()), message.size());
        // convert to json strint
        json jmsg = json::parse(smessage);
        this_ts = jmsg["T"];
      
        if (next_ts < this_ts) {
          long long int get_ts = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
          printf("Delay: %lld\n", get_ts - this_ts);
          next_ts += interval;
        }
        prices[next_ts].push_back(1);
    }

    //  Start our clock now
    return 0;
}

