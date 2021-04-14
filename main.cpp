#include "sjp.hpp"
#include <chrono>

typedef std::chrono::high_resolution_clock hclock;

int main(int argc, char** argv) {
    sjp::parser parser;

    auto t1 = hclock::now();
    parser.add_file("Example.java");
    auto t2 = hclock::now();
   std::cout << "Time difference:"
      << std::chrono::duration_cast<std::chrono::milliseconds>(t2-t1).count() << " milliseconds" << std::endl;
    parser.parse();
    auto t3 = hclock::now();
   std::cout << "Time difference:"
      << std::chrono::duration_cast<std::chrono::milliseconds>(t3-t1).count() << " milliseconds" << std::endl;
    return 0;
}
