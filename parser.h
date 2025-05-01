#pragma once
#include "scanner.h"
#include <memory>
#include <vector>
#include <string>
#include <stdexcept>

// Parser Error class
class ParserError : public std::runtime_error {
public:
    explicit ParserError(const std::string& message) : std::runtime_error(message) {}
};

// AST Node Types
enum class NodeType {
    PROGRAM,
    FUNCTION_DECL,
    VARIABLE_DECL,
    IF_TRUE_STMT,
    OTHERWISE_STMT,
    REPEATWHEN_STMT,
    REITERATE_STMT,
    TURNBACK_STMT,
    OUTLOOP_STMT,
    LOLI_DECL,
    BLOCK_STMT,
    EXPRESSION,
    BINARY_EXPR,
    UNARY_EXPR,
    LITERAL,
    IDENTIFIER,
    INCLUDE_STMT
};

// Base AST Node class
class ASTNode {
public:
    NodeType type;
    int line;
    int column;
    
    ASTNode(NodeType t, int l, int c) : type(t), line(l), column(c) {}
    virtual ~ASTNode() = default;
};

// Expression Node
class ExpressionNode : public ASTNode {
public:
    ExpressionNode(NodeType t, int l, int c) : ASTNode(t, l, c) {}
};

// Binary Expression Node
class BinaryExprNode : public ExpressionNode {
public:
    TokenType op;
    std::unique_ptr<ExpressionNode> left;
    std::unique_ptr<ExpressionNode> right;
    
    BinaryExprNode(TokenType o, std::unique_ptr<ExpressionNode> l, 
                   std::unique_ptr<ExpressionNode> r, int line, int col)
        : ExpressionNode(NodeType::BINARY_EXPR, line, col),
          op(o), left(std::move(l)), right(std::move(r)) {}
};

// Unary Expression Node
class UnaryExprNode : public ExpressionNode {
public:
    TokenType op;
    std::unique_ptr<ExpressionNode> operand;
    
    UnaryExprNode(TokenType o, std::unique_ptr<ExpressionNode> expr, int line, int col)
        : ExpressionNode(NodeType::UNARY_EXPR, line, col),
          op(o), operand(std::move(expr)) {}
};

// Literal Node
class LiteralNode : public ExpressionNode {
public:
    std::string value;
    TokenType literalType;
    
    LiteralNode(const std::string& v, TokenType t, int line, int col)
        : ExpressionNode(NodeType::LITERAL, line, col),
          value(v), literalType(t) {}
};

// Identifier Node
class IdentifierNode : public ExpressionNode {
public:
    std::string name;
    
    IdentifierNode(const std::string& n, int line, int col)
        : ExpressionNode(NodeType::IDENTIFIER, line, col),
          name(n) {}
};

// Function Declaration Node
class FunctionDeclNode : public ASTNode {
public:
    std::string name;
    TokenType returnType;
    std::vector<std::pair<std::string, TokenType>> parameters;
    std::unique_ptr<ASTNode> body;
    
    FunctionDeclNode(const std::string& n, TokenType rt, 
                    std::vector<std::pair<std::string, TokenType>> p,
                    std::unique_ptr<ASTNode> b, int line, int col)
        : ASTNode(NodeType::FUNCTION_DECL, line, col),
          name(n), returnType(rt), parameters(std::move(p)),
          body(std::move(b)) {}
};

// Variable Declaration Node
class VariableDeclNode : public ASTNode {
public:
    std::string name;
    TokenType varType;
    std::unique_ptr<ExpressionNode> initializer;
    
    VariableDeclNode(const std::string& n, TokenType t,
                    std::unique_ptr<ExpressionNode> i, int line, int col)
        : ASTNode(NodeType::VARIABLE_DECL, line, col),
          name(n), varType(t), initializer(std::move(i)) {}
};

// Block Node
class BlockNode : public ASTNode {
public:
    std::vector<std::unique_ptr<ASTNode>> statements;
    
    BlockNode(int line, int col)
        : ASTNode(NodeType::BLOCK_STMT, line, col) {}
};

// IfTrue Statement Node
class IfTrueStmtNode : public ASTNode {
public:
    std::unique_ptr<ExpressionNode> condition;
    std::unique_ptr<ASTNode> thenBranch;
    std::unique_ptr<ASTNode> elseBranch;
    
