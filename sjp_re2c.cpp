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

#include <cassert>
#include <fstream>
#include <iostream>
#include <souffle/SouffleInterface.h>
#include <streambuf>
#include <string>
#include <tuple>

#include "sjp.h"

namespace sjp {

parser::parser() : program(souffle::ProgramFactory::newInstance("parser")) {
    assert(program != nullptr);
}

parser::parser(const char* program_name)
    : program(souffle::ProgramFactory::newInstance(program_name)) {
    assert(program != nullptr);
}

void parser::add_file(const char* filename) {
    std::ifstream t(filename);
    add_string(filename, std::string((std::istreambuf_iterator<char>(t)),
                                     std::istreambuf_iterator<char>())
                             .c_str());
}

void parser::add_string(const char* filename, const char* content) {
    auto [i32_value, token_type] = lex_string(filename, content);
    // insert into token relation
    for (int32_t i = 0; i < tokens[filename].size(); i++) {
        souffle::tuple tuple(program->getRelation("token"));
        tuple << tokens[filename][i] << i;
        program->getRelation("token")->insert(tuple);
    }
    // insert into num_tokens relation
    souffle::tuple tuple(program->getRelation("num_tokens"));
    tuple << (int32_t)tokens[filename].size();
    program->getRelation("num_tokens")->insert(tuple);
    // insert into token_type relation
    for (auto& [id, type] : token_type) {
        souffle::tuple tuple(program->getRelation("token_type"));
        tuple << (int32_t)id << type;
        program->getRelation("token_type")->insert(tuple);
    }
}

void parser::run() { program->run(); }

std::vector<std::tuple<std::string, int, int>>
parser::get_tokens(const char* filename) {
    auto& limits = token_limits[filename];
    std::vector<std::tuple<std::string, int, int>> result;
    for (size_t i = 0; i < tokens[filename].size(); i++) {
        result.emplace_back(tokens[filename][i], limits[i].first,
                            limits[i].second);
    }
    return result;
}

std::tuple<std::string, int, int> parser::node_from_id(const char* filename,
                                                       int id) {
    auto& limits = token_limits[filename];
    const auto* record = program->getRecordTable().unpack(id, 3);
    assert(limits.find(record[1]) != limits.end());
    assert(limits.find(record[2] - 1) != limits.end());
    return std::tuple(program->getSymbolTable().decode(record[0]),
                      limits[record[1]].first, limits[record[2] - 1].second);
}

std::shared_ptr<tree_node> parser::build_node(
    const char* filename, std::map<int, std::map<std::string, int>>& parent_of,
    std::map<int, std::map<std::string, std::vector<int>>>& parent_of_list,
    int id) {
    if (id == 0) {
        return nullptr;
    }
    auto [name, start, end] = node_from_id(filename, id);
    auto ptr = std::make_shared<tree_node>();
    ptr->name = name;
    ptr->start_token = start;
    ptr->end_token = end;
    for (auto [symbol, child] : parent_of[id]) {
        ptr->parent_of[symbol] = build_node(filename, parent_of, parent_of_list, child);
    }
    for (auto [symbol, children] : parent_of_list[id]) {
        std::vector<std::shared_ptr<tree_node>> result;
        for (auto child : children) {
            result.push_back(
                build_node(filename, parent_of, parent_of_list, child));
        }
        ptr->parent_of_list[symbol] = result;
    }
    return ptr;
}

std::shared_ptr<tree_node> parser::get_ast(const char* filename) {

    // load the parent_of relation into memory
    std::map<int, std::map<std::string, int>> parent_of;
    for (auto& output : *program->getRelation("parent_of")) {
        int parent;
        int child;
        std::string symbol;
        output >> parent;
        output >> symbol;
        output >> child;
        assert(parent != 0);
        parent_of[parent][symbol] = child;
    }

    // load the parent_of_list relation into memory
    std::map<int, std::map<std::string, std::vector<int>>> parent_of_list;
    for (auto& output : *program->getRelation("parent_of_list")) {
        int parent;
        int list;
        std::string symbol;
        output >> parent;
        output >> symbol;
        output >> list;
        assert(parent != 0);
        std::vector<int> result;
        while (list != 0) {
            const auto* record = program->getRecordTable().unpack(list, 2);
            result.push_back(record[0]);
            list = record[1];
        }
        parent_of_list[parent][symbol] = result;
    }

    // load root relation into memory
    std::shared_ptr<tree_node> root = nullptr;
    for (auto& output : *program->getRelation("root")) {
        int id;
        output >> id;
        return build_node(filename, parent_of, parent_of_list, id);
    }

    return nullptr;
}

/**
 * Expects a null-terminated string.
 */
std::tuple<std::unordered_map<size_t, int32_t>,
           std::unordered_map<size_t, std::string>>
parser::lex_string(const char* filename, const char* content) {
    assert(filename != nullptr);
    std::unordered_map<size_t, int32_t> i32_value;
    std::unordered_map<size_t, std::string> token_type;
    const char* YYCURSOR = content;
    const char* YYMARKER;
    while (true) {
        const char* YYSTART = YYCURSOR;
        /*!re2c
        re2c:define:YYCTYPE = char;
        re2c:yyfill:enable = 0;

        // STRING LITERALS
        StringLiteral = '"' [^\x00"]* '"';

        // CHARACTER LITERALS
        CharacterLiteral = ['] [^\x00'] ['];

        // COMMENTS
        Comment = "//" [^\n\x00]* "\n"
                | "/" "*" ([^*\x00] | ("*" [^/\x00]))* "*" "/";

        // INTEGER LITERALS
        Underscores = "_"+;
        OctalDigit     = [0-7];
        OctalDigitOrUnderscore = OctalDigit | "_";
        OctalDigitsAndUnderscores = OctalDigitOrUnderscore+;
        OctalDigits    = OctalDigit
                       | OctalDigit OctalDigitsAndUnderscores? OctalDigit;
        OctalNumeral   = "0" OctalDigits
                       | "0" Underscores OctalDigits;
        BinaryDigit    = [01];
        BinaryDigitOrUnderscore = BinaryDigit | "_";
        BinaryDigitsAndUnderscores = BinaryDigitOrUnderscore+;
        BinaryDigits   = BinaryDigit
                       | BinaryDigit BinaryDigitsAndUnderscores? BinaryDigit;
        BinaryNumeral  = "0" [bB] BinaryDigits;
        HexDigit       = [0-9a-fA-F];
        HexDigitOrUnderscore = HexDigit | "_";
        HexDigitsAndUnderscores = HexDigitOrUnderscore+;
        HexDigits      = HexDigit
                       | HexDigit HexDigitsAndUnderscores? HexDigit;
        HexNumeral     = "0" [xX] HexDigits;
        NonZeroDigit   = [1-9];
        Digit          = "0"
                       | NonZeroDigit;
        DigitOrUnderscore = Digit | "_";
        DigitsAndUnderscores = DigitOrUnderscore+;
        Digits         = Digit
                       | Digit DigitsAndUnderscores? Digit;
        DecimalNumeral = "0"
                       | NonZeroDigit Digits?
                       | NonZeroDigit Underscores Digits;
        IntegerTypeSuffix = [lL];
        BinaryIntegerLiteral = BinaryNumeral IntegerTypeSuffix?;
        OctalIntegerLiteral = OctalNumeral IntegerTypeSuffix?;
        HexIntegerLiteral = HexNumeral IntegerTypeSuffix?;
        DecimalIntegerLiteral = DecimalNumeral IntegerTypeSuffix?;
        IntegerLiteral = DecimalIntegerLiteral
                       | HexIntegerLiteral
                       | OctalIntegerLiteral
                       | BinaryIntegerLiteral;

        // FLOATING POINT LITERALS
        Sign = [+-];
        SignedInteger = [Sign] Digits;
        HexSignificand = HexNumeral "."
                       | "0" [xX] HexDigits? "." HexDigits;
        BinaryExponentIndicator = [pP];
        BinaryExponent = BinaryExponentIndicator SignedInteger;
        HexadecimalFloatingPointLiteral =
            HexSignificand BinaryExponent [FloatTypeSuffix];
        FloatTypeSuffix = [fFdD];
        ExponentIndicator = [eE];
        ExponentPart = ExponentIndicator SignedInteger;
        DecimalFloatingPointLiteral =
              Digits "." Digits? ExponentPart? FloatTypeSuffix?
            | "." Digits ExponentPart? FloatTypeSuffix?
            | Digits ExponentPart FloatTypeSuffix?
            | Digits ExponentPart? FloatTypeSuffix;
        FloatingPointLiteral = DecimalFloatingPointLiteral
                             | HexadecimalFloatingPointLiteral;

        CharacterLiteral {
            tokens[filename].push_back(std::string(YYSTART, YYCURSOR));
            token_type.emplace(tokens[filename].size()-1, "char");
            token_limits[filename][tokens[filename].size()-1] = {
                YYSTART - content,
                YYCURSOR - content};
            continue;
        }

        StringLiteral {
            tokens[filename].push_back(std::string(YYSTART, YYCURSOR));
            token_type.emplace(tokens[filename].size()-1, "string");
            token_limits[filename][tokens[filename].size()-1] = {
                YYSTART - content,
                YYCURSOR - content};
            continue;
        }

        Comment {
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
            token_limits[filename][tokens[filename].size()-1] = {
                YYSTART - content,
                YYCURSOR - content};
            continue;
        }
        [ \t\v\n\r] {
            continue;
        }
        "{" | "}" | "(" | ")" | "[" | "]" {
            tokens[filename].push_back(std::string(YYSTART, YYCURSOR));
            token_limits[filename][tokens[filename].size()-1] = {
                YYSTART - content,
                YYCURSOR - content};
            continue;
        }
        "||" | "&&" | "|"  | "^"  | "&"  | "=="  | "!=" | "<" |
        ">"  | "<=" | ">=" | "<<" | ">>" | ">>>" | "+"  | "-" |
        "*"  | "/"  | "%"  | "++" | "--" | "!"   | "@"  | "?" |
        ":" {
            tokens[filename].push_back(std::string(YYSTART, YYCURSOR));
            token_limits[filename][tokens[filename].size()-1] = {
                YYSTART - content,
                YYCURSOR - content};
            continue;
        }
        "="  | "+="  | "-="  | "*="   | "/=" | "&=" | "|=" | "^=" |
        "%=" | "<<=" | ">>=" | ">>>=" {
            tokens[filename].push_back(std::string(YYSTART, YYCURSOR));
            token_limits[filename][tokens[filename].size()-1] = {
                YYSTART - content,
                YYCURSOR - content};
            continue;
        }
        IntegerLiteral {
            tokens[filename].push_back(std::string(YYSTART, YYCURSOR));
            i32_value.emplace(tokens[filename].size()-1,
                              std::stoi(tokens[filename].back()));
            token_type.emplace(tokens[filename].size()-1, "integer");
            token_limits[filename][tokens[filename].size()-1] = {
                YYSTART - content,
                YYCURSOR - content};
            continue;
        }
        FloatingPointLiteral {
            tokens[filename].push_back(std::string(YYSTART, YYCURSOR));
            token_type.emplace(tokens[filename].size()-1, "float");
            token_limits[filename][tokens[filename].size()-1] = {
                YYSTART - content,
                YYCURSOR - content};
            continue;
        }
        ";" | "," | "." {
            tokens[filename].push_back(std::string(YYSTART, YYCURSOR));
            token_limits[filename][tokens[filename].size()-1] = {
                YYSTART - content,
                YYCURSOR - content};
            continue;
        }
        [a-zA-Z_][a-zA-Z_0-9]* {
            tokens[filename].push_back(std::string(YYSTART, YYCURSOR));
            token_type.emplace(tokens[filename].size()-1, "identifier");
            token_limits[filename][tokens[filename].size()-1] = {
                YYSTART - content,
                YYCURSOR - content};
            continue;
        }
        * {
            break;
        }
        */
    }
    return {i32_value, token_type};
}
} // namespace sjp
