#include "scanner.h"
#include <iostream>
#include <cctype>
#include <unordered_map>
#include <sstream>
#include <algorithm>

// Keyword mapping
static const std::unordered_map<std::string, TokenType> keywords = {
    {"IfTrue-Otherwise", TokenType::CONDITION},
    {"Otherwise", TokenType::OTHERWISE},
    {"Imw", TokenType::INTEGER},
    {"SIMw", TokenType::SINTEGER},
    {"Chj", TokenType::CHARACTER},
    {"Series", TokenType::STRING},
    {"IMwf", TokenType::FLOAT},
    {"SIMwf", TokenType::SFLOAT},
    {"NOReturn", TokenType::VOID},
    {"RepeatWhen", TokenType::LOOP},
    {"Reiterate", TokenType::LOOP},
    {"Turnback", TokenType::RETURN},
    {"OutLoop", TokenType::BREAK},
    {"Loli", TokenType::STRUCT},
    {"Include", TokenType::INCLUDE},
    {"int", TokenType::TYPE}
};

Scanner::Scanner(const std::string& source) 
    : source(source), current(0), start(0), line(1), column(0) {}

Scanner::~Scanner() {
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
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

bool Scanner::isAlphaNumeric(char c) {
    return isAlpha(c) || isDigit(c);
}

void Scanner::skipWhitespace() {
    while (!isAtEnd()) {
        char c = peek();
        if (c == ' ' || c == '\t' || c == '\r') {
            advance();
        } else if (c == '\n') {
            advance();
        } else {
            break;
        }
    }
}

Token Scanner::scanIdentifier() {
    int startColumn = column;
    std::string value;
    value += peek();
    advance();

    // Special handling for IfTrue-Otherwise
    if (value == "I" && peek() == 'f' && peekNext() == 'T') {
        while (!isAtEnd() && (isAlpha(peek()) || peek() == '-')) {
            value += peek();
            advance();
        }
        if (value == "IfTrue-Otherwise") {
            return Token(TokenType::CONDITION, value, line, startColumn);
        }
    }

    // Regular identifier scanning
    while (!isAtEnd() && (isAlphaNumeric(peek()) || peek() == '-')) {
        value += peek();
        advance();
    }

    // Check if it's a keyword
    auto it = keywords.find(value);
    if (it != keywords.end()) {
        return Token(it->second, value, line, startColumn);
    }

    // Check if identifier starts with a digit
    if (isDigit(value[0])) {
        error("Invalid identifier: cannot start with a digit", line, startColumn);
        return Token(TokenType::ERROR, value, line, startColumn);
    }

    return Token(TokenType::IDENTIFIER, value, line, startColumn);
}

Token Scanner::scanNumber() {
    int startColumn = column;
    std::string value;
    bool hasDecimal = false;
    bool isFloat = false;
    bool hasF = false;

    // Handle sign if present
    if (peek() == '+' || peek() == '-') {
        value += peek();
        advance();
    }

    // Scan digits before decimal point
    while (!isAtEnd() && isDigit(peek())) {
        value += peek();
        advance();
    }

    // Handle decimal point and following digits
    if (peek() == '.') {
        hasDecimal = true;
        isFloat = true;
        value += peek();
        advance();

        while (!isAtEnd() && isDigit(peek())) {
            value += peek();
            advance();
        }
    }

    // Handle 'f' or 'F' suffix for explicit float literals
    if (peek() == 'f' || peek() == 'F') {
        hasF = true;
        isFloat = true;
        advance();
    }

    // Validate number format
    if (value.back() == '.') {
        error("Invalid number format: ends with decimal point", line, column);
        return Token(TokenType::ERROR, "", line, startColumn);
    }

    // Return appropriate token type
    if (isFloat) {
        return Token(TokenType::FLOAT, value, line, startColumn);
    } else {
        return Token(TokenType::INTEGER, value, line, startColumn);
    }
}

Token Scanner::scanString() {
    int startColumn = column;
    std::string value;
    advance(); // Consume opening quote

    while (!isAtEnd() && peek() != '"') {
        if (peek() == '\n') {
            error("Unterminated string literal", line, column);
            return Token(TokenType::ERROR, "", line, startColumn);
        }
        value += peek();
        advance();
    }

    if (isAtEnd()) {
        error("Unterminated string literal", line, column);
        return Token(TokenType::ERROR, "", line, startColumn);
    }

    advance(); // Consume closing quote
    return Token(TokenType::STRING, value, line, startColumn);
}

Token Scanner::scanCharacter() {
    int startColumn = column;
    advance(); // Consume opening quote

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

    advance(); // Consume closing quote
    return Token(TokenType::CHARACTER, value, line, startColumn);
}

Token Scanner::scanComment() {
    int startColumn = column;
    char first = advance(); // Consume /
    char second = peek();
    
    if (second == '@') {
        advance(); // Consume @
        Token startToken(TokenType::COMMENT_START, "/@", line, startColumn);
        std::string content;
        int contentLine = line;
        int contentColumn = column;
        
        while (!isAtEnd() && !(peek() == '@' && peekNext() == '/')) {
            content += peek();
            advance();
        }
        
        if (!isAtEnd()) {
            advance(); // Consume @
            advance(); // Consume /
            // Return comment content token
            return Token(TokenType::COMMENT_CONTENT, content, contentLine, contentColumn);
        }
        
        error("Unterminated multi-line comment", line, column);
        return Token(TokenType::ERROR, "", line, startColumn);
    } 
    else if (second == '^') {
        advance(); // Consume ^
        std::string content;
        int contentLine = line;
        int contentColumn = column;
        
        while (!isAtEnd() && peek() != '\n') {
            content += peek();
            advance();
        }
        
        // Return single-line comment content token
        return Token(TokenType::COMMENT_CONTENT, content, contentLine, contentColumn);
    }
    
    return Token(TokenType::DIVIDE, "/", line, startColumn);
}

Token Scanner::scanOperator() {
    int startColumn = column;
    char c = peek();
    std::string op;
    op += c;
    advance();

    switch (c) {
        case '=':
            if (peek() == '=') {
                op += advance();
                return Token(TokenType::EQUAL, op, line, startColumn);
            }
            return Token(TokenType::ASSIGN, op, line, startColumn);
        
        case '!':
            if (peek() == '=') {
                op += advance();
                return Token(TokenType::NOT_EQUAL, op, line, startColumn);
            }
            error("Invalid operator: single '!'", line, startColumn);
            return Token(TokenType::ERROR, op, line, startColumn);
        
        case '<':
            if (peek() == '=') {
                op += advance();
                return Token(TokenType::LESS_EQUAL, op, line, startColumn);
            }
            return Token(TokenType::LESS, op, line, startColumn);
        
        case '>':
            if (peek() == '=') {
                op += advance();
                return Token(TokenType::GREATER_EQUAL, op, line, startColumn);
            }
            return Token(TokenType::GREATER, op, line, startColumn);
        
        case '&':
            if (peek() == '&') {
                op += advance();
                return Token(TokenType::AND, op, line, startColumn);
            }
            error("Invalid operator: single '&'", line, startColumn);
            return Token(TokenType::ERROR, op, line, startColumn);
        
        case '|':
            if (peek() == '|') {
                op += advance();
                return Token(TokenType::OR, op, line, startColumn);
            }
            error("Invalid operator: single '|'", line, startColumn);
            return Token(TokenType::ERROR, op, line, startColumn);
        
        case '-':
            if (peek() == '>') {
                op += advance();
                return Token(TokenType::ACCESS, op, line, startColumn);
            }
            return Token(TokenType::MINUS, op, line, startColumn);
        
        case '~':
            return Token(TokenType::NOT, op, line, startColumn);
        
        case '+':
            return Token(TokenType::PLUS, op, line, startColumn);
        
        case '*':
            return Token(TokenType::MULTIPLY, op, line, startColumn);
        
        default:
            error("Invalid operator", line, startColumn);
            return Token(TokenType::ERROR, op, line, startColumn);
    }
}

void Scanner::handleInclude(const std::string& filename) {
    // Save current state
    fileStack.push_back(std::ifstream());
    filenameStack.push_back(filename);
    lineStack.push_back(line);
    columnStack.push_back(column);
    
    // Open the include file
    fileStack.back().open(filename);
    if (!fileStack.back().is_open()) {
        error("Could not open include file: " + filename, line, column);
        fileStack.pop_back();
        filenameStack.pop_back();
        lineStack.pop_back();
        columnStack.pop_back();
        return;
    }
    
    // Read the entire file into a string
    std::stringstream buffer;
    buffer << fileStack.back().rdbuf();
    std::string includeContent = buffer.str();
    
    // Insert the include file content at the current position
    source.insert(current, includeContent);
    
    // Close the file since we've read it
    fileStack.back().close();
    
    // Update line tracking for the included content
    size_t newlines = std::count(includeContent.begin(), includeContent.end(), '\n');
    line += newlines;
}

void Scanner::processIncludeDirective() {
    // Skip the "Include" keyword
    while (!isAtEnd() && peek() != '"') {
        advance();
    }
    
    if (isAtEnd() || peek() != '"') {
        error("Expected filename in quotes after Include", line, column);
        return;
    }
    
    advance(); // Skip opening quote
    std::string filename;
    
    while (!isAtEnd() && peek() != '"') {
        filename += peek();
        advance();
    }
    
    if (isAtEnd() || peek() != '"') {
        error("Unterminated filename in Include directive", line, column);
        return;
    }
    
    advance(); // Skip closing quote
    
    // Process the include file
    handleInclude(filename);
}

Token Scanner::getNextToken() {
    skipWhitespace();
    
    if (isAtEnd()) {
        return Token(TokenType::END_OF_FILE, "", line, column);
    }
    
    start = current;
    char c = peek();
    
    // Handle identifiers and keywords
    if (isAlpha(c)) {
        return scanIdentifier();
    }
    
    // Handle numbers
    if (isDigit(c) || (c == '-' && isDigit(peekNext()))) {
        return scanNumber();
    }
    
    // Handle string literals
    if (c == '"') {
        return scanString();
    }
    
    // Handle character literals
    if (c == '\'') {
        return scanCharacter();
    }
    
    // Handle comments
    if (c == '/') {
        char next = peekNext();
        if (next == '@' || next == '^') {
            return scanComment();
        }
        advance();
        return Token(TokenType::DIVIDE, "/", line, column-1);
    }
    
    // Handle operators and delimiters
    advance();
    switch (c) {
        case '(': return Token(TokenType::LEFT_PAREN, "(", line, column-1);
        case ')': return Token(TokenType::RIGHT_PAREN, ")", line, column-1);
        case '{': return Token(TokenType::LEFT_BRACE, "{", line, column-1);
        case '}': return Token(TokenType::RIGHT_BRACE, "}", line, column-1);
        case '[': return Token(TokenType::LEFT_BRACKET, "[", line, column-1);
        case ']': return Token(TokenType::RIGHT_BRACKET, "]", line, column-1);
        case ';': return Token(TokenType::SEMICOLON, ";", line, column-1);
        case ',': return Token(TokenType::COMMA, ",", line, column-1);
        case '+': return Token(TokenType::PLUS, "+", line, column-1);
        case '-': 
            if (peek() == '>') {
                advance();
                return Token(TokenType::ACCESS, "->", line, column-2);
            }
            return Token(TokenType::MINUS, "-", line, column-1);
        case '*': return Token(TokenType::MULTIPLY, "*", line, column-1);
        case '=':
            if (peek() == '=') {
                advance();
                return Token(TokenType::EQUAL, "==", line, column-2);
            }
            return Token(TokenType::ASSIGN, "=", line, column-1);
        case '<':
            if (peek() == '=') {
                advance();
                return Token(TokenType::LESS_EQUAL, "<=", line, column-2);
            }
            return Token(TokenType::LESS, "<", line, column-1);
        case '>':
            if (peek() == '=') {
                advance();
                return Token(TokenType::GREATER_EQUAL, ">=", line, column-2);
            }
            return Token(TokenType::GREATER, ">", line, column-1);
        case '!':
            if (peek() == '=') {
                advance();
                return Token(TokenType::NOT_EQUAL, "!=", line, column-2);
            }
            error("Invalid operator: single '!'", line, column-1);
            return Token(TokenType::ERROR, "!", line, column-1);
        case '&':
            if (peek() == '&') {
                advance();
                return Token(TokenType::AND, "&&", line, column-2);
            }
            error("Invalid operator: single '&'", line, column-1);
            return Token(TokenType::ERROR, "&", line, column-1);
        case '|':
            if (peek() == '|') {
                advance();
                return Token(TokenType::OR, "||", line, column-2);
            }
            error("Invalid operator: single '|'", line, column-1);
            return Token(TokenType::ERROR, "|", line, column-1);
        case '~': return Token(TokenType::NOT, "~", line, column-1);
    }
    
    // If we get here, we have an invalid character
    std::string errorMsg = "Unexpected character: ";
    errorMsg += c;
    error(errorMsg, line, column-1);
    return Token(TokenType::ERROR, std::string(1, c), line, column-1);
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
    ss << "Line " << line << ", Column " << column << ": " << message;
    errors.push_back(ss.str());
}

void Scanner::ungetToken() {
    if (current > 0) {
        current--;
        // Restore the previous line and column if we're moving back
        if (source[current] == '\n') {
            line--;
            // We need to count the characters in the previous line to restore column
            size_t lineStart = source.rfind('\n', current - 1);
            if (lineStart == std::string::npos) {
                lineStart = 0;
            } else {
                lineStart++;
            }
            column = current - lineStart;
        } else {
            column--;
        }
    }
} 