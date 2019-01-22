#ifndef payoff_hpp
#define payoff_hpp

#include <vector>


namespace dauphine
{
    class payoff
    {
        public:
            explicit payoff();
	    double get_payoff(std::vector<double> arguments) const;
            virtual ~payoff();
       
    };

}
#endif 
