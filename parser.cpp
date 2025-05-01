#include "parser.h"
#include <iostream>
#include <stdexcept>
#include <sstream>
#include <iomanip>

// AST Node Definitions
class BreakNode : public ASTNode {
public:
    BreakNode(int line, int col) : ASTNode(NodeType::OUTLOOP_STMT, line, col) {}
};

class ReturnNode : public ASTNode {
public:
    std::unique_ptr<ExpressionNode> value;
    
    ReturnNode(std::unique_ptr<ExpressionNode> v, int line, int col) 
        : ASTNode(NodeType::TURNBACK_STMT, line, col), value(std::move(v)) {}
};

class ExpressionStatementNode : public ASTNode {
public:
    std::unique_ptr<ExpressionNode> expression;
    
    ExpressionStatementNode(std::unique_ptr<ExpressionNode> expr, int line, int col)
        : ASTNode(NodeType::EXPRESSION, line, col), expression(std::move(expr)) {}
};

class AssignmentNode : public ExpressionNode {
public:
    Token name;
    std::unique_ptr<ExpressionNode> value;
    
    AssignmentNode(Token n, std::unique_ptr<ExpressionNode> v, int line, int col)
        : ExpressionNode(NodeType::BINARY_EXPR, line, col), name(n), value(std::move(v)) {}
};

class IfNode : public ASTNode {
public:
    std::unique_ptr<ExpressionNode> condition;
    std::unique_ptr<BlockNode> thenBranch;
    std::unique_ptr<BlockNode> elseBranch;
    
    IfNode(std::unique_ptr<ExpressionNode> c, 
           std::unique_ptr<BlockNode> t,
           std::unique_ptr<BlockNode> e,
           int line, int col)
        : ASTNode(NodeType::IF_TRUE_STMT, line, col),
          condition(std::move(c)),
          thenBranch(std::move(t)),
          elseBranch(std::move(e)) {}
};

class WhileNode : public ASTNode {
public:
    std::unique_ptr<ExpressionNode> condition;
    std::unique_ptr<BlockNode> body;
    
    WhileNode(std::unique_ptr<ExpressionNode> c,
              std::unique_ptr<BlockNode> b,
              int line, int col)
        : ASTNode(NodeType::REPEATWHEN_STMT, line, col),
          condition(std::move(c)),
          body(std::move(b)) {}
};

class StructDeclNode : public ASTNode {
public:
    Token name;
    std::vector<std::pair<Token, Token>> fields;
    
    StructDeclNode(Token n, std::vector<std::pair<Token, Token>> f, int line, int col)
        : ASTNode(NodeType::LOLI_DECL, line, col),
          name(n), fields(std::move(f)) {}
};

class IncludeNode : public ASTNode {
public:
    Token path;
    
    IncludeNode(Token p, int line, int col)
        : ASTNode(NodeType::INCLUDE_STMT, line, col), path(p) {}
};

Parser::Parser(const std::string& source) : scanner(source), errorCount(0) {
    advance();
}

void Parser::advance() {
    currentToken = scanner.getNextToken();
}

void Parser::match(TokenType expected) {
    if (currentToken.type == expected) {
        std::cout << "Line #: " << currentToken.line << " Matched Rule Used: " 
                  << static_cast<int>(currentToken.type) << std::endl;
        advance();
    } else {
        std::stringstream ss;
        ss << "Line #: " << currentToken.line << " Not Matched";
        error(ss.str());
    }
}

void Parser::error(const std::string& message) {
    std::stringstream ss;
    ss << "Line #: " << currentToken.line << " Error: " << message;
    errors.push_back(ss.str());
    errorCount++;
}

void Parser::parse() {
    program();
    std::cout << "Total NO of errors: " << errorCount << std::endl;
}

// Grammar rule implementations
void Parser::program() {
    // program → declaration-list | comment | include_command
    while (currentToken.type != TokenType::END_OF_FILE) {
        if (currentToken.type == TokenType::COMMENT_START) {
            comment();
        } else if (currentToken.type == TokenType::INCLUDE) {
            includeCommand();
        } else {
            declarationList();
        }
    }
}

void Parser::declarationList() {
    // declaration-list → declaration-list declaration | declaration
    declaration();
    while (currentToken.type != TokenType::END_OF_FILE && 
           currentToken.type != TokenType::RIGHT_BRACE) {
        declaration();
    }
}

