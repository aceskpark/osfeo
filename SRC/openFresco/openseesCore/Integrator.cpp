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
                                                                        
// $Revision: 314 $
// $Date: 2011-05-23 05:17:07 +0800 (星期一, 23 五月 2011) $
// $Source: /usr/local/cvs/OpenSees/SRC/analysis/integrator/Integrator.cpp,v $
                                                                        
                                                                        
// File: ~/analysis/integrator/Integrator.C
// 
// Written: fmk 
// Created: 11/96
// Revision: A
//
// Description: This file contains the implementation of Integrator.
//
// What: "@(#) Integrator.C, revA"

#include <Integrator.h>

Integrator::Integrator(int clasTag)
:MovableObject(clasTag)
{

}

Integrator::~Integrator()
{

}

int
Integrator::domainChanged()
{
    return 0;
}






