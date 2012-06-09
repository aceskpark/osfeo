/* ****************************************************************** **
**    OpenSees - Open System for Earthquake Engineering Simulation    **
**          Pacific Earthquake Engineering Research Center            **
**                                                                    **
**                                                                    **
** (C) Copyright 1999, The Regents of the University of California    **
** All Rights Reserved.                                               **
**                                                                    **
** Commercial use of this program without express permission of the   **
** University of California, Berkeley, is strictly prohibited.  See   **
** file 'COPYRIGHT'  in main directory for information on usage and   **
** redistribution,  and for a DISCLAIMER OF ALL WARRANTIES.           **
**                                                                    **
** Developed by:                                                      **
**   Frank McKenna (fmckenna@ce.berkeley.edu)                         **
**   Gregory L. Fenves (fenves@ce.berkeley.edu)                       **
**   Filip C. Filippou (filippou@ce.berkeley.edu)                     **
**                                                                    **
** ****************************************************************** */
                                                                        
// $Revision: 1.1.1.1 $
// $Date: 2000/09/15 08:23:20 $
// $Source: /usr/local/cvs/OpenSees/SRC/element/feap/fElmt01.h,v $
                                                                        
                                                                        
#ifndef fElmt01_h
#define fElmt01_h

// File: ~/element/fortran/fElmt01.h
// 
// Written: fmk 
// Created: 03/99
// Revision: A
//
// Description: This file contains the class definition for fElmt01. fElmt01
// is a wrapper used to call fortran element subroutine elmt02. elmt02 is a 
// linear elastic 2d element
//
// What: "@(#) fElmt01.h, revA"

#include <fElement.h>

class fElmt01 : public fElement
{
  public:
    // constructors
    fElmt01(int tag,
	    int Nd1, int Nd2,
	    double A, double E, double rho = 0.0);
    
    fElmt01(int tag,
	    int Nd1, int Nd2, int iow = 0);
    
    fElmt01();    
    
    // destructor
    ~fElmt01();

  protected:
	     
  private:

};

#endif



