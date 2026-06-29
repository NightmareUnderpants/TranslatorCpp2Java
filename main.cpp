#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

#include "JavaGenerator.h"
#include "SyntaxAnalyzer.h"
#include "TokenRecognition.h"

using namespace std;

int main() {
    try {
        string code =
            "int square(int n) { "
            "    return n * n; "
            "} "
            "int main() { "
            "    int x = 10; "
            "    if (x == 10) { "
            "        return square(x); "
            "    } else { "
            "        return 0; "
            "    } "
            "}";

        // ЭТАП 1. ЛЕКСИЧЕСКИЙ АНАЛИЗ
        vector<Token> tokens = TokenRecognition::recognizeTokens(code);

        cout << "TOKENS\n";
        cout << "--------------------------------\n";
        cout << TokenRecognition::toString(tokens) << "\n";

        // ЭТАП 2. СИНТАКСИЧЕСКИЙ АНАЛИЗ
        SyntaxAnalyzer parser(tokens);
        auto tree = parser.parse();

        cout << "SYNTAX TREE\n";
        cout << "--------------------------------\n";
        cout << tree->toString() << "\n";

        // ЭТАП 3. ГЕНЕРАЦИЯ JAVA-КОДА
        string javaCode = JavaGenerator::generate(tree);

        cout << "GENERATED JAVA CODE\n";
        cout << "--------------------------------\n";
        cout << javaCode << "\n";

        ofstream outputFile("Main.java");

        if (!outputFile.is_open()) {
            throw runtime_error("Cannot create Main.java.");
        }

        outputFile << javaCode;
        outputFile.close();

        cout << "Java code was saved to Main.java\n";
    }
    catch (const exception& error) {
        cerr << error.what() << '\n';
        return 1;
    }

    return 0;
}
