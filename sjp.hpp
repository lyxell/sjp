#pragma once
#include <string>
#include <fstream>
#include <iostream>
#include <streambuf>
#include <cassert>
#include <souffle/SouffleInterface.h>
#include <tuple>

namespace sjp {

    std::tuple<std::vector<std::string>,
               std::unordered_map<size_t, std::pair<size_t, size_t>>,
               std::unordered_map<size_t,int32_t>,
               std::unordered_map<size_t, std::string>>
    lex_string(const char *content);

    class parser {
    private:
        souffle::SouffleProgram *program;
        std::unordered_map<std::string,
            std::unordered_map<size_t, std::pair<size_t, size_t>>>
                token_limits;
    public:
        parser();
        void add_file(const char* filename);
        void add_string(const char* filename, const char* content);
        void parse();
        std::vector<std::tuple<std::string,int,int>>
        get_tuples(const char* filename);
        size_t num_asts();
        ~parser();
    };
}
