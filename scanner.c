#include <stdio.h>
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
            report("TOKEN_ASSIGNMENT_OPERATOR", YYSTART, YYCURSOR);
            continue;
        }
        [1-9][0-9]* {
            report("TOKEN_NUMBER", YYSTART, YYCURSOR);
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
int main() {
    lex("public class Main {}");
    report("TOKEN_EOF", 0, 0);
    return 0;
}

