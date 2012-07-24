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

#ifndef MCFTRCPlaneStress02_h
#define MCFTRCPlaneStress02_h

// $Revision: 1.2 $
// $Date: 2011/08/31 23:00:47 $
// $Source: /usr/local/cvs/OpenSees/SRC/material/nD/MCFTRCPlaneStress02.h,v $
// File: MCFTRCPlaneStress02.h
//
// Written: Lining
// Created: 2011.8
//
// Description: This file contains the class definition for 
// MCFTRCPlaneStress02 material.
//
// For Detailed explanation of the model, please refer to the journal paper
// entitled "The modified compression-filed theory for reinforced concrete element
// subjected to shear, ACI Journal. s83-22. 1986. pp. 219-231"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <NDMaterial.h>
#include <UniaxialMaterial.h>

#include <Matrix.h>
#include <Vector.h>
#include <ID.h>
#include <Tensor.h>

class MCFTRCPlaneStress02 : public NDMaterial
{
  public:
    MCFTRCPlaneStress02 ( int      tag, 
				      double   RHO,
				      UniaxialMaterial *s1,
				      UniaxialMaterial *s2,
				      UniaxialMaterial *c1,
				      UniaxialMaterial *c2,
				      double   ANGLE1,
				      double   ANGLE2,
				      double   ROU1,
				      double   ROU2,
					  double   DB1,
					  double   DB2,
				      double   FPC,
				      double   FY,
				      double   E,
				      double   EPSC0,
					  double   AGGR,
					  double   XD,
					  double   YD);
    MCFTRCPlaneStress02();
    ~MCFTRCPlaneStress02();				  
    
    double getRho(void);
    
    int setTrialStrain(const Vector &v); // really used one
    int setTrialStrain(const Vector &v, const Vector &r);
    int setTrialStrainIncr(const Vector &v);
    int setTrialStrainIncr(const Vector &v, const Vector &r);
    const Matrix &getTangent(void);
    const Matrix &getInitialTangent(void) {return this->getTangent();};

    const Vector &getStress(void);
    const Vector &getStrain(void);
    
    const Vector &getCommittedStress(void);
    const Vector &getCommittedStrain(void);    
    
    int commitState(void);
    int revertToLastCommit(void);
    int revertToStart(void);
    
    NDMaterial *getCopy(void);
    NDMaterial *getCopy(const char *type);

    void Print(OPS_Stream &s, int flag = 0);
    int sendSelf(int commitTag, Channel &theChannel);
    int recvSelf(int commitTag, Channel &theChannel, FEM_ObjectBroker &theBroker);

    const char *getType(void) const { return "PlaneStress"; };
    int getOrder(void) const { return 3;};

	Response *setResponse (const char **argv, int argc, OPS_Stream &s);
	int getResponse (int responseID, Information &matInformation);

  protected:
    
  private:

    double   rho; 
    UniaxialMaterial **theMaterial; // pointer of the materials 
    Response **theResponses; // pointer to material responses needed for Concrete
	
	double   citaS;     // angle of principle stress
	double   citaE;     // angle of principle strain
    double   angle1;    // angel of the first steel layer to x coordinate 
    double   angle2;    // angel of the second steel layer to x coordinate
    double   rou1;      // steel ratio of the first steel layer
    double   rou2;      // steel ratio of the second steel layer
	double   db1;       // steel diameter of the first steel layer
	double   db2;       // steel diameter of the second steel layer
    double   fpc;       // compressive strength of the concrete
    double   fy;        // yield stress of the bare steel bar
    double   E0;        // young's modulus of the steel
    double   epsc0;     // compressive strain of the concrete
	double   aggr;      // aggregate size
	double   xd;        // x- crack spacing
    double   yd;        // y- crack spacing
    Vector   Tstress;  // Trial stresses
    Vector   lastStress;  // Last committed stresses, added for x, k
    
    int      steelStatus;  // check if steel yield, 0 not yield, 1 yield
    int      dirStatus;    // check if principle direction has exceed 90 degree, 1 yes, 0 no
    
    double   citaStrain;      // principle strain direction
    double   citaStress;     // principle stress direction
    double   miu12;        // Hsu/Zhu ratio
    double   miu21;        // Hsu/Zhu ratio
    double   G12;          // Shear Modulus
    
    // for damgage factor D=1-0.4*epslonC'/epslon0; epslon0=0.002
    
    // Trial values
    int TOneReverseStatus;         // Trial reverse status for concrete One, 1 reverse, 0 no
    double TOneNowMaxComStrain;
    double TOneLastMaxComStrain;
    
    int TTwoReverseStatus;         // Trial reverse status for concrete Two, 1 reverse, 0 no
    double TTwoNowMaxComStrain;
    double TTwoLastMaxComStrain;
    
    // Converged values
    int COneReverseStatus;         // Converged reverse status for concrete One, 1 reverse, 0 no
    double COneNowMaxComStrain;
    double COneLastMaxComStrain;
    
    int CTwoReverseStatus;         // Converged reverse status for concrete Two, 1 reverse, 0 no
    double CTwoNowMaxComStrain;
    double CTwoLastMaxComStrain;
    
    double DDOne; // damage factor for concrete One
    double DDTwo; // damage factor for concrete Two
    
    
    Vector strain_vec;
	Vector strainC_vec;
	Vector strainSlip_vec;
	Vector strainC0_vec;
	Vector strainS0_vec;
	Vector strainCp_vec;
    Vector stress_vec;
    Matrix tangent_matrix;


    Vector determineTrialStress(Vector strain);
    double getPrincipalStressAngle(double inputAngle);
    double getAngleError(double inputCita);
 
	double kupferEnvelop(double Tstrain, double sig_p, double eps_p);
	int determineTangent(void);
};

#endif
