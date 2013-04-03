#ifndef MUPARO_H
#define MUPARO_H

#include "reglc_graph.h"
#include "reglc_dijkstra.h"
#include "nodes_filter.h"

using namespace std;

struct Invalid_Operation {};

namespace MuPaRo
{

typedef enum { SumCost, MaxCost, SumPlusWaitCost } CostCombination;
typedef enum { MaxArrival, FirstLayerArrival, SecondLayerArrival } ArrivalCombination;

class Muparo;

/**
 * Used to propagate results from one or several layers to another one.
 * 
 * Meaning : given a node with id `n` :
 * if nodes (l, n) are set for every layer l in conditions
 * then insert (l2, n) for every layer l2 in insertions
 */
struct PropagationRule
{
    PropagationRule(Muparo * mup) : mup(mup), cost_comb(MaxCost), arr_comb(MaxArrival), filter(NULL) {}
    ~PropagationRule() { if(filter != NULL) delete filter; }
    Muparo * mup;
    CostCombination cost_comb;
    ArrivalCombination arr_comb;
    vector<int> conditions;
    int insertion;
    inline int combine_costs(const int old, const int add) const {
        if(cost_comb == SumCost)
            return old + add;
        else if(cost_comb == MaxCost)
            return old > add ? old : add;
        else
            throw Invalid_Operation();
        
        return -1;
    }
    int arrival_in_insertion_layer( const int node ) const;
    int cost_in_insertion_layer( const int node ) const;
    bool applicable(const int node) const;
    void apply(const int node);
    
    NodeFilter * filter;
};

struct ConnectionRule
{
    ConnectionRule(Muparo * mup) : mup(mup), comb(SumCost) {}
    Muparo * mup;
    CostCombination comb;
    list<int> conditions;
    int get_cost( const int node, bool real_cost = false ) const;
    bool applicable(const int node) const;
    void apply(const int node);
    
    int l3_dest;
};

/**
 * First : id of the layer the node belongs to
 * Second : id of the node in transport graph
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
 */
typedef pair<StateFreeNode, int> StartNode;

struct Flag
{
    Flag() : set(false) {}
    bool set;
    int dfa_state;
    int arrival;
    int cost;
    /**
     * If node was inserted after application of rule, this list
     * contains the layers the predecessors must be searched in.
     */
    std::list< int > pred_layers;
};

typedef enum { DestNodes, Bidirectional, Connection } SearchType;

struct MuparoParameters
{
    MuparoParameters() : search_type(DestNodes) {}
    SearchType search_type;
    std::pair<int, int> bidir_layers;
};

class Muparo
{
public:
    Muparo(Transport::Graph * transport, int num_layers, MuparoParameters p = MuparoParameters());
    ~Muparo();
    
    MuparoParameters params;
    
    int num_layers;
    Transport::Graph * transport;
    vector<RLC::DFA> dfas;
    vector<RLC::AbstractGraph*> graphs;
    vector<RLC::Dijkstra*> dij;
    vector<PropagationRule*> propagation_rules;
    vector<ConnectionRule> connection_rules;
    Flag **flags;
    
    list<StartNode> start_nodes;
    list<StateFreeNode> goal_nodes;
    
    /** Bidirectional variables */
    bool connection_found;
    int best_cost;
    RLC::Vertice best_bidir_connection;
    int best_connection_node;
    
    
public:
    bool run();
    
    void clear_pred_layers(const StateFreeNode n) const { flags[n.first][n.second].pred_layers.clear(); }
    void add_pred_layer(const StateFreeNode n, int layer) { flags[n.first][n.second].pred_layers.push_back(layer); }
    
    void check_connections( const int modified_layer );
    
    /**
     * Returns True if this node was set (i.e. there is an accepting state in the dfa that was recahed for this 
     * node
     */
    bool is_set(const StateFreeNode n) const { return flags[n.first][n.second].set; }
    
    /**
     * Whan an accepting state is reached, this used to store the dfa state and arrival 
     * time at this node.
     */
    void set( const CompleteNode n) {
        flags[n.first][n.second.first].set = true;
        flags[n.first][n.second.first].dfa_state = n.second.second;
        flags[n.first][n.second.first].arrival = dij[n.first]->arrival(n.second);
        flags[n.first][n.second.first].cost = dij[n.first]->cost(n.second);
    }
    
    /**
     * Earliest arrival to an accepting state of this node.
     */
    int arrival(const StateFreeNode n) const { 
        BOOST_ASSERT(is_set(n));
        return flags[n.first][n.second].arrival;
    }
    
    int get_cost(const StateFreeNode n) const { 
        BOOST_ASSERT(is_set(n));
        return flags[n.first][n.second].cost;
    }
    
    /**
     * Returns True if there is nothing left to do :
     * When looking for goals :
     *  - etheir all heaps are empties
     *  - or all goals are set
     * 
     * When running a bidirectional search:
     *  - either all heaps are empties
     *  - or the connection point was found such as no shorter path can be found
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
     * 
     * Returns true if at least one node was inserted
     */
    bool insert(StateFreeNode n, int arrival, int cost);
    
    /**
     * For a given node, check all rules if it is appicable for this node.
     * If it is, it then proceed to insertion.
     */
    void apply_rules(int node);
    
    /**
     * Returns the minimal cost that might appear in a layer.
     * Whith no propagation_rules this is simply the minimal cost in heap.
     * 
     * This is more tricky than it sounds since, nodes might be insert in this layer when propagation_rules are applied.
     */
    int min_cost(const int layer) const;
    
    
    /** Results **/
    VisualResult vres;
    void build_result();
    VisualResult get_result() const;
    int visited_nodes() const;
};



void free(Muparo * mup);

} // end namespace MuPaRo


#endif
