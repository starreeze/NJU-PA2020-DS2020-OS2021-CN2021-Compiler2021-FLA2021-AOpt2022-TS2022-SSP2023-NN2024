#include "turing_machine.h"

int main(int argc, char** argv) {
    string file, input;
    bool verbose = parse_args(argc, argv, file, input);
    error = new Error(file, verbose);
    tm = new TuringMachine(verbose);
    construct_tm(tm, file);
    string res = tm->run(input);
    if (verbose)
        cout << "Result: ";
    cout << res << endl;
    if (verbose)
        cout << end_line << endl;
    return 0;
}