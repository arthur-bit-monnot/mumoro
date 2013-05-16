#include <iostream>

#include "LabelSettingAlgo.h"

std::ostream & operator<<(std::ostream & os, const RLC::Label l)
{
    os << "Label[" << l.node.first <<" "<< l.node.second <<" "<< l.time <<" "<< l.cost <<" "<< l.h << "]";
    return os;
}

