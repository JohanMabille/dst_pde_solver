#include "payoff.hpp"
#include <cmath>
#include <limits>
#include <algorithm>
#include <cstdlib>

namespace dauphine
{
payoff::payoff()
{
}

//Declare the fonction for the payoff
double payoff::get_payoff(std::vector<double> arguments) const
{
    //arguments[0]: Use in return if the payoff is path-dependent (depend on spot S)
    //arguments[1]: Use in return if the payoff is time-dependent (depend on time t)
    
    return std::max(arguments[0]-100,0.); //Call option with strike price 100
}


payoff::~payoff()
{
}
    
}
