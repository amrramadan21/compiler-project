#include "scanner.h"
#include "parser.h"
#include <iostream>
#include <fstream>
#include <string>
#include <memory>
#include <sstream>
#include <iomanip>

void printUsage() {
    std::cout << "Usage: compiler [options] <input_file>\n";
    std::cout << "Options:\n";
    std::cout << "  -i, --interactive  Run in interactive mode\n";
    std::cout << "  -h, --help         Show this help message\n";
}

std::string tokenTypeToString(TokenType type) {
    switch (type) {
        case TokenType::CONDITION: return "Condition";
        case TokenType::INTEGER: return "Integer";
        case TokenType::SINTEGER: return "SInteger";
        case TokenType::CHARACTER: return "Character";
        case TokenType::STRING: return "String";
        case TokenType::FLOAT: return "Float";
        case TokenType::SFLOAT: return "SFloat";
        case TokenType::VOID: return "Void";
        case TokenType::LOOP: return "Loop";
        case TokenType::RETURN: return "Return";
        case TokenType::BREAK: return "Break";
        case TokenType::STRUCT: return "Struct";
        case TokenType::INCLUDE: return "Inclusion";
        case TokenType::PLUS: return "Arithmetic Operation";
        case TokenType::MINUS: return "Arithmetic Operation";
        case TokenType::MULTIPLY: return "Arithmetic Operation";
        case TokenType::DIVIDE: return "Arithmetic Operation";
        case TokenType::AND: return "Logic operators";
        case TokenType::OR: return "Logic operators";
        case TokenType::NOT: return "Logic operators";
        case TokenType::EQUAL: return "relational operators";
        case TokenType::LESS: return "relational operators";
        case TokenType::GREATER: return "relational operators";
        case TokenType::NOT_EQUAL: return "relational operators";
        case TokenType::LESS_EQUAL: return "relational operators";
        case TokenType::GREATER_EQUAL: return "relational operators";
        case TokenType::ASSIGN: return "Assignment operator";
        case TokenType::ACCESS: return "Access Operator";
        case TokenType::LEFT_BRACE: return "Braces";
        case TokenType::RIGHT_BRACE: return "Braces";
        case TokenType::LEFT_BRACKET: return "Braces";
        case TokenType::RIGHT_BRACKET: return "Braces";
        case TokenType::SEMICOLON: return "Braces";
        case TokenType::COMMA: return "Braces";
        case TokenType::CONSTANT: return "Constant";
        case TokenType::QUOTE: return "Quotation Mark";
        case TokenType::COMMENT_START: return "Comment";
        case TokenType::COMMENT_END: return "Comment";
        case TokenType::COMMENT_CONTENT: return "Comment";
        case TokenType::END_OF_FILE: return "End of File";
        case TokenType::ERROR: return "Error";
        default: return "Unknown";
    }
}

void processToken(const Token& token) {
    std::cout << "[" << token.line << ":" << token.column << "] ";
    std::cout << tokenTypeToString(token.type);
    if (!token.value.empty()) {
        std::cout << " = '" << token.value << "'";
    }
    std::cout << std::endl;
}

void processInteractiveInput() {
    std::cout << "Enter code (type 'exit' to quit):\n";
    std::string line;
    std::string source;
    
    while (true) {
        std::cout << "> ";
        std::getline(std::cin, line);
        
        if (line == "exit") {
            break;
        }
        
        source += line + "\n";
        
        Scanner scanner(source);
        std::vector<Token> tokens;
        while (true) {
            Token token = scanner.getNextToken();
            if (token.type == TokenType::END_OF_FILE) {
                break;
            }
            tokens.push_back(token);
            processToken(token);
        }
        
        if (scanner.hasError()) {
            std::cerr << "Lexical errors found:\n";
            for (const auto& error : scanner.getErrors()) {
                std::cerr << error << std::endl;
            }
            source.clear();
            continue;
        }
        
        // Parser phase
        std::cout << "\nParser phase:\n";
        Scanner parserScanner(source);  // Create a new scanner instance for parser
        std::vector<Token> parserTokens;
        Token token;
        do {
            token = parserScanner.getNextToken();
            parserTokens.push_back(token);
        } while (token.type != TokenType::END_OF_FILE);
        
        Parser parser(parserTokens);
        
        // Parse the program
        auto ast = parser.parse();
        
        if (parser.hasError()) {
            std::cerr << "Syntax errors found:\n";
            std::cerr << parser.getError() << std::endl;
            source.clear();
            continue;
        }
        
        std::cout << "Parsing completed successfully!\n";
        source.clear();
    }
}

void printScannerOutput(const std::vector<Token>& tokens) {
    std::cout << "Scanner Phase Output:\n";
    for (const auto& token : tokens) {
        std::cout << "Token: " << tokenTypeToString(token.type) 
                  << " Value: " << token.value 
                  << " Line: " << token.line << "\n";
    }
    std::cout << "\n";
}

void printParserOutput(const std::vector<std::string>& matchedRules, 
                      const std::vector<std::string>& errors) {
    std::cout << "Parser Phase Output:\n";
    
    // Print matched rules
    for (const auto& rule : matchedRules) {
        std::cout << rule << "\n";
    }
    
    // Print errors
    for (const auto& error : errors) {
        std::cout << error << "\n";
    }
    
    // Print total error count
    std::cout << "\nTotal NO of errors: " << errors.size() << "\n";
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <input_file>\n";
        return 1;
    }

    std::ifstream file(argv[1]);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << argv[1] << "\n";
        return 1;
    }

    // Read file content
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string source = buffer.str();
    file.close();

    // Scanner phase
    std::cout << "\n=== Scanner Phase ===\n";
    Scanner scanner(source);
    std::vector<Token> tokens;
    Token token;
    do {
        token = scanner.getNextToken();
        tokens.push_back(token);
        processToken(token);  // Print each token as we get it
    } while (token.type != TokenType::END_OF_FILE);

    if (scanner.hasError()) {
        std::cerr << "\nLexical errors found:\n";
        for (const auto& error : scanner.getErrors()) {
            std::cerr << error << std::endl;
        }
        return 1;
    }

    // Parser phase
    std::cout << "\n=== Parser Phase ===\n";
    Parser parser(tokens);  // Use the tokens we already collected
    auto ast = parser.parse();

    if (parser.hasError()) {
        std::cerr << "\nSyntax errors found:\n";
        std::cerr << parser.getError() << std::endl;
        return 1;
    }

    std::cout << "\nParsing completed successfully!\n";
    return 0;
} 