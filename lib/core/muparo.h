#ifndef MUPARO_H
#define MUPARO_H

#include "reglc_graph.h"

using namespace std;

namespace MuPaRo
{

/**
 * Used to propagate results from one or several layers to another one.
 * 
 * Meaning : given a node with id `n` :
 * if nodes (l, n) are set for every layer l in conditions
 * then insert (l2, n) for every layer l2 in insertions
 */
struct ConnectionRule
{
    list<int> conditions;
    list<int> insertions;
};

/**
 * First : id of the layer the node belongs to
 * Second : id of the node in transport graphs
 */
typedef pair<int, int> StateFreeNode;

/**
 * First : id of the layer the node belongs to
 * Second : Vertice of the RLC Graph
 */
typedef pair<int, RLC::Vertice> CompleteNode;


/**
 * Defines a start node :
 * - The node (layer + node)
 * - departure time (seconds)
 * - departure day
 */
typedef pair<StateFreeNode, int> StartNode;

struct Flag
{
    Flag() : set(false) {}
    bool set;
    int dfa_state;
    int arrival;
};

class Muparo
{
public:
    Muparo(Transport::Graph * transport, int start1, int start2);
    ~Muparo();
    int num_layers;
    Transport::Graph * transport;
    vector<RLC::DFA> dfas;
    vector<RLC::Graph*> graphs;
    vector<RLC::Dijkstra*> dij;
    vector<ConnectionRule> rules;
    Flag **flags;
    
    list<StartNode> start_nodes;
    list<StateFreeNode> goal_nodes;
    
public:
    bool run();
    
    /**
     * Returns True if this node was set (i.e. there is an accepting state in the dfa that was recahed for this 
     * node
     */
    bool is_set(StateFreeNode n) { return flags[n.first][n.second].set; }
    
    /**
     * Whan an accepting state is reached, this used to store the dfa state and arrival 
     * time at this node.
     */
    void set(CompleteNode n) {
        flags[n.first][n.second.first].set = true;
        flags[n.first][n.second.first].dfa_state = n.second.second;
        flags[n.first][n.second.first].arrival = dij[n.first]->arrival(n.second);
    }
    
    /**
     * Earliest arrival to an accepting state of this node.
     */
    int arrival(StateFreeNode n) { 
        BOOST_ASSERT(is_set(n));
        return flags[n.first][n.second].arrival;
    }
    
    /**
     * Returns True if there is nothing left to do :
     * etheir all heaps are empties
     * or all goals are set
     */
    bool finished();
    
    /**
     * Returns the id of the next layer to treat.
     * This is the layer with the minimum cost in its heap.
     */
    int select_layer();
    
    /**
     * Inserts a node in the relevant layer (given in `n`) with arrival time `arrival`.
     * For every start state in the DFA, the corresponding node is insert in the layer.
     */
    void insert(StateFreeNode n, int arrival);
    
    /**
     * For a given node, check all rules if it is appicable for this node.
     * If it is, it then proceed to insertion.
     */
    void apply_rules(int node);
    
    
    /** Results **/
    VisualResult vres;
    void build_result();
    VisualResult get_result() { return vres; }
};

}


#endif
