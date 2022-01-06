string TuringMachine::run(const string& input) {
	string curState = startState;
	for (int step = 0;; ++step) {
		if (finalStates.find(curState) != finalStates.end()) // halt
			return result();
		string curSym;
		for (int i = 0; i < tapeNum; ++i) {
			char c = heads[i]->symbol;
			if (c == ' ')
				c = blank;
			curSym += c;
		}
		const Action& act = transit.query(curState, curSym);
		if (!act.valid())
			return result();
		move_write(act.direct, act.write);
		curState = act.next;
	}
}