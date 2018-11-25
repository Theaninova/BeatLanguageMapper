#include "BeatMapLanInterpreter.h"

BeatMapLanInterpreter::BeatMapLanInterpreter(extFuns functions)
{
	externalFunctions = functions;
}


BeatMapLanInterpreter::~BeatMapLanInterpreter()
{
}

void BeatMapLanInterpreter::interpret(string code)
{
	pos = 0;
	bmlCode = code;

	while (pos < bmlCode.length)
	{
		if (bmlCode[pos] == key_fun)
		{
			pos++;

			string fun_type = pop_next(' ');

			jumpSpaces();

			if (fun_type.compare(fun_type_function))
			{
				string function_name = pop_next(' ');
				functions.push_back({ function_name, pos });
				gotoNext(key_block_closing_bracket);
			}
			else if (fun_type.compare(fun_type_main))
			{
				executeFun(pos, vector<string>(), vector<string>());
				return;
			}
		}
		else if (code[pos] == key_comment)
		{
			pos++;
			gotoNext(key_comment);
		}
		else
		{
			pos++;
		}
	}
}

string BeatMapLanInterpreter::pop_next(char escape_char)
{
	string builder;
	bool variable = false;

	if (bmlCode[pos] == key_variable) {
		pos++;
		variable = true;
	}
	else if (bmlCode[pos] == key_math_opening_bracket) {
		pos++;
		return evalMathExpression();
	}
	else if (bmlCode[pos] == key_request_return) {
		pos++;
		string fun_name = pop_next(' ');
		gotoNext(key_block_opening_bracket);
		return callFunction(fun_name, popArgs());
	}

	while (bmlCode[pos] != escape_char)
	{
		if (bmlCode[pos] == key_section_marker)
		{
			while (bmlCode[pos] != key_section_marker)
			{
				builder.push_back(bmlCode[pos]);
				pos++;
			}
			pos++;
		}
		else
		{
			builder.push_back(bmlCode[pos]);
			pos++;
		}
	}
	pos++;

	if (variable)
	{
		return getVarVal(builder);
	}
	else
	{
		return builder;
	}
}

void BeatMapLanInterpreter::gotoNext(char key)
{
	while (bmlCode[pos] != key) pos++;
	pos++;
}

void BeatMapLanInterpreter::jumpSpaces()
{
	while (bmlCode[pos] == ' ') pos++;
}

vector<string> BeatMapLanInterpreter::popArgs()
{
	vector<string> argsList;

	while (bmlCode[pos] != key_fun_args_closing_bracket)
	{
		jumpSpaces();
		argsList.push_back(pop_next(key_args_seperator));
	}
	pos++;

	return argsList;
}

void BeatMapLanInterpreter::addOrAssignVar(string name, string value)
{
	for (BmlVariable &variable : variables)
	{
		if (name.compare(variable.name))
		{
			variable.value = value;
			return;
		}
	}

	variables.push_back({ name, value });
}

string BeatMapLanInterpreter::getVarVal(string name)
{
	for (BmlVariable &var : variables)
	{
		if (var.name == name)
		{
			return var.value;
		}
	}

	return error_variable_not_found;
}

string BeatMapLanInterpreter::evalMathExpression()
{
	jumpSpaces();
	string varOne = pop_next(' ');
	jumpSpaces();
	string operation = pop_next(' ');
	jumpSpaces();
	string varTwo = pop_next(' ');

	gotoNext(key_math_closing_bracket);

	if (operation.compare("+"))
	{
		return to_string(atof(varOne.c_str()) + atof(varTwo.c_str()));
	}
	else if (operation.compare("-"))
	{
		return to_string(atof(varOne.c_str()) - atof(varTwo.c_str()));
	}
	else if (operation.compare("*"))
	{
		return to_string(atof(varOne.c_str()) * atof(varTwo.c_str()));
	}
	else if (operation.compare("/"))
	{
		return to_string(atof(varOne.c_str()) / atof(varTwo.c_str()));
	}
	else
	{
		return error_invalid_math_operator;
	}
}

string BeatMapLanInterpreter::executeFun(int m_pos, vector<string> args_names, vector<string> args)
{
	int old_pos = pos;
	pos = m_pos;

	gotoNext(key_block_opening_bracket);
	jumpSpaces();

	while (bmlCode[pos] != key_block_closing_bracket)
	{
		if (bmlCode[pos] == key_comment)
		{
			pos++;
			gotoNext(key_comment);
		}
		else if (bmlCode[pos] == key_pattern)
		{
			pos++;

			string timestamp = pop_next(key_pattern_seperator);
			string t_inverted = pop_next(key_pattern_seperator);
			int runs = atof(pop_next(' ').c_str());

			double old_offset = currentOffset;
			currentOffset += atof(timestamp.c_str());

			bool old_inverted = inverted;
			inverted = t_inverted.compare("true"); //Converting from string to bool

			for (int i = 0; i < runs; i++)
			{
				currentOffset += atof(executeFun(pos, vector<string>{ "p_timestamp", "p_inverted", "p_total_runs", "p_current_run_index" }, vector<string>{ timestamp, t_inverted, to_string(runs), to_string(i)}).c_str());
			}

			gotoNext(key_block_closing_bracket);

			currentOffset = old_offset;
			inverted = old_inverted;
		}
		else if (bmlCode[pos] == key_return_p1 && bmlCode[pos + 1] == key_return_p2)
		{
			pos += 2;
			jumpSpaces();
			string s_out = pop_next(' ');
			pos = old_pos;
			return s_out;
		}
		else if (bmlCode[pos] == key_variable)
		{
			pos++;
			string var_name = pop_next(' ');
			jumpSpaces();
			if (bmlCode[pos] == key_assignment)
			{
				pos++;
				jumpSpaces();
				addOrAssignVar(var_name, pop_next(' '));
			}
		}
		else
		{
			string fun_call = pop_next(' ');
			gotoNext(key_fun_args_opening_bracket);
			callFunction(fun_call, popArgs());
		}

		jumpSpaces();
	}

	pos = old_pos;

	return no_return;
}

string BeatMapLanInterpreter::callFunction(string name, vector<string> args)
{
	if (name == fun_place_cube)
	{
		externalFunctions.p_cube(atof(args[0].c_str()), atoi(args[1].c_str()), atoi(args[2].c_str()));
		return no_return;
	}
	else
	{
		//Now we have to check for custom functions

		for (BmlFunction &fun : functions)
		{
			if (fun.name.compare(name))
			{
				int old_pos = pos;
				pos = fun.pos;

				vector<string> arg_names = popArgs();

				pos = old_pos;

				return executeFun(fun.pos, arg_names, args);
			}
		}
	}

	return error_function_not_found;
}
