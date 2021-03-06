/* ****************************************************************** **
** OpenSees - Open System for Earthquake Engineering Simulation       **
** Pacific Earthquake Engineering Research Center                     **
**                                                                    **
**                                                                    **
** (C) Copyright 1999, The Regents of the University of California    **
** All Rights Reserved.                                               **
**                                                                    **
** Commercial use of this program without express permission of the   **
** University of California, Berkeley, is strictly prohibited. See    **
** file 'COPYRIGHT' in main directory for information on usage and    **
** redistribution, and for a DISCLAIMER OF ALL WARRANTIES.            **
**                                                                    **
** Developed by:                                                      **
** Frank McKenna (fmckenna@ce.berkeley.edu)                           **
** Gregory L. Fenves (fenves@ce.berkeley.edu)                         **
** Filip C. Filippou (filippou@ce.berkeley.edu)                       **
**                                                                    **
** ****************************************************************** */

// $Revision: 1.00 $
// $Date: 2008/07/18 18:05:53 $
// $Source: /usr/local/cvs/OpenSees/SRC/element/pipe/Pipe.h,v $


// Written: Antonios Vytiniotis
// Created: 07/08
// Revision: A
//
// Description: This file contains the definition for the Pipelin2. A Pipelin2 object
// provides the abstraction of the small deformation bar element plus predicts the
// uncoupled pore pressure change according to Darcy Weisbach equation for laminar flow.
// Each pipe object is associated with a material object dealing with the axial compressibility
// of the drain. This Pipelin2 element will work in 2d problems in a 3DOF domain.
//
// What: "@(#) Pipelin2.h, revA"


#ifndef Pipelin2_h
#define Pipelin2_h

#include <Element.h>
#include <Matrix.h>

class Node;
class Channel;
class UniaxialMaterial;

class Pipelin2:public Element {
public:
	//constructors
	Pipelin2 (int tag, int Nd1, int Nd2, UniaxialMaterial &theMaterial,
		double A, double C_3, double Grav=0.0);
	Pipelin2();
	//destructor
	~Pipelin2();

	//public methods to obtain information about dof & connectivity
	int getNumExternalNodes(void) const;
	const ID &getExternalNodes(void);
	int getNumDOF(void);
	Node **getNodePtrs(void);

	//public methods to set the state of the element
	void setDomain(Domain *theDomain);
	int commitState(void);
	int revertToLastCommit(void);
	int revertToStart(void);
	int update(void);

	//public methods to obtain stiffness, mass, damping, and residual information
	const Matrix &getTangentStiff(void);
	const Matrix &getInitialStiff(void);
	const Matrix &getDamp(void);
	const Matrix &getMass(void);

	void zeroLoad(void);
	int addLoad(ElementalLoad *theLoad, double loadFactor);
	int addInertiaLoadToUnbalance(const Vector &accel);
	const Vector &getResistingForce(void);
	const Vector &getResistingForceIncInertia(void);

	//public methods for output
	int sendSelf(int commitTag, Channel &theChannel);
	int recvSelf(int commitTag, Channel &theChannel, FEM_ObjectBroker
		&theBroker);
	int displaySelf(Renderer &theViewer, int displayMode, float fact);
	void Print(OPS_Stream &s, int flag=0);
	Response *setResponse(const char **argv, int argc, OPS_Stream &s);
	int getResponse(int responseID, Information &eleInformation);

	//protected:
private:
	//private member function - only availabe to objects of the class
	double computeCurrentStrain(void) const;

	//private attributes - a copy for each object of the class
	UniaxialMaterial *theMaterial; //pointer to a material
	ID externalNodes; // contains the id's of end nodes
	Matrix trans; //hold the transformation matrix
//	Vector *theLoad; // pointer to the load vector P

	double L; //length of Pipe based on undeformed configuration
	double C_3;
	double A;
	double d_y_class;
	int eletag;
	double Gamma; //weight per unit volume
	Node *end1Ptr, *end2Ptr; //two pointer to the trusses nodes
	Node *theNodes[2]; //two pointer to the trusses nodes in a matrix form (AV)

	//private class attribute
	static Matrix trussK;
	static Matrix trussD;
	static Matrix trussM;
	static Vector trussR;
};
#endif