//
//================================================================================
//# COPYRIGHT (C):     :-))                                                      #
//# PROJECT:           Object Oriented Finite Element Program                    #
//# PURPOSE:           Manzari-Dafalias potential criterion(with Pc)             #
//# CLASS:             DPPotentialSurface01                                      #
//#                                                                              #
//# VERSION:                                                                     #
//# LANGUAGE:          C++.ver >= 2.0 ( Borland C++ ver=3.00, SUN C++ ver=2.1 )  #
//# TARGET OS:         DOS || UNIX || . . .                                      #
//# PROGRAMMER(S):     Boris Jeremic, Zhaohui Yang                               #
//#                                                                              #
//#                                                                              #
//# DATE:              August 08 '00                                             #
//# UPDATE HISTORY:    December 13 '00                                           #
//#                                                                              #
//#                                                                              #
//#                                                                              #
//#                                                                              #
//# SHORT EXPLANATION:                                                           #
//#                                                                              #
//#                                                                              #
//#                                                                              #
//#                                                                              #
//#                                                                              #
//#                                                                              #
//#                                                                              #
//#                                                                              #
//#                                                                              #
//#                                                                              #
//================================================================================
//

#ifndef DP_PS01_H    
#define DP_PS01_H

#include "EPState.h"
#include "PS.h"
#include <BJtensor.h>


class MDPotentialSurface : public PotentialSurface
{

  public:
    PotentialSurface *newObj();  //create a colne of itself
    MDPotentialSurface();        // Default constructor

    tensor dQods(const EPState *EPS) const; 
    tensor d2Qods2(const EPState *EPS) const ;   

    //aux. functions for d2Qods2
    tensor dnods(const EPState *EPS) const;
    tensor dthetaoverds(const EPState *EPS) const;
    double dgoverdt(double theta, double c) const;
    tensor dpqdnods(const EPState *EPS) const;
    
    void print() { cout << *this; };

    //================================================================================
    // Overloaded Insertion Operator
    // prints an PotentialSurface's contents 
    //================================================================================
    friend OPS_Stream& operator<< (OPS_Stream& os, const MDPotentialSurface &PS)
    {
       os << "Manzari-Dafalias Potential Surface Parameters: " << endlnn;
       return os;
    }
};

#endif

