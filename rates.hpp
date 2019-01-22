#ifndef rates_hpp
#define rates_hpp

#include <vector>

namespace dauphine
{
    class rates
    {
        public:
            explicit rates();
	    double get_rates(std::vector<double> arguments) const;
            virtual ~rates();
       
    };

}

#endif 
