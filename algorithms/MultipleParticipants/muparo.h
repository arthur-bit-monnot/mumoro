#ifndef MUPARO_H
#define MUPARO_H

#include "reglc_graph.h"
#include "DRegLC.h"
#include "nodes_filter.h"

using namespace std;

struct Invalid_Operation {};

namespace MuPaRo
{


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
    int dfa_state;
    int arrival;
    int cost;
    /**
     * If node was inserted after application of rule, this list
     * contains the layers the predecessors must be searched in.
     */
    unsigned char pred_layers;
};

struct MuparoParams
{
    MuparoParams(const Transport::Graph * transport, const int num_layers) : transport(transport), num_layers(num_layers) {}
    const Transport::Graph * transport;
    const int num_layers;
};

typedef enum { DestNodes, Bidirectional, Connection } SearchType;

template<typename Algo>
class Muparo
{
public:
    typedef Algo Dijkstra;
    typedef LISTPARAM<MuparoParams> ParamType;
    
    Muparo( ParamType p ) :
    num_layers(p.value.num_layers),
    transport(p.value.transport),
    vres(p.value.transport)
    {
        is_set = new boost::dynamic_bitset<>*[num_layers];
        flags = new Flag* [num_layers];
        for(int i=0; i<num_layers ; ++i) {
            is_set[i] = new boost::dynamic_bitset<>(transport->num_vertices());
            flags[i] = (Flag*) malloc( sizeof( Flag ) * transport->num_vertices() );
        }
    }
    
    virtual ~Muparo()
    {
        for(int i=0; i<num_layers ; ++i)
        {
            delete dij[i];
            delete graphs[i];
            delete is_set[i];
            delete[] flags[i];
        }
        delete is_set;
        delete[] flags;
    }
    
    const int num_layers;
    const Transport::Graph * transport;
    vector<RLC::DFA> dfas;
    vector<RLC::AbstractGraph*> graphs;
    vector<Algo*> dij;
    
    boost::dynamic_bitset<> ** is_set;
    Flag **flags;
    
    list<StartNode> start_nodes;
    list<StateFreeNode> goal_nodes;
    
    virtual CompleteNode proceed_one_step() 
    {
        const int layer = select_layer();
            
        RLC::Vertice vert = dij[layer]->treat_next();
        StateFreeNode node(layer, vert.first);
        CompleteNode c_node(layer, vert);
        
        if( graphs[layer]->is_accepting( vert ) && !is_node_set( node ) ) {
            set( c_node );
            apply_rules( node.second );
        }
        
        return c_node;
    }

    
    bool run()
    {
        BOOST_FOREACH(StartNode sn, start_nodes) {
            insert(sn.first, sn.second, 0);
        }
        
        while( !finished() ) {
            proceed_one_step();
        }

        return true;
    }
    
    void clear_pred_layers(const StateFreeNode n) const { flags[n.first][n.second].pred_layers = 0; }
    void add_pred_layer(const StateFreeNode n, const int layer) { flags[n.first][n.second].pred_layers |= (1 << layer); }
    
    void check_connections( const int modified_layer );
    
    /**
     * Returns True if this node was set (i.e. there is an accepting state in the dfa that was recahed for this 
     * node
     */
    bool is_node_set(const StateFreeNode n) const { return is_set[n.first]->test( n.second ); }
    
    /**
     * Whan an accepting state is reached, this used to store the dfa state and arrival 
     * time at this node.
     */
    void set( const CompleteNode n) {
        is_set[n.first]->set( n.second.first );
        flags[n.first][n.second.first].dfa_state = n.second.second;
        flags[n.first][n.second.first].arrival = dij[n.first]->arrival(n.second);
        flags[n.first][n.second.first].cost = dij[n.first]->cost(n.second);
    }
    
    /**
     * Earliest arrival to an accepting state of this node.
     */
    int arrival(const StateFreeNode n) const { 
        BOOST_ASSERT(is_node_set(n));
        return flags[n.first][n.second].arrival;
    }
    
    int get_cost(const StateFreeNode n) const { 
        BOOST_ASSERT(is_node_set(n));
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
    virtual bool finished() const 
    {
        bool heap_empties = true;
        
        BOOST_FOREACH(RLC::DRegLC *d, dij) {
            if(! d->finished() ) {
                heap_empties = false;
                break;
            }
        }
        
        return heap_empties;
    }
    
    /**
     * Returns the id of the next layer to treat.
     * This is the layer with the minimum cost in its heap.
     */
    int select_layer() const
    {
        int best_cost = std::numeric_limits<int>::max();
        int best_layer = -1;
        
        for(int i=0 ; i<num_layers ; ++i) {
            if( !dij[i]->finished() && ( dij[i]->cost( dij[i]->heap.top() ) <= best_cost ) ) {
                best_cost = dij[i]->cost( dij[i]->heap.top() );
                best_layer = i;
            }
        }
        
        return best_layer;
    }
    
    /**
     * Inserts a node in the relevant layer (given in `n`) with arrival time `arrival`.
     * For every start state in the DFA, the corresponding node is insert in the layer.
     * 
     * Returns true if at least one node was inserted
     */
    bool insert(const StateFreeNode & n, const int arrival, const int cost)
    {
        bool inserted = false;
        int layer = n.first;
        int node = n.second;
        BOOST_FOREACH(int dfa_start, graphs[layer]->dfa_start_states())  
        {
            RLC::Vertice rlc_node;
            rlc_node.first = node;
            rlc_node.second = dfa_start;
            
            if( dij[layer]->insert_node( rlc_node, arrival, cost ) ) {
                inserted = true;
                clear_pred_layers( n );
            }
        }
        
        return inserted;
    }    
    
    /**
     * For a given node, check all rules if it is appicable for this node.
     * If it is, it then proceed to insertion.
     */
    virtual void apply_rules( const int node ) { /* does nothing */ }
    
    virtual int solution_cost() const { return -1; }
    
    /** Results **/
    VisualResult vres;
    
    virtual void init_result_queue( std::list< CompleteNode > & queue ) { /* does nothing */ };
    
    void build_result() 
    {
        std::list< CompleteNode > queue;
        
        init_result_queue( queue );
        
        while( !queue.empty() ) {
            CompleteNode curr = queue.back();
            queue.pop_back();
            
            int l = curr.first;
            RLC::Vertice vert = curr.second;
            
            if( dij[l]->has_pred(vert) ) {
                vres.edges.push_back(transport->edgeIndex( dij[l]->get_pred(vert).first ));
                queue.push_back( CompleteNode(l, graphs[l]->source(dij[l]->get_pred(vert) )));
            }
            else if( flags[l][vert.first].pred_layers == 0 ) // no pred layers  
            {
            }
            else
            {
                for(uint layer=0 ; layer < sizeof(Flag::pred_layers)*8 ; ++layer) {
                    if(flags[l][vert.first].pred_layers & (1 << layer)) {
                        queue.push_back( CompleteNode(layer, RLC::Vertice(vert.first, flags[layer][vert.first].dfa_state )));
                        vres.c_nodes.push_back(vert.first);
                    }
                }
            }
        }
    }
    VisualResult get_result() const { return vres; };
};


template<typename Algo>
void free(Muparo<Algo> * mup);

} // end namespace MuPaRo


#endif
