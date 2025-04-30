#include "scanner.h"
#include <iostream>
#include <fstream>
#include <sstream>

int main() {
    // Open the test input file
    std::ifstream file("test_input.txt");
    if (!file.is_open()) {
        std::cerr << "Error: Could not open test_input.txt" << std::endl;
        return 1;
    }

    // Read the entire file into a string
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string source = buffer.str();
    file.close();

    // Create scanner with the source string
    Scanner scanner(source);

    // Test the scanner
    Token token = Token(TokenType::END_OF_FILE, "", 0, 0);  // Initialize with dummy values
    do {
        token = scanner.getNextToken();
        std::cout << "Token: " << token.value 
                  << " Type: " << static_cast<int>(token.type)
                  << " Line: " << token.line 
                  << " Column: " << token.column << std::endl;
    } while (token.type != TokenType::END_OF_FILE);

    // Check for errors
    if (scanner.hasError()) {
        std::cerr << "Scanner errors:" << std::endl;
        for (const auto& error : scanner.getErrors()) {
            std::cerr << error << std::endl;
        }
        return 1;
    }

    std::cout << "Scanner test completed successfully" << std::endl;
    return 0;
} 