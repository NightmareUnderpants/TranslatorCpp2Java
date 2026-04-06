#include <iostream>

#include "TokenRecognition.h"
#include <map>

using namespace std;

int main()
{
	string code = "int x = 10; if (x == 10) { return 1; }";

	auto tokens = TokenRecognition::recognizeTokens(code);
	
	cout << TokenRecognition::toString(tokens);
}
