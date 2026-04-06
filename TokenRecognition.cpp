#include "TokenRecognition.h"

#include <regex>
#include <map>
#include <vector>
#include <stdexcept>

using namespace std;

/// <summary>
/// Шаблоны для распознавания токенов. 
/// <para/>Ключ(string) - тип токена
/// <para/>Значение(string)  - регулярное выражение для его распознавания.
/// </summary>
const map<TokenType, string> PATTERNS = {
	/*
	\\b					- границы слова
	(int|float|...)		- список слов, которые могут быть распознаны как ключевые слова
	*/
	{KEYWORD, "\\b(int|float|if|else|while|for|return)\\b"},

	/*
	\\b				- границы слова
	[a-zA-Z_]		- идентификатор должен начинаться с буквы или подчеркивания
	[a-zA-Z0-9_]*	- после первой буквы могут следовать буквы, цифры или подчеркивания
	*/
	{IDENTIFIER, "\\b[a-zA-Z_][a-zA-Z0-9_]*\\b"},

	/*
	\\b			- границы слова
	\\d+		- одна или более цифр
	(\\.\\d+)?	- необязательная десятичная часть, которая начинается с точки и за которой следуют одна или более 
	*/
	{NUMBER, "\\b\\d+(\\.\\d+)?\\b"},

	// [+\\-*/=]	- любой из перечисленных операторов
	{OPERATOR, "(==|!=|<=|>=|[+\\-*/=<>])"},

	// [(){};,]	- любой из перечисленных разделителей
	{DELIMITER, "[(){};,]"}
};

vector<Token> TokenRecognition::recognizeTokens(string code) {
	if (code.empty()) {
		throw invalid_argument("Input code cannot be empty.");
	}

	// тут будем хранить наши распознанные токены
	vector<Token> tokens;

	while (!code.empty()) {
		while (!code.empty() && isspace(code[0])) {
			code.erase(0, 1);
		}

		if (code.empty()) {
			break;
		}

		bool matched = false;

		/// перебираем все шаблоны токенов, пытается найти совпадение в текущей строке кода
		/// с помощью регулярного выражения и, при успешном совпадении, сохраняет найденный
		/// токен, удаляет обработанную часть строки и помечает, что токен был распознан.
		for (const auto& pattern : PATTERNS) {
			regex re(pattern.second); // регулярное выражение
			smatch match;			  // объект для хранения результатов поиска

			if (regex_search(code, match, re) && match.position(0) == 0) {
				tokens.push_back({ pattern.first, match.str(0) });
				code.erase(0, match.position(0) + match.length(0));
				matched = true;
				break;
			}
		}

		if (!matched)
		{
			throw runtime_error("Illegal character: " + string(1, code[0]));
		}
	}

	return tokens;
}


string TokenRecognition::toString(vector<Token> tokens) {
	string out;
	
	for (const auto& token : tokens)
	{
		string typeStr;
		switch (token.type) {
			case KEYWORD: typeStr = "KEYWORD"; break;
			case IDENTIFIER: typeStr = "IDENTIFIER"; break;
			case NUMBER: typeStr = "NUMBER"; break;
			case OPERATOR: typeStr = "OPERATOR"; break;
			case DELIMITER: typeStr = "DELIMITER"; break;
		}

		int padding = 12 - static_cast<int>(typeStr.length());
		if (padding < 1) padding = 1;

		out += "Token Type: " + typeStr + string(padding, ' ') + " Value: " + token.value + "\n";
	}

	return out;
}
