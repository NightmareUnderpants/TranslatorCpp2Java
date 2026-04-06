#include <iostream>
#include <map>

using namespace std;

/// <summary>
/// Типы токенов, которые могут быть распознаны в исходном коде.
/// <para/>KEYWORD - ключевые слова языка программирования (например, int, float, if, else и т.д.)
/// <para/>IDENTIFIER - идентификаторы (имена переменных, функций и т.д.)
/// <para/>NUMBER - числовые литералы (целые и вещественные числа)
/// <para/>OPERATOR - операторы (например, +, -, *, /, = и т.д.)
/// <para/>DELIMITER - разделители (например, скобки, точки с запятой и т.д.)
/// </summary>
const enum TokenType {
	KEYWORD,
	IDENTIFIER,
	NUMBER,
	OPERATOR,
	DELIMITER
};

/// <summary>
/// Шаблоны для распознавания токенов. 
/// <para/>Ключ(string) - тип токена
/// <para/>Значение(string)  - регулярное выражение для его распознавания.
/// </summary>
const map<TokenType, string> PATTERNS = {
	{KEYWORD, "\\b(int|float|if|else|while|for|return)\\b"},
	{IDENTIFIER, "\\b[a-zA-Z_][a-zA-Z0-9_]*\\b"},
	{NUMBER, "\\b\\d+(\\.\\d+)?\\b"},
	{OPERATOR, "[+\\-*/=]"},
	{DELIMITER, "[(){};,]"}
};

class TokenRecognition {
private:
	map<string, string> patterns;

public:
	void recognizeTokens(const string& code) {
		// Здесь будет логика для распознавания токенов на основе шаблонов
		// Например, можно использовать регулярные выражения для поиска совпадений
	}

};