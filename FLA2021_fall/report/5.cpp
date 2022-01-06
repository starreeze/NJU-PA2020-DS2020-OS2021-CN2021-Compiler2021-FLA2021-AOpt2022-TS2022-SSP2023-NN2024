void split(const string& str, vector<string>& tokens) {
	int pre = 0, cur = str.find(' ');
	while (cur != string::npos) {
		tokens.push_back(str.substr(pre, cur - pre));
		pre = cur + 1;
		cur = str.find(' ', pre);
	}
	tokens.push_back(str.substr(pre));
}