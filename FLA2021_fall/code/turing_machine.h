#include "parser.h"
#ifndef TURING_MACHINE_H
#define TURING_MACHINE_H

struct Action {
    string write;
    string direct;
    string next;
    bool valid() const { return next.size(); }
};

struct Cell {
    int index;
    char symbol;
};

class Transit { // inputs: input[state_idx], action: action[state_idx][input_idx]
    vector<string> states;
    vector<vector<string>> inputs;
    vector<vector<Action>> actions;
    // return number of '*' matched, -1 means not match
    static int match(const string& src, const string& dst);
    static int find(const vector<string>& src, const string& dst);
    int match_state(const string& state) const { return find(states, state); }
    int match_input(int idx, const string& input) const { return find(inputs[idx], input); }
public:
    const void new_entry(const string& state, const string& input, const Action& action);
    Action query(const string& state, const string& input) const;
};

class TuringMachine {
    const bool verbose;
    unordered_set<string> states;
    unordered_set<string> finalStates;
    unordered_set<char> inputChar;
    unordered_set<char> tapeChar;
    const char blank = '_';
    string startState;
    int tapeNum;
    Transit transit;
    vector<list<Cell>> tapes;
    vector<list<Cell>::iterator> heads;
public:
    TuringMachine(bool verbose);
    friend void construct_tm(TuringMachine* tm, const string& filename);
    string run(const string& input);
    // move all tape headers and write
    void move_write(const string& dir, const string& input);
    void print_step(const string& currentState, int step);
    string result();
};

extern TuringMachine* tm;

#endif