// return number of '*' matched, -1 if not match
int Transit::match(const string& src, const string& dst) {
	if (src.size() != dst.size())
		return -1;
	int res = 0;
	for (int i = 0; i < src.size(); ++i) {
		if (src[i] == '*' || dst[i] == '*')
			++res;
		else if (src[i] != dst[i])
			return -1;
	}
	return res;
}
// find in src matched strings with dst, return the one with highest priority
int Transit::find(const vector<string>& src, const string& dst) {
	int minIdx = src.size(), minVal = 0x7fffffff;
	for (int i = 0; i < src.size(); ++i) {
		int val = match(src[i], dst);
		if (val != -1 && val < minVal) {
			minIdx = i; minVal = val;
		}
	}
	return minIdx;
}