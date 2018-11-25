#pragma once
#include <string>
#include <vector>

using namespace std;

class BeatMapLanInterpreter
{
public:
	//typedefs
	typedef void placeCube(double timestamp, int type, int value);

	struct extFuns {
		placeCube * p_cube;
	};

	extFuns externalFunctions;

	//Public Constants
	const string error_variable_not_found = "VARIABLE_NOT_FOUND";
	const string error_invalid_math_operator = "INVALID_OPERATOR";
	const string error_function_not_found = "FUNCTION_NOT_FOUND";
	const string no_return = "NO_RETURN";

	struct BmlVariable {
		string name;
		string value;
	};

	struct BmlFunction {
		string name;
		int pos;
	};

	//Functions
	BeatMapLanInterpreter(extFuns functions);
	~BeatMapLanInterpreter();

	void interpret(string code);
private:
	//Constants
	const char key_fun = '#';
	const char key_comment = '§';
	const char key_pattern = '!';
	const char key_pattern_seperator = ':';
	const char key_block_opening_bracket = '{';
	const char key_block_closing_bracket = '}';
	const char key_math_opening_bracket = '[';
	const char key_math_closing_bracket = ']';
	const char key_section_marker = '"';
	const char key_variable = '_';
	const char key_assignment = '=';
	const char key_request_return = '?';
	const char key_return_p1 = '<';
	const char key_return_p2 = '-';
	const char key_fun_args_opening_bracket = '(';
	const char key_fun_args_closing_bracket = ')';
	const char key_args_seperator = ',';

	const string fun_type_function = "fun";
	const string fun_type_main = "main";

	const string fun_place_cube = "Cube";

	//Global Vriables
	double currentOffset = 0.0;
	bool inverted = false;

	string bmlCode = "";
	int pos = 0;

	vector<BmlVariable> variables;
	vector<BmlFunction> functions;

	//Fuctions
	string pop_next(char escape_char);
	void gotoNext(char key);
	void jumpSpaces();
	vector<string> popArgs();
	void addOrAssignVar(string name, string value);
	string getVarVal(string name);
	string evalMathExpression();
	string executeFun(int m_pos, vector<string> args_names, vector<string> args);
	string callFunction(string name, vector<string> args);
};