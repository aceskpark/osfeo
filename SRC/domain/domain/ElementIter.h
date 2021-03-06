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
// $Date: 2000/09/15 08:23:18 $
// $Source: /usr/local/cvs/OpenSees/SRC/domain/domain/ElementIter.h,v $
                                                                        
                                                                        
// File: ~/domain/domain/ElementIter.h
//
// Written: fmk 
// Created: Fri Sep 20 15:27:47: 1996
// Revision: A
//
// Description: This file contains the class definition for ElementIter.
// ElementIter is an abstract base class. An ElementIter is an iter for 
// returning the elements of an object of class  Domain. ElementIters 
// must be written for each subclass of Domain (this is done for 
// efficiency reasons), hence the abstract base class.

#ifndef ElementIter_h
#define ElementIter_h

class Element;

class ElementIter
{
  public:
    ElementIter() {};
    virtual ~ElementIter() {};

    virtual Element *operator()(void) =0;

  protected:
    
  private:

};

#endif





