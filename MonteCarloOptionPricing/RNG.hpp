//
// RNG.hpp
//
// Random Number Generator for the FDM.
// Use STL distribution and random devices to generate random numbers with a Standard Normal Distribution(0,1)
//
// One Base class : IRNG
// Three Derived classes : Mersenne Twister on Normal Distribution, BoxMuller, and PolarMarsaglia
//
//
//

#ifndef RNG_HPP
#define RNG_HPP

#include<random>									//for STL Distributions
#include<boost\math\constants\constants.hpp>		//for pi
#include<iostream>
#include<functional>

//universal function wrapper for generating random numbers
using RNGFunction = std::function<double()>;

//Abstract Base(Interface) RNG class
class IRNG
{
protected:
	RNGFunction rng;           //function wrapper
public:
	//Default Constructor
	IRNG() {};

	//make function call - Using universal function wrapper
	//this performs the same functionality as pure virtual function
	virtual double GenerateRng() final
	{
		return rng();
	}
};

class MTNormalRNG : public IRNG
{
private:
	std::mt19937 mt;
	std::normal_distribution<double> normal;
public:
	//Constructor
	MTNormalRNG(double v1, double v2) : IRNG()
	{
		normal = std::normal_distribution<double>(v1, v2);
		rng = [&]() { return normal(mt); }; // specify the function implementation
	}
};
/////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////


//Concrete Derived RNG class : Box Muller Method
class BoxMullerRNG : public IRNG
{
private:
	std::default_random_engine eng;	//random engine
	std::uniform_real_distribution<double> uniform;	//uniform distribution(0,1)
public:
	//Constructor
	BoxMullerRNG() : IRNG()
	{
		eng = std::default_random_engine();
		uniform = std::uniform_real_distribution<double>(0.0, 1.0);	//uniform distribution(0,1)

																	// r and phi are independent uniform random numbers in (0,1)
		rng = [&]()
		{
			double r = uniform(eng);
			double phi = uniform(eng);

			//This will return a number with the standard normal distribution
			return std::sqrt(-2.0*std::log(r))*std::cos(2.0 * boost::math::constants::pi<double>()*phi);
		};
	}

};


/////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////


//Concrete Derived RNG class : Polar Marsaglia Method
class PolarMarsagliaRNG : public IRNG
{
private:
	std::default_random_engine eng;					//random engine
	std::uniform_real_distribution<double> uniform;	//uniform distribution(0,1)
public:
	//Constructor
	PolarMarsagliaRNG() : IRNG()
	{
		eng = std::default_random_engine();
		uniform = std::uniform_real_distribution<double>(0.0, 1.0);

		rng = [&]()
		{
			double V1, V2, W;												//Loop until W is in between of 0 and 1
			do
			{
				V1 = 2.0 * uniform(eng) - 1.0;	//V1 is (-1,1)
				V2 = 2.0 * uniform(eng) - 1.0;	//V2 is(-1,1)
				W = V1 * V1 + V2 * V2;	//W = V1^2 + V2^2

			} while (W > 1.0);	//Stop when W is smaller than 1

			double Y = std::sqrt(-2.0 *std::log(W) / W);

			//This will return a number with the standard normal distribution
			return V1 * Y;
		};
	}
};

#endif

