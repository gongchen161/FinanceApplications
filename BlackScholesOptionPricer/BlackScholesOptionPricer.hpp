#ifndef BLACK_SCHOLES_OPTION_PRICER
#define BLACK_SCHOLES_OPTION_PRICER


class BlackScholesOptionPricer {

public:
    BlackScholesOptionPricer(double stockPrice, double strikePrice, double interestRate, double dividend, double volatility, double expiryTime);
    double callPrice();
    double putPrice();

private:
    double getNd1();
    double getNd2();
    //pdf of standard normal distribution
    double f(double x);	
    //Simpson's rule of finding integral(cumulative distribution of the standard normal distribution in this case)  
    double N(double d);	

private:
    double stockPrice;
    double strikePrice;
    double interestRate;
    double dividend;
    double volatility;
    double expiryTime;            // in years
};

#endif