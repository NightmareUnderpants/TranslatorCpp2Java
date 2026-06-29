#include "SyntaxAnalyzer.h"

#include <sstream>
#include <stdexcept>
#include <utility>

using namespace std;

SyntaxNode::SyntaxNode(string nodeName, string nodeValue)
    : name(std::move(nodeName)), value(std::move(nodeValue)) {
}

void SyntaxNode::add(const shared_ptr<SyntaxNode>& child) {
    if (child != nullptr) {
        children.push_back(child);
    }
}

string SyntaxNode::toString(int depth) const {
    string result(depth * 2, ' ');
    result += name;

    if (!value.empty()) {
        result += ": " + value;
    }

    result += "\n";

    for (const auto& child : children) {
        result += child->toString(depth + 1);
    }

    return result;
}

SyntaxAnalyzer::SyntaxAnalyzer(const vector<Token>& inputTokens)
    : tokens(inputTokens), current(0) {
}

shared_ptr<SyntaxNode> SyntaxAnalyzer::parse() {
    if (tokens.empty()) {
        throw invalid_argument("Syntax analyzer received an empty token list.");
    }

    return parseProgram();
}

// <program> -> <declaration>* EOF
shared_ptr<SyntaxNode> SyntaxAnalyzer::parseProgram() {
    auto node = make_shared<SyntaxNode>("Program");

    while (!isAtEnd()) {
        node->add(parseDeclaration());
    }

    return node;
}

shared_ptr<SyntaxNode> SyntaxAnalyzer::parseDeclaration() {
    if (isFunctionDeclaration()) {
        return parseFunctionDeclaration();
    }

    if (isTypeKeyword()) {
        return parseVariableDeclaration();
    }

    return parseStatement();
}

// <function> -> <type> id "(" <parameters>? ")" <block>
shared_ptr<SyntaxNode> SyntaxAnalyzer::parseFunctionDeclaration() {
    const Token& typeToken = advance();
    const Token& nameToken = consumeType(
        IDENTIFIER,
        "after the function type, a function name was expected"
    );

    auto node = make_shared<SyntaxNode>("FunctionDeclaration", nameToken.value);
    node->add(make_shared<SyntaxNode>("ReturnType", typeToken.value));

    consumeValue("(", "after the function name, '(' was expected");

    auto parameters = make_shared<SyntaxNode>("Parameters");

    if (!checkValue(")")) {
        do {
            if (!isTypeKeyword()) {
                syntaxError("a parameter type was expected");
            }

            const Token& parameterType = advance();
            const Token& parameterName = consumeType(
                IDENTIFIER,
                "after the parameter type, a parameter name was expected"
            );

            auto parameter = make_shared<SyntaxNode>("Parameter", parameterName.value);
            parameter->add(make_shared<SyntaxNode>("Type", parameterType.value));
            parameters->add(parameter);
        } while (matchValues({","}));
    }

    consumeValue(")", "after function parameters, ')' was expected");
    node->add(parameters);

    consumeValue("{", "after the function header, '{' was expected");
    node->add(parseBlock());

    return node;
}

// <variable> -> <type> id ("=" <expression>)? ";"
shared_ptr<SyntaxNode> SyntaxAnalyzer::parseVariableDeclaration() {
    const Token& typeToken = advance();
    const Token& nameToken = consumeType(
        IDENTIFIER,
        "after the data type, a variable name was expected"
    );

    auto node = make_shared<SyntaxNode>("VariableDeclaration", nameToken.value);
    node->add(make_shared<SyntaxNode>("Type", typeToken.value));

    if (matchValues({"="})) {
        auto initializer = make_shared<SyntaxNode>("Initializer");
        initializer->add(parseExpression());
        node->add(initializer);
    }

    consumeValue(";", "after the variable declaration, ';' was expected");
    return node;
}

shared_ptr<SyntaxNode> SyntaxAnalyzer::parseStatement() {
    if (matchValues({"{"})) return parseBlock();
    if (matchValues({"if"})) return parseIfStatement();
    if (matchValues({"while"})) return parseWhileStatement();
    if (matchValues({"for"})) return parseForStatement();
    if (matchValues({"return"})) return parseReturnStatement();

    return parseExpressionStatement();
}

// "{" уже считана вызывающим методом.
shared_ptr<SyntaxNode> SyntaxAnalyzer::parseBlock() {
    auto node = make_shared<SyntaxNode>("Block");

    while (!checkValue("}") && !isAtEnd()) {
        node->add(parseDeclaration());
    }

    consumeValue("}", "at the end of the block, '}' was expected");
    return node;
}

