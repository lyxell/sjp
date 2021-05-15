#pragma once
#include <cassert>
#include <fstream>
#include <iostream>
#include <souffle/SouffleInterface.h>
#include <streambuf>
#include <string>
#include <tuple>

namespace sjp {

using ast_node = int;
using token = std::tuple<std::string, std::string, size_t, size_t>;

struct ast {
    ast_node root;
    std::unordered_map<ast_node, std::string> name;
    std::unordered_map<ast_node, size_t> starts_at;
    std::unordered_map<ast_node, size_t> ends_at;
    std::unordered_map<ast_node, std::vector<std::pair<std::string, ast_node>>> parent_of;
    std::unordered_map<ast_node, std::vector<std::pair<std::string, std::vector<ast_node>>>> parent_of_list;
};

std::vector<token> lex(const char* content);

ast parse_file(const char* filename);
ast parse_string(const char* content);

} // namespace sjp
