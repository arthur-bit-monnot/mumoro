/* Copyright : Université Toulouse 1 (2010)

Contributors : 
Tristram Gräbener
Odysseas Gabrielides
Arthur Bit-Monnot (arthur.bit-monnot@laas.fr)

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
knowledge of the CeCILL-B license and that you accept its terms. */

#ifndef UTILS_H
#define UTILS_H

double get_run_time_sec();

extern double ticking_start, ticking_end;

#define START_TICKING ticking_start = get_run_time_sec()
#define STOP_TICKING ticking_end = get_run_time_sec()
#define RUNTIME (ticking_end - ticking_start)*((double) 1000)

struct END {};
static END End;

template<typename T, typename NEXT=END> 
struct LISTPARAM {
    T value;
    NEXT next;
    
    LISTPARAM(T t) : value(t), next(End) {}
    template<typename T2> LISTPARAM(T2 t2, T t) : value(t), next(t2) {}
    template<typename T2, typename T3> LISTPARAM(T3 t3, T2 t2, T t) : value(t), next(t3, t2) {}
    template<typename T2, typename T3, typename T4> LISTPARAM(T4 t4, T3 t3, T2 t2, T t) : value(t), next(t4, t3, t2) {}
};

#endif