#include <stdlib.h>
#include <iostream>
using std::cerr;
using std::cout;
using std::endl;
#include <fstream>
using std::ifstream;

#include <ctime>
#include <sys/times.h>

#include "reglc_graph.h"

double get_run_time() {

  struct tms usage;
  static int clock_ticks = sysconf(_SC_CLK_TCK);
  times(&usage);
  double df=((double)usage.tms_utime+(double)usage.tms_stime)/clock_ticks;

  return df;
}

int main()
{
    Transport::Graph transport("/home/arthur/LAAS/Data/Graphs/midi-pyrennees.dump");
    RLC::DFA dfa = RLC::pt_foot_dfa();
    RLC::Graph g(&transport, dfa);
    
    ifstream indata; // indata is like cin
    int source, target, start, day; // variables for input value
    indata.open("/home/arthur/LAAS/Data/Test-Confs/perf-mp.conf"); // opens the file
    if(!indata) { // file couldn't be opened
        cerr << "Error: file could not be opened" << endl;
        exit(1);
    }
    
    double total = 0;
    int num_tests = 0;
  
    while ( !indata.eof() ) { // keep reading until end-of-file
        indata >> source >> target >> start >> day; // sets EOF flag if no value found
        cout << source <<" "<< target <<" "<< start <<" "<< day << endl;
        
        
        double start_run = get_run_time();        
        RLC::Dijkstra dij(&g, source, target, start, day);

        bool path_found = dij.run();
        double end_run = get_run_time();
        
        if(path_found) {
            total += end_run - start_run;
            ++num_tests;
        }
    }
    
    cout<< "Total time over "<< num_tests << " instances : "<<total;
    
    indata.close();
    cout << "End-of-file reached.." << endl;
    
    
    
    
    
    
    
    return 0;
}