shared_ptr<SyntaxNode> SyntaxAnalyzer::parseIfStatement() {
    auto node = make_shared<SyntaxNode>("IfStatement");

    consumeValue("(", "after if, '(' was expected");

    auto condition = make_shared<SyntaxNode>("Condition");
    condition->add(parseExpression());
    node->add(condition);

    consumeValue(")", "after the if condition, ')' was expected");

    auto thenBranch = make_shared<SyntaxNode>("Then");
    thenBranch->add(parseStatement());
    node->add(thenBranch);

    if (matchValues({"else"})) {
        auto elseBranch = make_shared<SyntaxNode>("Else");
        elseBranch->add(parseStatement());
        node->add(elseBranch);
    }

    return node;
}

shared_ptr<SyntaxNode> SyntaxAnalyzer::parseWhileStatement() {
    auto node = make_shared<SyntaxNode>("WhileStatement");

    consumeValue("(", "after while, '(' was expected");

    auto condition = make_shared<SyntaxNode>("Condition");
    condition->add(parseExpression());
    node->add(condition);

    consumeValue(")", "after the while condition, ')' was expected");

    auto body = make_shared<SyntaxNode>("Body");
    body->add(parseStatement());
    node->add(body);

    return node;
}

shared_ptr<SyntaxNode> SyntaxAnalyzer::parseForStatement() {
    auto node = make_shared<SyntaxNode>("ForStatement");

    consumeValue("(", "after for, '(' was expected");

    auto initializer = make_shared<SyntaxNode>("Initializer");

    if (matchValues({";"})) {
        initializer->add(make_shared<SyntaxNode>("Empty"));
    } else if (isTypeKeyword()) {
        initializer->add(parseVariableDeclaration());
    } else {
        initializer->add(parseExpression());
        consumeValue(";", "after the for initializer, ';' was expected");
    }

    node->add(initializer);

    auto condition = make_shared<SyntaxNode>("Condition");

    if (!checkValue(";")) {
        condition->add(parseExpression());
    } else {
        condition->add(make_shared<SyntaxNode>("Empty"));
    }

    consumeValue(";", "after the for condition, ';' was expected");
    node->add(condition);

    auto increment = make_shared<SyntaxNode>("Increment");

    if (!checkValue(")")) {
        increment->add(parseExpression());
    } else {
        increment->add(make_shared<SyntaxNode>("Empty"));
    }

    consumeValue(")", "after the for header, ')' was expected");
    node->add(increment);

    auto body = make_shared<SyntaxNode>("Body");
    body->add(parseStatement());
    node->add(body);

    return node;
}

shared_ptr<SyntaxNode> SyntaxAnalyzer::parseReturnStatement() {
    auto node = make_shared<SyntaxNode>("ReturnStatement");

    if (!checkValue(";")) {
        node->add(parseExpression());
    }

    consumeValue(";", "after return, ';' was expected");
    return node;
}

shared_ptr<SyntaxNode> SyntaxAnalyzer::parseExpressionStatement() {
    auto node = make_shared<SyntaxNode>("ExpressionStatement");
    node->add(parseExpression());
    consumeValue(";", "after the expression, ';' was expected");
    return node;
}

shared_ptr<SyntaxNode> SyntaxAnalyzer::parseExpression() {
    return parseAssignment();
}

shared_ptr<SyntaxNode> SyntaxAnalyzer::parseAssignment() {
    auto left = parseEquality();

    if (matchValues({"="})) {
        const string operation = previous().value;
        auto right = parseAssignment();

        if (left->name != "Identifier") {
            syntaxError("the left side of assignment must be an identifier");
        }

        auto node = make_shared<SyntaxNode>("AssignmentExpression", operation);
        node->add(left);
        node->add(right);
        return node;
    }

    return left;
}

shared_ptr<SyntaxNode> SyntaxAnalyzer::parseEquality() {
    auto expression = parseComparison();

    while (matchValues({"==", "!="})) {
        const string operation = previous().value;
        auto right = parseComparison();

        auto node = make_shared<SyntaxNode>("BinaryExpression", operation);
        node->add(expression);
        node->add(right);
        expression = node;
    }

    return expression;
}

shared_ptr<SyntaxNode> SyntaxAnalyzer::parseComparison() {
    auto expression = parseTerm();

    while (matchValues({">", ">=", "<", "<="})) {
        const string operation = previous().value;
        auto right = parseTerm();

        auto node = make_shared<SyntaxNode>("BinaryExpression", operation);
        node->add(expression);
        node->add(right);
        expression = node;
    }

    return expression;
}

