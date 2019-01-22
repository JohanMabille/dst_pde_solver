#include <iostream>
#include "closed_form.hpp"
#include "solver.hpp"
#include "volatility.hpp"
#include "rates.hpp"
#include "payoff.hpp"
//#include "solverT.hpp"
//#include "params.hpp"
//#include "mesh.hpp"
#include <algorithm>
#include <cmath>
#include <limits>

namespace dauphine
{	
	//Initial functions to be changed according to the user's needs
	//modify this function to modify the payoff, arguments=[spot,maturity,mesh_boundaries_up,mesh_boundaries_down,theta]
	
	double payoff_function(std::vector<double> arguments) {
		return std::max(arguments[0]-100,0.);
	}
	/*double rate_function(std::vector<double> arguments) {
		return 0.0;
	} */
	/*double volatility_function(std::vector<double> arguments) {
		return 0.20;
	} */
	//double boundaries_up(std::vector<double> arguments) {
	//	return  std::max(arguments[2] - 100, 0.);
	//}
	//double boundaries_down(std::vector<double> arguments) {
	//	return std::max(arguments[3] - 100, 0.);
	//}

	void test()
	{
		// make sure the arguments of the payoff are properly defined
		int number_arguments(5);
		double spot=100.;
		double maturity=1.;
		

		//the client should be able to specify the value of θ
		double theta = 0.5;

		std::vector<double> mesh_boundaries(2);
		mesh_boundaries[0] = std::exp(std::log(spot) - 1.);
		mesh_boundaries[1] = std::exp(std::log(spot) + 1.);
		
		initial_function payoff(payoff_function);
		//initial_function rate(rate_function);
       
		//initial_function volatility(volatility_function);
		//initial_function up_boundaries(boundaries_up);
		//initial_function down_boundaries(boundaries_down);

		//mesh(double dt, double dx, double maturity, double spot, std::vector<double> boundaries, double theta)
		
        //Nbr of points
        int nb_x = 1001;
		
        //Creation mesh
	//the client should be able to specify to specify the mesh
	//Mesh(double dt, double dx,double maturity(in years), double spot,boundaries)


        mesh m(1./365.,1001,1.,100.,mesh_boundaries);
        
        //Creation vol
        volatility vol;
        
	//Creation rates
        rates rate;

	//Creation payoff
        //payoff p;



        //Compute price
		std::vector<double> result = price_today(theta,m,rate,vol,payoff,false);
		
        // Spot index
        int i = (nb_x - 1) / 2;
        
        //Greeks
		double price = result[i];
		double delta = (result[i] - result[i-1]) / (m.spot_vect[i] - m.spot_vect[i-1]);
		double gamma = (result[i + 1] - 2 * result[i] + result[i - 1]) / (pow((m.spot_vect[i+1] - m.spot_vect[i-1])/2.0, 2));
		double theta_product = (result[0]-price);
		// for the vega, we should have a discretization w.r.t the volatility
        
        //Print results (Price and Greeks)
		std::cout << "Price: " <<price << std::endl;
		std::cout << "Delta: " << delta << std::endl;
		std::cout << "Gamma: " << gamma << std::endl;
		std::cout << "Theta: " << theta_product << std::endl;

	}
}


int main(int argc, char* argv[])
{
    std::cout <<"Price BS: " << dauphine::bs_price(100,100,0.20,1.0,true) << std::endl;
	dauphine::test();
    return 0;
}