void Parser::declaration() {
    // declaration → var-declaration | fun-declaration
    if (isTypeSpecifier(currentToken.type)) {
        Token typeToken = currentToken;
        advance(); // Consume type specifier
        
        if (currentToken.type != TokenType::IDENTIFIER) {
            error("Expected identifier after type specifier");
            return;
        }
        
        Token nameToken = currentToken;
        advance(); // Consume identifier
        
        if (currentToken.type == TokenType::LEFT_PAREN) {
            // Function declaration
            match(TokenType::LEFT_PAREN);
            params();
            match(TokenType::RIGHT_PAREN);
            compoundStmt();
        } else if (currentToken.type == TokenType::ASSIGN) {
            // Variable declaration with initialization
            match(TokenType::ASSIGN);
            expression();
            match(TokenType::SEMICOLON);
        } else {
            // Simple variable declaration
            match(TokenType::SEMICOLON);
        }
    } else {
        error("Expected type specifier (Imw, SIMw, IMwf, SIMwf, Chj, Series, or NOReturn)");
        advance();
    }
}

void Parser::varDeclaration() {
    // var-declaration → type-specifier ID ;
    typeSpecifier();
    if (currentToken.type == TokenType::IDENTIFIER) {
        match(TokenType::IDENTIFIER);
        match(TokenType::SEMICOLON);
    } else {
        error("Expected identifier");
        advance();
    }
}

void Parser::funDeclaration() {
    // fun-declaration → type-specifier ID ( params ) compound-stmt
    typeSpecifier();
    if (currentToken.type == TokenType::IDENTIFIER) {
        match(TokenType::IDENTIFIER);
        match(TokenType::LEFT_PAREN);
        params();
        match(TokenType::RIGHT_PAREN);
        compoundStmt();
    } else {
        error("Expected function name");
        advance();
    }
}

void Parser::typeSpecifier() {
    // type-specifier → Imw | SIMw | Chj | Series | IMwf | SIMwf | NOReturn
    if (isTypeSpecifier(currentToken.type)) {
        std::cout << "Matched type specifier: " << currentToken.value << std::endl;
        advance();
    } else {
        error("Invalid type specifier. Expected one of: Imw, SIMw, IMwf, SIMwf, Chj, Series, or NOReturn");
        advance();
    }
}

bool Parser::isTypeSpecifier(TokenType type) {
    switch (type) {
        case TokenType::INTEGER:    // Imw
        case TokenType::SINTEGER:   // SIMw
        case TokenType::CHARACTER:  // Chj
        case TokenType::STRING:     // Series
        case TokenType::FLOAT:      // IMwf
        case TokenType::SFLOAT:     // SIMwf
        case TokenType::VOID:       // NOReturn
            return true;
        default:
            return false;
    }
}

void Parser::params() {
    // params → param-list | NOReturn | ε
    if (currentToken.type == TokenType::VOID) {
        match(TokenType::VOID);
    } else if (isTypeSpecifier(currentToken.type)) {
        paramList();
    }
}

void Parser::paramList() {
    // param-list → param-list , param | param
    param();
    while (currentToken.type == TokenType::COMMA) {
        match(TokenType::COMMA);
        param();
    }
}

void Parser::param() {
    // param → type-specifier ID
    typeSpecifier();
    if (currentToken.type == TokenType::IDENTIFIER) {
        match(TokenType::IDENTIFIER);
    } else {
        error("Expected parameter name");
        advance();
    }
}

void Parser::compoundStmt() {
    // compound-stmt → { local-declarations statement-list }
    match(TokenType::LEFT_BRACE);
    localDeclarations();
    statementList();
    match(TokenType::RIGHT_BRACE);
}

void Parser::localDeclarations() {
    // local-declarations → local-declarations var-declaration | ε
    while (isTypeSpecifier(currentToken.type)) {
        varDeclaration();
    }
}

void Parser::statementList() {
    // statement-list → statement-list statement | ε
    while (currentToken.type != TokenType::RIGHT_BRACE && 
           currentToken.type != TokenType::END_OF_FILE) {
        statement();
    }
}

void Parser::statement() {
    // statement → expression-stmt | compound-stmt | selection-stmt | iteration-stmt | jump-stmt
    switch (currentToken.type) {
        case TokenType::CONDITION:
            selectionStmt();
            break;
        case TokenType::LOOP:
            iterationStmt();
            break;
        case TokenType::RETURN:
        case TokenType::BREAK:
            jumpStmt();
            break;
        case TokenType::LEFT_BRACE:
            compoundStmt();
            break;
        default:
            expressionStmt();
            break;
    }
}

void Parser::expressionStmt() {
    // expression-stmt → expression ; | ;
    if (currentToken.type != TokenType::SEMICOLON) {
        expression();
    }
    match(TokenType::SEMICOLON);
}

void Parser::selectionStmt() {
    // selection-stmt → IfTrue ( expression ) statement | IfTrue ( expression ) statement Otherwise statement
    match(TokenType::CONDITION);
    match(TokenType::LEFT_PAREN);
    expression();
    match(TokenType::RIGHT_PAREN);
    statement();
    if (currentToken.type == TokenType::OTHERWISE) {
        match(TokenType::OTHERWISE);
        statement();
    }
}

