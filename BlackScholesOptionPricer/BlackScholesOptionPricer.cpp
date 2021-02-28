

#include "BlackScholesOptionPricer.hpp"
#include <cmath>
#include <math.h>

BlackScholesOptionPricer::BlackScholesOptionPricer(double stockPrice, double strikePrice, double interestRate, double dividend, double volatility, double expiryTime) 
    : stockPrice(stockPrice), strikePrice(strikePrice), interestRate(interestRate), dividend(dividend), volatility(volatility), expiryTime(expiryTime) {

}

double BlackScholesOptionPricer::callPrice() {

    double nd1 = getNd1();
	double nd2 = getNd2();

    return stockPrice * exp( -dividend * (expiryTime) ) * nd1 - strikePrice * exp( -interestRate * (expiryTime) ) *nd2;
}


double BlackScholesOptionPricer::putPrice() {

    double nd1 = getNd1();
	double nd2 = getNd2();
    
    return strikePrice * exp(-interestRate * (expiryTime)) * (1 - nd2) - stockPrice * exp(-dividend * (expiryTime)) * (1 - nd1);
}

double BlackScholesOptionPricer::getNd1() {
    double d1 = ( log(stockPrice / strikePrice) + (interestRate - dividend + volatility * volatility / 2)*(expiryTime)) / (volatility * sqrt(expiryTime));

    return N(d1);
}

double BlackScholesOptionPricer::getNd2() {
    double d1 = (log(stockPrice / strikePrice) + (interestRate - dividend + volatility * volatility / 2) * (expiryTime)) / (volatility * sqrt(expiryTime));
    double d2 = d1 - volatility * (sqrt(expiryTime));

    return N(d2);
}

double BlackScholesOptionPricer::f(double x) {
	return 1 / (sqrt(2.0 * M_PI))*exp(-x*x / 2.0);
}


//Simpson's rule of finding integral(cumulative distribution of the standard normal distribution in this case)  
double BlackScholesOptionPricer::N(double d) {
    double n = 4;		//initial nunber of interval
	double value = 0;
	double total = 0;
	double tol = pow(10, -12);		//tolerance


	double a = 0;	//starting point
	double b = d;	//ending point

	double old_value;
	double h;	//interval length

	do
	{
		h = (b - a) / (n * 1.0);
		old_value = value;
		total = 0;
		value = 0;

		total = (f(a) * 1 / 6) + (f(b) * 1 / 6);

		for (int i = 1; i <= n - 1; i++)
		{

			total += 1 / 3.0 * f(a + i * h) + 2 / 3.0 * f(a + i * h - 0.5 * h);
		}

		total = total + 2 / 3.0 * f(a + n * h - 0.5 * h);


		total *= h;
		value = total + 0.5;
		n *= 2;
	} while (abs(value - old_value) > tol);	//loop until breaking the tolerance


	return value;
}