shared_ptr<SyntaxNode> SyntaxAnalyzer::parseTerm() {
    auto expression = parseFactor();

    while (matchValues({"+", "-"})) {
        const string operation = previous().value;
        auto right = parseFactor();

        auto node = make_shared<SyntaxNode>("BinaryExpression", operation);
        node->add(expression);
        node->add(right);
        expression = node;
    }

    return expression;
}

shared_ptr<SyntaxNode> SyntaxAnalyzer::parseFactor() {
    auto expression = parseUnary();

    while (matchValues({"*", "/"})) {
        const string operation = previous().value;
        auto right = parseUnary();

        auto node = make_shared<SyntaxNode>("BinaryExpression", operation);
        node->add(expression);
        node->add(right);
        expression = node;
    }

    return expression;
}

shared_ptr<SyntaxNode> SyntaxAnalyzer::parseUnary() {
    if (matchValues({"+", "-"})) {
        const string operation = previous().value;
        auto node = make_shared<SyntaxNode>("UnaryExpression", operation);
        node->add(parseUnary());
        return node;
    }

    return parseCall();
}

shared_ptr<SyntaxNode> SyntaxAnalyzer::parseCall() {
    auto expression = parsePrimary();

    while (matchValues({"("})) {
        expression = finishCall(expression);
    }

    return expression;
}

shared_ptr<SyntaxNode> SyntaxAnalyzer::finishCall(
    const shared_ptr<SyntaxNode>& callee
) {
    auto node = make_shared<SyntaxNode>("FunctionCall");
    node->add(callee);

    auto arguments = make_shared<SyntaxNode>("Arguments");

    if (!checkValue(")")) {
        do {
            arguments->add(parseExpression());
        } while (matchValues({","}));
    }

    consumeValue(")", "after function arguments, ')' was expected");
    node->add(arguments);
    return node;
}

shared_ptr<SyntaxNode> SyntaxAnalyzer::parsePrimary() {
    if (checkType(NUMBER)) {
        const Token& token = advance();
        return make_shared<SyntaxNode>("NumberLiteral", token.value);
    }

    if (checkType(IDENTIFIER)) {
        const Token& token = advance();
        return make_shared<SyntaxNode>("Identifier", token.value);
    }

    if (matchValues({"("})) {
        auto node = make_shared<SyntaxNode>("GroupedExpression");
        node->add(parseExpression());
        consumeValue(")", "after the grouped expression, ')' was expected");
        return node;
    }

    syntaxError("a number, identifier, or parenthesized expression was expected");
}

bool SyntaxAnalyzer::isAtEnd() const {
    return current >= tokens.size();
}

const Token& SyntaxAnalyzer::peek() const {
    if (isAtEnd()) {
        throw out_of_range("Attempt to read a token after end of input.");
    }
    return tokens[current];
}

const Token& SyntaxAnalyzer::previous() const {
    return tokens[current - 1];
}

const Token& SyntaxAnalyzer::advance() {
    if (!isAtEnd()) current++;
    return previous();
}

bool SyntaxAnalyzer::checkValue(const string& value) const {
    return !isAtEnd() && peek().value == value;
}

bool SyntaxAnalyzer::checkType(TokenType type) const {
    return !isAtEnd() && peek().type == type;
}

bool SyntaxAnalyzer::matchValues(initializer_list<string> values) {
    for (const auto& value : values) {
        if (checkValue(value)) {
            advance();
            return true;
        }
    }
    return false;
}

const Token& SyntaxAnalyzer::consumeValue(
    const string& value,
    const string& message
) {
    if (checkValue(value)) return advance();
    syntaxError(message);
}

const Token& SyntaxAnalyzer::consumeType(
    TokenType type,
    const string& message
) {
    if (checkType(type)) return advance();
    syntaxError(message);
}

bool SyntaxAnalyzer::isTypeKeyword(size_t offset) const {
    if (current + offset >= tokens.size()) return false;

    const Token& token = tokens[current + offset];
    return token.type == KEYWORD &&
           (token.value == "int" || token.value == "float");
}

bool SyntaxAnalyzer::isFunctionDeclaration() const {
    if (!isTypeKeyword()) return false;
    if (current + 2 >= tokens.size()) return false;

    return tokens[current + 1].type == IDENTIFIER &&
           tokens[current + 2].value == "(";
}

[[noreturn]] void SyntaxAnalyzer::syntaxError(const string& message) const {
    ostringstream out;
    out << "Syntax error at token " << current;

    if (isAtEnd()) {
        out << " near end of input";
    } else {
        out << " near '" << peek().value << "'";
    }

    out << ": " << message;
    throw runtime_error(out.str());
}
