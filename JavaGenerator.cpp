#include "JavaGenerator.h"

#include <stdexcept>

using namespace std;

string JavaGenerator::generate(const shared_ptr<SyntaxNode>& root) {
    if (root == nullptr) {
        throw invalid_argument("The syntax tree root cannot be null.");
    }

    if (root->name != "Program") {
        throw invalid_argument("The root node must have the name Program.");
    }

    return generateProgram(root);
}

string JavaGenerator::generateProgram(
    const shared_ptr<SyntaxNode>& node
) {
    string out;
    out += "public class Main {\n";

    for (size_t i = 0; i < node->children.size(); i++) {
        out += generateTopLevel(node->children[i], 1);

        if (i + 1 < node->children.size()) {
            out += "\n";
        }
    }

    out += "}\n";
    return out;
}

string JavaGenerator::generateTopLevel(
    const shared_ptr<SyntaxNode>& node,
    int depth
) {
    if (node->name == "FunctionDeclaration") {
        return generateFunction(node, depth);
    }

    if (node->name == "VariableDeclaration") {
        return generateVariable(node, depth, true);
    }

    throw runtime_error(
        "Unsupported top-level syntax node: " + node->name
    );
}

string JavaGenerator::generateFunction(
    const shared_ptr<SyntaxNode>& node,
    int depth
) {
    auto returnType = findChild(node, "ReturnType");
    auto parameters = findChild(node, "Parameters");
    auto block = findChild(node, "Block");

    if (returnType == nullptr || parameters == nullptr || block == nullptr) {
        throw runtime_error(
            "The function node has an invalid structure: " + node->value
        );
    }

    const bool isMain = node->value == "main";

    string out = indent(depth);

    if (isMain) {
        out += "public static void main(String[] args) ";
    } else {
        out += "public static " + mapType(returnType->value) + " ";
        out += node->value + "(";

        for (size_t i = 0; i < parameters->children.size(); i++) {
            const auto& parameter = parameters->children[i];
            auto parameterType = findChild(parameter, "Type");

            if (parameterType == nullptr) {
                throw runtime_error(
                    "The parameter node has no Type child."
                );
            }

            if (i > 0) {
                out += ", ";
            }

            out += mapType(parameterType->value) + " " + parameter->value;
        }

        out += ") ";
    }

    out += "{\n";
    out += generateBlockContents(block, depth + 1, isMain);
    out += indent(depth) + "}\n";

    return out;
}

string JavaGenerator::generateVariable(
    const shared_ptr<SyntaxNode>& node,
    int depth,
    bool topLevel,
    bool withSemicolon
) {
    auto type = findChild(node, "Type");

    if (type == nullptr) {
        throw runtime_error(
            "The variable declaration has no Type child."
        );
    }

    string out = indent(depth);

    if (topLevel) {
        out += "public static ";
    }

    out += mapType(type->value) + " " + node->value;

    auto initializer = findChild(node, "Initializer");

    if (initializer != nullptr && !initializer->children.empty()) {
        out += " = ";
        out += generateExpression(initializer->children[0]);
    }

    if (withSemicolon) {
        out += ";";
    }

    out += "\n";
    return out;
}

string JavaGenerator::generateBlockContents(
    const shared_ptr<SyntaxNode>& block,
    int depth,
    bool insideMain
) {
    if (block == nullptr || block->name != "Block") {
        throw runtime_error("A Block node was expected.");
    }

    string out;

    for (const auto& child : block->children) {
        out += generateStatement(child, depth, insideMain);
    }

    return out;
}

string JavaGenerator::generateStatement(
    const shared_ptr<SyntaxNode>& node,
    int depth,
    bool insideMain
) {
    if (node->name == "VariableDeclaration") {
        return generateVariable(node, depth, false);
    }

    if (node->name == "ExpressionStatement") {
        if (node->children.empty()) {
            throw runtime_error("ExpressionStatement has no expression.");
        }

        return indent(depth) +
               generateExpression(node->children[0]) +
               ";\n";
    }

    if (node->name == "ReturnStatement") {
        if (insideMain) {
            if (node->children.empty()) {
                return indent(depth) + "return;\n";
            }

            // C++ main возвращает код завершения процесса.
            // В Java для этого используется System.exit(...).
            return indent(depth) +
                   "System.exit(" +
                   generateExpression(node->children[0]) +
                   ");\n";
        }

        string out = indent(depth) + "return";

        if (!node->children.empty()) {
            out += " " + generateExpression(node->children[0]);
        }

        out += ";\n";
        return out;
    }

    if (node->name == "IfStatement") {
        auto condition = findChild(node, "Condition");
        auto thenNode = findChild(node, "Then");
        auto elseNode = findChild(node, "Else");

        if (condition == nullptr || condition->children.empty() ||
            thenNode == nullptr || thenNode->children.empty()) {
            throw runtime_error("The IfStatement node has an invalid structure.");
        }

        string out = indent(depth) + "if (";
        out += generateExpression(condition->children[0]);
        out += ") ";
        out += generateBody(thenNode->children[0], depth, insideMain);

        if (elseNode != nullptr && !elseNode->children.empty()) {
            out.erase(out.size() - 1); // временно удаляем последний \n
            out += " else ";
            out += generateBody(elseNode->children[0], depth, insideMain);
        }

        return out;
    }

    if (node->name == "WhileStatement") {
        auto condition = findChild(node, "Condition");
        auto body = findChild(node, "Body");

        if (condition == nullptr || condition->children.empty() ||
            body == nullptr || body->children.empty()) {
            throw runtime_error("The WhileStatement node has an invalid structure.");
        }

        string out = indent(depth) + "while (";
        out += generateExpression(condition->children[0]);
        out += ") ";
        out += generateBody(body->children[0], depth, insideMain);
        return out;
    }

    if (node->name == "ForStatement") {
        auto initializer = findChild(node, "Initializer");
        auto condition = findChild(node, "Condition");
        auto increment = findChild(node, "Increment");
        auto body = findChild(node, "Body");

        if (initializer == nullptr || condition == nullptr ||
            increment == nullptr || body == nullptr ||
            body->children.empty()) {
            throw runtime_error("The ForStatement node has an invalid structure.");
        }

        string out = indent(depth) + "for (";
        out += generateForPart(initializer);
        out += "; ";
        out += generateForPart(condition);
        out += "; ";
        out += generateForPart(increment);
        out += ") ";
        out += generateBody(body->children[0], depth, insideMain);
        return out;
    }

    if (node->name == "Block") {
        return indent(depth) + generateBody(node, depth, insideMain);
    }

    throw runtime_error(
        "Unsupported statement syntax node: " + node->name
    );
}

