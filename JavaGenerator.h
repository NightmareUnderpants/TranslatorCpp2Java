#pragma once

#include "SyntaxAnalyzer.h"

#include <memory>
#include <string>

using namespace std;

/// <summary>
/// Генератор Java-кода.
/// Обходит синтаксическое дерево и формирует исходный код класса Main.
/// </summary>
class JavaGenerator {
public:
    /// <summary>
    /// Генерирует Java-код по корневому узлу Program.
    /// </summary>
    static string generate(const shared_ptr<SyntaxNode>& root);

private:
    static string generateProgram(const shared_ptr<SyntaxNode>& node);
    static string generateTopLevel(const shared_ptr<SyntaxNode>& node, int depth);
    static string generateFunction(const shared_ptr<SyntaxNode>& node, int depth);
    static string generateVariable(
        const shared_ptr<SyntaxNode>& node,
        int depth,
        bool topLevel,
        bool withSemicolon = true
    );

    static string generateBlockContents(
        const shared_ptr<SyntaxNode>& block,
        int depth,
        bool insideMain
    );

    static string generateStatement(
        const shared_ptr<SyntaxNode>& node,
        int depth,
        bool insideMain
    );

    static string generateBody(
        const shared_ptr<SyntaxNode>& node,
        int depth,
        bool insideMain
    );

    static string generateExpression(const shared_ptr<SyntaxNode>& node);
    static string generateForPart(const shared_ptr<SyntaxNode>& wrapper);

    static shared_ptr<SyntaxNode> findChild(
        const shared_ptr<SyntaxNode>& node,
        const string& childName
    );

    static string mapType(const string& cppType);
    static string indent(int depth);
};
