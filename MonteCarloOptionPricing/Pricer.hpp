//
// Pricer.hpp
//
// Calculate the price of the option
// PayoffFunction, DiscountingFactor and KnockFunction are used to configure the Pricers
//
//	One Base class : IPricer
//  Three Derived classes : EuropeanPricer, AsianPricer and BarrierPricer
//
//
//


#ifndef PRICER_HPP
#define PRICER_HPP

#include<vector>
#include<algorithm>
#include<iostream>
#include<functional>
#include<iomanip>	//format output

// The payoff function - input a double(Stock price) and return a double(the payoff)
using PayoffFunction = std::function<double(const double&)>;


//Abstract Base(Interface) Pricer class
class IPricer
{
protected:
	PayoffFunction m_payoff;			//Payoff function
	double m_discounter;				//discounting factor
	double m_price;						//to store the final price
	double m_squaredpayoff;				//used for standard deviation calculation
	double m_sum;						//sum of all the simulations
	int m_NSim;							//number of simulations
public:
	//Constructor
	IPricer(PayoffFunction payoff, double discounter)
		: m_payoff(payoff), m_discounter(discounter), m_price(0.0), m_squaredpayoff(0.0), m_sum(0.0), m_NSim(0) {}

	//Pure Virtual Functions
	virtual void ProcessPath(const std::vector<double>& arr) = 0; // Process the payoff and increase NSim each time
	virtual void PostProcess() = 0;								 // print final results

																 //Getters (Template Method Pattern)
	virtual double DiscountFactor() const final
	{// return discounting factor
		return m_discounter;
	}
	virtual double Price() const final
	{// return the option price
		return m_price;
	}
};


/////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////


//Concrete Derived Pricer class : European Option Pricer
class EuropeanPricer : public IPricer
{
public:
	//Constrcuctor
	EuropeanPricer(PayoffFunction payoff, double discounter) : IPricer(payoff, discounter) {}

	//Derived Functions
	virtual void ProcessPath(const std::vector<double>& arr) override
	{// Process the payoff and increase NSim each time

		double current_payoff = m_payoff(arr.back());			//call payoff function to get current payoff base on the ending price

		m_squaredpayoff += (current_payoff * current_payoff);	//squared sum, used for standard deviation calculation

		m_sum += current_payoff;	// accumulate the sum

		m_NSim++;		//increase the number of simulations after each call
	}

	virtual void PostProcess() override
	{//Calculating the final price

		double payoff = m_sum / (m_NSim*1.0);	//Averaged future value of the payoff
		m_price = DiscountFactor() * payoff;	//present value(price)

												//Note: VAR(x) = sum(xi*xi)/N - (avg)^2.
		double sd = std::sqrt((m_squaredpayoff / (m_NSim*1.0)) - payoff*payoff);	//standard deviation
		double se = sd / std::sqrt(m_NSim);		//standard error

												//print the result
		std::cout << std::showpoint << std::setprecision(6) << std::fixed		//format the output
			<< "European Option Post Process - Final Price = " << m_price
			<< ", Standard Deviation = " << sd << ", Standard Error = " << se << std::endl;
	}
};


/////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////

//Asian pricer's average function
using AverageFunction = std::function<double(const std::vector<double>&)>;

//Concrete Derived Pricer class : Asian Option Pricer
//Base on the average(arithmetic or geometric) price instead of the ending price
class AsianPricer : public IPricer
{
private:
	double m_average;				//average price
	AverageFunction m_avgfunc;		//Assign Asian Pricer's average function(Geometric or Arithmetic)
public:
	//Constructor
	AsianPricer(PayoffFunction payoff, double discounter, AverageFunction avgfunc)
		: IPricer(payoff, discounter), m_avgfunc(avgfunc), m_average(0.0) {}

	virtual void ProcessPath(const std::vector<double>& arr) override
	{// Process the payoff and increase NSim each time

		m_average = m_avgfunc(arr);		//call payoff function to get the average price over the period

		double current_payoff = m_payoff(m_average);		//call payoff function to get current payoff base on the average price

		m_squaredpayoff += (current_payoff * current_payoff);	//squared sum, used for standard deviation calculation

		m_sum += current_payoff;	//accumulate the sum

		m_NSim++;			//increase the number of simulations after each call
	}

	virtual void PostProcess() override
	{//Calculating the final price

		double payoff = m_sum / (m_NSim*1.0);	//average future value of the payoff
		m_price = DiscountFactor() * payoff;	//present value(price)

												//Note: VAR(x) = sum(xi*xi)/N - (avg)^2.
		double sd = std::sqrt((m_squaredpayoff / (m_NSim*1.0)) - payoff*payoff);	//standard deviation
		double se = sd / std::sqrt(m_NSim);			//standard error

													//print the result
		std::cout << std::showpoint << std::setprecision(6) << std::fixed		//format the output
			<< "Asian Option Post Process - Final Price = " << m_price
			<< ", Standard Deviation = " << sd << ", Standard Error = " << se << std::endl;
	}
};


/////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////

//return bool from a vector to show whether knock in or knock out
using KnockFunction = std::function<bool(const std::vector<double>&)>;

//Concrete Derived Pricer class : Barrier Option Pricer
class BarrierPricer : public IPricer
{
private:
	KnockFunction m_knock;
public:
	//Constructor
	BarrierPricer(PayoffFunction payoff, double discounter, KnockFunction knock)
		: IPricer(payoff, discounter), m_knock(knock) {}

	virtual void ProcessPath(const std::vector<double>& arr) override
	{
		//if not knocked out(if return false), there will be payoff
		if (!m_knock(arr))
		{
			double current_payoff = m_payoff(arr.back());			//call payoff function base on the ending price
			m_squaredpayoff += (current_payoff * current_payoff);	//used for standard deviation calculation
			m_sum += current_payoff;								//accumulate the sum
		}

		m_NSim++;		//increase the number of simulations after each call
	}

	virtual void PostProcess() override
	{//Calculating the final price

		double payoff = m_sum / (m_NSim*1.0);	//average future value of the payoff
		m_price = DiscountFactor() * payoff;	//present value(price)

												//Note: VAR(x) = sum(xi*xi)/N - (avg)^2.
		double sd = std::sqrt((m_squaredpayoff / (m_NSim*1.0)) - payoff*payoff);	//standard deviation
		double se = sd / std::sqrt(m_NSim);			//standard error

													//print the result
		std::cout << std::showpoint << std::setprecision(6) << std::fixed		//format the output
			<< "Barrier Option Post Process - Final Price = " << m_price
			<< ", Standard Deviation = " << sd << ", Standard Error = " << se << std::endl;
	}
};

#endif