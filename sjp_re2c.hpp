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

namespace sjp {

    /**
     * Expects a null-terminated string.
     */
    std::pair<std::vector<std::string>,std::unordered_map<size_t,int32_t>>
    lex_string(const char *content) {
        std::vector<std::string> tokens;
        std::unordered_map<size_t, int32_t> i32_value;
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
                continue;
            }
            [ \t\v\n\r] {
                continue;
            }
            "{" | "}" | "(" | ")" | "[" | "]" {
                tokens.push_back(std::string(YYSTART, YYCURSOR));
                continue;
            }
            "||" | "&&" | "|"  | "^"  | "&"  | "=="  | "!=" | "<" |
            ">"  | "<=" | ">=" | "<<" | ">>" | ">>>" | "+"  | "-" |
            "*"  | "/"  | "%" {
                tokens.push_back(std::string(YYSTART, YYCURSOR));
                continue;
            }
            "="  | "+="  | "-="  | "*="   | "/=" | "&=" | "|=" | "^=" |
            "%=" | "<<=" | ">>=" | ">>>=" {
                tokens.push_back(std::string(YYSTART, YYCURSOR));
                continue;
            }
            "0" | [1-9][0-9]* {
                tokens.push_back(std::string(YYSTART, YYCURSOR));
                i32_value.emplace(tokens.size()-1, std::stoi(tokens.back()));
                continue;
            }
            ";" | "," | "." {
                tokens.push_back(std::string(YYSTART, YYCURSOR));
                continue;
            }
            [a-zA-Z_][a-zA-Z_0-9]* {
                tokens.push_back(std::string(YYSTART, YYCURSOR));
                continue;
            }
            * {
                break;
            }
            */
        }
        return {tokens, i32_value};
    }

    /**
     * Expects a null-terminated filename.
     */
    std::pair<std::vector<std::string>,std::unordered_map<size_t,int32_t>>
    lex_file(const char *filename) {
        std::ifstream t(filename);
        return lex_string(std::string((std::istreambuf_iterator<char>(t)),
                                    std::istreambuf_iterator<char>()).c_str());
    }

    class parser {
        souffle::SouffleProgram *program;
    public:
        parser() {
            program = souffle::ProgramFactory::newInstance("parser");
            assert(program != NULL);
        }
        void add_file(const char* filename) {
            auto [tokens, i32_value] = lex_file(filename);
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
        }
        void parse() {
            program->run();
            program->printAll();
            //
            souffle::Relation* relation = program->getRelation("root");
            assert(relation != NULL);
            std::cout << relation->size() << std::endl;
            std::cout << relation->getAttrType(0) << std::endl;
            /*
            std::string x;
            for (auto &output : *relation) {
                output >> x;
                std::cout << x << std::endl;
            }*/
        }
        ~parser() {
            delete program;
        }
    };

}
