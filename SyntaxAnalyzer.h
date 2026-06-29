#pragma once

#include "TokenRecognition.h"

#include <initializer_list>
#include <memory>
#include <string>
#include <vector>

using namespace std;

struct SyntaxNode {
    string name;
    string value;
    vector<shared_ptr<SyntaxNode>> children;

    explicit SyntaxNode(string nodeName, string nodeValue = "");
    void add(const shared_ptr<SyntaxNode>& child);
    string toString(int depth = 0) const;
};

class SyntaxAnalyzer {
public:
    explicit SyntaxAnalyzer(const vector<Token>& inputTokens);
    shared_ptr<SyntaxNode> parse();

private:
    const vector<Token>& tokens;
    size_t current;

    shared_ptr<SyntaxNode> parseProgram();
    shared_ptr<SyntaxNode> parseDeclaration();
    shared_ptr<SyntaxNode> parseFunctionDeclaration();
    shared_ptr<SyntaxNode> parseVariableDeclaration();

    shared_ptr<SyntaxNode> parseStatement();
    shared_ptr<SyntaxNode> parseBlock();
    shared_ptr<SyntaxNode> parseIfStatement();
    shared_ptr<SyntaxNode> parseWhileStatement();
    shared_ptr<SyntaxNode> parseForStatement();
    shared_ptr<SyntaxNode> parseReturnStatement();
    shared_ptr<SyntaxNode> parseExpressionStatement();

    shared_ptr<SyntaxNode> parseExpression();
    shared_ptr<SyntaxNode> parseAssignment();
    shared_ptr<SyntaxNode> parseEquality();
    shared_ptr<SyntaxNode> parseComparison();
    shared_ptr<SyntaxNode> parseTerm();
    shared_ptr<SyntaxNode> parseFactor();
    shared_ptr<SyntaxNode> parseUnary();
    shared_ptr<SyntaxNode> parseCall();
    shared_ptr<SyntaxNode> parsePrimary();
    shared_ptr<SyntaxNode> finishCall(const shared_ptr<SyntaxNode>& callee);

    bool isAtEnd() const;
    const Token& peek() const;
    const Token& previous() const;
    const Token& advance();

    bool checkValue(const string& value) const;
    bool checkType(TokenType type) const;
    bool matchValues(initializer_list<string> values);

    const Token& consumeValue(const string& value, const string& message);
    const Token& consumeType(TokenType type, const string& message);

    bool isTypeKeyword(size_t offset = 0) const;
    bool isFunctionDeclaration() const;

    [[noreturn]] void syntaxError(const string& message) const;
};
