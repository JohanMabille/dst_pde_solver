#include "rates.hpp"
#include <cmath>
#include <limits>
#include <algorithm>
#include <cstdlib>

namespace dauphine
{
rates::rates()
{
}

//Declare the fonction for the rates
double rates::get_rates(std::vector<double> arguments) const
{
    //arguments[0]: Use in return if the rates is path-dependent (depend on spot S)
    //arguments[1]: Use in return if the rates is time-dependent (depend on time t)
    
    return 0.0; //Constant rate (equal to 0.0)
}
rates::~rates()
{
}
}

