#include "solver.hpp"
#include "mesh.hpp"
#include "volatility.hpp"
#include "boundaries.hpp"
#include <cmath>
#include <limits>
#include "rates.hpp"
#include "payoff.hpp"
#include <algorithm>
#include <cstdlib>
namespace dauphine
{

	// ici je compute le vecteur ‡ la maturitÈ pour avoir le prix ‡ matu et pouvoir faire backward, donc c'est juste appliquÈ le payoff pour le spot

	std::vector<double> initial_price_vector(const mesh& m, const payoff& p, const rates& rate) {
		std::vector<double> result(m.spot_vect.size());
        // The terminal condition is on the spot and at t = maturity, fwd = spot, therefore you must use
        // the spot mesh to compute the payoff
        std::transform(m.spot_vect.cbegin(), m.spot_vect.cend(), result.begin(), [&p](double s) { return p.get_payoff(s); });
		return result;
	}

    // Compute coeffs Matrix
	std::vector<double> diag_vector(const mesh& m, const rates& rate, const volatility& v, const double& time, double spot, const double& theta)
	{
        // Why not using m.spot_vect instrad of making a (useless) copy?
		std::vector<double> a = m.spot_vect;
		long size = a.size();
		std::vector<double> result(size, 0.0);
        // First and last values depend on the boundary conditions
		result[0] = 1.0;
		result[size - 1] = 1.0;
		for (std::size_t i = 1; i < size - 1; ++i)
		{
			spot = a[i]; // coeff depends on S if rate or vol depend on S
			result[i] = 1.0 + theta * m.get_mesh_dt()*((pow(v.get_volatility(time, spot), 2) / pow(m.d_x, 2)) + rate.get_rates(time,spot));
		}
		return result;
	}


	std::vector<double> sub_vector(const mesh& m, const rates& rate, const volatility& v,const double& time, double spot, const double& theta)
	{
        // Why not using m.spot_vect instrad of making a (useless) copy?
		std::vector<double> a = m.spot_vect;
		long size = a.size();
		std::vector<double> result(size, 0.0);
        // First and last values depend on the boundary conditions
		result[size - 1] = 0;
		result[0] = 0;
		for (std::size_t i = 1; i < size - 1; ++i)
		{
			spot = a[i]; // coeff depends on S if rate or vol depend on S
            // This is due to a typo in the subject, so this not considered as a mistake
			result[i] = -theta * m.get_mesh_dt()*(0.5 * (pow(v.get_volatility(time,spot), 2) / pow(m.d_x, 2)) + ((0.5 * pow(v.get_volatility(time,spot), 2) - rate.get_rates(time,spot)) / (2.0*m.d_x)));
		}
		return result;
	}


	std::vector<double> up_vector(const mesh& m, const rates& rate, const volatility& v, const double& time, double spot, const double& theta)
	{
        // Why not using m.spot_vect instrad of making a (useless) copy?
		std::vector<double> a = m.spot_vect;
		long size = a.size();
		std::vector<double> result(size, 0.0);
        // First and last values depend on the boundary conditions
		result[0] = 0.0;
		result[size - 1] = 0.0;
		for (std::size_t i = 1; i < size - 1; ++i)
		{
			spot= a[i]; // coeff depends on S if rate or vol depend on S
            // This is due to a typo in the subject, so this not considered as a mistake
			result[i] = theta * m.get_mesh_dt()*(0.5 * (-pow(v.get_volatility(time,spot), 2) / pow(m.d_x, 2)) + ((0.5 * pow(v.get_volatility(time,spot), 2) - rate.get_rates(time,spot)) / (2.0*m.d_x)));
		}
		return result;
	}

