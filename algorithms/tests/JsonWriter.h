#ifndef JSON_WRITER_H
#define JSON_WRITER_H

#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>

using std::string;
using std::cerr;

class JsonWriter {

    ofstream out;
    int level = 0;
    bool isFirst = true;
    
    void mark_level() {
        for(int i=0 ; i<level ; ++i)
            out << "  ";
    }
    
    void separator() {
        if(!isFirst) 
            out << ",\n";
        else
            out << "\n";
        isFirst = false;
    }

public:
    
    JsonWriter(string file) {
        out.open(file);
        out << "[\n";
    }
    
    ~JsonWriter() {
        out << "\n]";
        out.close();
    }
    
    void step_in( string name ) {
        add(name);
        out << "{" ;
        level++;
        isFirst = true;
    }
    
    void step_in() {
        separator();
        mark_level();
        out << "{" ;
        level++;
        isFirst = true;
    }
    
    void step_out() {
        isFirst = false;
        level--;
        out << "\n";
        mark_level();
        out << "}";
    }
    
    void add(string name) {
        separator();
        mark_level();
        out << "\"" << name << "\": ";
    }
    
    void add(string name, int value) {
        add(name);
        out << value;
    }
    
    void add(string name, std::vector<int> values) {
        add(name);
        out << "[";
        for(uint i=0 ; i<values.size() ; ++i) {
            out << values[i];
            if(i < values.size() -1)
                out <<", ";
        }
        out << "]";
    }
    
    
};



#endif