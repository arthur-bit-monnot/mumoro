/** Copyright : Arthur Bit-Monnot (2013)  arthur.bit-monnot@laas.fr

This software is a computer program whose purpose is to [describe
functionalities and technical features of your software].

This software is governed by the CeCILL-B license under French law and
abiding by the rules of distribution of free software.  You can  use, 
modify and/ or redistribute the software under the terms of the CeCILL-B
license as circulated by CEA, CNRS and INRIA at the following URL
"http://www.cecill.info". 

As a counterpart to the access to the source code and  rights to copy,
modify and redistribute granted by the license, users are provided only
with a limited warranty  and the software's author,  the holder of the
economic rights,  and the successive licensors  have only  limited
liability. 

In this respect, the user's attention is drawn to the risks associated
with loading,  using,  modifying and/or developing or reproducing the
software by the user in light of its specific status of free software,
that may mean  that it is complicated to manipulate,  and  that  also
therefore means  that it is reserved for developers  and  experienced
professionals having in-depth computer knowledge. Users are therefore
encouraged to load and test the software's suitability as regards their
requirements in conditions enabling the security of their systems and/or 
data to be ensured and,  more generally, to use and operate it in the 
same conditions as regards security. 

The fact that you are presently reading this means that you have had
knowledge of the CeCILL-B license and that you accept its terms. 
*/

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
        out.flush();
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
    
    void add(string name, float value) {
        add(name);
        out << value;
    }
    
    void add(string name, double value) {
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
    
    void skip_line() {
        out << endl;
    }
    
    
};



#endif