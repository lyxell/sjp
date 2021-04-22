#pragma once
#include <string>
#include <fstream>
#include <iostream>
#include <streambuf>
#include <cassert>
#include <souffle/SouffleInterface.h>
#include <tuple>

namespace sjp {

    class tree_node {
    private:
        std::string name;
        int start_token;
        int end_token;
        std::vector<std::shared_ptr<tree_node>> children;
    public:
        tree_node(std::tuple<std::string,int,int>);
        std::string get_name();
        int get_start_token();
        int get_end_token();
        std::vector<std::shared_ptr<tree_node>> get_children();
        void add_child(std::shared_ptr<tree_node>);
    };

    class parser {
    private:
        souffle::SouffleProgram *program;
        std::unordered_map<std::string,
            std::unordered_map<size_t, std::pair<size_t, size_t>>>
                token_limits;
        std::unordered_map<std::string, std::shared_ptr<tree_node>> asts;
        std::tuple<std::vector<std::string>,
                   std::unordered_map<size_t, std::pair<size_t, size_t>>,
                   std::unordered_map<size_t,int32_t>,
                   std::unordered_map<size_t, std::string>>
        lex_string(const char *content);
    public:
        parser();
        parser(const char* program_name);
        ~parser();
        void add_file(const char* filename);
        void add_string(const char* filename, const char* content);
        void run();
        souffle::Relation* get_relation(const char* relation_name);
        std::vector<std::tuple<std::string,int,int>>
        get_ast_nodes(const char* filename);
        std::tuple<std::string,int,int>
        get_ast_node_from_id(const char* filename, int id);
        std::shared_ptr<tree_node>
        get_ast(const char* filename);
    };
}
