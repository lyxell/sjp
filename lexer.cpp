#include "sjp.h"

#include <string>
#include <vector>
#include <unordered_map>

namespace sjp {

/**
 * Expects a null-terminated string.
 */
std::pair<std::vector<std::tuple<std::string, size_t, size_t>>,
          std::unordered_map<size_t, std::string>>
lex(const char* content) {
    std::vector<std::tuple<std::string, size_t, size_t>> tokens;
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
            tokens.emplace_back(std::string(YYSTART, YYCURSOR),
                                YYSTART - content,
                                YYCURSOR - content);
            token_type.emplace(YYSTART - content, "char");
            continue;
        }

        StringLiteral {
            tokens.emplace_back(std::string(YYSTART, YYCURSOR),
                                YYSTART - content,
                                YYCURSOR - content);
            token_type.emplace(YYSTART - content, "string");
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
            tokens.emplace_back(std::string(YYSTART, YYCURSOR),
                                YYSTART - content,
                                YYCURSOR - content);
            continue;
        }
        [ \t\v\n\r] {
            continue;
        }
        "{" | "}" | "(" | ")" | "[" | "]" {
            tokens.emplace_back(std::string(YYSTART, YYCURSOR),
                                YYSTART - content,
                                YYCURSOR - content);
            continue;
        }
        "||" | "&&" | "|"  | "^"  | "&"  | "=="  | "!=" | "<" |
        ">"  | "<=" | ">=" | "<<" | ">>>" | "+"  | "-" |
        "*"  | "/"  | "%"  | "++" | "--" | "!"   | "@"  | "?" | "..." |
        ":"  | "->" {
            tokens.emplace_back(std::string(YYSTART, YYCURSOR),
                                YYSTART - content,
                                YYCURSOR - content);
            continue;
        }
        "="  | "+="  | "-="  | "*="   | "/=" | "&=" | "|=" | "^=" |
        "%=" | "<<=" | ">>=" | ">>>=" {
            tokens.emplace_back(std::string(YYSTART, YYCURSOR),
                                YYSTART - content,
                                YYCURSOR - content);
            continue;
        }
        IntegerLiteral {
            tokens.emplace_back(std::string(YYSTART, YYCURSOR),
                                YYSTART - content,
                                YYCURSOR - content);
            continue;
        }
        FloatingPointLiteral {
            tokens.emplace_back(std::string(YYSTART, YYCURSOR),
                                YYSTART - content,
                                YYCURSOR - content);
            token_type.emplace(YYSTART - content, "float");
            continue;
        }
        ";" | "," | "." {
            tokens.emplace_back(std::string(YYSTART, YYCURSOR),
                                YYSTART - content,
                                YYCURSOR - content);
            continue;
        }
        [a-zA-Z_][a-zA-Z_0-9]* {
            tokens.emplace_back(std::string(YYSTART, YYCURSOR),
                                YYSTART - content,
                                YYCURSOR - content);
            continue;
        }
        * {
            break;
        }
        */
    }
    return {tokens, token_type};
}

}
