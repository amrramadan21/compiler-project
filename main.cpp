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
        case TokenType::INCLUDE: return "Include";
        case TokenType::IDENTIFIER: return "Identifier";
        case TokenType::PLUS: return "Plus";
        case TokenType::MINUS: return "Minus";
        case TokenType::MULTIPLY: return "Multiply";
        case TokenType::DIVIDE: return "Divide";
        case TokenType::AND: return "And";
        case TokenType::OR: return "Or";
        case TokenType::NOT: return "Not";
        case TokenType::EQUAL: return "Equal";
        case TokenType::LESS: return "Less";
        case TokenType::GREATER: return "Greater";
        case TokenType::NOT_EQUAL: return "NotEqual";
        case TokenType::LESS_EQUAL: return "LessEqual";
        case TokenType::GREATER_EQUAL: return "GreaterEqual";
        case TokenType::ASSIGN: return "Assign";
        case TokenType::ACCESS: return "Access";
        case TokenType::LEFT_PAREN: return "LeftParen";
        case TokenType::RIGHT_PAREN: return "RightParen";
        case TokenType::LEFT_BRACE: return "LeftBrace";
        case TokenType::RIGHT_BRACE: return "RightBrace";
        case TokenType::LEFT_BRACKET: return "LeftBracket";
        case TokenType::RIGHT_BRACKET: return "RightBracket";
        case TokenType::SEMICOLON: return "Semicolon";
        case TokenType::COMMA: return "Comma";
        case TokenType::CONSTANT: return "Constant";
        case TokenType::QUOTE: return "Quote";
        case TokenType::COMMENT_START: return "CommentStart";
        case TokenType::COMMENT_END: return "CommentEnd";
        case TokenType::COMMENT_CONTENT: return "CommentContent";
        case TokenType::END_OF_FILE: return "EndOfFile";
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
    std::cout << "Enter code (type 'end' on a new line to process, 'exit' to quit):\n";
    std::string line;
    std::string source;
    
    while (true) {
        std::cout << "> ";
        std::getline(std::cin, line);
        
        if (line == "exit") {
            break;
        }
        
        if (line == "end") {
            if (!source.empty()) {
                // Process the accumulated code
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
                
                std::cout << "\nParser phase:\n";
                Parser parser(source);  // Create parser with source string
                
                parser.parse();  // Parse without assignment
                
                if (parser.hasError()) {
                    std::cerr << "Syntax errors found:\n";
                    for (const auto& error : parser.getErrors()) {
                        std::cerr << error << std::endl;
                    }
                } else {
                    std::cout << "Parsing completed successfully!\n";
                }
                
                source.clear();
            }
            continue;
        }
        
        source += line + "\n";
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

void processFile(const std::string& source) {
    std::cout << "Source code to process:\n" << source << "\n";
    
    // Scanner Phase
    std::cout << "\n=== Scanner Phase ===\n";
    Scanner scanner(source);
    Token token;
    int errorCount = 0;
    
    do {
        token = scanner.getNextToken();
        
        if (token.type == TokenType::ERROR) {
            std::cout << "Line #: " << token.line << " Error in Token Text: " << token.value << std::endl;
            errorCount++;
        } else {
            std::cout << "Line #: " << token.line << " Token Text: " << token.value 
                      << " Token Type: " << tokenTypeToString(token.type) << std::endl;
        }
    } while (token.type != TokenType::END_OF_FILE);
    
    std::cout << "Total NO of errors: " << errorCount << std::endl;
    
    // Parser Phase
    if (errorCount == 0) {
        std::cout << "\n=== Parser Phase ===\n";
        try {
            Parser parser(source);
            parser.parse();
            
            if (parser.hasError()) {
                std::cout << "\nParser errors found:\n";
                for (const auto& error : parser.getErrors()) {
                    std::cout << error << std::endl;
                }
                std::cout << "\nTotal NO of errors: " << parser.getErrorCount() << std::endl;
            } else {
                std::cout << "\nParsing completed successfully!\n";
            }
        } catch (const std::exception& e) {
            std::cout << "Parser error: " << e.what() << std::endl;
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc > 1) {
        std::string arg = argv[1];
        if (arg == "-i" || arg == "--interactive") {
            processInteractiveInput();
            return 0;
        } else if (arg == "-h" || arg == "--help") {
            printUsage();
            return 0;
        }
        
        // File input mode
        std::ifstream file(argv[1]);
        if (!file.is_open()) {
            std::cerr << "Error: Could not open file " << argv[1] << std::endl;
            return 1;
        }
        
        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string source = buffer.str();
        processFile(source);
        return 0;
    }
    
    // Test program mode
    std::string testProgram = R"(
/@ This is a test program @/

Include("test.txt");

Imw x;
SIMw y;

NOReturn main() {
    Series message;
    Imw count;
    
    count = 10;
    
    IfTrue(count > 0) {
        message = "Count is positive";
    } Otherwise {
        message = "Count is zero or negative";
    }
    
    RepeatWhen(count > 0) {
        count = count - 1;
        IfTrue(count == 5) {
            OutLoop;
        }
    }
    
    Reiterate(Imw i = 0; i < 10; i = i + 1) {
        /^ Loop body ^/
    }
    
    Turnback;
}
)";

    std::cout << "Processing test program...\n";
    processFile(testProgram);
    
    return 0;
} 