#include <iostream>
#include <fstream>
#include <sstream>
#include <ctime>
#include "parser.h"
#include "output.h"
#include "abacus.h"


using namespace std;

// write legal pl



int main(int argc, char* argv[]) {
    double start_time = clock();
    if (argc != 2) {
        cerr << "Usage: " << "<exe_file>  <aux_file>\n";
        return 1;
    }
    cout << "Start calculate time ..." << endl;

    parseAux(argv[1]);

    
    if (ROWS.empty()) {
        cerr << "ERROR: No rows parsed from .scl file!" << endl;
        return 1;
    }

    Abacus abc;
    
    abc.slice_row();
    abc.algo();


    writeOutFile();
    double second = (clock() - start_time) / CLOCKS_PER_SEC;

    cout << "Total time: " << (int)second/60 << " minutes, " << (int)second%60 << " seconds." << endl;
    return 0;
}