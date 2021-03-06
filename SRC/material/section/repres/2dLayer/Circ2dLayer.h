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
                                                                        
// $Revision: 1.3 $
// $Date: 2003/02/14 23:01:37 $
// $Source: /usr/local/cvs/OpenSees/SRC/material/section/repres/2dLayer/Circ2dLayer.h,v $
                                                                        
                                                                        
// File: Circ2dLayer.h
// Written by Remo M. de Souza
// December 1998


#ifndef Circ2dLayer_h 
#define Circ2dLayer_h 

#include <ReinfLayer.h>

class ReinfBar;

class Circ2dLayer : public ReinfLayer
{
  public:

    Circ2dLayer();
	// Constructor for an arc
    Circ2dLayer(int materialID, int numReinfBars, double  reinfBarArea,
                   const Vector &centerPosition, double arcRadius, double
                   initialAngle, double finalAngle);
    // Constructor for full circle
	Circ2dLayer(int materialID, int numReinfBars, double  reinfBarArea,
                   const Vector &centerPosition, double radius);

    ~Circ2dLayer();
    
    // edition functions

    void setNumReinfBars     (int numReinfBars);
    void setMaterialID       (int materialID);  
    void setReinfBarDiameter (double reinfBarDiameter);
    void setReinfBarArea     (double reinfBarArea);

    void setCenterPosition   (const Vector &centerPosition);
    void setArcRadius        (double arcRadius);
    void setInitAngle        (double initialAngle);
    void setFinalAngle       (double finalAngle);  

    // inquiring functions

    int           getNumReinfBars     (void) const;
    int           getMaterialID       (void) const;
    double        getReinfBarDiameter (void) const;
    double        getReinfBarArea     (void) const;
    ReinfBar     *getReinfBars        (void) const;

  
    ReinfLayer   *getCopy             (void) const;
    const Vector &getCenterPosition   (void) const;
    double        getArcRadius        (void) const;    
    double        getInitAngle        (void) const;
    double        getFinalAngle       (void) const;  
    

    void Print(OPS_Stream &s, int flag =0) const;   
    friend OPS_Stream &operator<<(OPS_Stream &s, const Circ2dLayer &Circ2dLayer);    
    
  protected:
    
  private:
    int    nReinfBars;
    int    matID;
    double barDiam;
    double area;
    Vector centerPosit;
    double arcRad;
    double initAng;
    double finalAng;
};


#endif