    IfTrueStmtNode(std::unique_ptr<ExpressionNode> c,
                  std::unique_ptr<ASTNode> t,
                  std::unique_ptr<ASTNode> e,
                  int line, int col)
        : ASTNode(NodeType::IF_TRUE_STMT, line, col),
          condition(std::move(c)),
          thenBranch(std::move(t)),
          elseBranch(std::move(e)) {}
};

// Otherwise Statement Node
class OtherwiseStmtNode : public ASTNode {
public:
    std::unique_ptr<ASTNode> body;
    
    OtherwiseStmtNode(std::unique_ptr<ASTNode> b, int line, int col)
        : ASTNode(NodeType::OTHERWISE_STMT, line, col),
          body(std::move(b)) {}
};

// RepeatWhen Statement Node
class RepeatWhenStmtNode : public ASTNode {
public:
    std::unique_ptr<ExpressionNode> condition;
    std::unique_ptr<ASTNode> body;
    
    RepeatWhenStmtNode(std::unique_ptr<ExpressionNode> c,
                      std::unique_ptr<ASTNode> b,
                      int line, int col)
        : ASTNode(NodeType::REPEATWHEN_STMT, line, col),
          condition(std::move(c)),
          body(std::move(b)) {}
};

// Reiterate Statement Node
class ReiterateStmtNode : public ASTNode {
public:
    std::unique_ptr<ExpressionNode> condition;
    std::unique_ptr<ASTNode> body;
    
    ReiterateStmtNode(std::unique_ptr<ExpressionNode> c,
                     std::unique_ptr<ASTNode> b,
                     int line, int col)
        : ASTNode(NodeType::REITERATE_STMT, line, col),
          condition(std::move(c)),
          body(std::move(b)) {}
};

// Turnback Statement Node
class TurnbackStmtNode : public ASTNode {
public:
    std::unique_ptr<ExpressionNode> value;
    
    TurnbackStmtNode(std::unique_ptr<ExpressionNode> v, int line, int col)
        : ASTNode(NodeType::TURNBACK_STMT, line, col),
          value(std::move(v)) {}
};

// OutLoop Statement Node
class OutLoopStmtNode : public ASTNode {
public:
    OutLoopStmtNode(int line, int col)
        : ASTNode(NodeType::OUTLOOP_STMT, line, col) {}
};

// Loli (Struct) Declaration Node
class LoliDeclNode : public ASTNode {
public:
    std::string name;
    std::vector<std::pair<std::string, TokenType>> members;
    
    LoliDeclNode(const std::string& n,
                std::vector<std::pair<std::string, TokenType>> m,
                int line, int col)
        : ASTNode(NodeType::LOLI_DECL, line, col),
          name(n), members(std::move(m)) {}
};

// Include Statement Node
class IncludeStmtNode : public ASTNode {
public:
    std::string path;
    
    IncludeStmtNode(const std::string& p, int line, int col)
        : ASTNode(NodeType::INCLUDE_STMT, line, col),
          path(p) {}
};

// Variable Node
class VariableNode : public ExpressionNode {
public:
    Token name;
    
    VariableNode(Token n)
        : ExpressionNode(NodeType::IDENTIFIER, n.line, n.column),
          name(n) {}
};

// Grouping Node
class GroupingNode : public ExpressionNode {
public:
    std::unique_ptr<ExpressionNode> expression;
    
    GroupingNode(std::unique_ptr<ExpressionNode> expr, int line, int col)
        : ExpressionNode(NodeType::EXPRESSION, line, col),
          expression(std::move(expr)) {}
};

// Parser Class
class Parser {
private:
    Scanner scanner;
    Token currentToken;
    std::vector<std::string> errors;
    int errorCount;
    
    // Helper methods
    void advance();
    void match(TokenType type);
    void error(const std::string& message);
    bool isTypeSpecifier(TokenType type);
    bool isRelop(TokenType type);
    
    // Grammar rule parsing methods
    void program();
    void declarationList();
    void declaration();
    void varDeclaration();
    void funDeclaration();
    void typeSpecifier();
    void params();
    void paramList();
    void param();
    void compoundStmt();
    void localDeclarations();
    void statementList();
    void statement();
    void expressionStmt();
    void selectionStmt();
    void iterationStmt();
    void jumpStmt();
    void expression();
    void idAssign();
    void simpleExpression();
    void additiveExpression();
    void term();
    void factor();
    void call();
    void args();
    void argList();
    void num();
    void comment();
    void includeCommand();
    
public:
    explicit Parser(const std::string& source);
    void parse();
    bool hasError() const;
    const std::vector<std::string>& getErrors() const;
    int getErrorCount() const;
}; 