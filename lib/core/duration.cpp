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


#include "graph_wrapper.h"

DurationPT::DurationPT(float d) : const_duration(d), dur_type(ConstDur) { }

DurationPT::DurationPT(const DurationType dur_type) : const_duration(-1), dur_type(dur_type) {}

void DurationPT::append_timetable(float start, float arrival, const std::string & services)
{
    BOOST_ASSERT(start < arrival);
    BOOST_ASSERT(dur_type == TimetableDur);
    timetable.push_back(Time(start, arrival, Services(services)));
}

void DurationPT::append_frequency(int start, int end, int duration, const std::string & services)
{
    BOOST_ASSERT(start < end);
    BOOST_ASSERT(dur_type == FrequencyDur);
    frequencies.push_back(Frequency(start, end, duration, Services(services)));
}

bool compare_times(const Time & a, const Time & b)
{
    return get<0>(a) < get<0>(b);
}

bool compare_frequencies(const Frequency & a, const Frequency & b)
{
    return get<0>(a) < get<0>(b);
}

void DurationPT::sort()
{
    if(dur_type == TimetableDur)
    {
        std::sort(timetable.begin(), timetable.end(), compare_times);
    }
    else if(dur_type == FrequencyDur)
    {
        std::sort(frequencies.begin(), frequencies.end(), compare_frequencies);
    }
}

std::pair<bool, int> DurationPT::operator()(float start, int day, bool backward) const
{
    if(backward) {
        if (dur_type == ConstDur) {
            return std::pair<bool, int>(true, const_duration);
        } else if(dur_type == FrequencyDur) {
            return freq_duration_backward(start, day);
        } else {
            return tt_duration_backward(start, day);       
        }
    }
    else /** forward **/
    {
        if (dur_type == ConstDur) {
            return std::pair<bool, int>(true, const_duration);
        } else if(dur_type == FrequencyDur) {
            return freq_duration_forward(start, day);
        } else {
            return tt_duration_forward(start, day);
        }
    }
}

std::pair<bool, int> DurationPT::min_duration() const
{
    if(const_duration < 0) {
        std::cerr << "WARNING CONST DURATION < 0\n";
        return std::pair<bool, int>(false, const_duration);
    }
    return std::pair<bool, int>(true, const_duration);
}



std::pair<bool, int> DurationPT::freq_duration_forward(float start_time, int day, int allowed_lookups) const
{
    bool has_traffic = false;
    int cost = -1;
    
    int f_start, f_arrival, f_duration;
    Services s;
    
    for(int i=0 ; i< (int) frequencies.size() ; ++i)
    {
        boost::tie(f_start, f_arrival, f_duration, s) = frequencies[i];
        if(start_time >= f_start && start_time < f_arrival)
        {
            has_traffic = true;
            cost = f_duration;
            break;
        }
    }
    
    //there might be trips on the previous day for first few hours of the current day
    if(start_time < 2*3600 && allowed_lookups & PrevDay)
    {
        std::pair<bool, int> prev_day;
        prev_day = freq_duration_forward(start_time + 24*3600, day-1, PrevDay);
        prev_day.second -= 24*3600;
        
        if((prev_day.first && !has_traffic) || (prev_day.first && prev_day.second < cost))
            return prev_day;
    }
    
    // Start is on the next day, we'd better lookup in this one too
    if(start_time >= 24*3600 && allowed_lookups & NextDay)
    {   
        std::pair<bool, int> next_day;
        next_day = freq_duration_forward(start_time - 24*3600, day+1, NextDay);
        next_day.second += 24*3600;
        
        if((next_day.first && !has_traffic) || (next_day.first && next_day.second < cost))
            return next_day;
    }
    
    return std::pair<bool, int>(has_traffic, cost);
}

std::pair<bool, int> DurationPT::freq_duration_backward(float start_time, int day, int allowed_lookups) const
{
    bool has_traffic = false;
    int cost = -1;
    
    int f_start, f_arrival, f_duration;
    Services s;
    
    for(int i=0 ; i< (int) frequencies.size() ; ++i)
    {
        boost::tie(f_start, f_arrival, f_duration, s) = frequencies[i];
        if(start_time >= f_start && start_time < f_arrival)
        {
            has_traffic = true;
            cost = f_duration;
            break;
        }
    }
    
    //there might be trips on the previous day for first few hours of the current day
    if(start_time < 2*3600 && allowed_lookups & PrevDay)
    {
        std::pair<bool, int> prev_day;
        prev_day = freq_duration_backward(start_time + 24*3600, day-1, PrevDay);
        prev_day.second -= 24*3600;
        
        if((prev_day.first && !has_traffic) || (prev_day.first && prev_day.second < cost))
            return prev_day;
    }
    
    // Start is on the next day, we'd better lookup in this one too
    if(start_time >= 24*3600 && allowed_lookups & NextDay)
    {   
        std::pair<bool, int> next_day;
        next_day = freq_duration_backward(start_time - 24*3600, day+1, NextDay);
        next_day.second += 24*3600;
        
        if((next_day.first && !has_traffic) || (next_day.first && next_day.second < cost))
            return next_day;
    }
    
    return std::pair<bool, int>(has_traffic, cost);
}


