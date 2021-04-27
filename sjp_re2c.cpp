/**
 * MIT License
 *
 * Copyright (c) 2021 Anton Lyxell
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <string>
#include <fstream>
#include <iostream>
#include <streambuf>
#include <cassert>
#include <souffle/SouffleInterface.h>
#include <tuple>
#include "sjp.hpp"

namespace sjp {

    tree_node::tree_node(std::tuple<std::string,int,int> tuple)
        : name(std::get<0>(tuple)),
          start_token(std::get<1>(tuple)),
          end_token(std::get<2>(tuple)) {}

    std::map<std::string,std::vector<std::shared_ptr<tree_node>>>
    tree_node::get_children() {
        return children;
    }

    std::string tree_node::get_name() {
        return name;
    }

    int tree_node::get_start_token() {
        return start_token;
    }

    int tree_node::get_end_token() {
        return end_token;
    }

    void tree_node::add_child(std::string symbol, std::shared_ptr<tree_node> child) {
        children[symbol].push_back(child);
    }

    parser::parser() : tokens() {
        program = souffle::ProgramFactory::newInstance("parser");
        assert(program != NULL);
    }

    parser::parser(const char* program_name) {
        program = souffle::ProgramFactory::newInstance(program_name);
        assert(program != NULL);
    }

    void parser::add_file(const char* filename) {
        std::ifstream t(filename);
        add_string(filename,
            std::string((std::istreambuf_iterator<char>(t)),
            std::istreambuf_iterator<char>()).c_str());
    }

    void parser::add_string(const char* filename, const char* content) {
        auto [tl, i32_value, token_type] = lex_string(filename, content);
        token_limits.emplace(filename, tl);
        souffle::Relation* relation = program->getRelation("token");
        assert(relation != NULL);
        for (int32_t i = 0; i < tokens[filename].size(); i++) {
            souffle::tuple tuple(relation);
            tuple << tokens[filename][i] << i;
            relation->insert(tuple);
        }
        relation = program->getRelation("num_tokens");
        assert(relation != NULL);
        souffle::tuple tuple(relation);
        tuple << (int32_t) tokens[filename].size();
        relation->insert(tuple);
        relation = program->getRelation("token_type");
        assert(relation != NULL);
        for (auto& [id, type] : token_type) {
            souffle::tuple tuple(relation);
            tuple << (int32_t) id << type;
            relation->insert(tuple);
        }
    }

    void parser::run() {
        program->run();
    }

    std::vector<std::tuple<std::string, int, int>>
    parser::get_tokens(const char* filename) {
        auto& limits = token_limits[filename];
        std::vector<std::tuple<std::string,int,int>> result;
        for (size_t i = 0; i < tokens[filename].size(); i++) {
           result.emplace_back(tokens[filename][i],
                               limits[i].first,
                               limits[i].second);
        }
        return result;
    }

    std::shared_ptr<tree_node>
    parser::get_ast(const char* filename) {

        auto& limits = token_limits[filename];

        souffle::Relation* parent_of_rel = program->getRelation("parent_of");
        assert(parent_of_rel != NULL);
        std::map<int, std::vector<std::pair<std::string, int>>> parent_of;
        for (auto& output : *parent_of_rel) {
            int child, parent;
            std::string symbol;
            output >> parent;
            output >> symbol;
            output >> child;
            assert(parent != 0);
            parent_of[parent].emplace_back(symbol, child);
        }

        souffle::Relation* ast_node_rel = program->getRelation("ast_node");
        assert(ast_node_rel != NULL);
        std::map<int, std::tuple<std::string,int,int>> ast_node;
        for (auto& output : *ast_node_rel) {
            int id;
            output >> id;
            if (id == 0) continue;
            auto record = program->getRecordTable().unpack(id, 3);
            assert(record != NULL);
            ast_node.emplace(
                    id,
                    std::tuple(
                        program->getSymbolTable().decode(record[0]),
                        limits[record[1]].first,
                        limits[record[2]-1].second));
        }

        souffle::Relation* root_rel = program->getRelation("root");
        assert(root_rel != NULL);

        std::unordered_map<std::shared_ptr<tree_node>, int> ptr_to_id;
        std::shared_ptr<tree_node> root = nullptr;

        for (auto& output : *root_rel) {
            int id;
            output >> id;
            root = std::make_shared<tree_node>(ast_node[id]);
            ptr_to_id[root] = id;
        }

        std::vector<std::shared_ptr<tree_node>> unpopulated_nodes = {root};

        while (unpopulated_nodes.size()) {
            auto node = unpopulated_nodes.back();
            unpopulated_nodes.pop_back();
            for (auto [symbol, id] : parent_of[ptr_to_id[node]]) {
                if (id) {
                    std::shared_ptr<tree_node> ptr =
                        std::make_shared<tree_node>(ast_node[id]);
                    ptr_to_id[ptr] = id;
                    node->add_child(symbol, ptr);
                    unpopulated_nodes.push_back(ptr);
                } else {
                    node->add_child(symbol, nullptr);
                }
            }
        }

        return root;

    }

    parser::~parser() {
        delete program;
    }

    /**
     * Expects a null-terminated string.
     */
    std::tuple<std::unordered_map<size_t, std::pair<size_t, size_t>>,
               std::unordered_map<size_t,int32_t>,
               std::unordered_map<size_t, std::string>>
    parser::lex_string(const char* filename, const char *content) {
        assert(filename != NULL);
        std::unordered_map<size_t, std::pair<size_t, size_t>> token_limits;
        std::unordered_map<size_t, int32_t> i32_value;
        std::unordered_map<size_t, std::string> token_type;
        const char* YYCURSOR = content;
        const char* YYMARKER;
        while (1) {
            const char *YYSTART = YYCURSOR;
            /*!re2c
            re2c:define:YYCTYPE = char;
            re2c:yyfill:enable = 0;

            '"' [^\x00"]* '"' {
                tokens[filename].push_back(std::string(YYSTART, YYCURSOR));
                token_type.emplace(tokens[filename].size()-1, "string");
                token_limits[tokens[filename].size()-1] = {
                    YYSTART - content,
                    YYCURSOR - content};
                continue;
            }
          
            "abstract" | "assert" | "boolean" | "break" | "byte" | "case" |
            "catch" | "char" | "class" | "const" | "continue" | "default" |
            "do" | "double" | "else" | "enum" | "extends" | "final" |
            "finally" | "float" | "for" | "goto" | "if" | "implements" |
            "import" | "instanceof" | "int" | "interface" | "long" |
            "native" | "new" | "package" | "private" | "protected" |
            "public" | "return" | "short" | "static" | "strictfp" |
            "super" | "switch" | "synchronized" | "this" | "throw" |
            "throws" | "transient" | "try" | "void" | "volatile" | "while" {
                tokens[filename].push_back(std::string(YYSTART, YYCURSOR));
                token_limits[tokens[filename].size()-1] = {
                    YYSTART - content,
                    YYCURSOR - content};
                continue;
            }
            [ \t\v\n\r] {
                continue;
            }
            "{" | "}" | "(" | ")" | "[" | "]" {
                tokens[filename].push_back(std::string(YYSTART, YYCURSOR));
                token_limits[tokens[filename].size()-1] = {
                    YYSTART - content,
                    YYCURSOR - content};
                continue;
            }
            "||" | "&&" | "|"  | "^"  | "&"  | "=="  | "!=" | "<" |
            ">"  | "<=" | ">=" | "<<" | ">>" | ">>>" | "+"  | "-" |
            "*"  | "/"  | "%" {
                tokens[filename].push_back(std::string(YYSTART, YYCURSOR));
                token_limits[tokens[filename].size()-1] = {
                    YYSTART - content,
                    YYCURSOR - content};
                continue;
            }
            "="  | "+="  | "-="  | "*="   | "/=" | "&=" | "|=" | "^=" |
            "%=" | "<<=" | ">>=" | ">>>=" {
                tokens[filename].push_back(std::string(YYSTART, YYCURSOR));
                token_limits[tokens[filename].size()-1] = {
                    YYSTART - content,
                    YYCURSOR - content};
                continue;
            }
            "0" | [1-9][0-9]* {
                tokens[filename].push_back(std::string(YYSTART, YYCURSOR));
                i32_value.emplace(tokens[filename].size()-1,
                                  std::stoi(tokens[filename].back()));
                token_type.emplace(tokens[filename].size()-1, "integer");
                token_limits[tokens[filename].size()-1] = {
                    YYSTART - content,
                    YYCURSOR - content};
                continue;
            }
            ";" | "," | "." {
                tokens[filename].push_back(std::string(YYSTART, YYCURSOR));
                token_limits[tokens[filename].size()-1] = {
                    YYSTART - content,
                    YYCURSOR - content};
                continue;
            }
            [a-zA-Z_][a-zA-Z_0-9]* {
                tokens[filename].push_back(std::string(YYSTART, YYCURSOR));
                token_type.emplace(tokens[filename].size()-1, "identifier");
                token_limits[tokens[filename].size()-1] = {
                    YYSTART - content,
                    YYCURSOR - content};
                continue;
            }
            * {
                break;
            }
            */
        }
        return {token_limits, i32_value, token_type};
    }
}

