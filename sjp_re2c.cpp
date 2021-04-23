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

    std::vector<std::shared_ptr<tree_node>>
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

    void tree_node::add_child(std::shared_ptr<tree_node> child) {
        return children.push_back(child);
    }

    parser::parser() {
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
        auto [tokens, tl, i32_value, token_type] = lex_string(content);
        token_limits.emplace(filename, tl);
        souffle::Relation* relation = program->getRelation("token");
        assert(relation != NULL);
        for (int32_t i = 0; i < tokens.size(); i++) {
            souffle::tuple tuple(relation);
            tuple << tokens[i] << i;
            relation->insert(tuple);
        }
        relation = program->getRelation("num_tokens");
        assert(relation != NULL);
        souffle::tuple tuple(relation);
        tuple << (int32_t) tokens.size();
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

    std::shared_ptr<tree_node>
    parser::get_ast(const char* filename) {
        if (asts.find(filename) != asts.end())
            return asts[filename];
        auto nodes = get_ast_nodes(filename);
        auto contains = [](std::shared_ptr<tree_node> a,
                           std::shared_ptr<tree_node> b) {
            return a->get_start_token() <= b->get_start_token() &&
                   a->get_end_token()   >= b->get_end_token();
        };
        std::sort(nodes.begin(), nodes.end(),
                [](std::tuple<std::string,int,int> a,
                   std::tuple<std::string,int,int> b) {
            return std::pair(std::get<1>(a), std::get<2>(b)) <
                   std::pair(std::get<1>(b), std::get<2>(a));
        });
        if (!nodes.size()) return nullptr;
        std::shared_ptr<tree_node> root =
            std::make_shared<tree_node>(nodes[0]);
        std::vector<std::shared_ptr<tree_node>> stack {root};
        for (size_t i = 1; i < nodes.size(); i++) {
            auto child = std::make_shared<tree_node>(nodes[i]);
            while (!contains(stack.back(), child)) stack.pop_back();
            stack.back()->add_child(child);
            stack.push_back(child);
        }
        asts[filename] = root;
        return root;
    }

    souffle::Relation*
    parser::get_relation(const char* relation_name) {
        return program->getRelation(relation_name);
    }

    std::tuple<std::string,int,int>
    parser::get_ast_node_from_id(const char* filename, int id) {
        auto& limits = token_limits[filename];
        auto record = program->getRecordTable().unpack(id, 3);
        assert(record != NULL);
        return std::tuple(
                program->getSymbolTable().decode(record[0]),
                limits[record[1]].first,
                limits[record[2]-1].second);
    }

    std::vector<std::tuple<std::string,int,int>>
    parser::get_ast_nodes(const char* filename) {
        std::vector<std::tuple<std::string,int,int>> result;
        souffle::Relation* relation = get_relation("in_tree");
        assert(relation != NULL);
        for (auto &output : *relation) {
            int id;
            output >> id;
            // Skip nil
            if (id == 0) continue;
            result.push_back(get_ast_node_from_id(filename, id));
        }
        return result;
    }

    parser::~parser() {
        delete program;
    }

    /**
     * Expects a null-terminated string.
     */
    std::tuple<std::vector<std::string>,
               std::unordered_map<size_t, std::pair<size_t, size_t>>,
               std::unordered_map<size_t,int32_t>,
               std::unordered_map<size_t, std::string>>
    parser::lex_string(const char *content) {
        std::vector<std::string> tokens;
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
            re2c:flags:case-ranges = 1;

            '"' [^\x00"]* '"' {
                tokens.push_back(std::string(YYSTART, YYCURSOR));
                token_type.emplace(tokens.size()-1, "string");
                token_limits[tokens.size()-1] = {
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
                tokens.push_back(std::string(YYSTART, YYCURSOR));
                token_limits[tokens.size()-1] = {
                    YYSTART - content,
                    YYCURSOR - content};
                continue;
            }
            [ \t\v\n\r] {
                continue;
            }
            "{" | "}" | "(" | ")" | "[" | "]" {
                tokens.push_back(std::string(YYSTART, YYCURSOR));
                token_limits[tokens.size()-1] = {
                    YYSTART - content,
                    YYCURSOR - content};
                continue;
            }
            "||" | "&&" | "|"  | "^"  | "&"  | "=="  | "!=" | "<" |
            ">"  | "<=" | ">=" | "<<" | ">>" | ">>>" | "+"  | "-" |
            "*"  | "/"  | "%" {
                tokens.push_back(std::string(YYSTART, YYCURSOR));
                token_limits[tokens.size()-1] = {
                    YYSTART - content,
                    YYCURSOR - content};
                continue;
            }
            "="  | "+="  | "-="  | "*="   | "/=" | "&=" | "|=" | "^=" |
            "%=" | "<<=" | ">>=" | ">>>=" {
                tokens.push_back(std::string(YYSTART, YYCURSOR));
                token_limits[tokens.size()-1] = {
                    YYSTART - content,
                    YYCURSOR - content};
                continue;
            }
            "0" | [1-9][0-9]* {
                tokens.push_back(std::string(YYSTART, YYCURSOR));
                i32_value.emplace(tokens.size()-1, std::stoi(tokens.back()));
                token_type.emplace(tokens.size()-1, "integer");
                token_limits[tokens.size()-1] = {
                    YYSTART - content,
                    YYCURSOR - content};
                continue;
            }
            ";" | "," | "." {
                tokens.push_back(std::string(YYSTART, YYCURSOR));
                token_limits[tokens.size()-1] = {
                    YYSTART - content,
                    YYCURSOR - content};
                continue;
            }
            [a-zA-Z_][a-zA-Z_0-9]* {
                tokens.push_back(std::string(YYSTART, YYCURSOR));
                token_type.emplace(tokens.size()-1, "identifier");
                token_limits[tokens.size()-1] = {
                    YYSTART - content,
                    YYCURSOR - content};
                continue;
            }
            * {
                break;
            }
            */
        }
        return {tokens, token_limits, i32_value, token_type};
    }
}

