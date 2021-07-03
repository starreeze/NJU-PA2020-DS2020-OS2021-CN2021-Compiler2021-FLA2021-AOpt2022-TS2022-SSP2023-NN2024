// mystl parser
#include<iostream>
#include<fstream>
#include<cstring>
#include<string>
#include<cassert>
#include<unordered_set>
using namespace std; 

unordered_set<string> includes; 

void parse(const char* file_in, const char* file_out) {
    ifstream in(file_in); 
    if(!in.is_open()) {
        cout << "Fail to open file '" << file_in << "'.\n"; 
        exit(-1); 
    }
    ofstream out(file_out, ios_base::app); 
    if(!out.is_open()) {
        cout << "Fail to open file '" << file_out << "'.\n"; 
        exit(-1); 
    }
    char buf[256]; 
    while(in.peek() != EOF) {
        assert(out.is_open()); 
        in.getline(buf, 256); 
        int n = strlen(buf); 
        if(buf[n-1] == '\r') { // DOS
            buf[n-1] = 0; 
            --n; 
        }
        if(n > 10 && buf[8] == '\"') {
            buf[8] = 0; 
            if(!strcmp("#include", buf)) {
                char file[64]; 
                strcpy(file, buf + 9); 
                file[strlen(file)-1] = 0; 
                if(includes.find(file) == includes.end()) {
                    out << "\n\n"; 
                    out.close(); 
                    parse(file, file_out); 
                    out.open(file_out, ios_base::app); 
                    out << "\n\n"; 
                    includes.insert(file); 
                }
            }
            else {
                buf[8] = '\"'; 
                out << buf << '\n'; 
            }
        }
        else if(n > 10 && buf[8] == ' ' && buf[9] == '\"') {
            buf[9] = 0;
            if(!strcmp("#include ", buf)) {
                char file[64];
                strcpy(file, buf + 10);
                file[strlen(file)-1] = 0;
                if(includes.find(file) == includes.end()) {
                    out << "\n\n";
                    out.close();
                    parse(file, file_out);
                    out.open(file_out, ios_base::app);
                    out << "\n\n";
                    includes.insert(file);
                }
            }
            else {
                buf[9] = '\"';
                out << buf << '\n';
            }
        }
        else    out << buf << '\n'; 
    }
    in.close(); out.close(); 
}

int main(int argc, char* argv[]) {
    if(argc < 2) {
        cout << "Invalid args.\n"; 
        return -1; 
    }
    const char* output_name = "parsed.cpp"; 
    ofstream(output_name).close(); 
    parse(argv[1], output_name); 
    return 0; 
}
