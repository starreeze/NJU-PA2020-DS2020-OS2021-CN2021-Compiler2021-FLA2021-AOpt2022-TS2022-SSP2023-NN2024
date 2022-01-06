#ifndef PARSER_H
#define PARSER_H
#include <iostream>
#include <string>
#include <fstream>
#include <cassert>
#include <algorithm>
#include <vector>
#include <unordered_set>
#include <list>

using namespace std;

constexpr const char* help_output = "usage: turing [-v|--verbose] [-h|--help] <tm> <input>";
constexpr const char* error_line = "==================== ERR ====================";
constexpr const char* end_line = "==================== END ====================";
constexpr const char* run_line = "==================== RUN ====================";
constexpr const char* split_line = "---------------------------------------------";
const unordered_set<string> Def = { "Q", "S", "G", "q0", "B", "F", "N" };

// return verbose
bool parse_args(int argc, char** argv, string& file, string& input);

enum ErrorType {
	FILE_OPEN_ERROR = 1,
	INCOMPLETE_LINE,
	INVALID_CHAR,
	STATE_MISS,
	NOT_CHAR,
	NOT_NUM,
	INCOMPLETE_FUNC,
	CHAR_MISS,
	STATUS_MISS,
	COMMAND_LINE_ERROR,
	INVALID_INPUT,
	DEF_MISS,
	CHAR_NUM_ERR,
	ERR_DIR,
	DEF_UNREC
};
class Error {
	const string filename;
	const bool verbose;
public:
	string line;
	int lineNo = 1;
	Error(const string& file, bool v) : filename(file), verbose(v) {}
	void report(const vector<string>& info, ErrorType type);
};
extern Error* error;

class TuringMachine;
void construct_tm(TuringMachine* tm, const string& filename);

#endif
