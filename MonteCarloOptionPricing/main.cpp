//
// main.cpp
//
//
// Application interface
//
//
//

#include<iostream>
#include<algorithm>
#include<string>
#include<numeric>
#include<set>
#include<boost\algorithm\string.hpp>
#include<boost\lexical_cast.hpp>
#include"Mediator.hpp"

//Getting user input in runtime
OptionTuple GetInput()
{
	//Key option data
	double r;		// Interest rate
	double v;		// Volatility
	double d;		// Dividend
	double IC;		// Initial Condition(price)
	double K;		// Strike
	double T;		// Time to Maturity

					//get user input
	std::cout << "Enter rate : ";
	std::cin >> r;

	std::cout << "Enter volatility : ";
	std::cin >> v;

	std::cout << "Enter dividend : ";
	std::cin >> d;

	std::cout << "Enter Initial Stock Price : ";
	std::cin >> IC;

	std::cout << "Enter Strike Price : ";
	std::cin >> K;

	std::cout << "Enter expiry time : ";
	std::cin >> T;

	//return the Option tuple with the input
	return std::make_tuple(r, v, d, IC, K, T);
}

//convert a string input to set of ints
std::set<int> ToSet(const std::string& s)
{
	std::set<int> res;			//result int set

	std::vector<std::string> str_vec;	//from string to string vector first

										//split the string into vectort of string first
										//trim_copy_if to avoid input ending with ","
	boost::split(str_vec, boost::trim_copy_if(s, boost::is_any_of(",")), boost::is_any_of(","));		//delimiters: ,

																										//cast to set of ints, repeated selections are deleted automatically
	for (auto it = str_vec.begin(); it != str_vec.end(); ++it)
	{
		res.emplace(boost::lexical_cast<int>(*it));
	}

	return res;
}

