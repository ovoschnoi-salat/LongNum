#include <iostream>
#include <fstream>
#include <vector>
#include "LN.hpp"

using namespace std;

int printError(int r, const char *str) {
    cout << str << endl;
    return r;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        return printError(1, "Incorrect input arguments.\n"
                             "This program expect launching with 2 arguments: *name* input output\n"
                             "Where:\n"
                             "    input - name of input file\n"
                             "    output - name of output file");
    }
    vector<LN> st = vector<LN>();
    ifstream input = ifstream(argv[1]);
    if (!input.is_open() || !input.good()) {
        return printError(1, "Error occurred while opening input file.");
    }
    //work with Long Numbers
    try {
        string in;
        unsigned long size = 0;
        while (input >> in) {
            switch (in.at(0)) {
            case '+':
                st[size - 2] += st[size - 1];
                size--;
                st.pop_back();
                break;
            case '*':
                st[size - 2] *= st[size - 1];
                size--;
                st.pop_back();
                break;
            case '/':
                st[size - 2] /= st[size - 1];
                size--;
                st.pop_back();
                break;
            case '%':
                st[size - 2] %= st[size - 1];
                size--;
                st.pop_back();
                break;
            case '~':
                st[size - 1] = ~st[size - 1];
                break;
            case '_':
                st[size - 1].changeSign();
                break;
            case '<':
                if (in.length() == 2) {
                    st[size - 2] = st[size - 2] <= st[size - 1];
                } else {
                    st[size - 2] = st[size - 2] < st[size - 1];
                }
                st.pop_back();
                size--;
                break;
            case '>':
                if (in.length() == 2) {
                    st[size - 2] = st[size - 2] >= st[size - 1];
                } else {
                    st[size - 2] = st[size - 2] > st[size - 1];
                }
                st.pop_back();
                size--;
                break;
            case '=':
                st[size - 2] = st[size - 2] == st[size - 1];
                st.pop_back();
                size--;
                break;
            case '!':
                st[size - 2] = st[size - 2] != st[size - 1];
                st.pop_back();
                size--;
                break;
            case '-':
                if (in.length() == 1) {
                    st[size - 2] -= st[size - 1];
                    size--;
                    st.pop_back();
                    break;
                }
            default:
                st.emplace_back(LN(in));
                size++;
                break;
            }
        }
    } catch (exception &exception) {
        cout << "Error: " << exception.what() << endl;
        input.close();
        if (input.bad()) {
            return printError(2, "Error occurred while closing input file.");
        }
        return 2;
    }
    input.close();
    if (input.bad()) {
        return printError(2, "Error occurred while closing input file.");
    }
    //stack dump
    ofstream output = ofstream(argv[2]);
    if (!output.is_open() || !output.good()) {
        return printError(1, "Error occurred while opening output file.");
    }
    for (const LN &a : st) {
        output << (string) a << endl;
    }
    output.close();
    if (output.bad()) {
        return printError(2, "Error occurred while closing output file.");
    }
    return 0;
}
