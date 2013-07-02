#ifndef INTERFACE_PATH_H
#define INTERFACE_PATH_H
#include <list>

typedef std::list<int> Edges;

struct Path {
  
    int start_node;
    int end_node;
    
    Edges edges;
};



#endif