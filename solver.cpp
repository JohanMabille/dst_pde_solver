#include "solver.hpp"
#include "volatility.hpp"
#include <cmath>
#include <limits>
#include "rates.hpp"
#include "payoff.hpp"
#include <algorithm>
#include <cstdlib>
namespace dauphine
{

	mesh::mesh(double dt, double dx, double maturity, double spot, std::vector<double> boundaries)
		: m_dt(dt), m_dx(dx), m_maturity(maturity), m_spot(spot), m_spot_boundaries(boundaries)
	{
		std::vector<double> result(dx,0.);
		double inter_t =floor( maturity / dt);
		std::vector<double> result2(inter_t);
		double log_spot_min = std::log(boundaries[0]);
		double log_spot_max = std::log(boundaries[1]);
		double dlog = (log_spot_max - log_spot_min) /( dx-1);

		for (std::size_t i = 0; i < dx; ++i)
		{
			result[i] = std::exp(log_spot_min + i * dlog);
		}
		spot_vect = result;
		d_x = dlog;

		for (std::size_t i = 0; i < inter_t; ++i)
		{
			result2[i] = maturity-i*dt;
		}
		t_vect = result2;
	}
	mesh::~mesh()
	{
	}
	double const mesh::get_mesh_maturity() const {
		return m_maturity;
	}
	double const mesh::get_mesh_dt() const {
		return m_dt;
	}
	double const mesh::get_mesh_dx() const {
		return m_dx;
	}
	double const mesh::get_mesh_spot() const {
		return m_spot;
	}


	initial_function::initial_function(double(*f)(std::vector<double>))
		: m_f(f)
	{
	}
	double initial_function::function_operator(std::vector<double> arguments)
	{
		return m_f(arguments);
	}

	initial_function::~initial_function()
	{
	}

	// AUTRE METHODE - TEST
	// ici je compute le vecteur ‡ la maturitÈ pour avoir le prix ‡ matu et pouvoir faire backward, donc c'est juste appliquÈ le payoff pour le spot

	std::vector<double> initial_price_vector(mesh m, initial_function payoff) {
		std::vector<double> result = m.spot_vect;
		for (std::size_t i = 0; i < result.size(); i++) {
			std::vector<double> result2(1);
			result2[0] = result[i];
			if (payoff.function_operator(result2) == 0) {
				result[i] = 0;
			}
			else {
				result[i] = payoff.function_operator(result2);
			}
		}
		return result;
	}

	std::vector<double> diag_vector(mesh m, rates rate, volatility vol, std::vector<double> arguments, double theta)
	{
		std::vector<double> a = m.spot_vect;
		long size = a.size();
		std::vector<double> result(size, 0.0);
		result[0] = 1.0;
		result[size - 1] = 1.0;
		for (std::size_t i = 1; i < size - 1; ++i)
		{
			arguments[0] = a[i]; // coeff depends on S if rate or vol depend on S
			result[i] = 1.0 + theta * m.get_mesh_dt()*((pow(vol.get_volatility(arguments), 2) / pow(m.d_x, 2)) + rate.get_rates(arguments));
		}
		return result;
	}


	std::vector<double> sub_vector(mesh m, rates rate, volatility vol, std::vector<double> arguments,double theta)
	{
		std::vector<double> a = m.spot_vect;
		long size = a.size();
		std::vector<double> result(size, 0.0);
		result[size - 1] = 0;
		result[0] = 0;
		for (std::size_t i = 1; i < size - 1; ++i)
		{
			arguments[0] = a[i]; // coeff depends on S if rate or vol depend on S
			result[i] = -0.5*theta * m.get_mesh_dt()*((pow(vol.get_volatility(arguments), 2) / pow(m.d_x, 2)) + ((pow(vol.get_volatility(arguments), 2) - rate.get_rates(arguments)) / (2.0*m.d_x)));
		}
		return result;
	}


