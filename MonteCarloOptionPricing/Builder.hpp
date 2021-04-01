//
// Builder.hpp
//
// Build tuples containing the SDE,FDM and RNG
//
// One Base class : IBuilder
//
// Two Derived classes : MCBuilder(user can customize the selections in run time) 
//		and MCDefaultBuilder(use default selected SDE(GBM), FDM(Euller), RNG(MTNormal))
//
// One factory class : BuilderFactory which will just return the corresponding tuple built by selecting Default
//      or Customized Builder
//
//
//


#ifndef BUILDER_HPP
#define BUILDER_HPP

#include<tuple>
#include"SDE.hpp"
#include"FDM.hpp"
#include"RNG.hpp"
#include<memory>
#include<iostream>

// Be careful with the order of the tuple : rate,sigma(vol),dividend,IC,K,T
using OptionTuple = std::tuple<double, double, double, double, double, double>;

//for readability
using FDMPointer = std::shared_ptr<IFDM>;
using RNGPointer = std::shared_ptr<IRNG>;

//Parts of the builder that we wish to build
using BuilderTuple = std::tuple<SDEPointer, FDMPointer, RNGPointer>;

//Abstract Base Class
class IBuilder
{
protected:
	//Key option data
	double m_rate;		//Interest rate
	double m_vol;		//Volatility
	double m_div;		//Dividend
	double m_IC;		// Initial Condition(price)
	double m_K;			//Strike
	double m_T;			//Time to Maturity
private:
	//Pure Virtual Functions
	virtual SDEPointer GetSde() const = 0;
	virtual FDMPointer GetFdm(SDEPointer sde) const = 0;
	virtual RNGPointer GetRng() const = 0;
public:
	//Constructor
	IBuilder(OptionTuple optionData)
	{
		// rate, sigma(vol), dividend, IC, K, T, beta
		m_rate = std::get<0>(optionData);
		m_vol = std::get<1>(optionData);
		m_div = std::get<2>(optionData);
		m_IC = std::get<3>(optionData);
		m_K = std::get<4>(optionData);
		m_T = std::get<5>(optionData);
	}

	//Return the parts with SDE,FDM and RNG (Template Method Pattern)

	// Version 1: parts initialised from the outside
	virtual BuilderTuple Parts(SDEPointer sde, FDMPointer fdm, RNGPointer rng) const final
	{
		return std::make_tuple(sde, fdm, rng);
	}

	// Version 2, parts initialised from the inside classes (Template Method Pattern)
	virtual BuilderTuple Parts() const final
	{
		// Get the SDE, FDM and RNG
		SDEPointer sde = GetSde();

		FDMPointer fdm = GetFdm(sde);

		RNGPointer rng = GetRng();

		return std::make_tuple(sde, fdm, rng);
	}

};


/////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////


//Concrete Builder class 
//Build the full model containg the SDE,FDM and RNG in runtime
class MCBuilder : public IBuilder
{
private:
	//Choose SDE
	virtual SDEPointer GetSde() const override
	{
		int c;
		std::cout << "----------Choosing the SDE----------\n";

		std::cout << "Enter 1 = GBM, 2 = CEV : ";
		std::cin >> c;

		switch (c)
		{
		case 1://GBM Case
			return std::make_shared<GBM>(m_rate, m_vol, m_div, m_IC, m_T);
		case 2://CEV Case, beta is required in runtime
			double beta;
			std::cout << "Enter bata of the CEV Model : ";
			std::cin >> beta;
			return std::make_shared<CEV>(m_rate, m_vol, m_div, m_IC, m_T, beta);
		default://for all other input including wrong input, we return GBM as default
			return std::make_shared<GBM>(m_rate, m_vol, m_div, m_IC, m_T);
		}
	}

	//Choose FDM
	virtual FDMPointer GetFdm(SDEPointer sde) const override
	{
		std::cout << "----------Choosing the FDM----------\n";
		int c;

		std::cout << "Enter 1 = Euler, 2 = Milstein, 3 = ModifiedPredictorCorrector : ";
		std::cin >> c;

		int NT;
		std::cout << "How many NT(Number of Time intervals)? : ";
		std::cin >> NT;

		switch (c)
		{
		case 1://Euler 
			return std::make_shared<EulerFDM>(sde, NT);
		case 2://Milstein
			return std::make_shared<MilsteinFDM>(sde, NT);
		case 3://ModifiedPredictorCorrector, factor a and b are required in runtime
			double a, b;
			std::cout << "Enter a of the Modified Predictor Corrector : "; std::cin >> a;
			std::cout << "Enter b of the Modified Predictor Corrector : "; std::cin >> b;
			return std::make_shared<ModifiedPredictorCorrectorFDM>(sde, NT, a, b);
		default://for all other input including wrong input, we return Euler as default
			return std::make_shared<EulerFDM>(sde, NT);
		}
	}

	//Choose RNG
	virtual RNGPointer GetRng() const override
	{
		std::cout << "----------Choosing the RNG----------\n";
		int c;

		std::cout << "Enter 1 = Mersenne Twister Normal Distribution, 2 = BoxMuller, 3 = PolarMarsaglia : ";
		std::cin >> c;

		switch (c)
		{
		case 1://Mersenne Twister Normal
			return std::make_shared<MTNormalRNG>(0, 1);//mean 0 standard deviation 1
		case 2://Box Muller
			return  std::make_shared<BoxMullerRNG>();
		case 3://Polar Marsaglia
			return std::make_shared<PolarMarsagliaRNG>();
		default://for all other input including wrong input, we return MTNormal as default
			return std::make_shared<MTNormalRNG>(0, 1);
		}
	}
public:
	// Constructor
	MCBuilder(OptionTuple optionData) : IBuilder(optionData) {}
};


/////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////


//Concrete Default Builder class 
//A default combination is returned
class MCDefaultBuilder : public IBuilder
{
	//return the default SDE model, GBM in this case
	virtual SDEPointer GetSde() const override
	{
		return std::make_shared<GBM>(m_rate, m_vol, m_div, m_IC, m_T);
	}

	//return the default FDM model, Euler in this case
	virtual FDMPointer GetFdm(SDEPointer sde) const override
	{
		int NT;
		std::cout << "How many NT(Number of Time intervals)? : ";
		std::cin >> NT;

		return std::make_shared<EulerFDM>(sde, NT);
	}

	//return the default RNG model, MT Normal in this case
	virtual RNGPointer GetRng() const override
	{
		return std::make_shared<MTNormalRNG>(0, 1);//mean 0 standard deviation 1
	}
public:
	// Constructor
	MCDefaultBuilder(OptionTuple optionData) : IBuilder(optionData) {}
};


/////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////


//Concrete Builder factory
//Select Default Builder or the customized MC Builder
class BuilderFactory
{
public:
	// Choose which builder you want in runtime
	static BuilderTuple ChooseBuilder(OptionTuple data)
	{// Factory method to choose your builder

		std::cout << "----------Choosing the Builder----------\n";

		int c;

		std::cout << "Enter 1 = Default Builder, 2 = Customized MCBuilder : ";
		std::cin >> c;

		MCDefaultBuilder builder1(data);
		MCBuilder builder2(data);

		switch (c)
		{
		case 1://return default parts
			return builder1.Parts();
		case 2://return customized parts
			return builder2.Parts();
		default://for all other input return default
			return builder1.Parts();
		}
	}
};

#endif
