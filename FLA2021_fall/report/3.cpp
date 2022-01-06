struct Action {
	string write;
	string direct;
	string next;
};
class Transit { // inputs: input[state_idx], action: action[state_idx][input_idx]
	vector<string> states;
	vector<vector<string>> inputs;
	vector<vector<Action>> actions;
public:
	// methods ...
};
