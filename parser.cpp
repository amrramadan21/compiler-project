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

Parser::Parser(const std::vector<Token>& tokens) 
    : tokens(tokens), currentIndex(0),
      current(TokenType::UNKNOWN, "", 0, 0),
      previousToken(TokenType::UNKNOWN, "", 0, 0) {
    if (!tokens.empty()) {
        current = tokens[0];
    }
}

Token Parser::advance() {
    previousToken = current;
    if (!isAtEnd()) {
        currentIndex++;
        current = tokens[currentIndex];
    }
    return previousToken;
}

Token Parser::peek() const {
    return current;
}

Token Parser::previous() const {
    return previousToken;
}

bool Parser::match(TokenType type) {
    if (check(type)) {
        advance();
        return true;
    }
    return false;
}

bool Parser::check(TokenType type) const {
    if (isAtEnd()) return false;
    return current.type == type;
}

Token Parser::consume(TokenType type, const std::string& message) {
    if (check(type)) return advance();
    throw error(current, message);
}

Parser::ParseError Parser::error(const Token& token, const std::string& message) {
    std::stringstream ss;
    ss << "[Line " << token.line << ", Column " << token.column << "] Error: " << message;
    errorMessage = ss.str();
    return ParseError{};
}

void Parser::reportError(const std::string& message) {
    errorMessage = message;
}

void Parser::reportMatch(const std::string& rule) {
    matchedRules.push_back(rule);
}

void Parser::synchronize() {
    advance();
    while (!isAtEnd()) {
        if (previousToken.type == TokenType::SEMICOLON) return;
        
        switch (current.type) {
            case TokenType::CONDITION:
            case TokenType::LOOP:
            case TokenType::RETURN:
            case TokenType::BREAK:
            case TokenType::INTEGER:
            case TokenType::FLOAT:
            case TokenType::STRING:
            case TokenType::CHARACTER:
                return;
            default:
                advance();
        }
    }
}

bool Parser::isAtEnd() const {
    return currentIndex >= tokens.size();
}

bool Parser::isUnaryOperator(TokenType type) const {
    return type == TokenType::NOT || type == TokenType::MINUS;
}

bool Parser::isBinaryOperator(TokenType type) const {
    return type == TokenType::PLUS || type == TokenType::MINUS ||
           type == TokenType::MULTIPLY || type == TokenType::DIVIDE ||
           type == TokenType::EQUAL || type == TokenType::NOT_EQUAL ||
           type == TokenType::LESS || type == TokenType::LESS_EQUAL ||
           type == TokenType::GREATER || type == TokenType::GREATER_EQUAL ||
           type == TokenType::AND || type == TokenType::OR;
}

int Parser::getOperatorPrecedence(TokenType type) const {
    switch (type) {
        case TokenType::OR: return 1;
        case TokenType::AND: return 2;
        case TokenType::EQUAL:
        case TokenType::NOT_EQUAL: return 3;
        case TokenType::LESS:
        case TokenType::LESS_EQUAL:
        case TokenType::GREATER:
        case TokenType::GREATER_EQUAL: return 4;
        case TokenType::PLUS:
        case TokenType::MINUS: return 5;
        case TokenType::MULTIPLY:
        case TokenType::DIVIDE: return 6;
        default: return 0;
    }
}

