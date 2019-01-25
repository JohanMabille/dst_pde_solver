#ifndef payoff_hpp
#define payoff_hpp

#include <vector>


namespace dauphine
{
    class payoff
    {
        public:
            explicit payoff();
            virtual double get_payoff(const double& time, const double& spot) const;
            virtual ~payoff();
       
    };
    
    class bs_call: public payoff
    {
    public:
        explicit bs_call(const double& strike);
        virtual double get_payoff(const double& time, const double& spot) const;
        virtual ~bs_call();
    private:
        double m_strike;
    };

}
#endif 
