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
    receiver.connect("tcp://localhost:5558");

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
    
    long long int this_ts;
    float prices[10000];
    int idx = 0;

    // always running
    while(true) {
      // assume binance
        receiver.recv(&message);
        std::string smessage(static_cast<char*>(message.data()), message.size());
        // convert to json string
        json jmsg = json::parse(smessage);
        this_ts = jmsg["T"];
        // get timestamp
        if (next_ts >= this_ts) {
          // aggregate and send
          idx = 0;
        }
    }

    //  Start our clock now
    struct timeval tstart;
    gettimeofday (&tstart, NULL);

    //  Process 100 confirmations
    int task_nbr;
    int total_msec = 0;     //  Total calculated cost in msecs
    for (task_nbr = 0; task_nbr < 100; task_nbr++) {

        receiver.recv(&message);
        std::string smessage(static_cast<char*>(message.data()), message.size());
        std::cout << smessage << std::endl;
        if ((task_nbr / 10) * 10 == task_nbr)
            std::cout << ":" << std::flush;
        else
            std::cout << "." << std::flush;
    }
    //  Calculate and report duration of batch
    struct timeval tend, tdiff;
    gettimeofday (&tend, NULL);

    if (tend.tv_usec < tstart.tv_usec) {
        tdiff.tv_sec = tend.tv_sec - tstart.tv_sec - 1;
        tdiff.tv_usec = 1000000 + tend.tv_usec - tstart.tv_usec;
    }
    else {
        tdiff.tv_sec = tend.tv_sec - tstart.tv_sec;
        tdiff.tv_usec = tend.tv_usec - tstart.tv_usec;
    }
    total_msec = tdiff.tv_sec * 1000 + tdiff.tv_usec / 1000;
    std::cout << "\nTotal elapsed time: " << total_msec << " msec\n" << std::endl;
    return 0;
}