//main interface
//No data modification needed, input will be received during runtime
std::vector<double> Interface()
{

	std::cout << "Enter the Option's data below : \n";
	OptionTuple option_data = GetInput();
	auto builder = BuilderFactory::ChooseBuilder(option_data);

	int num_simulations;
	std::cout << "Enter the Number of simulations : ";
	std::cin >> num_simulations;

	MCMediator mediator(builder, num_simulations);

	//Some Key functions for designing the European,Asian and Barrier options below
	//The functions below are the most common ones, but users can customized them if they want

	//As usual, discounting factor is just exp(-r*T) here
	double Dis = std::exp(-std::get<0>(option_data)*std::get<5>(option_data));

	//Typical Call Put Payoff functions with Strike captured
	PayoffFunction Call = [&](const double& x) { return std::max(0.0, x - std::get<4>(option_data));  };
	PayoffFunction Put = [&](const double& x) {  return std::max(0.0, std::get<4>(option_data) - x);  };

	//Common Average functions for Asian Options
	//Arithmetic Average
	AverageFunction ArithmeticAverage = [](const std::vector<double>& arr)
	{
		return (std::accumulate(arr.begin(), arr.end(), 0.0) / double(arr.size()));
	};

	//Geometric Average
	AverageFunction GeometricAverage = [](const std::vector<double>& arr)
	{
		return std::pow(std::accumulate(arr.begin(), arr.end(), 1.0, std::multiplies<double>()), 1.0 / double(arr.size()));
	};

	double barrier = 0;		//will be getting/changed in rumtime , captured variable
							//Some typical functions for barrier options(false = calculate payoff, true = don't calculate payoff)
	KnockFunction UpAndIn = [&barrier](const std::vector<double>& v)
	{
		for (auto it = v.begin(); it != v.end(); ++it)
		{
			//if any value >= upper limit -- IN
			if (*it >= barrier)
				return false;
		}
		return true;
	};
	KnockFunction UpAndOut = [&barrier](const std::vector<double>& v)
	{
		for (auto it = v.begin(); it != v.end(); ++it)
		{
			//if any value is >= upper limit -- OUT
			if (*it >= barrier)
				return true;
		}
		return false;
	};
	KnockFunction DownAndIn = [&barrier](const std::vector<double>& v)
	{
		for (auto it = v.begin(); it != v.end(); ++it)
		{
			//if any value <= lower limit -- IN
			if (*it <= barrier)
				return false;
		}
		return true;
	};

	KnockFunction DownAndOut = [&barrier](const std::vector<double> v)
	{
		for (auto it = v.begin(); it != v.end(); ++it)
		{
			//if any value is <= lower limit -- OUT
			if (*it <= barrier)
				return true;
		}
		return false;
	};

	std::string option_choices;	//choices as a string, expected input : e.g. 1,3,4,5,9,10
	std::cout << "----------Choose the Option types----------\n";
	std::cout << "1 = European Call.\n";
	std::cout << "2 = European Put.\n";
	std::cout << "3 = Asian Call(Arithmetic).\n";
	std::cout << "4 = Asian Call(Geometric).\n";
	std::cout << "5 = Asian Put(Arithmetic).\n";
	std::cout << "6 = Asian Put(Geometric).\n";
	std::cout << "7 = Barrier Call(Up-And-In).\n";
	std::cout << "8 = Barrier Call(Up-And-Out).\n";
	std::cout << "9 = Barrier Call(Down-And-In).\n";
	std::cout << "10 = Barrier Call(Down-And-Out).\n";
	std::cout << "11 = Barrier Put(Up-And-In).\n";
	std::cout << "12 = Barrier Put(Up-And-Out).\n";
	std::cout << "13 = Barrier Put(Down-And-In).\n";
	std::cout << "14 = Barrier Put(Down-And-Out).\n";
	std::cout << "Enter the options indexes that you wish to calculate prices for(seperate by commas(,)) : \n";
	std::cin >> option_choices; //expected input : e.g. 1,3,4,5,9,10

	std::set<int> choices = ToSet(option_choices);	//convert the string input into set of ints(non-repeat)

	std::vector<PricerPointer> p;		//to store the selected pricers

	bool change_barrier = false;		//to signal if we need to change the barrier
	for (auto it = choices.begin(); it != choices.end(); ++it)
	{
		//set barrier if a barrier option is chosen
		if (!change_barrier)
		{
			if (*it >= 7 && *it <= 14)		//if a barrier option is chosen
			{
				std::cout << "Enter Barrier : ";
				std::cin >> barrier;
				change_barrier = true;		//set to true, so no more reset
			}
		}

		switch (*it)
		{
		case 1://European Call
			p.push_back(std::make_shared<EuropeanPricer>(Call, Dis));
			break;
		case 2://European Put
			p.push_back(std::make_shared<EuropeanPricer>(Put, Dis));
			break;
		case 3://Asian Arithmetic Call
			p.push_back(std::make_shared<AsianPricer>(Call, Dis, ArithmeticAverage));
			break;
		case 4://Asian Geometric Call
			p.push_back(std::make_shared<AsianPricer>(Call, Dis, GeometricAverage));
			break;
		case 5://Asian Arithmetic Put
			p.push_back(std::make_shared<AsianPricer>(Put, Dis, ArithmeticAverage));
			break;
		case 6://Asian Geometric Put
			p.push_back(std::make_shared<AsianPricer>(Put, Dis, GeometricAverage));
			break;
		case 7://Barrier Call(Up-And-In)
			p.push_back(std::make_shared<BarrierPricer>(Call, Dis, UpAndIn));
			break;
		case 8://Barrier Call(Up-And-Out)
			p.push_back(std::make_shared<BarrierPricer>(Call, Dis, UpAndOut));
			break;
		case 9://Barrier Call(Down-And-In)
			p.push_back(std::make_shared<BarrierPricer>(Call, Dis, DownAndIn));
			break;
		case 10://Barrier Call(Down-And-Out)
			p.push_back(std::make_shared<BarrierPricer>(Call, Dis, DownAndOut));
			break;
		case 11://Barrier Put(Up-And-In)
			p.push_back(std::make_shared<BarrierPricer>(Put, Dis, UpAndIn));
			break;
		case 12://Barrier Put(Up-And-Out)
			p.push_back(std::make_shared<BarrierPricer>(Put, Dis, UpAndOut));
			break;
		case 13://Barrier Put(Down-And-In)
			p.push_back(std::make_shared<BarrierPricer>(Put, Dis, DownAndIn));
			break;
		case 14://Barrier Put(Down-And-Out)
			p.push_back(std::make_shared<BarrierPricer>(Put, Dis, DownAndOut));
			break;
		default://invalid input
			break;
		}
	}

	//add the pricers to the mediator
	for (auto it = p.begin(); it != p.end(); ++it)
	{
		mediator.AddPricer(*it);
	}

	//start calculating the price
	mediator.start();

	//we can also store all the prices as a vector of pure numbers and return as a result
	std::vector<double> final_prices;
	for (auto it = p.begin(); it != p.end(); ++it)
	{
		final_prices.push_back((*it)->Price());	//store all prices to the vector
	}

	return final_prices;

}

int main()
{

	auto my_prices = Interface();

	std::cout << "\n-------------------------------------------------\n";
	std::cout << "Printing the prices from the result returned : \n";
	for (std::size_t i = 0; i < my_prices.size(); ++i)
	{
		std::cout << "Option " << (i + 1) << " Price = " << my_prices[i] << std::endl;

	}

	return 0;
}