string JavaGenerator::generateBody(
    const shared_ptr<SyntaxNode>& node,
    int depth,
    bool insideMain
) {
    string out = "{\n";

    if (node->name == "Block") {
        out += generateBlockContents(node, depth + 1, insideMain);
    } else {
        out += generateStatement(node, depth + 1, insideMain);
    }

    out += indent(depth) + "}\n";
    return out;
}

string JavaGenerator::generateExpression(
    const shared_ptr<SyntaxNode>& node
) {
    if (node == nullptr) {
        throw invalid_argument("Expression node cannot be null.");
    }

    if (node->name == "NumberLiteral" ||
        node->name == "Identifier") {
        return node->value;
    }

    if (node->name == "AssignmentExpression") {
        if (node->children.size() != 2) {
            throw runtime_error("AssignmentExpression must have two children.");
        }

        return generateExpression(node->children[0]) +
               " " + node->value + " " +
               generateExpression(node->children[1]);
    }

    if (node->name == "BinaryExpression") {
        if (node->children.size() != 2) {
            throw runtime_error("BinaryExpression must have two children.");
        }

        return "(" +
               generateExpression(node->children[0]) +
               " " + node->value + " " +
               generateExpression(node->children[1]) +
               ")";
    }

    if (node->name == "UnaryExpression") {
        if (node->children.size() != 1) {
            throw runtime_error("UnaryExpression must have one child.");
        }

        return node->value + generateExpression(node->children[0]);
    }

    if (node->name == "GroupedExpression") {
        if (node->children.size() != 1) {
            throw runtime_error("GroupedExpression must have one child.");
        }

        return "(" + generateExpression(node->children[0]) + ")";
    }

    if (node->name == "FunctionCall") {
        if (node->children.size() != 2) {
            throw runtime_error("FunctionCall must have two children.");
        }

        const auto& callee = node->children[0];
        const auto& arguments = node->children[1];

        string out = generateExpression(callee) + "(";

        for (size_t i = 0; i < arguments->children.size(); i++) {
            if (i > 0) {
                out += ", ";
            }

            out += generateExpression(arguments->children[i]);
        }

        out += ")";
        return out;
    }

    if (node->name == "Empty") {
        return "";
    }

    throw runtime_error(
        "Unsupported expression syntax node: " + node->name
    );
}

string JavaGenerator::generateForPart(
    const shared_ptr<SyntaxNode>& wrapper
) {
    if (wrapper == nullptr || wrapper->children.empty()) {
        return "";
    }

    const auto& child = wrapper->children[0];

    if (child->name == "Empty") {
        return "";
    }

    if (child->name == "VariableDeclaration") {
        string declaration = generateVariable(child, 0, false, false);

        while (!declaration.empty() &&
               (declaration.back() == '\n' || declaration.back() == '\r')) {
            declaration.pop_back();
        }

        return declaration;
    }

    return generateExpression(child);
}

shared_ptr<SyntaxNode> JavaGenerator::findChild(
    const shared_ptr<SyntaxNode>& node,
    const string& childName
) {
    if (node == nullptr) {
        return nullptr;
    }

    for (const auto& child : node->children) {
        if (child != nullptr && child->name == childName) {
            return child;
        }
    }

    return nullptr;
}

string JavaGenerator::mapType(const string& cppType) {
    if (cppType == "int") {
        return "int";
    }

    if (cppType == "float") {
        return "float";
    }

    throw runtime_error("Unsupported C++ type: " + cppType);
}

string JavaGenerator::indent(int depth) {
    return string(depth * 4, ' ');
}
