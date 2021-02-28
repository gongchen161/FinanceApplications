#ifndef COMPUTE_YIELD_NEWTON_METHOD_HPP
#define COMPUTE_YIELD_NEWTON_METHOD_HPP

#include <tuple>

class ComputeYieldNewtonMethod {

public:
    ComputeYieldNewtonMethod(int monthToExpiry, int couponInterval, double couponRate, double faceValue, double price);

    double getYield(double x0);

    double getDuration(double x0);

    double getConvexity(double x0);

    std::tuple<double, double, double> getYieldDurationConvexity(double x0);

private:
    double newtonMethod(double x0);

    //f(x),bond value function
    double f(double x);

    //first derivative of f(x)
    double df(double x);

    //second derivative of f(x)
    double ddf(double x);

private:
    int couponInterval;		        //e.g. semiannual = 6, annual = 12
    int monthToExpiry;				//in months
    int couponNumber;
    double couponRate;
    double coupon;
    double price;
    double faceValue;
};
#endif