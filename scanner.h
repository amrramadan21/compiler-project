#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <unordered_map>
#include <memory>
#include <iostream>
#include <sstream>

// Token types
enum class TokenType {
    // Keywords
    CONDITION,      // IfTrue-Otherwise
    OTHERWISE,      // Otherwise part of IfTrue-Otherwise
    INTEGER,        // Imw
    SINTEGER,       // SIMw
    CHARACTER,      // Chj
    STRING,         // Series
    FLOAT,          // IMwf
    SFLOAT,         // SIMwf
    VOID,           // NOReturn
    LOOP,           // RepeatWhen/Reiterate
    RETURN,         // Turnback
    BREAK,          // OutLoop
    STRUCT,         // Loli
    INCLUDE,        // Include
    IDENTIFIER,     // Variable names
    DOT,            // Member access
    ARROW,          // Pointer access
    
    // Operators
    PLUS,           // +
    MINUS,          // -
    MULTIPLY,       // *
    DIVIDE,         // /
    AND,            // &&
    OR,             // ||
    NOT,            // ~
    EQUAL,          // ==
    LESS,           // <
    GREATER,        // >
    NOT_EQUAL,      // !=
    LESS_EQUAL,     // <=
    GREATER_EQUAL,  // >=
    ASSIGN,         // =
    ACCESS,         // ->
    
    // Delimiters
    LEFT_BRACE,     // {
    RIGHT_BRACE,    // }
    LEFT_BRACKET,   // [
    RIGHT_BRACKET,  // ]
    LEFT_PAREN,     // (
    RIGHT_PAREN,    // )
    SEMICOLON,      // ;
    COMMA,          // ,
    
    // Literals
    CONSTANT,       // [0-9] and combinations
    QUOTE,          // " or '
    
    // Comments
    COMMENT_START,  // /@ or /^
    COMMENT_END,    // @/
    COMMENT_CONTENT,// Comment text
    
    // Special
    END_OF_FILE,    // End of file
    ERROR,          // Error token
    UNKNOWN        // Unknown token type
};

struct Token {
    TokenType type;
    std::string value;
    int line;
    int column;
    
    Token() : type(TokenType::UNKNOWN), value(""), line(0), column(0) {}
    Token(TokenType t, const std::string& v, int l, int c)
        : type(t), value(v), line(l), column(c) {}
};

class Scanner {
private:
    std::string source;
    size_t current;
    size_t start;
    int line;
    int column;
    std::vector<std::ifstream> fileStack;
    std::vector<std::string> filenameStack;
    std::vector<int> lineStack;
    std::vector<int> columnStack;
    std::vector<std::string> errors;
    
    char advance();
    char peek();
    char peekNext();
    bool isAtEnd();
    bool isDigit(char c);
    bool isAlpha(char c);
    bool isAlphaNumeric(char c);
    
    void skipWhitespace();
    void skipComment();
    Token scanIdentifier();
    Token scanNumber();
    Token scanString();
    Token scanCharacter();
    Token scanOperator();
    void handleInclude(const std::string& filename);
    void error(const std::string& message, int line, int column);
    
public:
    explicit Scanner(const std::string& source);
    ~Scanner();
    
    Token getNextToken();
    bool hasError() const;
    const std::vector<std::string>& getErrors() const;
    size_t getErrorCount() const;
    void reportError(const std::string& message);
}; 