std::string Parser::tokenToString(TokenType type) const {
    switch (type) {
        case TokenType::INTEGER: return "INTEGER";
        case TokenType::FLOAT: return "FLOAT";
        case TokenType::STRING: return "STRING";
        case TokenType::CHARACTER: return "CHARACTER";
        case TokenType::CONDITION: return "CONDITION";
        case TokenType::LOOP: return "LOOP";
        case TokenType::RETURN: return "RETURN";
        case TokenType::BREAK: return "BREAK";
        case TokenType::IDENTIFIER: return "IDENTIFIER";
        case TokenType::CONSTANT: return "CONSTANT";
        case TokenType::SEMICOLON: return ";";
        case TokenType::LEFT_BRACE: return "(";
        case TokenType::RIGHT_BRACE: return ")";
        case TokenType::LEFT_BRACKET: return "{";
        case TokenType::RIGHT_BRACKET: return "}";
        case TokenType::EQUAL: return "=";
        case TokenType::NOT_EQUAL: return "!=";
        case TokenType::LESS: return "<";
        case TokenType::LESS_EQUAL: return "<=";
        case TokenType::GREATER: return ">";
        case TokenType::GREATER_EQUAL: return ">=";
        case TokenType::PLUS: return "+";
        case TokenType::MINUS: return "-";
        case TokenType::MULTIPLY: return "*";
        case TokenType::DIVIDE: return "/";
        case TokenType::AND: return "&&";
        case TokenType::OR: return "||";
        case TokenType::NOT: return "!";
        case TokenType::END_OF_FILE: return "EOF";
        default: return "UNKNOWN";
    }
}

bool Parser::isTypeToken(TokenType type) const {
    return type == TokenType::INTEGER ||
           type == TokenType::FLOAT ||
           type == TokenType::STRING ||
           type == TokenType::CHARACTER;
}

bool Parser::isTypeSpecifier(TokenType type) const {
    return isTypeToken(type);
}

bool Parser::isRelop(TokenType type) const {
    return type == TokenType::EQUAL ||
           type == TokenType::NOT_EQUAL ||
           type == TokenType::LESS ||
           type == TokenType::LESS_EQUAL ||
           type == TokenType::GREATER ||
           type == TokenType::GREATER_EQUAL;
}

bool Parser::isAddop(TokenType type) const {
    return type == TokenType::PLUS ||
           type == TokenType::MINUS;
}

bool Parser::isMulop(TokenType type) const {
    return type == TokenType::MULTIPLY ||
           type == TokenType::DIVIDE;
}

bool Parser::checkNext(TokenType type) const {
    if (isAtEnd()) return false;
    return tokens[currentIndex + 1].type == type;
}

std::unique_ptr<ASTNode> Parser::parse() {
    try {
        Token firstToken = peek();
        auto program = std::make_unique<BlockNode>(firstToken.line, firstToken.column);
        
        while (!isAtEnd()) {
            auto statement = parseStatement();
            if (statement) {
                program->statements.push_back(std::move(statement));
            }
        }
        
        return program;
    } catch (const ParseError&) {
        return nullptr;
    }
}

std::unique_ptr<ASTNode> Parser::parseStatement() {
    try {
        // Handle variable declarations
        if (match(TokenType::INTEGER) || match(TokenType::SINTEGER) || 
            match(TokenType::FLOAT) || match(TokenType::SFLOAT) ||
            match(TokenType::STRING) || match(TokenType::CHARACTER)) {
            return parseVariableDeclaration();
        }
        
        // Handle other statements
        if (match(TokenType::CONDITION)) {
            return parseIfTrueStatement();
        }
        
        if (match(TokenType::LOOP)) {
            return parseRepeatWhenStatement();
        }
        
        if (match(TokenType::RETURN)) {
            return parseTurnbackStatement();
        }
        
        if (match(TokenType::BREAK)) {
            consume(TokenType::SEMICOLON, "Expected ';' after break");
            return std::make_unique<OutLoopStmtNode>(previous().line, previous().column);
        }
        
        if (match(TokenType::STRUCT)) {
            return parseLoliDeclaration();
        }
        
        if (match(TokenType::INCLUDE)) {
            return parseIncludeStatement();
        }
        
        // Expression statement
        auto expr = parseExpression();
        consume(TokenType::SEMICOLON, "Expected ';' after expression");
        return std::make_unique<ExpressionStatementNode>(std::move(expr), previous().line, previous().column);
    } catch (const ParseError&) {
        synchronize();
        return nullptr;
    }
}

std::unique_ptr<ExpressionNode> Parser::parseExpression() {
    return parseAssignment();
}

