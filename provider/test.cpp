// testing condition variable
#include <iostream>
#include <string>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/posix_time/posix_time_io.hpp>
#include <boost/date_time.hpp>
#include <sstream>
#include <json.hpp>

using json = nlohmann::json;

int main(int argc, char* argv[]) {
  json j;
  j["T"] = 123901452804194012;
  j["p"] = 0.1239801;
  j["id"] = 1;

  std::cout << j.dump() << std::endl;


  return 0;
}
