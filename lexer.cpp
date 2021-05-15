#include "sjp.h"

#include <string>
#include <vector>
#include <unordered_map>
#include <iostream>

namespace sjp {

/**
 * Expects a null-terminated string.
 */
std::vector<std::tuple<std::string, std::string, size_t, size_t>>
lex(const char* content) {
    std::vector<std::tuple<std::string, std::string, size_t, size_t>> tokens;
    const char* YYCURSOR = content;
    const char* YYLIMIT = content + strlen(content);
    const char* YYMARKER;
    while (true) {
        const char* YYSTART = YYCURSOR;
        /*!re2c
        re2c:define:YYCTYPE = char;
        re2c:yyfill:enable = 0;
        re2c:eof = 0;

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

        Comment {
            continue;
        }

        [ \t\v\n\r] {
            continue;
        }

        CharacterLiteral {
            tokens.emplace_back(std::string(YYSTART, YYCURSOR), "character_literal", YYSTART - content, YYCURSOR - content);
            continue;
        }

        StringLiteral {
            tokens.emplace_back(std::string(YYSTART, YYCURSOR), "string_literal", YYSTART - content, YYCURSOR - content);
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
            tokens.emplace_back(std::string(YYSTART, YYCURSOR), "keyword", YYSTART - content, YYCURSOR - content);
            continue;
        }
        "{"  | "}"   | "("   | ")"  | "["  | "]"   | "||"  | "&&" | "|"   |
        "^"  | "=="  | "!="  | "<"  | ">"  | "<="  | ">="  | "<<" | ">>>" |
        "+"  | "-"   | "*"   | "/"  | "%"  | "++"  | "--"  | "!"  | "@"   |
        "?"  | "..." | "&"   | ":"  | "->" | "="   | "+="  | "-=" | "*="  |
        "/=" | "&="  | "|="  | "^=" | "%=" | "<<=" | ">>=" | ">>>=" | ";" |
        "," | "." {
            tokens.emplace_back(std::string(YYSTART, YYCURSOR), "symbols", YYSTART - content, YYCURSOR - content);
            continue;
        }
        IntegerLiteral {
            tokens.emplace_back(std::string(YYSTART, YYCURSOR), "integer_literal", YYSTART - content, YYCURSOR - content);
            continue;
        }
        FloatingPointLiteral {
            tokens.emplace_back(std::string(YYSTART, YYCURSOR), "floating_point_literal", YYSTART - content, YYCURSOR - content);
            continue;
        }
        [a-zA-Z_][a-zA-Z_0-9]* {
            tokens.emplace_back(std::string(YYSTART, YYCURSOR), "identifier", YYSTART - content, YYCURSOR - content);
            continue;
        }
        $ {
            tokens.emplace_back(std::string(), "eof", YYSTART - content, YYSTART - content);
            break;
        }
        * {
            std::cerr << "invalid token" << std::endl;
            return {};
        }
        */
    }
    return tokens;
}

}
