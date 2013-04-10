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