std::pair<bool, int> DurationPT::tt_duration_forward(float start, int day, int allowed_lookups) const
{
    bool has_traffic = false;
    int cost = -1;
    
    uint min = 0;
    uint max = timetable.size()-1;
    uint i = (max + min) /2;
    
    float tt_start, tt_arrival;
    Services s;
    boost::tie(tt_start, tt_arrival, s) = timetable[i];
        
    while(min != max) 
    {
        if(!(tt_start >= start)) 
        {
            min = i+1;
            max = max;
            i = (max + min) /2;   
            boost::tie(tt_start, tt_arrival, s) = timetable[i];
        }
        else 
        {
            if(i==min) 
            {
                break;
            }
            else if(timetable[i-1].get<0>() < start)
            {
                break;
            }
            else
            {
                min = min;
                max = i-1;
                i = (min+max)/2;
                boost::tie(tt_start, tt_arrival, s) = timetable[i];
            }
        }
    }
    if( start > tt_start)
        cost = -1;
    else
    {
        while(i < timetable.size()) {
            boost::tie(tt_start, tt_arrival, s) = timetable[i];
            if(s[day]) 
            {
                has_traffic = true;
                BOOST_ASSERT(tt_arrival >= start);
                cost = tt_arrival - start;
                break;
            }
            ++i;
        }
    }
    
    //there might be trips on the previous day for first few hours of the current day
    if(start < 2*3600 && allowed_lookups & PrevDay)
    {
        std::pair<bool, int> prev_day;
        prev_day = tt_duration_forward(start + 24*3600, day-1, PrevDay);
        prev_day.second -= 24*3600;
        
        if((prev_day.first && !has_traffic) || (prev_day.first && prev_day.second < cost))
            return prev_day;
    }
    
    // Start is on the next day, we'd better lookup in this one too
    if(start >= 24*3600 && allowed_lookups & NextDay)
    {   
        std::pair<bool, int> next_day;
        next_day = tt_duration_forward(start - 24*3600, day+1, NextDay);
        next_day.second += 24*3600;
        
        if((next_day.first && !has_traffic) || (next_day.first && next_day.second < cost))
            return next_day;
    }

    return std::pair<bool, int>(has_traffic, cost);
}

std::pair<bool, int> DurationPT::tt_duration_backward(float start, int day, int allowed_lookups) const
{
    bool has_traffic = false;
    int cost = -1;
    
    int min = 0;
    int max = timetable.size()-1;
    int i = (max + min) /2;
    
    float tt_start, tt_arrival;
    Services s;
    boost::tie(tt_start, tt_arrival, s) = timetable[i];
        
    while(min != max) 
    {
        if(!(tt_arrival <= start)) 
        {
            min = min;
            max = i;
            i = (max + min) /2;   
            BOOST_ASSERT(i < (int) timetable.size());
            boost::tie(tt_start, tt_arrival, s) = timetable[i];
        }
        else 
        {
            BOOST_ASSERT(i < (int) timetable.size());
            if(i==max) 
            {
                break;
            }
            else if(timetable[i+1].get<1>() > start)
            {
                break;
            }
            else
            {
                min = i+1;
                max = max;
                i = (min+max)/2;
                BOOST_ASSERT(i < (int) timetable.size());
                boost::tie(tt_start, tt_arrival, s) = timetable[i];
            }
        }
    }
    if(tt_arrival > start)
        cost = -1;
    else
    {
        while(i >= 0) {
            BOOST_ASSERT(i < (int) timetable.size());
            boost::tie(tt_start, tt_arrival, s) = timetable[i];
            if(s[day]) 
            {
                has_traffic = true;
                BOOST_ASSERT(start >= tt_start);
                cost = start - tt_start;
                break;
            }
            --i;
        }
    }
    
    //there might be trips on the previous day for first few hours of the current day
    if(start < 2*3600 && allowed_lookups & PrevDay)
    {
        std::pair<bool, int> prev_day;
        prev_day = tt_duration_backward(start + 24*3600, day-1, PrevDay);
        prev_day.second -= 24*3600;
        
        if((prev_day.first && !has_traffic) || (prev_day.first && prev_day.second < cost)) 
            return prev_day;
    }
    
    // Start is on the next day, we'd better lookup in this one too
    if(start >= 24*3600 && allowed_lookups & NextDay)
    {   
        std::pair<bool, int> next_day;
        next_day = tt_duration_backward(start - 24*3600, day+1, NextDay);
        next_day.second += 24*3600;
        
        if((next_day.first && !has_traffic) || (next_day.first && next_day.second < cost))
            return next_day;
    }

    return std::pair<bool, int>(has_traffic, cost);
}

void DurationPT::set_min()
{
    if(dur_type == ConstDur) {
        return;
    } else if(dur_type == FrequencyDur) {
        int min_dur = 999999;
        int f_start, f_arrival, f_duration;
        Services s;
        
        for(uint i=0 ; i< frequencies.size() ; ++i)
        {
            boost::tie(f_start, f_arrival, f_duration, s) = frequencies[i];
            if(f_duration < min_dur)
                min_dur = f_duration;
        }
        
        const_duration = min_dur;
        
    } else if(dur_type == TimetableDur) {
        float tt_start, tt_arrival;
        Services s;
        float min_dur = 99999999;
        
        for(uint i=0 ; i < timetable.size() ; ++i) {
            boost::tie(tt_start, tt_arrival, s) = timetable[i];
            if(tt_arrival - tt_start < min_dur)
                min_dur = tt_arrival - tt_start;
        }
        
        const_duration = min_dur;
    }
    
}

