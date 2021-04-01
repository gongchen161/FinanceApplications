//
// Mediator.hpp
//
// This is where we put everything together and start calculating the price
//
// One concrete MCMediator is created to perform all kinds of options price claculation
//
//
//

#ifndef MEDIATOR_HPP
#define MEDIATOR_HPP

#include"SDE.hpp"
#include"FDM.hpp"
#include"RNG.hpp"
#include"Pricer.hpp"
#include"Builder.hpp"
#include<tuple>
#include<memory>
#include<functional>
#include<chrono>
#include<boost\signals2\signal.hpp> //for connecting the pricers
#include<boost\bind.hpp>

//For readability
using PricerPointer = std::shared_ptr<IPricer>;

//Concrete Mediator Class
class MCMediator
{
private:
	//Main components
	SDEPointer m_sde;
	FDMPointer m_fdm;
	RNGPointer m_rng;

	// Other MC-related data 
	int m_NSim;											//number of simulations
	std::vector<double> m_result;						//to store the generated price vector
	boost::signals2::signal<void(const std::vector<double>&)> m_path;	//trigger pricer's process path function
	boost::signals2::signal<void()>	m_finish;	//trigger pricer's post process function to print the result
public:
	//Constructor
	MCMediator(BuilderTuple parts, int numberSimulations)
	{
		//Assign the SDE,FDM and RNG from the builder
		m_sde = std::get<0>(parts);
		m_fdm = std::get<1>(parts);
		m_rng = std::get<2>(parts);

		m_NSim = numberSimulations;	//assign the number of simulations

		m_result.resize(m_fdm->m_NT + 1);	//resize the final price vector
	}

	//Add a pricer to the signal
	void AddPricer(PricerPointer p)
	{
		//Connect the pricer's ProcessPath and PostProcess functions to the signals
		m_path.connect(boost::bind(&IPricer::ProcessPath, boost::ref(*p), boost::placeholders::_1));
		m_finish.connect(boost::bind(&(IPricer::PostProcess), boost::ref(*p)));
	}

	//remove a pricer from the signal
	void RemovePricer(PricerPointer p)
	{
		//Remove the functions from signal
		m_path.disconnect(boost::bind(&IPricer::ProcessPath, boost::ref(*p), boost::placeholders::_1));
		m_finish.disconnect(boost::bind(&(IPricer::PostProcess), boost::ref(*p)));
	}

	//Main algorithm
	//Start Price Calculation
	void start()
	{
		double VOld, VNew;
		double percentage_complete = 0;		//for displaying the progress

		std::chrono::time_point <std::chrono::system_clock> start = std::chrono::system_clock::now();		//set timmer to now

		std::cout << "Simulation began...\n";

		for (int i = 1; i <= m_NSim; ++i)
		{
			VOld = m_sde->InitialCondition();	//Initialize VOld with the initial price
			m_result[0] = VOld;					//first price is the initial price

												//generate price on the NT time intervals
			for (int n = 1; n <= (m_fdm->m_NT); n++)
			{
				//calling advance function to generate the price on the next time interval
				VNew = m_fdm->advance(VOld, m_fdm->m_vec.back(), m_fdm->m_k, m_rng->GenerateRng());
				m_result[n] = VNew;	//set the price vector
				VOld = VNew;
			}

			m_path(m_result);	// Send path data to the Pricers

								//display the progress in %
			double completed = i / (m_NSim*1.0);
			if (completed > percentage_complete)
			{
				std::cout << int(completed * 100) << "%.";
				percentage_complete += 0.01;
			}
		}
		std::cout << "\nSimulation completed.\n";

		m_finish();  // Signal the pricers to perform the post process and display the price, SD and SE.

					 //end timer
		std::chrono::time_point <std::chrono::system_clock> end = std::chrono::system_clock::now();
		std::chrono::duration<double> elapsed_seconds = end - start;		//calculatet the runtime
		std::cout << "Whole process took " << elapsed_seconds.count() << "s\n";
	}
};

#endif