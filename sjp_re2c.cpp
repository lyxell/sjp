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

    parser::parser() {
        program = souffle::ProgramFactory::newInstance("parser");
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

    void parser::parse() {
        program->run();
        program->printAll();
    }

    std::vector<std::tuple<std::string,int,int>>
    parser::get_tuples(const char* filename) {
        std::vector<std::tuple<std::string,int,int>> result;
        auto& limits = token_limits[filename];
        souffle::Relation* relation = program->getRelation("in_tree");
        for (auto &output : *relation) {
            int record_reference;
            output >> record_reference;
            auto record = program->getRecordTable().unpack(record_reference, 3);
            if (!record) continue;
            int symbol_reference = *record;
            
            result.emplace_back(
                program->getSymbolTable().decode(symbol_reference),
                limits[record[1]].first,
                limits[record[2]-1].second);
        }
        return result;
    }

    size_t parser::num_asts() {
        souffle::Relation* relation = program->getRelation("root");
        assert(relation != NULL);
        return relation->size();
        return 0;
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
    lex_string(const char *content) {
        std::vector<std::string> tokens;
        std::unordered_map<size_t, std::pair<size_t, size_t>> token_limits;
        std::unordered_map<size_t, int32_t> i32_value;
        std::unordered_map<size_t, std::string> token_type;
        const char* YYCURSOR = content;
        while (1) {
            const char *YYSTART = YYCURSOR;
            /*!re2c
            re2c:define:YYCTYPE = char;
            re2c:yyfill:enable = 0;
            re2c:flags:case-ranges = 1;
          
            "abstract" | "assert" | "boolean" | "break" | "byte" | "case" |
            "catch" | "char" | "class" | "const" | "continue" | "default" |
            "do" | "double" | "else" | "enum" | "extends" | "final" | "finally" |
            "float" | "for" | "goto" | "if" | "implements" | "import" |
            "instanceof" | "int" | "interface" | "long" | "native" | "new" |
            "package" | "private" | "protected" | "public" | "return" | "short" |
            "static" | "strictfp" | "super" | "switch" | "synchronized" | "this" |
            "throw" | "throws" | "transient" | "try" | "void" | "volatile" |
            "while" {
                tokens.push_back(std::string(YYSTART, YYCURSOR));
                token_limits[tokens.size()-1] = {YYSTART - content, YYCURSOR - content};
                continue;
            }
            [ \t\v\n\r] {
                continue;
            }
            "{" | "}" | "(" | ")" | "[" | "]" {
                tokens.push_back(std::string(YYSTART, YYCURSOR));
                token_limits[tokens.size()-1] = {YYSTART - content, YYCURSOR - content};
                continue;
            }
            "||" | "&&" | "|"  | "^"  | "&"  | "=="  | "!=" | "<" |
            ">"  | "<=" | ">=" | "<<" | ">>" | ">>>" | "+"  | "-" |
            "*"  | "/"  | "%" {
                tokens.push_back(std::string(YYSTART, YYCURSOR));
                token_limits[tokens.size()-1] = {YYSTART - content, YYCURSOR - content};
                continue;
            }
            "="  | "+="  | "-="  | "*="   | "/=" | "&=" | "|=" | "^=" |
            "%=" | "<<=" | ">>=" | ">>>=" {
                tokens.push_back(std::string(YYSTART, YYCURSOR));
                token_limits[tokens.size()-1] = {YYSTART - content, YYCURSOR - content};
                continue;
            }
            "0" | [1-9][0-9]* {
                tokens.push_back(std::string(YYSTART, YYCURSOR));
                i32_value.emplace(tokens.size()-1, std::stoi(tokens.back()));
                token_type.emplace(tokens.size()-1, "integer");
                token_limits[tokens.size()-1] = {YYSTART - content, YYCURSOR - content};
                continue;
            }
            ";" | "," | "." {
                tokens.push_back(std::string(YYSTART, YYCURSOR));
                token_limits[tokens.size()-1] = {YYSTART - content, YYCURSOR - content};
                continue;
            }
            [a-zA-Z_][a-zA-Z_0-9]* {
                tokens.push_back(std::string(YYSTART, YYCURSOR));
                token_type.emplace(tokens.size()-1, "identifier");
                token_limits[tokens.size()-1] = {YYSTART - content, YYCURSOR - content};
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

