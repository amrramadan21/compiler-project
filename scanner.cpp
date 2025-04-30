#include "scanner.h"
#include <iostream>
#include <cctype>
#include <unordered_map>
#include <sstream>

// Keyword mapping
static const std::unordered_map<std::string, TokenType> keywords = {
    {"if", TokenType::CONDITION},
    {"else", TokenType::OTHERWISE},
    {"int", TokenType::INTEGER},
    {"signed", TokenType::SINTEGER},
    {"char", TokenType::CHARACTER},
    {"string", TokenType::STRING},
    {"float", TokenType::FLOAT},
    {"void", TokenType::VOID},
    {"while", TokenType::LOOP},
    {"for", TokenType::LOOP},
    {"return", TokenType::RETURN},
    {"break", TokenType::BREAK},
    {"struct", TokenType::STRUCT},
    {"include", TokenType::INCLUDE},
    {"true", TokenType::INTEGER},  // Using INTEGER for true (1)
    {"false", TokenType::INTEGER}  // Using INTEGER for false (0)
};

Scanner::Scanner(const std::string& source) 
    : source(source), current(0), start(0), line(1), column(0) {
    // Initialize the scanner with the source code
}

Scanner::~Scanner() {
    // Clean up any open files
    while (!fileStack.empty()) {
        if (fileStack.back().is_open()) {
            fileStack.back().close();
        }
        fileStack.pop_back();
        filenameStack.pop_back();
        lineStack.pop_back();
        columnStack.pop_back();
    }
}

bool Scanner::isAtEnd() {
    return current >= source.length();
}

char Scanner::advance() {
    if (current < source.length()) {
        char c = source[current];
        current++;
        if (c == '\n') {
            line++;
            column = 0;
        } else {
            column++;
        }
        return c;
    }
    return '\0';
}

char Scanner::peek() {
    if (current < source.length()) {
        return source[current];
    }
    return '\0';
}

char Scanner::peekNext() {
    if (current + 1 < source.length()) {
        return source[current + 1];
    }
    return '\0';
}

bool Scanner::isDigit(char c) {
    return c >= '0' && c <= '9';
}

