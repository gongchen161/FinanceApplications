//
// FDM.hpp
//
// Finite Difference Methods for the SDE classes
//
// One Base class : IFDM
// Three selected FDM Models as the Derived classes : Euler, Milstein, and ModifiedPredictorCorrector
//
//
//

#ifndef FDM_HPP
#define FDM_HPP

#include"SDE.hpp"
#include<vector>
#include<memory>

//For readability
using SDEPointer = std::shared_ptr<ISDE>;

//Abstract Base(Interface) FDM class : contains the mandatory elements and functions of DFM
//For one-factor FDM in particular
class IFDM
{
protected:
	//For derived class to access
	SDEPointer m_sde;				//SDE
public:
	//Public member data for outside class to access
	int m_NT;						//Number of time interval
	std::vector<double> m_vec;		//The mesh array
	double m_k;						//Mesh size

									//Constructor
	IFDM(SDEPointer stochasticEquation, int numSubdivisions)
	{
		m_sde = stochasticEquation;		//assign the SDE
		m_NT = numSubdivisions;			//assign the number of time interval

										//if negative number inputed, set to 0
		if (m_NT < 0)
			m_NT = 0;

		m_k = m_sde->ExpiryTime() / double(m_NT);	//set the difference between each time interval

													// Create the mesh array
		m_vec.resize(m_NT + 1);	//resize to fit the mesh array

		int n = 0;
		//using iterator to initialize the mesh array since it's faster than [] or push_back
		for (auto it = m_vec.begin(); it != m_vec.end(); ++it)
		{
			*it = (m_k*(n++));	//e.g. 0, 0+1k, 0+2k, 0+3k, 0+4k....
		}
	}

	//Setters and Getters(Template Method Pattern)
	virtual SDEPointer StochasticEquation() const final
	{
		return m_sde;
	}
	virtual void StochasticEquation(SDEPointer sde) final
	{
		m_sde = sde;
	}

	//Pure virtual functions
	virtual double advance(double  xn, double  tn, double  dt, double  normalVar) = 0;
};


/////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////


//Concrete Derived FDM class : Euler FDM Method
//Typical formula is : X(n+1) = X(n) + mu*dt + sig*dW
class EulerFDM : public IFDM
{
public:
	//Constructor
	EulerFDM(SDEPointer stochasticEquation, int numSubdivisions) : IFDM(stochasticEquation, numSubdivisions) {}

	//Derived Advance Function
	virtual double advance(double  xn, double  tn, double  dt, double  normalVar) override
	{//Compute the value at tn+dt using Euler's Method
		return xn + m_sde->Drift(xn) * dt + m_sde->Diffusion(xn) *  std::sqrt(dt) * normalVar;
	}
};


/////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////


//Concrete Derived FDM class : Milstein FDM Method
class MilsteinFDM : public IFDM
{
public:
	//Constructor
	MilsteinFDM(SDEPointer stochasticEquation, int numSubdivisions) : IFDM(stochasticEquation, numSubdivisions) {}

	//Derived Advance Function
	virtual double advance(double  xn, double  tn, double  dt, double  normalVar) override
	{//Compute the value at tn+dt using Milstein Method
		return xn + m_sde->Drift(xn) * dt + m_sde->Diffusion(xn) * std::sqrt(dt) * normalVar
			+ 0.5 * dt * m_sde->Diffusion(xn) * m_sde->DiffusionDerivative(xn) * (normalVar * normalVar - 1.0);
	}
};


/////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////


//Concrete Derived FDM class : Nodified Predictor Corrector FDM Method
class ModifiedPredictorCorrectorFDM : public IFDM
{
private:
	//Some factors
	double m_A;
	double m_B;
	double m_VMid;
public:
	//Constructors
	ModifiedPredictorCorrectorFDM(SDEPointer stochasticEquation, int numSubdivisions, double  a, double  b)
		: IFDM(stochasticEquation, numSubdivisions), m_A(a), m_B(b), m_VMid(0) {}

	//Derived Advance Function
	virtual double  advance(double  xn, double  tn, double  dt, double  normalVar) override
	{//Compute the value at tn+dt using Modified Predictor Corrector

	 //Euler for predictor
		m_VMid = xn + m_sde->Drift(xn) * dt + m_sde->Diffusion(xn) * std::sqrt(dt) * normalVar;

		// Modified Trapezoidal rule, using adjusted drift
		double  driftTerm = (m_A * m_sde->DriftCorrected(m_VMid, m_B) + ((1.0 - m_A) * m_sde->DriftCorrected(xn, m_B))) * dt;
		double  diffusionTerm = (m_B * m_sde->Diffusion(m_VMid) + ((1.0 - m_B) * m_sde->Diffusion(xn))) * std::sqrt(dt) * normalVar;

		//return the result
		return xn + driftTerm + diffusionTerm;
	}
};

#endif