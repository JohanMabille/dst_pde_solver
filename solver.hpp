#ifndef SOLVER_HPP
#define SOLVER_HPP
#include "volatility.hpp"
#include "rates.hpp"
#include "payoff.hpp"

#include <vector>

namespace dauphine
{
	class mesh {
	public:
		//mesh();
		mesh(double dt, double dx,double maturity, double spot, std::vector<double> spot_boundaries);
		double const get_mesh_dt() const;
		double const get_mesh_maturity() const;
		double const get_mesh_dx() const;
		double const get_mesh_spot() const;
        std::vector<double> spot_vect;
		std::vector<double> t_vect;
        double d_x;
		~mesh();
	private:
		double m_dt;
		double m_dx;
		double m_maturity;
		double m_spot;
		std::vector<double> m_spot_boundaries; 
	};


	//pour initialiser payoff, rate, vol, boundaries
	class initial_function {
	public:
		initial_function(double(*f)(std::vector<double>));
		double function_operator(std::vector<double> arguments);
		~initial_function();
	private:
		double(*m_f)(std::vector<double>);
	};

	//double diag_coeff(mesh m, initial_function rate, initial_function vol, std::vector<double> arguments, double theta);
	//double subdiag_coeff(mesh m, initial_function rate, initial_function vol, std::vector<double> arguments,double theta);
	//double updiag_coeff(mesh m, initial_function rate, initial_function vol, std::vector<double> arguments,double theta);
	std::vector<double> initial_price_vector(mesh m, initial_function rate, initial_function vol, std::vector<double> arguments,initial_function payoff);
	std::vector<double> column_up(mesh m, initial_function rate, initial_function vol, std::vector<double> arguments, initial_function payoff,std::vector<double> up_price);
//	std::vector<double> price_vector(mesh m, initial_function rate, initial_function vol, std::vector<double> arguments, initial_function payoff, std::vector<double> col_up);
	std::vector<double> price_today(double theta, mesh m, rates rate, volatility vol,  initial_function payoff, bool time_S_dependent);

  
    // Autre methode - TEST
    
    std::vector<double> up_vector(mesh m, rates rate,volatility vol, std::vector<double> arguments);
    std::vector<double> sub_vector(mesh m, rates rate,volatility vol, std::vector<double> arguments);
    std::vector<double> diag_vector(mesh m, rates rate,volatility vol, std::vector<double> arguments);
    std::vector<double> tridiagonal_solver(std::vector<double>  a, std::vector<double>  b, std::vector<double>  c, std::vector<double>  f);
}

#endif
