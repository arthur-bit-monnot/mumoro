#include <stdlib.h>
#include <iostream>
using std::cerr;
using std::cout;
using std::endl;
#include <fstream>
using std::ifstream;
#include <time.h>

#include "GraphFactory.h"
#include "muparo.h"
#include "run_configurations.h"
#include "utils.h"
#include "node_filter_utils.h"
#include "../RegLC/AlgoTypedefs.h"
#include "Landmark.h"

#include "../RegLC/AspectTarget.h"
#include "../RegLC/AlgoTypedefs.h"
#include "../RegLC/AspectTargetLandmark.h"
#include "../RegLC/AspectTargetAreaLandmark.h"

#include "CsvWriter.h"
#include <graph_wrapper.h>

using namespace RLC;

CsvWriter * out;

void run_test(const Transport::Graph * g, const int start, const int dest_center, const int area_radius, Landmark * lm)
{
    Area * area = build_area_around(g, dest_center, dest_center, area_radius, car_dfa());
    area->center = dest_center;
    area->init();
    RLC::Graph rlc(g, car_dfa());
    
    int with_landmarks, without_landmarks;
    
    {
        typedef RLC::AspectCount<RLC::AspectTargetAreaStop<AspectTargetAreaLandmark<RLC::DRegLC>>> Algo;

        Algo::ParamType p(
            DRegLCParams(&rlc, 10),
            AspectTargetAreaLandmarkParams<>( area, lm ),
            AspectTargetAreaStopParams( area )        
        );
        
        Algo algo( p );
        algo.insert_node(RLC::Vertice(start, 0), 0, 0);
        algo.run();
        
        with_landmarks = algo.count;
    }
    {
        typedef RLC::AspectCount<RLC::AspectTargetAreaStop<RLC::DRegLC>> Algo;

        Algo::ParamType p(
            DRegLCParams(&rlc, 10),
            AspectTargetAreaStopParams( area )            
        );
        
        Algo algo( p );
        algo.insert_node(RLC::Vertice(start, 0), 0, 0);
        algo.run();
        
        without_landmarks = algo.count;
    }
    
    cout << area->radius <<" "<< with_landmarks <<" "<< without_landmarks <<" "<< (float) with_landmarks / (float) without_landmarks << endl;
    out->add_line( area->radius, (float) with_landmarks / (float) without_landmarks );
    delete area;
}

bool run_test(const Transport::Graph * g, const int start, const int dest_center)
{
    Landmark * lm = RLC::create_car_landmark(g, dest_center);
    if(lm->dist_lb(start, dest_center, true) > 5400) {
        run_test(g, start, dest_center, 0, lm);
        run_test(g, start, dest_center, 300, lm);
        run_test(g, start, dest_center, 600, lm);
        run_test(g, start, dest_center, 900, lm); 
        run_test(g, start, dest_center, 1200, lm);
        run_test(g, start, dest_center, 1800, lm);
        run_test(g, start, dest_center, 2400, lm);
        run_test(g, start, dest_center, 3000, lm);
        run_test(g, start, dest_center, 3600, lm);
        delete lm;
        return true;
    } else {
        delete lm;
        return false;
    }
}


int main(void)
{
    CsvWriter writer("/home/arthur/LAAS/Data/Results/LandmarkAreas.csv");
    out = &writer;
    
    srand (time(NULL));
    
    std::string file( "/home/arthur/LAAS/Data/Graphs/sud-ouest.dump" );
    Transport::GraphFactory gf( file );
    
    const Transport::Graph * g = gf.get();
    
    for(int i=0 ; i<50 ;) {
        int source = -1;
        int dest = -1;
        while( dest < 0 ) {
            int tmp = rand() % g->num_vertices();
            if( g->car_accessible( tmp ) ) {
                if(source < 0)
                    source = tmp;
                else
                    dest = tmp;
            }
        }
        if( run_test(g, source, dest) )
            ++i;
        cout << i<<endl;
    }
    
    
}