#include <iostream>
#include <iomanip>
#include "ComputeYieldNewtonMethod.hpp"

int main() {

    // e.g. a bond with
    // maturity date in 5 years (60 months)
    // coupon paid annually (every 12 months)
    // coupon rate = 2% (0.02)
    // face value = 100
    // current bond price = 104
    ComputeYieldNewtonMethod yieldComputer(60, 12, 0.02, 100, 104);
    double startingValue = 0.01;
   
    double yield = yieldComputer.getYield(startingValue);
    double duration = yieldComputer.getDuration(startingValue);
    double convexity = yieldComputer.getConvexity(startingValue);

    std::cout << std::setprecision(12) << std::fixed;
    std::cout << "Yield = " << yield << std::endl;
    std::cout << "Duration = " << duration << std::endl;
    std::cout << "Convexity = " << convexity << std::endl;

    std::tuple<double,double,double> result = yieldComputer.getYieldDurationConvexity(startingValue);
    
    std::cout << "Yield (tuple) = " << std::get<0>(result) << std::endl;
    std::cout << "Duration (tuple) = " << std::get<1>(result) << std::endl;
    std::cout << "Convexity (tuple) = " << std::get<2>(result) << std::endl;
}