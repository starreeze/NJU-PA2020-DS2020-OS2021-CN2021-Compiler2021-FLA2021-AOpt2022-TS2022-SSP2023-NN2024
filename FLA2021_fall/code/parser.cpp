#include "turing_machine.h"

bool parse_args(int argc, char** argv, string& file, string& input) {
	bool verbose = false;
	if (argc == 1) {
		cout << help_output << endl;
		return -1;
	}
	for (int i = 1; i < argc;i++) {
		string arg = argv[i];
		if (arg[0] == '-') {
			if (arg == "-h" || arg == "--help") {
				cout << help_output << endl;
				break;
			}
			else if (arg == "-v" || arg == "--verbose")
				verbose = true;
			else {
				cerr << "unknown option:" << arg << endl;
				exit(10);
			}
		}
		else {
			if (file.empty())
				file = arg;
			else if (input.empty())
				input = arg;
			else {
				cerr << "unknown command:" << arg << endl;
				exit(10);
			}
		}
	}
	return verbose;
}

void Error::report(const vector<string>& info, ErrorType type) {
	switch (type) {
	case 1:
		cerr << "Failure opening file." << endl;
		if (verbose)
			cerr << error_line << endl
			<< "error: \""
			<< filename << "\" cannot open." << endl;
		break;
	case 2:
		cerr << "Syntax error." << endl;
		if (verbose) {
			cerr << error_line << endl
				<< "error: Incomplete definition format in file '" << filename << "' at line " << lineNo << endl
				<< "\t" << line << endl;
		}
		break;
	case 3:
		cerr << "Syntax error." << endl;
		if (verbose) {
			cerr << error_line << endl
				<< "error: Invalid charactor '" << info[0] << "' in file '" << filename << "' at line " << lineNo << endl
				<< "\t" << line << endl;
		}
		break;
	case 4:
		cerr << "Syntax error." << endl;
		if (verbose) {
			cerr << error_line << endl
				<< "error: Assignment failed because of invalid state: '" << info[0] << "' in file '" << filename << "' at line " << lineNo << endl
				<< "\t" << line << endl;
		}
		break;
	case 5:
		cerr << "Syntax error." << endl;
		if (verbose) {
			cerr << error_line << endl
				<< "error: Invalid " << info[1] << " character '" << info[0] << "' in file '" << filename
				<< "' at line " << lineNo << endl
				<< "\t" << line << endl;
		}
		break;
	case 6:
		cerr << "Syntax error." << endl;
		if (verbose) {
			cerr << error_line << endl
				<< "error: Invalid tape number '" << info[0] << "' in file '" << filename
				<< "' at line " << lineNo << endl
				<< "\t" << line << endl;
		}
		break;
	case 7:
		cerr << "Syntax error." << endl;
		if (verbose) {
			cerr << error_line << endl
				<< "error: Transition function definition error"
				<< " in file '" << filename
				<< "' at line " << lineNo << endl
				<< "\t" << line << endl;
		}
		break;
	case 8:
		cerr << "Syntax error." << endl;
		if (verbose) {
			cerr << error_line << endl
				<< "error: Incomplete line in file '" << filename << "' at line " << lineNo << endl
				<< "\t" << line << endl;
		}
		break;
	case 9:
		cerr << "Syntax error." << endl;
		if (verbose) {
			cerr << error_line << endl
				<< "error: Status match error."
				<< "Status '" << info[0] << "' does not match any trans function" << endl;
		}
		break;
	case 11:
		if (verbose) {
			cerr << error_line << endl
				<< "error: '" << info[0] << "' was not declared in the set of input symbols" << endl
				<< "Input: " << info[1] << endl
				<< info[2] << endl;
		}
		else
			cerr << "Input error." << endl;
		break;
	case 12:
		cerr << "Syntax error." << endl;
		if (verbose) {
			cerr << error_line << endl
				<< "error: " << info[0] << info[1] << "Undefined." << endl;
		}
	case 13:
		cerr << "Syntax error." << endl;
		if (verbose) {
			cerr << error_line << endl
				<< " in file '" << filename
				<< "' at line " << lineNo << endl
				<< "error: " << info[0] << info[1] << "' incorrect num of characters" << endl;
		}
		break;
	case 14:
		cerr << "Syntax error." << endl;
		if (verbose) {
			cerr << error_line << endl
				<< "error: tape can be only moved to l,r,*" << endl;
		}
		break;
	case DEF_UNREC:
		cerr << "Syntax error." << endl;
		if (verbose) {
			cerr << error_line << endl
				<< "error: " << info[0] << "is not definition" << endl;
		}
		break;
	default:
		break;
	}
	if (verbose)
		cerr << end_line << endl;
	exit(type);
}

Error* error;

// ******** some helper functions & micros for constructing tm  ********
// delete whitespace & comment
void extract_content(string& line) {
	int pos = line.find(';');
	if (pos != line.npos)
		line = line.erase(pos);
	const char ch = ' ';
	line.erase(line.find_last_not_of(ch) + 1);
	line.erase(0, line.find_first_not_of(ch));
}

bool ch_valid(char ch, char type) {
	switch (type) {
	case 'S':
		return ch >= 32 && ch <= 126 && ch != ' ' && ch != ',' && ch != ';' && ch != '{' && ch != '}' && ch != '*' && ch != '_';
	case 'G':
	case 'B':
		return ch >= 32 && ch <= 126 && ch != ' ' && ch != ',' && ch != ';' && ch != '{' && ch != '}' && ch != '*';
	case 'Q':
	case 'q':
	case 'F': //state
		if (ch >= 'a' && ch <= 'z')
			return true;
		else if (ch >= 'A' && ch <= 'Z')
			return true;
		else if (ch >= '0' && ch <= '9')
			return true;
		else if (ch == '_')
			return true;
		return false;
	default:
		return false;
	}
}

