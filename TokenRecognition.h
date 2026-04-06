#pragma once

#include <map>
#include <vector>
#include <string>

using namespace std;

/// <summary>
/// Типы токенов, которые могут быть распознаны в исходном коде.
/// <para/>KEYWORD - ключевые слова языка программирования (например, int, float, if, else и т.д.)
/// <para/>IDENTIFIER - идентификаторы (имена переменных, функций и т.д.)
/// <para/>NUMBER - числовые литералы (целые и вещественные числа)
/// <para/>OPERATOR - операторы (например, +, -, *, /, = и т.д.)
/// <para/>DELIMITER - разделители (например, скобки, точки с запятой и т.д.)
/// </summary>
enum TokenType {
	KEYWORD,
	IDENTIFIER,
	NUMBER,
	OPERATOR,
	DELIMITER
};

struct Token {
	TokenType type;
	string value;
};

/// <summary>
/// Класс для лексического анализа исходного кода.
/// <para/>Распознаёт токены (ключевые слова, идентификаторы, числа, операторы, разделители)
/// с помощью регулярных выражений и возвращает карту найденных токенов.
/// </summary>
class TokenRecognition {
	public:

		/// <summary>
		/// Распознаёт токены в исходном коде.
		/// </summary>
		/// <param name="code">Строка с исходным кодом для анализа</param>
		/// <returns>Карта, где ключ - тип токена (TokenType), значение - распознанная строка токена</returns>
		/// <exception cref="invalid_argument">Если входная строка пуста</exception>
		/// <exception cref="runtime_error">Если обнаружен нераспознанный символ</exception>
		static vector<Token> recognizeTokens(string code);

		/// <summary>
		/// Выводит распознанные токены в удобочитаемом формате.
		/// </summary>
		/// <param name="tokens">Вектор токенов</param>
		/// <returns>Строка содержащая все токены</returns>
		static string toString(vector<Token> tokens);
};