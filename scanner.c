#include <stdio.h>
#include <assert.h>

int curr = 1;

#define report(a,b,c) printf("%s\t%d\n", a, curr++);

int lex(const char *YYCURSOR) {
    while (1) {
        const char *YYSTART = YYCURSOR;
        /*!re2c
        re2c:define:YYCTYPE = char;
        re2c:yyfill:enable = 0;
        re2c:flags:case-ranges = 1;

        identifier = [a-zA-Z_][a-zA-Z_0-9]*;
       
        "if"  {
            report("TOKEN_IF", YYSTART, YYCURSOR);
            continue;
        }
        identifier {
            report("TOKEN_IDENTIFIER", YYSTART, YYCURSOR);
            continue;
        }
        [ \t\v\n\r] {
            //report("TOKEN_WHITESPACE", YYSTART, YYCURSOR);
            continue;
        }
        "(" {
            report("TOKEN_LEFT_PAREN", YYSTART, YYCURSOR);
            continue;
        }
        ")" {
            report("TOKEN_RIGHT_PAREN", YYSTART, YYCURSOR);
            continue;
        }
        "{" {
            report("TOKEN_LEFT_CURLY", YYSTART, YYCURSOR);
            continue;
        }
        "}" {
            report("TOKEN_RIGHT_CURLY", YYSTART, YYCURSOR);
            continue;
        }
        "==" {
            report("TOKEN_EQUALS", YYSTART, YYCURSOR);
            continue;
        }
        "=" {
            report("TOKEN_ASSIGN", YYSTART, YYCURSOR);
            continue;
        }
        [1-9][0-9]* {
            report("TOKEN_NUMBER", YYSTART, YYCURSOR);
            continue;
        }
        ";" {
            report("TOKEN_SEMICOLON", YYSTART, YYCURSOR);
            continue;
        }
        * {
            return 1;
        }
        */
    }
}
int main() {
    lex("if (x == 20) { if (x == 20) {} }");
    return 0;
}

