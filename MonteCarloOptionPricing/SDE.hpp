//
// SDE.hpp
//
// One-Factor Stochastic Differential Equations
// e.g. dX = a(X,t)dt + b(X,t)*dW
// where a = drift, b = duffusion, dW = Wiener Process
//
// One Base class: ISDE
// Two SDE Models as the Derived classes: GBM(Geometric Brownian Motion) and CEV(Constant Elasticity of Variance)
//
//
//

#ifndef SDE_HPP
#define SDE_HPP

#include<cmath>	//for power function in the CEV Model

//Abstract Base(Interface) SDE class : contains the mandatory elements and functions of SDE
//Standard SDE: e.g. dX = a(X,t)dt + b(X,t)*dW
class ISDE
{
protected:	//protected so derived classes can access them
	double m_ic;		//initial condition
	double m_exp;		//expiry time
public:
	//Constructors
	ISDE(double ic, double exp) : m_ic(ic), m_exp(exp) {};	//Constructor

															//Pure Virtual Functions - will be implemented in the derived classes

															//Drift and Diffusion
	virtual double Drift(double x) = 0;          // Calculate Drift - the a(mu) in the SDE
	virtual double Diffusion(double x) = 0;      // Calculate Diffusion - the b(sig) in the SDE

												 //Some extra functions associated with the SDE
	virtual double DriftCorrected(double x, double B) = 0;
	virtual double DiffusionDerivative(double x) = 0;

	//Getters and Setters(Template Method Pattern)
	virtual void InitialCondition(double val) final
	{//set InitialCondition
		m_ic = val;
	}
	virtual double InitialCondition() const final
	{//get InitialCondition
		return m_ic;
	}
	virtual void ExpiryTime(double val) final
	{//set ExpiryTime
		m_exp = val;
	}
	virtual double ExpiryTime() const final
	{//get ExpiratyTime
		return m_exp;
	}
};


/////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////


//Concrete Derived SDE class : Geometric Brownian Motion (GBM) 
//Typical GBM Function : dS = mu(t)Sdt + sig(t)SdW
class GBM : public ISDE
{
private:
	double m_mu;				// Drift
	double m_vol;				// Constant volatility
	double m_div;				// Constant dividend yield
public:
	//Default Constructor
	GBM(double driftCoeff, double diffusionCoeff, double dividend, double initialCondition, double expiry)
		: ISDE(initialCondition, expiry), m_mu(driftCoeff), m_vol(diffusionCoeff), m_div(dividend) {}

	//Derived Functions below

	//Drift and Diffusion
	virtual double Drift(double x) override
	{//Calculate Drift
		return (m_mu - m_div) * x;
	}
	virtual double Diffusion(double x) override
	{//Calculate Diffusion 
		return m_vol * x;
	}

	//Some extra functions associated with the SDE
	virtual double DriftCorrected(double x, double B)	override
	{
		return Drift(x) - B * Diffusion(x) * DiffusionDerivative(x);
	}

	virtual double DiffusionDerivative(double x) override
	{
		return m_vol;
	}
};


/////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////


//Concrete Derived SDE class : Constant Elasticity of Variance (CEV) 
//Note: when beta = 1.0, CEV == GBM
class CEV : public ISDE
{
private:
	double m_mu;		// r
	double m_vol;		// Constant volatility
	double m_div;		// Constant dividend yield
	double m_beta;      // Beta
public:
	//Constructor
	CEV(double driftCoeff, double diffusionCoeff, double dividend, double initialCondition, double expiry, double beta)
		: ISDE(initialCondition, expiry), m_mu(driftCoeff), m_div(dividend), m_beta(beta)
	{
		m_vol = diffusionCoeff * std::pow(initialCondition, 1.0 - beta);
	}

	//Derived Functions below

	//Drift and Diffusion
	double Drift(double x) override
	{//Calculate Drift
		return (m_mu - m_div) * x;
	}
	double Diffusion(double x) override
	{//Calculate Diffusion
		return m_vol * std::pow(x, m_beta);
	}

	//Some extra functions associated with the SDE
	double DriftCorrected(double x, double B) override
	{
		return Drift(x) - B * Diffusion(x) * DiffusionDerivative(x);
	}
	double DiffusionDerivative(double x) override
	{
		if (m_beta > 1.0)
		{
			return m_vol * m_beta * std::pow(x, m_beta - 1.0);
		}
		else
		{
			return m_vol * m_beta / std::pow(x, 1.0 - m_beta);
		}
	}
};

#endif