// check def sentense & return value
string check_def(const string& line) {
	// 'var = value'
	int first_space = line.find_first_of(' ');
	int last_space = line.find_last_of(' ');
	string variable = line.substr(1, first_space - 1);
	string equal = line.substr(first_space + 1, last_space - first_space - 1);
	string value = line.substr(last_space + 1);
	if (first_space == last_space) {
		error->report({}, INCOMPLETE_LINE);
	}
	if (Def.find(variable) == Def.end())
		error->report({ variable }, INVALID_CHAR);
	if (equal != "=")
		error->report({ equal }, INVALID_CHAR);
	return value;
}

void split_judge(const string& s, unordered_set<string>& words, char type) {
	string tmp;
	for (char c : s) {
		if (c == ',') {
			if (tmp.empty())
				error->report({ "," }, INVALID_CHAR);
			if (type == 'S' && tmp.length() > 1)
				error->report({ tmp, "Input symbol" }, NOT_CHAR);
			if (type == 'G' && tmp.length() > 1)
				error->report({ tmp, "Alphabet" }, NOT_CHAR);
			words.insert(tmp);
			tmp.clear();
		}
		else {
			if (!ch_valid(c, type))
				error->report({ {c} }, INVALID_CHAR);
			else
				tmp = tmp + c;
		}
	}
	if (tmp.empty())
		error->report({ "," }, INVALID_CHAR);
	if (type == 'S' && tmp.length() > 1)
		error->report({ tmp, "Input symbol" }, NOT_CHAR);
	if (type == 'G' && tmp.length() > 1)
		error->report({ tmp, "Alphabet" }, NOT_CHAR);
	words.insert(tmp);
}

void split(const string& str, vector<string>& tokens) {
	int pre = 0, cur = str.find(' ');
	while (cur != string::npos) {
		tokens.push_back(str.substr(pre, cur - pre));
		pre = cur + 1;
		cur = str.find(' ', pre);
	}
	tokens.push_back(str.substr(pre));
}

void check_valid(const unordered_set<char> charSet, const string& str) {
	for (auto c : str)
		if (c != '*' && charSet.find(c) == charSet.end())
			error->report({ {c} }, INVALID_CHAR);
}

#define JUDGE(type, dst) \
    case type: \
        if (value[0] != '{') \
            error->report({ "'{'" }, CHAR_MISS); \
        if (value.back() != '}') \
            error->report({ "'}'" }, CHAR_MISS); \
        split_judge(value.substr(1, value.length() - 2), dst, type);

void construct_tm(TuringMachine* tm, const string& filename) {
	ifstream f(filename);
	if (!f.is_open()) {
		error->report({}, FILE_OPEN_ERROR);
		return;
	}
	string tmp;
	unordered_set<string> tmpset;
	string curState;
	while (getline(f, tmp)) {
		error->line = tmp;
		extract_content(tmp);
		if (tmp.empty())
			;
		else if (tmp[0] == '#') {
			string value = check_def(tmp);
			switch (tmp[1]) {
				JUDGE('Q', tm->states);
				break;
				JUDGE('S', tmpset);
				for (const string& c : tmpset)
					tm->inputChar.insert(c[0]);
				tmpset.clear();
				break;
				JUDGE('G', tmpset);
				for (const string& c : tmpset)
					tm->tapeChar.insert(c[0]);
				tmpset.clear();
				break;
			case 'B':
				if (value != "_")
					error->report({ value }, INVALID_CHAR);
				break;
				JUDGE('F', tm->finalStates);
				for (const string& state : tm->finalStates)
					if (tm->states.find(state) == tm->states.end())
						error->report({ state, "final_state" }, STATE_MISS);
				break;
			case 'N':
				if (find_if(value.begin(), value.end(), [](char c) {return c < '0' || c > '9';}) != value.end())
					error->report({ value }, NOT_NUM);
				tm->tapeNum = atoi(value.c_str());
				break;
			default:
				if (!(tmp[1] == 'q' && tmp[2] == '0'))
					error->report({ tmp }, DEF_UNREC);
				if (tm->states.find(value) == tm->states.end())
					error->report({ value, "startState" }, STATE_MISS);
				tm->startState = value;
				break;
			}
		}
		else { // transition function
			vector<string> tokens;
			split(tmp, tokens);
			if (tokens.size() != 5) // 5 parts of transition function
				error->report({}, INCOMPLETE_FUNC);
			if (tokens[1].length() != tm->tapeNum)
				error->report({ "Scanned_symbol '", tokens[1] }, CHAR_NUM_ERR);
			check_valid(tm->tapeChar, tokens[1]);
			if (tokens[2].length() != tm->tapeNum)
				error->report({ "Print_symbol '", tokens[2] }, CHAR_NUM_ERR);
			check_valid(tm->tapeChar, tokens[2]);
			if (tokens[3].length() != tm->tapeNum)
				error->report({ "Move_tape '", tokens[3] }, CHAR_NUM_ERR);
			if (tm->states.find(tokens[0]) == tm->states.end())
				error->report({ tokens[0], "current_state" }, STATE_MISS);
			tm->transit.new_entry(tokens[0], tokens[1], { tokens[2], tokens[3], tokens[4] });
		}
		error->lineNo += 1;
	}
	f.close();
}