	std::vector<double> up_vector(mesh m, rates rate, volatility vol, std::vector<double> arguments,double theta)
	{
		std::vector<double> a = m.spot_vect;
		long size = a.size();
		std::vector<double> result(size, 0.0);
		result[0] = 0.0;
		result[size - 1] = 0.0;
		for (std::size_t i = 1; i < size - 1; ++i)
		{
			arguments[0] = a[i]; // coeff depends on S if rate or vol depend on S
			result[i] = 0.5*theta * m.get_mesh_dt()*((-pow(vol.get_volatility(arguments), 2) / pow(m.d_x, 2)) + ((pow(vol.get_volatility(arguments), 2) - rate.get_rates(arguments)) / (2.0*m.d_x)));
		}
		return result;
	}

	// Triadiag algo qui fonctionne !
	std::vector<double> tridiagonal_solver(std::vector<double>  a, std::vector<double>  b, std::vector<double>  c, std::vector<double>  f)
	{

		long n = f.size();
		std::vector<double> x(n);
		x[0] = f[0];
		for (int i = 1; i < n; i++) {

			double m = a[i] / b[i - 1];
			b[i] -= m*c[i - 1];
			f[i] -= m*f[i - 1];
		}
		// solve for last x value
		x[n - 1] = f[n - 1] / b[n - 1];

		// solve for remaining x values by back substitution
		for (int i = n - 2; i >= 0; i--)
			x[i] = (f[i] - c[i] * x[i + 1]) / b[i];

		return x;

	}

	std::vector<double> price_today(double theta, mesh m, rates rate, volatility vol,  initial_function payoff, bool time_S_dependent)
	{
		// arguments allow to follow S,t and 
		std::vector<double> arguments(2);
		arguments[0] = m.spot_vect[0];
		arguments[1] = m.t_vect[0];

		std::vector<double> f_old = initial_price_vector(m, payoff);
		long N = f_old.size();
		std::vector<double> d(N, 0.0);
		std::vector<double> f_new(N, 0.0);
		double dt = m.get_mesh_dt();
		int nb_step = floor(m.get_mesh_maturity()/ dt);

		//Coeffs de la Matrice
		std::vector<double> a_1 = sub_vector(m, rate, vol, arguments,theta-1); //a(theta-1)
		std::vector<double> b_1 = diag_vector(m, rate, vol, arguments, theta - 1); //b(theta-1)
		std::vector<double> c_1 = up_vector(m, rate, vol, arguments, theta - 1); //c(theta-1)
		std::vector<double> a = sub_vector(m, rate, vol, arguments,theta); //a(theta)
		std::vector<double> b = diag_vector(m, rate, vol, arguments,theta); //b(theta)
		std::vector<double> c = up_vector(m, rate, vol, arguments,theta); //c(theta)
		std::vector<double> f_before(N);
		
        //Condition aux bords (Test pour un call)
		d[N - 1] = f_old[N - 1]; //Smax
		d[0] = f_old[0];

		//return c_1;
		for (int j = 0; j < nb_step; j++)
		{	
			if (time_S_dependent) {
				std::vector<double> a_1 = sub_vector(m, rate, vol, arguments, theta - 1); //a(theta-1)
				std::vector<double> b_1 = diag_vector(m, rate, vol, arguments, theta - 1); //b(theta-1)
				std::vector<double> c_1 = up_vector(m, rate, vol, arguments, theta - 1); //c(theta-1)
				arguments[1] = m.t_vect[j]; //on modifie le temps pour changer les calculs de rate et vol si besoin
				std::vector<double> a = sub_vector(m, rate, vol, arguments, theta); //a(theta)
				std::vector<double> b = diag_vector(m, rate, vol, arguments, theta); //b(theta)
				std::vector<double> c = up_vector(m, rate, vol, arguments, theta); //c(theta)
			}

			// Creation 2nd membre
			d[N - 1] = f_old[N - 1] * exp(-rate.get_rates(arguments)*m.get_mesh_dt());
			for (long i = 1; i < N - 1; i++)
			{
				d[i] = c_1[i] * f_old[i + 1] + b_1[i] * f_old[i] + a_1[i] * f_old[i - 1];

			}
			if (j == nb_step - 1) { // I keep the value at the before last step, to compute the theta
				f_before = f_new;
			}
			// Now we solve the tridiagonal system
			f_new = tridiagonal_solver(a, b, c, d);
			f_old = f_new;
		}
		f_new[0] = f_before[floor(N / 2)];// to compute the theta, not very academic
		return f_new;
	}
}
    
    



