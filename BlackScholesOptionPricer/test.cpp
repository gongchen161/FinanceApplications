#include <iostream>
#include "BlackScholesOptionPricer.hpp"



int main() {

    BlackScholesOptionPricer blacksScholesOptionPricer(60, 65, 0.08, 0, 0.3, 0.25);

    double callPrice = blacksScholesOptionPricer.callPrice();
    double putPrice = blacksScholesOptionPricer.putPrice();

    std::cout << "Call Price = " << callPrice << std::endl;
    std::cout << "Put Price = " << putPrice << std::endl;



}