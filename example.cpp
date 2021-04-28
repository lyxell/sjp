#include "sjp.hpp"
#include <chrono>

typedef std::chrono::high_resolution_clock hclock;

int main(int argc, char** argv) {
    sjp::parser parser;
    const char* filename = "Example.java";

    auto t1 = hclock::now();
    parser.add_file(filename);
    auto t2 = hclock::now();
    std::cout << "Time difference:"
              << std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1)
                     .count()
              << " milliseconds" << std::endl;
    parser.parse();
    auto t3 = hclock::now();
    std::cout << "Time difference:"
              << std::chrono::duration_cast<std::chrono::milliseconds>(t3 - t1)
                     .count()
              << " milliseconds" << std::endl;
    std::cout << "Parsed " << parser.num_asts() << " ASTs" << std::endl;
    /*
    for (auto& [t, a, b] : parser.get_tuples(filename)) {
        std::cout << t << " " << a << " " << b << std::endl;
    }*/
    return 0;
}
