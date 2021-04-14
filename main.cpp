#include "sjp.hpp"

int main(int argc, char** argv) {
    sjp::parser parser;
    parser.add_file("Example.java");
    parser.parse();
    return 0;
}
