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

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

int curr = 1;

void report(const char* token, const char* start, const char* end) {
    char buf[120];
    snprintf(buf, end - start + 1, "%s", start);
    printf("%s\t%s\t%d\n", token, buf, curr++);
}

int lex(const char *YYCURSOR) {
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
            report("TOKEN_KEYWORD", YYSTART, YYCURSOR);
            continue;
        }
        [ \t\v\n\r] {
            continue;
        }
        "{" | "}" | "(" | ")" | "[" | "]" {
            report("TOKEN_BRACKET", YYSTART, YYCURSOR);
            continue;
        }
        "||" | "&&" | "|"  | "^"  | "&"  | "=="  | "!=" | "<" |
        ">"  | "<=" | ">=" | "<<" | ">>" | ">>>" | "+"  | "-" |
        "*"  | "/"  | "%" {
            report("TOKEN_OPERATOR", YYSTART, YYCURSOR);
            continue;
        }
        "="  | "+="  | "-="  | "*="   | "/=" | "&=" | "|=" | "^=" |
        "%=" | "<<=" | ">>=" | ">>>=" {
            report("TOKEN_OPERATOR", YYSTART, YYCURSOR);
            continue;
        }
        "0" | [1-9][0-9]* {
            report("TOKEN_LITERAL_INTEGER", YYSTART, YYCURSOR);
            continue;
        }
        ";" | "," | "." {
            report("TOKEN_SEPARATOR", YYSTART, YYCURSOR);
            continue;
        }
        [a-zA-Z_][a-zA-Z_0-9]* {
            report("TOKEN_IDENTIFIER", YYSTART, YYCURSOR);
            continue;
        }
        * {
            return 1;
        }
        */
    }
}
int main(int argc, char** argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s InputClass.Java\n", argv[0]);
        return 1;
    }
    FILE* f = fopen(argv[1], "r");
    if (!f) {
        fprintf(stderr, "Failed to open %s\n", argv[1]);
        return 1;
    }
    fseek(f, 0L, SEEK_END);
    long size = ftell(f);
    rewind(f);
    char* buffer = malloc(size + 1);
    if (!buffer) {
        fprintf(stderr, "Failed to allocate %ld bytes of memory\n", size);
        return 1;
    }
    fread(buffer, 1, size, f);
    lex(buffer);
    report("TOKEN_EOF", 0, 0);
    return 0;
}