	// Triadiag algo qui fonctionne !
    // f should be taken by reference so you can solve in place
	std::vector<double> tridiagonal_solver(const std::vector<double>&  a, std::vector<double>  b,const std::vector<double>&  c, std::vector<double>  f)
	{
		long n = f.size();
		std::vector<double> x(n);
		x[0] =f[0];
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

    //Compute result price vector
	std::vector<double> price_today(const double& theta,const mesh& m, const rates& rate, const volatility& v,const  payoff& p, const boundaries& bnd_down, const boundaries& bnd_up, const bool& time_S_dependent)
	{
		// arguments allow to follow S,t and 
		double time = m.t_vect[0];
		double spot = m.spot_vect[0];
    
        // arguments is never used
		std::vector<double> arguments(2);
		arguments[0] = m.spot_vect[0];
		arguments[1] = m.t_vect[0];

		std::vector<double> f_old = initial_price_vector(m, p,rate);
		long N = f_old.size();
		std::vector<double> d(N, 0.0);
		std::vector<double> f_new(N, 0.0);
		double dt = m.get_mesh_dt();
		int nb_step = floor(m.get_mesh_maturity()/ dt);

		//Coeffs de la Matrice
        // Since you know whether the coefficient are time dependent, you should pass this information
        // to the following functions so you can avoid computing the same coefficient many times
		std::vector<double> a_1 = sub_vector(m, rate, v, time,spot,theta-1); //a(theta-1)
		std::vector<double> b_1 = diag_vector(m, rate, v, time,spot, theta - 1); //b(theta-1)
		std::vector<double> c_1 = up_vector(m, rate, v, time,spot, theta - 1); //c(theta-1)
		std::vector<double> a = sub_vector(m, rate, v, time,spot,theta); //a(theta)
		std::vector<double> b = diag_vector(m, rate, v, time,spot,theta); //b(theta)
		std::vector<double> c = up_vector(m, rate, v, time,spot,theta); //c(theta)
		std::vector<double> f_before(N);
		
        //Condition aux bords (Test pour un call)
        d[N - 1] = bnd_up.get_boundaries(f_old[N - 1],time,spot,rate,m);
		d[0] = bnd_down.get_boundaries(f_old[0], time, spot, rate, m); //boundary down

		//return c_1;
		for (int j = 0; j < nb_step; j++)
		{	
            // If the coeffient are time dependent, you are allocating new vectors
            // at each iteration, which is really inefficient. It would habe been better
            // to pass the coefficient vector as non constant references to the functions
            // (the resizing is done once before the loop).
			if (time_S_dependent) {
				std::vector<double> a_1 = sub_vector(m, rate, v, time, spot, theta - 1); //a(theta-1)
				std::vector<double> b_1 = diag_vector(m, rate, v, time, spot, theta - 1); //b(theta-1)
				std::vector<double> c_1 = up_vector(m, rate, v, time, spot, theta - 1); //c(theta-1)
				arguments[1] = m.t_vect[j]; //on modifie le temps pour changer les calculs de rate et vol si besoin
				std::vector<double> a = sub_vector(m, rate, v, time, spot, theta); //a(theta)
				std::vector<double> b = diag_vector(m, rate, v, time, spot, theta); //b(theta)
				std::vector<double> c = up_vector(m, rate, v, time, spot, theta); //c(theta)
			}

			// Creation 2nd membre
            d[N - 1] = bnd_up.get_boundaries(f_old[N - 1],time,spot,rate,m);
            d[0]=bnd_down.get_boundaries(f_old[0], time, spot, rate, m);
            for (long i = 1; i < N - 1; i++)
			{
				d[i] = c_1[i] * f_old[i + 1] + b_1[i] * f_old[i] + a_1[i] * f_old[i - 1];

			}

			if (j == nb_step - 1) { // I keep the value at the before last step, to compute the theta
                // Why storing a full vector if you only use one value?
				f_before = f_new;
			}
			// Now we solve the tridiagonal system
			f_new = tridiagonal_solver(a, b, c, d);
			f_old = f_new;
		}
        // This does not allow you to compute the theta for all the spots.
        // Exposing both f_old and f_new would have been a better solution.
		f_new[0] = f_before[floor(N / 2)];// to keep to compute the theta, not very academic

		return f_new;
	}
}
    
    



