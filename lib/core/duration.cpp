
#include "graph_wrapper.h"

Duration::Duration(float d) : const_duration(d), dur_type(ConstDur) { }

Duration::Duration() : const_duration(-1) {}

void Duration::append_timetable(float start, float arrival, const std::string & services)
{
    BOOST_ASSERT(start < arrival);
    BOOST_ASSERT(dur_type == TimetableDur);
    timetable.push_back(Time(start, arrival, Services(services)));
}

void Duration::append_frequency(int start, int end, int duration, const std::string & services)
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

void Duration::sort()
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

std::pair<bool, int> Duration::operator()(float start, int day, bool backward) const
{
    if(backward) {
        if (dur_type == ConstDur) {
            return std::make_pair<bool, int>(true, const_duration);
        } else if(dur_type == FrequencyDur) {
            return freq_duration_backward(start, day);
        } else {
            return tt_duration_backward(start, day);       
        }
    }
    else /** forward **/
    {
        if (dur_type == ConstDur) {
            return std::make_pair<bool, int>(true, const_duration);
        } else if(dur_type == FrequencyDur) {
            return freq_duration_forward(start, day);
        } else {
            return tt_duration_forward(start, day);
        }
    }
}

std::pair<bool, int> Duration::freq_duration_forward(float start_time, int day, int allowed_lookups) const
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
    
    return std::make_pair<bool, int>(has_traffic, cost);
}

std::pair<bool, int> Duration::freq_duration_backward(float start_time, int day, int allowed_lookups) const
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
    
    return std::make_pair<bool, int>(has_traffic, cost);
}


std::pair<bool, int> Duration::tt_duration_forward(float start, int day, int allowed_lookups) const
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

    return std::make_pair<bool, int>(has_traffic, cost);
}

std::pair<bool, int> Duration::tt_duration_backward(float start, int day, int allowed_lookups) const
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

    return std::make_pair<bool, int>(has_traffic, cost);
}

