struct Cell {
    int index;
    char symbol;
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
    // methods ...
};