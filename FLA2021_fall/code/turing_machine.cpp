#include "turing_machine.h"
#include "parser.h"

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

const void Transit::new_entry(const string& state, const string& input, const Action& action) {
    int idx1 = match_state(state);
    if (idx1 == states.size()) {
        states.push_back(state);
        inputs.push_back({});
        actions.push_back({});
    }
    inputs[idx1].push_back(input);
    actions[idx1].push_back(action);
}

Action Transit::query(const string& state, const string& input) const {
    int idx1 = match_state(state);
    if (idx1 == states.size())
        return {};
    int idx2 = match_input(idx1, input);
    if (idx2 == inputs[idx1].size())
        return {};
    return actions[idx1][idx2];
}


TuringMachine::TuringMachine(bool v) : verbose(v) {}

string TuringMachine::run(const string& input) {
    if (verbose)
        cout << "Input: " << input << endl;
    for (int i = 0; i < tapeNum; ++i) {
        list<Cell> tape(1, { 0, '_' });
        tapes.push_back(tape);
        heads.push_back(tapes[i].begin());
    }
    if (input.size()) {
        tapes[0].clear();
        for (int i = 0; i < input.size(); ++i) {
            char c = input[i];
            if (inputChar.find(c) == inputChar.end()) {
                string head_pos = "       ";
                head_pos += string(i, ' ') + '^';
                error->report({ {c}, input, head_pos }, INVALID_INPUT);
            }
            tapes[0].push_back({ i, c });
        }
        heads[0] = tapes[0].begin();
    }
    string curState = startState;
    if (verbose)
        cout << run_line << endl;
    for (int step = 0;; ++step) {
        if (verbose)
            print_step(curState, step);
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
        if (!act.valid()) {
            if (verbose)
                cout << "Halted. No rule for state: '" << curState << "' and symbol: '" << curSym << "'" << endl;
            return result();
        }
        move_write(act.direct, act.write);
        curState = act.next;
    }
}

string TuringMachine::result() {
    string res;
    for (auto c : tapes[0])
        if (c.symbol != blank)
            res = res + c.symbol;
    return res;
}

void TuringMachine::move_write(const string& dir, const string& modify) {
    assert(dir.length() == tapeNum && modify.length() == tapeNum);
    for (int i = 0; i < tapeNum; ++i) {
        if (modify[i] != '*')
            heads[i]->symbol = modify[i];
        switch (dir[i]) {
        case 'l':
            if (heads[i] == tapes[i].begin())
                tapes[i].push_front({ heads[i]->index - 1, '_' });
            --heads[i];
            break;
        case 'r':
            if (next(heads[i]) == tapes[i].end())
                tapes[i].push_back({ heads[i]->index + 1, '_' });
            ++heads[i];
            break;
        case '*':
            break;
        default:
            error->report({}, ERR_DIR);
            break;
        }
    }
}

void TuringMachine::print_step(const string& current_state, int step) {
    cout << "Step\t: " << step << endl;
    for (int i = 0; i < tapeNum; ++i) {
        string idxStr, tapeStr, headStr;
        auto not_blank = [&](const Cell& c) {return c.symbol != blank || c.index == heads[i]->index;};
        auto llimit = find_if(tapes[i].begin(), tapes[i].end(), not_blank),
            rlimit = find_if(tapes[i].rbegin(), tapes[i].rend(), not_blank).base();
        for (; llimit != rlimit; ++llimit) {
            string tmpIdxStr = to_string(abs(llimit->index));
            int idxLen = tmpIdxStr.length();
            idxStr += tmpIdxStr + ' ';
            tapeStr += llimit->symbol + string(idxLen, ' ');
            if (llimit == heads[i])
                headStr += '^';
            headStr += string(idxLen + 1, ' ');
        }
        idxStr.erase(idxStr.find_last_not_of(' ') + 1);
        tapeStr.erase(tapeStr.find_last_not_of(' ') + 1);
        headStr.erase(headStr.find_last_not_of(' ') + 1);
        cout << "Index" << i << "\t: " << idxStr << endl
            << "Tape" << i << "\t: " << tapeStr << endl
            << "Head" << i << "\t: " << headStr << endl;
    }
    cout << "State\t: " << current_state << endl
        << split_line << endl;
}

TuringMachine* tm;