bool Scanner::isAlpha(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

bool Scanner::isAlphaNumeric(char c) {
    return isAlpha(c) || isDigit(c) || c == '_';
}

void Scanner::skipWhitespace() {
    while (!isAtEnd() && (isspace(peek()) || peek() == '\t')) {
        advance();
    }
}

void Scanner::skipComment() {
    char first = advance();
    char second = advance();
    
    if (second == '@') {
        // Multi-line comment
        while (!isAtEnd() && !(peek() == '@' && peekNext() == '/')) {
            advance();
        }
        if (!isAtEnd()) {
            advance(); // Consume @
            advance(); // Consume /
        }
    } else if (second == '^') {
        // Single-line comment
        while (!isAtEnd() && peek() != '\n') {
            advance();
        }
    }
}

Token Scanner::scanIdentifier() {
    int startColumn = column;
    std::string value;
    value += peek();
    advance();

    while (!isAtEnd() && (isAlpha(peek()) || isDigit(peek()))) {
        value += peek();
        advance();
    }

    // Check if it's a keyword
    auto it = keywords.find(value);
    if (it != keywords.end()) {
        return Token(it->second, value, line, startColumn);
    }

    return Token(TokenType::IDENTIFIER, value, line, startColumn);
}

Token Scanner::scanNumber() {
    int startColumn = column;
    std::string value;
    bool hasDecimal = false;

    while (!isAtEnd() && (isdigit(peek()) || peek() == '.')) {
        if (peek() == '.') {
            if (hasDecimal) {
                error("Invalid number format", line, column);
                return Token(TokenType::ERROR, "", line, startColumn);
            }
            hasDecimal = true;
        }
        value += peek();
        advance();
    }

    // If the number ends with a decimal point, it's invalid
    if (value.back() == '.') {
        error("Invalid number format", line, column);
        return Token(TokenType::ERROR, "", line, startColumn);
    }

    return Token(hasDecimal ? TokenType::FLOAT : TokenType::INTEGER, value, line, startColumn);
}

Token Scanner::scanCharacter() {
    int startColumn = column;
    advance(); // Consume the opening quote

    if (isAtEnd()) {
        error("Unterminated character literal", line, column);
        return Token(TokenType::ERROR, "", line, startColumn);
    }

    std::string value(1, peek());
    advance();

    if (isAtEnd() || peek() != '\'') {
        error("Unterminated character literal", line, column);
        return Token(TokenType::ERROR, "", line, startColumn);
    }

    advance(); // Consume the closing quote
    return Token(TokenType::CHARACTER, value, line, startColumn);
}

Token Scanner::scanString() {
    int startColumn = column;
    std::string value;
    advance(); // Consume the opening quote

    while (!isAtEnd() && peek() != '"') {
        if (peek() == '\n') {
            error("Unterminated string", line, column);
            return Token(TokenType::ERROR, "", line, startColumn);
        }
        value += peek();
        advance();
    }

    if (isAtEnd()) {
        error("Unterminated string", line, column);
        return Token(TokenType::ERROR, "", line, startColumn);
    }

    advance(); // Consume the closing quote
    return Token(TokenType::STRING, value, line, startColumn);
}

Token Scanner::scanOperator() {
    int startColumn = column;
    char first = peek();
    advance();
    char second = peek();

    // Check for two-character operators
    if (!isAtEnd()) {
        std::string twoChar(1, first);
        twoChar += second;
        
        if (twoChar == "&&") {
            advance();
            return Token(TokenType::AND, twoChar, line, startColumn);
        } else if (twoChar == "||") {
            advance();
            return Token(TokenType::OR, twoChar, line, startColumn);
        } else if (twoChar == "==") {
            advance();
            return Token(TokenType::EQUAL, twoChar, line, startColumn);
        } else if (twoChar == "!=") {
            advance();
            return Token(TokenType::NOT_EQUAL, twoChar, line, startColumn);
        } else if (twoChar == "<=") {
            advance();
            return Token(TokenType::LESS_EQUAL, twoChar, line, startColumn);
        } else if (twoChar == ">=") {
            advance();
            return Token(TokenType::GREATER_EQUAL, twoChar, line, startColumn);
        }
    }

    // Single character operators
    switch (first) {
        case '+': return Token(TokenType::PLUS, std::string(1, first), line, startColumn);
        case '-': return Token(TokenType::MINUS, std::string(1, first), line, startColumn);
        case '*': return Token(TokenType::MULTIPLY, std::string(1, first), line, startColumn);
        case '/': return Token(TokenType::DIVIDE, std::string(1, first), line, startColumn);
        case '=': return Token(TokenType::ASSIGN, std::string(1, first), line, startColumn);
        case '<': return Token(TokenType::LESS, std::string(1, first), line, startColumn);
        case '>': return Token(TokenType::GREATER, std::string(1, first), line, startColumn);
        case '!': return Token(TokenType::NOT, std::string(1, first), line, startColumn);
        case '(': return Token(TokenType::LEFT_PAREN, std::string(1, first), line, startColumn);
        case ')': return Token(TokenType::RIGHT_PAREN, std::string(1, first), line, startColumn);
        case '{': return Token(TokenType::LEFT_BRACE, std::string(1, first), line, startColumn);
        case '}': return Token(TokenType::RIGHT_BRACE, std::string(1, first), line, startColumn);
        case ';': return Token(TokenType::SEMICOLON, std::string(1, first), line, startColumn);
        case ',': return Token(TokenType::COMMA, std::string(1, first), line, startColumn);
        case '.': return Token(TokenType::DOT, std::string(1, first), line, startColumn);
        case '->': return Token(TokenType::ARROW, "->", line, startColumn);
        default:
            reportError("Unknown operator: " + std::string(1, first));
            return Token(TokenType::ERROR, std::string(1, first), line, startColumn);
    }
}

void Scanner::handleInclude(const std::string& filename) {
    std::ifstream includeFile(filename);
    if (!includeFile.is_open()) {
        reportError("Could not open include file: " + filename);
        return;
    }

    // Save current state
    fileStack.push_back(std::move(includeFile));
    filenameStack.push_back(filename);
    lineStack.push_back(line);
    columnStack.push_back(column);

    // Read the include file
    std::stringstream buffer;
    buffer << includeFile.rdbuf();
    source = buffer.str() + source.substr(current);
    current = 0;
    line = 1;
    column = 0;
}

Token Scanner::getNextToken() {
    skipWhitespace();
    
    if (isAtEnd()) {
        return Token(TokenType::END_OF_FILE, "", line, column);
    }
    
    char c = peek();
    int startColumn = column;
    
    // Handle comments
    if (c == '/') {
        char next = peekNext();
        if (next == '@' || next == '^') {
            skipComment();
            return getNextToken(); // Skip to next token after comment
        }
    }
    
    // Handle parentheses
    if (c == '(') {
        advance();
        return Token(TokenType::LEFT_PAREN, "(", line, startColumn);
    }
    if (c == ')') {
        advance();
        return Token(TokenType::RIGHT_PAREN, ")", line, startColumn);
    }
    
    // Handle numbers
    if (isDigit(c)) {
        return scanNumber();
    }
    
    // Handle identifiers and keywords
    if (isAlpha(c) || c == '_') {
        return scanIdentifier();
    }
    
    // Handle strings and characters
    if (c == '"') {
        return scanString();
    }
    if (c == '\'') {
        return scanCharacter();
    }
    
    // Handle operators and delimiters
    if (ispunct(c)) {
        return scanOperator();
    }
    
    // Invalid character
    std::string invalid(1, c);
    reportError(invalid + " Invalid character");
    advance();
    return Token(TokenType::ERROR, invalid, line, column);
}

bool Scanner::hasError() const {
    return !errors.empty();
}

const std::vector<std::string>& Scanner::getErrors() const {
    return errors;
}

size_t Scanner::getErrorCount() const {
    return errors.size();
}

void Scanner::reportError(const std::string& message) {
    std::stringstream ss;
    ss << "Line: " << line << " Error in Token Text: " << message;
    errors.push_back(ss.str());
}

void Scanner::error(const std::string& message, int line, int column) {
    std::stringstream ss;
    ss << "Line: " << line << " Error in Token Text: " << message;
    errors.push_back(ss.str());
} 