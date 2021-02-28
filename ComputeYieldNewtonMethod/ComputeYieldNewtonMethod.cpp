#include "ComputeYieldNewtonMethod.hpp"
#include <tuple>
#include <cmath>

ComputeYieldNewtonMethod::ComputeYieldNewtonMethod(int monthToExpiry, int couponInterval, double couponRate, double faceValue, double price) :
    monthToExpiry(monthToExpiry), couponInterval(couponInterval), couponRate(couponRate), faceValue(faceValue), price(price) {

    int offset = 1;
    if (monthToExpiry % couponInterval == 0)
        offset = 0;

    couponNumber = monthToExpiry / couponInterval + offset;

    coupon = faceValue*couponRate * (couponInterval / 12.0);
}

double ComputeYieldNewtonMethod::getYield(double x0) {
    return newtonMethod(x0);
}

double ComputeYieldNewtonMethod::getDuration(double x0) {
    double xnew = newtonMethod(x0);
    return -1.0 / price*(df(xnew));
}

double ComputeYieldNewtonMethod::getConvexity(double x0) {
    double xnew = newtonMethod(x0);
    return 1.0 / price*(ddf(xnew));
}

std::tuple<double, double, double> ComputeYieldNewtonMethod::getYieldDurationConvexity(double x0) {
    double xnew = newtonMethod(x0);
    return std::make_tuple(xnew, -1.0 / price*(df(xnew)), 1.0 / price*(ddf(xnew)));

}

double ComputeYieldNewtonMethod::newtonMethod(double x0) {
    double xnew = x0;
    double xold = x0 - 1;
    double tolConsec = pow(10, -6);
    while (abs(xnew - xold) > tolConsec)
    {
        xold = xnew;
        xnew = xold - f(xold) / df(xold);
    }
    return xnew;
}

//f(x),bond value function
double ComputeYieldNewtonMethod::f(double x)
{
    double total = 0;;
    for (int i = 1; i <= couponNumber; i++)
    {
        total += coupon*exp(-x*(monthToExpiry - (couponNumber - i)*couponInterval) / 12.0);
    }

    total += (faceValue)*exp(-x*(monthToExpiry) / 12.0);

    return total - price;
}


//first derivative of f(x)
double ComputeYieldNewtonMethod::df(double x)
{
    double total = 0;

    for (int i = 1; i <= couponInterval; i++)
    {
        total += (monthToExpiry - (couponNumber - i)*couponInterval) / 12.0 * coupon*exp(-x*(monthToExpiry - (couponNumber - i)*couponInterval) / 12.0);
    }

    total += (monthToExpiry / 12.0)*(faceValue)*exp(-x*(monthToExpiry) / 12.0);

    return (-total);
}

//second derivative of f(x)
double ComputeYieldNewtonMethod::ddf(double x)
{
    double total = 0;

    for (int i = 1; i <= couponInterval; i++)
    {
        total += pow(((monthToExpiry - (couponNumber - i)*couponInterval) / 12.0), 2) * coupon*exp(-x*(monthToExpiry - (couponNumber - i)*couponInterval) / 12.0);
    }

    total += pow((monthToExpiry / 12.0), 2)*(faceValue)*exp(-x*(monthToExpiry) / 12.0);

    return total;
}