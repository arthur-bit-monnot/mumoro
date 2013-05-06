#ifndef CSV_WRITER_H
#define CSV_WRITER_H

#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>

using std::string;
using std::cerr;

class CsvWriter {

    ofstream out;

public:
    
    CsvWriter(string file) {
        out.open(file);
    }
    
    ~CsvWriter() {
        out.close();
    }
    
    template<typename T1, typename T2>
    void add_line(T1 first, T2 second) {
        out << first <<", " << second << std::endl;
    }
    
};



#endif