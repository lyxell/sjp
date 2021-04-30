#pragma once
#include <cassert>
#include <fstream>
#include <iostream>
#include <souffle/SouffleInterface.h>
#include <streambuf>
#include <string>
#include <tuple>

namespace sjp {

struct tree_node {
    std::string name;
    int start_token;
    int end_token;
    std::map<std::string, std::shared_ptr<tree_node>> parent_of;
    std::map<std::string, std::vector<std::shared_ptr<tree_node>>>
        parent_of_list;
};

class parser {
  private:
    std::unique_ptr<souffle::SouffleProgram> program;
    std::unordered_map<std::string,
                       std::unordered_map<size_t, std::pair<size_t, size_t>>>
        token_limits;
    std::unordered_map<std::string, std::shared_ptr<tree_node>> asts;
    std::unordered_map<std::string, std::vector<std::string>> tokens;
    std::tuple<std::unordered_map<size_t, int32_t>,
               std::unordered_map<size_t, std::string>>
    lex_string(const char* filename, const char* content);
    std::tuple<std::string, int, int> node_from_id(const char* filename,
                                                   int id);
    std::shared_ptr<tree_node> build_node(
        const char* filename,
        std::map<int, std::map<std::string, int>>& parent_of,
        std::map<int, std::map<std::string, std::vector<int>>>& parent_of_list,
        int id);

  public:
    parser();
    parser(const char* program_name);
    void add_file(const char* filename);
    void add_string(const char* filename, const char* content);
    void run();
    std::shared_ptr<tree_node> get_ast(const char* filename);
    std::vector<std::tuple<std::string, int, int>>
    get_tokens(const char* filename);
};
} // namespace sjp
