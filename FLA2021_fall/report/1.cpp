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