std::unique_ptr<ExpressionNode> Parser::parseAssignment() {
    auto expr = parseOr();
    
    if (match(TokenType::EQUAL)) {
        Token equals = previous();
        auto value = parseAssignment();
        
        if (auto* varExpr = dynamic_cast<VariableNode*>(expr.get())) {
            Token name = varExpr->name;
            return std::make_unique<AssignmentNode>(name, std::move(value), 
                                                  equals.line, equals.column);
        }
        
        error(equals, "Invalid assignment target");
    }
    
    return expr;
}

std::unique_ptr<ExpressionNode> Parser::parseOr() {
    auto expr = parseAnd();
    
    while (match(TokenType::OR)) {
        Token op = previous();
        auto right = parseAnd();
        expr = std::make_unique<BinaryExprNode>(op.type, std::move(expr), 
                                              std::move(right), op.line, op.column);
    }
    
    return expr;
}

std::unique_ptr<ExpressionNode> Parser::parseAnd() {
    auto expr = parseEquality();
    
    while (match(TokenType::AND)) {
        Token op = previous();
        auto right = parseEquality();
        expr = std::make_unique<BinaryExprNode>(op.type, std::move(expr), 
                                              std::move(right), op.line, op.column);
    }
    
    return expr;
}

std::unique_ptr<ExpressionNode> Parser::parseEquality() {
    auto expr = parseComparison();
    
    while (match(TokenType::EQUAL) || match(TokenType::NOT_EQUAL)) {
        Token op = previous();
        auto right = parseComparison();
        expr = std::make_unique<BinaryExprNode>(op.type, std::move(expr), 
                                              std::move(right), op.line, op.column);
    }
    
    return expr;
}

std::unique_ptr<ExpressionNode> Parser::parseComparison() {
    auto expr = parseTerm();
    
    while (match(TokenType::GREATER) || match(TokenType::GREATER_EQUAL) ||
           match(TokenType::LESS) || match(TokenType::LESS_EQUAL)) {
        Token op = previous();
        auto right = parseTerm();
        expr = std::make_unique<BinaryExprNode>(op.type, std::move(expr), 
                                              std::move(right), op.line, op.column);
    }
    
    return expr;
}

std::unique_ptr<ExpressionNode> Parser::parseTerm() {
    auto expr = parseFactor();
    
    while (match(TokenType::PLUS) || match(TokenType::MINUS)) {
        Token op = previous();
        auto right = parseFactor();
        expr = std::make_unique<BinaryExprNode>(op.type, std::move(expr), 
                                              std::move(right), op.line, op.column);
    }
    
    return expr;
}

std::unique_ptr<ExpressionNode> Parser::parseFactor() {
    auto expr = parseUnary();
    
    while (match(TokenType::MULTIPLY) || match(TokenType::DIVIDE)) {
        Token op = previous();
        auto right = parseUnary();
        expr = std::make_unique<BinaryExprNode>(op.type, std::move(expr), 
                                              std::move(right), op.line, op.column);
    }
    
    return expr;
}

std::unique_ptr<ExpressionNode> Parser::parseUnary() {
    if (match(TokenType::NOT) || match(TokenType::MINUS)) {
        Token op = previous();
        auto right = parseUnary();
        return std::make_unique<UnaryExprNode>(op.type, std::move(right), 
                                             op.line, op.column);
    }
    
    return parsePrimary();
}

std::unique_ptr<ExpressionNode> Parser::parsePrimary() {
    if (match(TokenType::CONSTANT)) {
        Token token = previous();
        return std::make_unique<LiteralNode>(token.value, token.type, token.line, token.column);
    }
    
    if (match(TokenType::IDENTIFIER)) {
        return std::make_unique<VariableNode>(previous());
    }
    
    if (match(TokenType::LEFT_PAREN)) {
        auto expr = parseExpression();
        consume(TokenType::RIGHT_PAREN, "Expected ')' after expression");
        return std::make_unique<GroupingNode>(std::move(expr), 
                                            previous().line, previous().column);
    }
    
    throw error(peek(), "Expected expression");
}