void Parser::iterationStmt() {
    // iteration-stmt → RepeatWhen ( expression ) statement | Reiterate ( expression ; expression ; expression ) statement
    match(TokenType::LOOP);
    match(TokenType::LEFT_PAREN);
    expression();
    if (currentToken.type == TokenType::SEMICOLON) {
        // Reiterate statement
        match(TokenType::SEMICOLON);
        expression();
        match(TokenType::SEMICOLON);
        expression();
    }
    match(TokenType::RIGHT_PAREN);
    statement();
}

void Parser::jumpStmt() {
    // jump-stmt → Turnback expression ; | Stop ;
    if (currentToken.type == TokenType::RETURN) {
        match(TokenType::RETURN);
        if (currentToken.type != TokenType::SEMICOLON) {
            expression();
        }
    } else if (currentToken.type == TokenType::BREAK) {
        match(TokenType::BREAK);
    }
    match(TokenType::SEMICOLON);
}

void Parser::expression() {
    // expression → id-assign = expression | simple-expression
    if (currentToken.type == TokenType::IDENTIFIER) {
        Token lookAhead = scanner.getNextToken();
        scanner.ungetToken();
        
        if (lookAhead.type == TokenType::ASSIGN) {
            idAssign();
            match(TokenType::ASSIGN);
            expression();
        } else {
            simpleExpression();
        }
    } else {
        simpleExpression();
    }
}

void Parser::idAssign() {
    // id-assign → ID
    match(TokenType::IDENTIFIER);
}

void Parser::simpleExpression() {
    // simple-expression → additive-expression relop additive-expression | additive-expression
    additiveExpression();
    if (isRelop(currentToken.type)) {
        advance();
        additiveExpression();
    }
}

bool Parser::isRelop(TokenType type) {
    return type == TokenType::LESS_EQUAL || type == TokenType::LESS ||
           type == TokenType::GREATER || type == TokenType::GREATER_EQUAL ||
           type == TokenType::EQUAL || type == TokenType::NOT_EQUAL ||
           type == TokenType::AND || type == TokenType::OR;
}

void Parser::additiveExpression() {
    // additive-expression → additive-expression addop term | term
    term();
    while (currentToken.type == TokenType::PLUS || 
           currentToken.type == TokenType::MINUS) {
        advance();
        term();
    }
}

void Parser::term() {
    // term → term mulop factor | factor
    factor();
    while (currentToken.type == TokenType::MULTIPLY || 
           currentToken.type == TokenType::DIVIDE) {
        advance();
        factor();
    }
}

void Parser::factor() {
    // factor → ( expression ) | id-assign | call | num
    Token lookAhead;
    
    switch (currentToken.type) {
        case TokenType::LEFT_PAREN:
            match(TokenType::LEFT_PAREN);
            expression();
            match(TokenType::RIGHT_PAREN);
            break;
            
        case TokenType::IDENTIFIER:
            lookAhead = scanner.getNextToken();
            scanner.ungetToken();
            if (lookAhead.type == TokenType::LEFT_PAREN) {
                call();
            } else {
                idAssign();
            }
            break;
            
        case TokenType::INTEGER:
        case TokenType::FLOAT:
            num();
            break;
            
        default:
            error("Invalid factor");
            advance();
            break;
    }
}

void Parser::call() {
    // call → ID ( args )
    match(TokenType::IDENTIFIER);
    match(TokenType::LEFT_PAREN);
    args();
    match(TokenType::RIGHT_PAREN);
}

void Parser::args() {
    // args → arg-list | ε
    if (currentToken.type != TokenType::RIGHT_PAREN) {
        argList();
    }
}

void Parser::argList() {
    // arg-list → arg-list , expression | expression
    expression();
    while (currentToken.type == TokenType::COMMA) {
        match(TokenType::COMMA);
        expression();
    }
}

void Parser::num() {
    // num → INTEGER | FLOAT
    if (currentToken.type == TokenType::INTEGER || currentToken.type == TokenType::FLOAT) {
        advance();
    } else {
        error("Expected number");
        advance();
    }
}

void Parser::comment() {
    // comment → /@ STR @/ | /^ STR
    if (currentToken.type == TokenType::COMMENT_START) {
        advance();
        while (currentToken.type == TokenType::COMMENT_CONTENT) {
            advance();
        }
        if (currentToken.type == TokenType::COMMENT_END) {
            advance();
        }
    }
}

void Parser::includeCommand() {
    // include_command → Include ( F_name.txt );
    match(TokenType::INCLUDE);
    match(TokenType::LEFT_PAREN);
    if (currentToken.type == TokenType::STRING) {
        advance();
    } else {
        error("Expected file name");
    }
    match(TokenType::RIGHT_PAREN);
    match(TokenType::SEMICOLON);
}

bool Parser::hasError() const {
    return errorCount > 0;
}

const std::vector<std::string>& Parser::getErrors() const {
    return errors;
}

int Parser::getErrorCount() const {
    return errorCount;
} 