std::unique_ptr<ASTNode> Parser::parseIfTrueStatement() {
    consume(TokenType::LEFT_PAREN, "Expected '(' after 'if_true'");
    auto condition = parseExpression();
    consume(TokenType::RIGHT_PAREN, "Expected ')' after condition");
    
    consume(TokenType::LEFT_BRACE, "Expected '{' before if_true body");
    auto thenBranch = parseBlock();
    
    std::unique_ptr<ASTNode> elseBranch = nullptr;
    if (match(TokenType::OTHERWISE)) {
        consume(TokenType::LEFT_BRACE, "Expected '{' before otherwise body");
        elseBranch = parseBlock();
    }
    
    return std::make_unique<IfTrueStmtNode>(std::move(condition), std::move(thenBranch), std::move(elseBranch), previous().line, previous().column);
}

std::unique_ptr<ASTNode> Parser::parseRepeatWhenStatement() {
    consume(TokenType::LEFT_PAREN, "Expected '(' after 'repeatwhen'");
    auto condition = parseExpression();
    consume(TokenType::RIGHT_PAREN, "Expected ')' after condition");
    
    consume(TokenType::LEFT_BRACE, "Expected '{' before repeatwhen body");
    auto body = parseBlock();
    
    return std::make_unique<RepeatWhenStmtNode>(std::move(condition), std::move(body), previous().line, previous().column);
}

std::unique_ptr<ASTNode> Parser::parseTurnbackStatement() {
    auto value = parseExpression();
    consume(TokenType::SEMICOLON, "Expected ';' after turnback value");
    return std::make_unique<TurnbackStmtNode>(std::move(value), previous().line, previous().column);
}

std::unique_ptr<ASTNode> Parser::parseLoliDeclaration() {
    Token name = consume(TokenType::IDENTIFIER, "Expected loli name");
    consume(TokenType::LEFT_BRACE, "Expected '{' before loli body");
    
    std::vector<std::pair<std::string, TokenType>> members;
    while (!check(TokenType::RIGHT_BRACE) && !isAtEnd()) {
        if (!isTypeToken(current.type)) {
            throw error(current, "Expected type name");
        }
        Token type = advance();
        Token field = consume(TokenType::IDENTIFIER, "Expected field name");
        consume(TokenType::SEMICOLON, "Expected ';' after field declaration");
        members.push_back({field.value, type.type});
    }
    
    consume(TokenType::RIGHT_BRACE, "Expected '}' after loli body");
    return std::make_unique<LoliDeclNode>(name.value, std::move(members), previous().line, previous().column);
}

std::unique_ptr<ASTNode> Parser::parseIncludeStatement() {
    Token path = consume(TokenType::STRING, "Expected file path in quotes");
    consume(TokenType::SEMICOLON, "Expected ';' after include statement");
    return std::make_unique<IncludeStmtNode>(path.value, previous().line, previous().column);
}

std::unique_ptr<ASTNode> Parser::parseBlock() {
    auto block = std::make_unique<BlockNode>(previous().line, previous().column);
    
    while (!check(TokenType::RIGHT_BRACE) && !isAtEnd()) {
        auto stmt = parseStatement();
        if (stmt) {
            block->statements.push_back(std::move(stmt));
        }
    }
    
    consume(TokenType::RIGHT_BRACE, "Expected '}' after block");
    return block;
}

std::unique_ptr<ASTNode> Parser::parseVariableDeclaration() {
    Token type = previous();
    Token name = consume(TokenType::IDENTIFIER, "Expected variable name");
    
    std::unique_ptr<ExpressionNode> initializer = nullptr;
    if (match(TokenType::ASSIGN)) {
        initializer = parseExpression();
    }
    
    consume(TokenType::SEMICOLON, "Expected ';' after variable declaration");
    return std::make_unique<VariableDeclNode>(name.value, type.type, std::move(initializer), name.line, name.column);
} 