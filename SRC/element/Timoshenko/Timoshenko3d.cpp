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

// $Source: /usr/local/cvs/OpenSees/SRC/element/Timoshenko/Timoshenko3d.cpp,v $
// $Revision: 1.2 $
// $Date: 2009/01/10 21:22:20 $

// Created: 09/09
// Created by: Li Ning (neallee@tju.edu.cn)
// Description: This file contains the class implementation of Timoshenko2d.
//              Make use of Neddy(1997) Interdependent Integration Element 
//              procecess and fiber section model.

// Reference: LI Ning, LI Zhong-Xian, XIE Li-Li. A Fiber-Section Model Based
//            Timoshenko Beam Element Using Shear-Bending Interdependent Shape 
//            Function. Earthquake Engineering & Engineering Vibration. 2013, 
//            12(3): 421-432.

#include <Timoshenko3d.h>
#include <Node.h>
#include <FiberSection3d.h>
#include <SectionForceDeformation.h>
#include <TimoshenkoSection3d.h>
#include <CrdTransf.h>
#include <Matrix.h>
#include <Vector.h>
#include <ID.h>
#include <Renderer.h>
#include <Domain.h>
#include <string.h>
#include <Information.h>
#include <Channel.h>
#include <FEM_ObjectBroker.h>
#include <ElementResponse.h>
#include <CompositeResponse.h>
#include <ElementalLoad.h>
#include <BeamIntegration.h>
#include <Parameter.h>
#include <math.h>

Matrix Timoshenko3d::K(12,12);
Vector Timoshenko3d::P(12);
double Timoshenko3d::workArea[200];

Matrix *Timoshenko3d::bd = 0;
Matrix *Timoshenko3d::nd = 0;

Timoshenko3d::Timoshenko3d(int tag, 
					 int nd1, int nd2,
					 int numSec, 
					 SectionForceDeformation **s,
					 CrdTransf &coordTransf, 
					 BeamIntegration& bi,
					 double r)
    :Element (tag, ELE_TAG_Timoshenko3d), 
    numSections(numSec), theSections(0), crdTransf(0), beamInt(0),
    connectedExternalNodes(2), Rslt(6), Defo(6),
	Q(12), q(6), rho(r), parameterID(0)
{
  // Allocate arrays of pointers to SectionForceDeformations
  theSections = new SectionForceDeformation *[numSections];
    
  if (theSections == 0) {
    opserr << "Timoshenko3d::Timoshenko3d - failed to allocate section model pointer\n";
    exit(-1);
  }

  for (int i = 0; i< numSections; i++){
    // Get copies of the material model for each integration point
    theSections[i] = s[i]->getCopy();

    // Check allocation
    if (theSections[i] == 0 ) { //|| s[i]->getClassTag() != SEC_TAG_TimoshenkoSection3d
      opserr << "Timoshenko3d::Timoshenko3d() --failed to get a copy of section model " << endln;
      exit(-1);
    }
  }

  crdTransf = coordTransf.getCopy3d();
  if (crdTransf == 0) {
    opserr << "Timoshenko3d::Timoshenko3d - failed to copy coordinate transformation\n";
    exit(-1);
  }
  
  beamInt = bi.getCopy();

  if (beamInt == 0) {
	opserr << "Timoshenko3d::Timoshenko3d() - failed to copy beam integration\n";
	exit(-1);
  }

  // Set connected external node IDs
  connectedExternalNodes(0) = nd1;
  connectedExternalNodes(1) = nd2;

  theNodes[0] = 0;
  theNodes[1] = 0;

  q0[0] = 0.0;
  q0[1] = 0.0;
  q0[2] = 0.0;
  q0[3] = 0.0;
  q0[4] = 0.0;

  p0[0] = 0.0;
  p0[1] = 0.0;
  p0[2] = 0.0;
  p0[3] = 0.0;
  p0[4] = 0.0;

  if (nd == 0) 	nd = new Matrix [maxNumSections];
  if (bd == 0)	bd = new Matrix [maxNumSections];

  if (!nd || !bd ) {
    opserr << "Timoshenko3d::Timoshenko3d() -- failed to allocate static section arrays";
    exit(-1);
  }

// AddingSensitivity:BEGIN /////////////////////////////////////
	parameterID = 0;
// AddingSensitivity:END //////////////////////////////////////
}

Timoshenko3d::Timoshenko3d()
	:Element (0, ELE_TAG_Timoshenko3d),
	numSections(0), theSections(0), crdTransf(0), beamInt(0),
	connectedExternalNodes(2), Rslt(6), Defo(6),
	Q(12), q(6), rho(0.0), parameterID(0)
{
  q0[0] = 0.0;
  q0[1] = 0.0;
  q0[2] = 0.0;
  q0[3] = 0.0;
  q0[4] = 0.0;

  p0[0] = 0.0;
  p0[1] = 0.0;
  p0[2] = 0.0;
  p0[3] = 0.0;
  p0[4] = 0.0;

  theNodes[0] = 0;
  theNodes[1] = 0;

  if (nd == 0) 	nd  = new Matrix [maxNumSections];
  if (bd == 0)	bd  = new Matrix [maxNumSections];

  if (!nd || !bd ) {
    opserr << "Timoshenko3d::Timoshenko3d() -- failed to allocate static section arrays";
    exit(-1);
  }

// AddingSensitivity:BEGIN /////////////////////////////////////
	parameterID = 0;
// AddingSensitivity:END //////////////////////////////////////
}

Timoshenko3d::~Timoshenko3d()
{    
    for (int i = 0; i< numSections; i++)
      if (theSections[i])
        delete theSections[i];
	
    // Delete the array of pointers to SectionForceDeformation pointer arrays
    if (theSections)
		delete [] theSections;

	if (crdTransf)
		delete crdTransf;

	if (beamInt != 0)
		delete beamInt;
}

int
Timoshenko3d::getNumExternalNodes() const
{
    return 2;
}

const ID&
Timoshenko3d::getExternalNodes()
{
    return connectedExternalNodes;
}

Node **
Timoshenko3d::getNodePtrs()
{
    return theNodes;
}

int
Timoshenko3d::getNumDOF()
{
    return 12;
}

void
Timoshenko3d::setDomain(Domain *theDomain)
{
	// Check Domain is not null - invoked when object removed from a domain
    if (theDomain == 0) {
	theNodes[0] = 0;
	theNodes[1] = 0;
	return;
    }

    int Nd1 = connectedExternalNodes(0);
    int Nd2 = connectedExternalNodes(1);

    theNodes[0] = theDomain->getNode(Nd1);
    theNodes[1] = theDomain->getNode(Nd2);

    if (theNodes[0] == 0 || theNodes[1] == 0) {

	return;
    }

    int dofNd1 = theNodes[0]->getNumberDOF();
    int dofNd2 = theNodes[1]->getNumberDOF();
    
    if (dofNd1 != 6 || dofNd2 != 6) {
		//opserr << "FATAL ERROR Timoshenko3d (tag: %d), has differing number of DOFs at its nodes",
		//	this->getTag());
		return;
    }

	if (crdTransf->initialize(theNodes[0], theNodes[1])) {
		// Add some error check
	}

	double L = crdTransf->getInitialLength();

	if (L == 0.0) {
		// Add some error check
	}

    this->DomainComponent::setDomain(theDomain);

	this->update();
}

int
Timoshenko3d::commitState()
{
    int retVal = 0;

    // call element commitState to do any base class stuff
    if ((retVal = this->Element::commitState()) != 0) {
      opserr << "Timoshenko3d::commitState () - failed in base class";
    }    

    // Loop over the integration points and commit the material states
    for (int i = 0; i< numSections; i++)
	  retVal += theSections[i]->commitState();

    retVal += crdTransf->commitState();					

    return retVal;
}

int
Timoshenko3d::revertToLastCommit()
{
    int retVal = 0;

	double L = crdTransf->getInitialLength();

    // Loop over the integration points and revert to last committed state
    for (int i = 0; i< numSections; i++)
	  retVal += theSections[i]->revertToLastCommit(L);

    retVal += crdTransf->revertToLastCommit();

    return retVal;
}

int
Timoshenko3d::revertToStart()
{
    int retVal = 0;
	
    //crdTransf->getInitialLength();

    // Loop over the integration points and revert states to start
    for (int i = 0; i< numSections; i++)
	  retVal += theSections[i]->revertToStart();

    retVal += crdTransf->revertToStart();

    return retVal;
}

int
Timoshenko3d::update(void)
{
  int err = 0;

  // Update the transformation
  crdTransf->update();
  
  // Get basic deformations
  const Vector &v = crdTransf->getBasicTrialDisp();		
  double L = crdTransf->getInitialLength();
  double oneOverL = 1.0/L;

  double pts[maxNumSections];
  beamInt->getSectionLocations(numSections, L, pts);

  double x, error = 1.0, temp=0., OmegaM;
  double OmegaZ[maxNumSections], muZ, phi3Z, phi4Z, phi1pZ, phi2pZ, phi3pZ, phi4pZ;
  double OmegaY[maxNumSections], muY, phi3Y, phi4Y, phi1pY, phi2pY, phi3pY, phi4pY;

  while (error > 1.e-3) {
	OmegaM = 0.;
    // Loop over the integration points
    for (int i = 0; i< numSections; i++) {
      x      = L * pts[i];
      int order = theSections[i]->getOrder();
      const ID &code = theSections[i]->getType();
	  Rslt = theSections[i]->getStressResultant();
	  Defo  = theSections[i]->getSectionDeformation();

      if (Rslt(5)*Defo(2) != 0.0)  OmegaZ[i] = Rslt(2)*Defo(5)/Rslt(5)/Defo(2)/L/L; 
	  else                         OmegaZ[i] = 0.;

      if (Rslt(4)*Defo(3) != 0.0)  OmegaY[i] = Rslt(3)*Defo(4)/Rslt(4)/Defo(3)/L/L;
	  else                         OmegaY[i] = 0.;

      muZ    = 1./(1.+12.*OmegaZ[i]);
      //phi1Z  =  muZ*x*(L-x)*(L-x-6*Omega[i]*L)                    /L/L;
      phi1pZ =  muZ*(3*x*x-4*L*x*(1-3*OmegaZ[i])-L*L*(6*OmegaZ[i]-1))/L/L;
      //phi2Z  =  muZ*x*(L-x)*(6*OmegaZ[i]*L-x)                      /L/L;
      phi2pZ =  muZ*(6*L*(L-2*x)*OmegaZ[i]-(2*L-3*x)*x)             /L/L;
      phi3Z  =  (L-x)*muZ*(L-3*x-12*L*OmegaZ[i])                    /L/L;
      phi3pZ =  2*muZ*(3*x+L*(6*OmegaZ[i]-2))                       /L/L;
      phi4Z  =  x*muZ*(3*x-2*L*(1+6*OmegaZ[i]))                     /L/L;
      phi4pZ =  -2*muZ*(L-3*x+6*L*OmegaZ[i])                        /L/L;
      
      muY    = 1./(1.+12.*OmegaY[i]);
      //phi1Y  =  muY*x*(L-x)*(L-x-6*OmegaY[i]*L)                    /L/L;
      phi1pY =  muY*(3*x*x-4*L*x*(1-3*OmegaY[i])-L*L*(6*OmegaY[i]-1))/L/L;
      //phi2Y  =  muY*x*(L-x)*(6*OmegaY[i]*L-x)                      /L/L;
      phi2pY =  muY*(6*L*(L-2*x)*OmegaY[i]-(2*L-3*x)*x)             /L/L;
      phi3Y  =  (L-x)*muY*(L-3*x-12*L*OmegaY[i])                    /L/L;
      phi3pY =  2*muY*(3*x+L*(6*OmegaY[i]-2))                       /L/L;
      phi4Y  =  x*muY*(3*x-2*L*(1+6*OmegaY[i]))                     /L/L;
      phi4pY =  -2*muY*(L-3*x+6*L*OmegaY[i])                        /L/L;
      
      Vector e(workArea, order);
      //P, Mz, My, Vy, Vz, T
      for (int j = 0; j < order; j++) {
        switch(code(j)) {
        case SECTION_RESPONSE_P:     // axial strain
      e(j) = oneOverL*v(0); break;
        case SECTION_RESPONSE_MZ:    // curvature
      e(j) = phi3pZ* v(1) + phi4pZ* v(2); break;
        case SECTION_RESPONSE_MY:    // curvature
      e(j) = phi3pY* v(3) + phi4pY* v(4); break;
        case SECTION_RESPONSE_VY:    // shear strain
      e(j) = (phi1pY - phi3Y) * v(1) + (phi2pY-phi4Y) * v(2); break;
        case SECTION_RESPONSE_VZ:    // shear strain
      e(j) = (phi1pZ - phi3Z) * v(3) + (phi2pZ-phi4Z) * v(4); break;
        case SECTION_RESPONSE_T:     // Torsion
      e(j) = oneOverL*v(5); break;
        default:
      break;
        }
      }
      
      // Set the section deformations
      err += theSections[i]->setTrialSectionDeformation(e);

	  OmegaM += (OmegaY[i]+OmegaZ[i]);
    }

    error = fabs(OmegaM-temp);
    temp = OmegaM;
  }

  if (err != 0) {
    opserr << "Timoshenko3d::update() - failed setTrialSectionDeformations(e)\n";
	return err;
  }

  return 0;
}

const Matrix&
Timoshenko3d::getTangentStiff(void)
{
  static Matrix kb(6,6);

  // Zero for integral
  kb.Zero();
  q.Zero();
  
  double L = crdTransf->getInitialLength();
  const Vector &v = crdTransf->getBasicTrialDisp();
  double oneOverL = 1.0/L;
  
  double pts[maxNumSections];
  beamInt->getSectionLocations(numSections, L, pts);
  double wts[maxNumSections];
  beamInt->getSectionWeights(numSections, L, wts);
  
  // Loop over the integration points
  for (int i = 0; i< numSections; i++) {
    
    int order = theSections[i]->getOrder(); // P, Mz, My, Vy, Vz, T
    const ID &code = theSections[i]->getType();

	// Get the section tangent stiffness and stress resultant
    const Matrix &ks = theSections[i]->getSectionTangent();			
    const Vector &s = theSections[i]->getStressResultant();			

    // Perform numerical integration
	bd[i] = this->getBd(i, v, L);
	//kb = kb + wts[i] * bdT[i] * ks * bd[i];
    kb.addMatrixTripleProduct(1.0, bd[i], ks, L*wts[i]);
	//q = q + wts[i] * bdT[i] * s;
	q.addMatrixTransposeVector(1.0, bd[i], s, L*wts[i]);
  }

  // Add effects of element loads, q = q(v) + q0		
  q(0) += q0[0];
  q(1) += q0[1];
  q(2) += q0[2];
  q(3) += q0[3];
  q(4) += q0[4];

  // Transform to global stiffness
  K = crdTransf->getGlobalStiffMatrix(kb, q);

  return K;
}

const Matrix&
Timoshenko3d::getInitialBasicStiff()
{
  static Matrix kb(6,6);

  // Zero for integral
  kb.Zero();
  
  double L = crdTransf->getInitialLength();
  const Vector &v = crdTransf->getBasicTrialDisp();
  double oneOverL = 1.0/L;
  
  double pts[maxNumSections];
  beamInt->getSectionLocations(numSections, L, pts);
  double wts[maxNumSections];
  beamInt->getSectionWeights(numSections, L, wts);
  
  // Loop over the integration points
  for (int i = 0; i< numSections; i++) {
    
    int order = theSections[i]->getOrder();
    const ID &code = theSections[i]->getType();
  
    // Get the section tangent stiffness and stress resultant
    const Matrix &ks = theSections[i]->getInitialTangent();

    // Perform numerical integration
    bd[i] = this->getBd(i, v, L);
    //kb = kb + wts[i] * bdT[i] * ks * bd[i];
    kb.addMatrixTripleProduct(1.0, bd[i], ks, L*wts[i]);

  }
  return kb;
}

const Matrix&
Timoshenko3d::getInitialStiff()
{
  const Matrix &kb = this->getInitialBasicStiff();

  // Transform to global stiffness
  K = crdTransf->getInitialGlobalStiffMatrix(kb);

  return K;
}

const Matrix&
Timoshenko3d::getMass()
{
  K.Zero();

  if (rho == 0.0)
    return K;
  
  double L = crdTransf->getInitialLength();
  double m = 0.5*rho*L;
  
  K(0,0) = K(1,1) = K(2,2) = K(6,6) = K(7,7) = K(8,8) = m;
  
  return K;
}

void
Timoshenko3d::zeroLoad(void)
{
  Q.Zero();

  q0[0] = 0.0;
  q0[1] = 0.0;
  q0[2] = 0.0;
  q0[3] = 0.0;
  q0[4] = 0.0;

  p0[0] = 0.0;
  p0[1] = 0.0;
  p0[2] = 0.0;
  p0[3] = 0.0;
  p0[4] = 0.0;

  return;
}

int 
Timoshenko3d::addLoad(ElementalLoad *theLoad, double loadFactor)
{
  int type;
  const Vector &data = theLoad->getData(type, loadFactor);
  double L = crdTransf->getInitialLength();
  
  if (type == LOAD_TAG_Beam3dUniformLoad) {
    double wy = data(0)*loadFactor;  // Transverse
    double wz = data(1)*loadFactor;  // Transverse
    double wx = data(2)*loadFactor;  // Axial (+ve from node I to J)

    double Vy = 0.5*wy*L;
    double Mz = Vy*L/6.0; // wy*L*L/12
    double Vz = 0.5*wz*L;
    double My = Vz*L/6.0; // wz*L*L/12
    double P = wx*L;

    // Reactions in basic system
    p0[0] -= P;
    p0[1] -= Vy;
    p0[2] -= Vy;
    p0[3] -= Vz;
    p0[4] -= Vz;

    // Fixed end forces in basic system
    q0[0] -= 0.5*P;
    q0[1] -= Mz;
    q0[2] += Mz;
    q0[3] += My;
    q0[4] -= My;

  } else if (type == LOAD_TAG_Beam3dPointLoad) {
    double Py = data(0)*loadFactor;
    double Pz = data(1)*loadFactor;
    double N  = data(2)*loadFactor;
    double aOverL = data(3);				

	if (aOverL < 0.0 || aOverL > 1.0)
		return 0;

	double a = aOverL*L;
	double b = L-a;

    // Reactions in basic system
    p0[0] -= N;
    double V1, V2;
    V1 = Py*(1.0-aOverL);
    V2 = Py*aOverL;
    p0[1] -= V1;
    p0[2] -= V2;
    V1 = Pz*(1.0-aOverL);
    V2 = Pz*aOverL;
    p0[3] -= V1;
    p0[4] -= V2;

    double L2 = 1.0/(L*L);
    double a2 = a*a;
    double b2 = b*b;

    // Fixed end forces in basic system
    q0[0] -= N*aOverL;
    double M1, M2;
    M1 = -a * b2 * Py * L2;
    M2 = a2 * b * Py * L2;
    q0[1] += M1;
    q0[2] += M2;
    M1 = -a * b2 * Pz * L2;
    M2 = a2 * b * Pz * L2;
    q0[3] -= M1;
    q0[4] -= M2;

  } else {
    opserr << "Timoshenko3d::Timoshenko3d -- load type unknown for element with tag: "
	   << this->getTag() << "Timoshenko3d::addLoad()\n"; 
			    
    return -1;
  }

  return 0;
}

int 
Timoshenko3d::addInertiaLoadToUnbalance(const Vector &accel)
{
	// Check for a quick return
	if (rho == 0.0) 
		return 0;

	// Get R * accel from the nodes
	const Vector &Raccel1 = theNodes[0]->getRV(accel);
	const Vector &Raccel2 = theNodes[1]->getRV(accel);

    if (6 != Raccel1.Size() || 6 != Raccel2.Size()) {
      opserr << "Timoshenko3d::addInertiaLoadToUnbalance matrix and vector sizes are incompatable\n";
      return -1;
    }

	double L = crdTransf->getInitialLength();
	double m = 0.5*rho*L;

    // Want to add ( - fact * M R * accel ) to unbalance
	// Take advantage of lumped mass matrix
    Q(0) -= m*Raccel1(0);
    Q(1) -= m*Raccel1(1);
    Q(2) -= m*Raccel1(2);
    Q(6) -= m*Raccel2(0);
    Q(7) -= m*Raccel2(1);
    Q(8) -= m*Raccel2(2);

    return 0;
}

const Vector&
Timoshenko3d::getResistingForce()
{
  crdTransf->update();  // Will remove once we clean up the corotational 3d transformation -- MHS
  double L = crdTransf->getInitialLength();

  double pts[maxNumSections];
  beamInt->getSectionLocations(numSections, L, pts);
  double wts[maxNumSections];
  beamInt->getSectionWeights(numSections, L, wts);
  
  // Zero for integration
  q.Zero();
  
  // Loop over the integration points
  for (int i = 0; i< numSections; i++) {

    //int order = theSections[i]->getOrder();
    //const ID &code = theSections[i]->getType();
  
    double x = L * pts[i]; //xi;
	const Vector &v = crdTransf->getBasicTrialDisp();

    // Get section stress resultant
    const Vector &s = theSections[i]->getStressResultant();

    // Perform numerical integration on internal force
	bd[i] = this->getBd(i, v, L);
	//q = q + wts[i] * bdT[i] * s;
	q.addMatrixTransposeVector(1.0, bd[i], s, L*wts[i]);
  }

  // Add effects of element loads, q = q(v) + q0		
  q(0) += q0[0];
  q(1) += q0[1];
  q(2) += q0[2];
  q(3) += q0[3];
  q(4) += q0[4];
  
  // Vector for reactions in basic system
  Vector p0Vec(p0, 5);
  P = crdTransf->getGlobalResistingForce(q, p0Vec);	
  
  // Subtract other external nodal loads ... P_res = P_int - P_ext
  P.addVector(1.0, Q, -1.0);

  return P;
}

const Vector&
Timoshenko3d::getResistingForceIncInertia()
{
  // Add the inertial forces
  if (rho != 0.0) {
    const Vector &accel1 = theNodes[0]->getTrialAccel();
    const Vector &accel2 = theNodes[1]->getTrialAccel();
    
    // Compute the current resisting force
    this->getResistingForce();
    
    double L = crdTransf->getInitialLength();
    double m = 0.5*rho*L;
    
    P(0) += m*accel1(0);
    P(1) += m*accel1(1);
    P(2) += m*accel1(2);
    P(6) += m*accel2(0);
    P(7) += m*accel2(1);
    P(8) += m*accel2(2);
    
  }
  
  // Add the damping forces
  if (alphaM != 0.0 || betaK != 0.0 || betaK0 != 0.0 || betaKc != 0.0) {
    P += this->getRayleighDampingForces();
  }

  return P;
}

int
Timoshenko3d::sendSelf(int commitTag, Channel &theChannel)
{
  // place the integer data into an ID

  int dbTag = this->getDbTag();
  int i, j;
  int loc = 0;
  
  static ID idData(7);  // one bigger than needed so no clash later
  idData(0) = this->getTag();
  idData(1) = connectedExternalNodes(0);
  idData(2) = connectedExternalNodes(1);
  idData(3) = numSections;
  idData(4) = crdTransf->getClassTag();
  int crdTransfDbTag  = crdTransf->getDbTag();
  if (crdTransfDbTag  == 0) {
    crdTransfDbTag = theChannel.getDbTag();
    if (crdTransfDbTag  != 0) 
      crdTransf->setDbTag(crdTransfDbTag);
  }
  idData(5) = crdTransfDbTag;
  
  if (theChannel.sendID(dbTag, commitTag, idData) < 0) {
    opserr << "Timoshenko3d::sendSelf() - failed to send ID data\n";
     return -1;
  }    

  // send the coordinate transformation
  
  if (crdTransf->sendSelf(commitTag, theChannel) < 0) {
     opserr << "Timoshenko3d::sendSelf() - failed to send crdTranf\n";
     return -1;
  }      

  //
  // send an ID for the sections containing each sections dbTag and classTag
  // if section ha no dbTag get one and assign it
  //

  ID idSections(2*numSections);
  loc = 0;
  for (i = 0; i<numSections; i++) {
    int sectClassTag = theSections[i]->getClassTag();
    int sectDbTag = theSections[i]->getDbTag();
    if (sectDbTag == 0) {
      sectDbTag = theChannel.getDbTag();
      theSections[i]->setDbTag(sectDbTag);
    }

    idSections(loc) = sectClassTag;
    idSections(loc+1) = sectDbTag;
    loc += 2;
  }

  if (theChannel.sendID(dbTag, commitTag, idSections) < 0)  {
    opserr << "DispBeamColumn2d::sendSelf() - failed to send ID data\n";
    return -1;
  }    

  // send the sections
  
  for (j = 0; j<numSections; j++) {
    if (theSections[j]->sendSelf(commitTag, theChannel) < 0) {
      opserr << "Timoshenko3d::sendSelf() - section " << 
	j << "failed to send itself\n";
      return -1;
    }
  }

  return 0;
}

int
Timoshenko3d::recvSelf(int commitTag, Channel &theChannel,
			   FEM_ObjectBroker &theBroker)
{
  // receive the integer data containing tag, numSections and coord transformation info
  int dbTag = this->getDbTag();
  int i;
  
  static ID idData(7); // one bigger than needed so no clash with section ID

  if (theChannel.recvID(dbTag, commitTag, idData) < 0)  {
    opserr << "Timoshenko3d::recvSelf() - failed to recv ID data\n";
    return -1;
  }    

  this->setTag(idData(0));
  connectedExternalNodes(0) = idData(1);
  connectedExternalNodes(1) = idData(2);
  
  int crdTransfClassTag = idData(4);
  int crdTransfDbTag = idData(5);

  // create a new crdTransf object if one needed
  if (crdTransf == 0 || crdTransf->getClassTag() != crdTransfClassTag) {
      if (crdTransf != 0)
	  delete crdTransf;

      crdTransf = theBroker.getNewCrdTransf(crdTransfClassTag);

      if (crdTransf == 0) {
	opserr << "Timoshenko3d::recvSelf() - failed to obtain a CrdTrans object with classTag " <<
	  crdTransfClassTag << endln;
	  return -2;	  
      }
  }

  crdTransf->setDbTag(crdTransfDbTag);

  // invoke recvSelf on the crdTransf object
  if (crdTransf->recvSelf(commitTag, theChannel, theBroker) < 0) {
    opserr << "Timoshenko3d::sendSelf() - failed to recv crdTranf\n";
    return -3;
  }      
  
  //
  // recv an ID for the sections containing each sections dbTag and classTag
  //

  ID idSections(2*idData(3));
  int loc = 0;

  if (theChannel.recvID(dbTag, commitTag, idSections) < 0)  {
    opserr << "Timoshenko3d::recvSelf() - failed to recv ID data\n";
    return -1;
  }    

  // now receive the sections
   
  if (numSections != idData(3)) {

    //
    // we do not have correct number of sections, must delete the old and create
    // new ones before can recvSelf on the sections
    //

    // delete the old
    if (numSections != 0) {
      for (i=0; i<numSections; i++)
	delete theSections[i];
      delete [] theSections;
    }

    // create a new array to hold pointers
    theSections = new SectionForceDeformation *[idData(3)];
    if (theSections == 0) {
      opserr << "Timoshenko3d::recvSelf() - out of memory creating sections array of size " <<
      idData(3) << endln;
      return -1;
    }    

    // create a section and recvSelf on it
    numSections = idData(3);
    loc = 0;
    
    for (i=0; i<numSections; i++) {
      int sectClassTag = idSections(loc);
      int sectDbTag = idSections(loc+1);
      loc += 2;

      theSections[i] = theBroker.getNewSection(sectClassTag);
	  if (theSections[i] == 0) {
		opserr << "Timoshenko3d::recvSelf() Broker could not create Section of class type" <<
		sectClassTag << endln;
	    exit(-1);
      }

      theSections[i]->setDbTag(sectDbTag);
      if (theSections[i]->recvSelf(commitTag, theChannel, theBroker) < 0) {
	opserr << "Timoshenko3d::recvSelf() - section " << i << " failed to recv itself\n";
	return -1;
      }     
    }

  } else {

    // for each existing section, check it is of correct type
    // (if not delete old & create a new one) then recvSelf on it
      
    loc = 0;
    for (i=0; i<numSections; i++) {
      int sectClassTag = idSections(loc);
      int sectDbTag = idSections(loc+1);
      loc += 2;

      // check of correct type
      if (theSections[i]->getClassTag() !=  sectClassTag) {
	// delete the old section[i] and create a new one
	delete theSections[i];
	theSections[i] = theBroker.getNewSection(sectClassTag);
	if (theSections[i] == 0) {
	opserr << "Timoshenko3d::recvSelf() - Broker could not create Section of class type " <<
	  sectClassTag << endln;
	exit(-1);
	}
      }

      // recvSelf on it
      theSections[i]->setDbTag(sectDbTag);
      if (theSections[i]->recvSelf(commitTag, theChannel, theBroker) < 0) {
	opserr << "Timoshenko3d::recvSelf() - section " << i << " failed to recv itself\n";
	return -1;
      }     
    }
  }

  return 0;
}

void
Timoshenko3d::Print(OPS_Stream &s, int flag)		
{
  s << "\nTimoshenko3d, element id:  " << this->getTag() << endln;
  s << "\tConnected external nodes:  " << connectedExternalNodes;
  s << "\tCoordTransf: " << crdTransf->getTag() << endln;
  s << "\tmass density:  " << rho << endln;

  double N, Mz1, Mz2, Vy, My1, My2, Vz, T;
  double L = crdTransf->getInitialLength();
  double oneOverL = 1.0/L;

  N   = q(0);
  Mz1 = q(1);
  Mz2 = q(2);
  Vy  = (Mz1+Mz2)*oneOverL;
  My1 = q(3);
  My2 = q(4);
  Vz  = -(My1+My2)*oneOverL;
  T   = q(5);

  s << "\tEnd 1 Forces (P Mz Vy My Vz T): "
    << -N+p0[0] << ' ' << Mz1 << ' ' <<  Vy+p0[1] << ' ' << My1 << ' ' <<  Vz+p0[3] << ' ' << -T << endln;
  s << "\tEnd 2 Forces (P Mz Vy My Vz T): "
    <<  N << ' ' << Mz2 << ' ' << -Vy+p0[2] << ' ' << My2 << ' ' << -Vz+p0[4] << ' ' <<  T << endln;
  
  beamInt->Print(s, flag);

  for (int i = 0; i < numSections; i++)
	  theSections[i]->Print(s,flag);
}

int
Timoshenko3d::displaySelf(Renderer &theViewer, int displayMode, float fact)
{
  // first determine the end points of the quad based on
  // the display factor (a measure of the distorted image)
  const Vector &end1Crd = theNodes[0]->getCrds();
  const Vector &end2Crd = theNodes[1]->getCrds();	
  
  static Vector v1(3);
  static Vector v2(3);

  if (displayMode >= 0) {
    const Vector &end1Disp = theNodes[0]->getDisp();
    const Vector &end2Disp = theNodes[1]->getDisp();

    for (int i = 0; i < 3; i++) {
      v1(i) = end1Crd(i) + end1Disp(i)*fact;
      v2(i) = end2Crd(i) + end2Disp(i)*fact;    
    }
  } else {
    int mode = displayMode * -1;
    const Matrix &eigen1 = theNodes[0]->getEigenvectors();
    const Matrix &eigen2 = theNodes[1]->getEigenvectors();
    if (eigen1.noCols() >= mode) {
      for (int i = 0; i < 3; i++) {
	v1(i) = end1Crd(i) + eigen1(i,mode-1)*fact;
	v2(i) = end2Crd(i) + eigen2(i,mode-1)*fact;    
      }    

    } else {
      for (int i = 0; i < 3; i++) {
	v1(i) = end1Crd(i);
	v2(i) = end2Crd(i);
      }    
    }
  }
  return theViewer.drawLine (v1, v2, 1.0, 1.0);
}

Response*
Timoshenko3d::setResponse(const char **argv, int argc, OPS_Stream &s)
{
    Response *theResponse = 0;

    s.tag("ElementOutput");
    s.attr("eleType","DispBeamColumn3d");
    s.attr("eleTag",this->getTag());
    s.attr("node1",connectedExternalNodes[0]);
    s.attr("node2",connectedExternalNodes[1]);

    //
    // we compare argv[0] for known response types 
    //

    // global force - 
    if (strcmp(argv[0],"forces") == 0 || strcmp(argv[0],"force") == 0
	|| strcmp(argv[0],"globalForce") == 0 || strcmp(argv[0],"globalForces") == 0) {

      s.tag("ResponseType","Px_1");
      s.tag("ResponseType","Py_1");
      s.tag("ResponseType","Pz_1");
      s.tag("ResponseType","Mx_1");
      s.tag("ResponseType","My_1");
      s.tag("ResponseType","Mz_1");
      s.tag("ResponseType","Px_2");
      s.tag("ResponseType","Py_2");
      s.tag("ResponseType","Pz_2");
      s.tag("ResponseType","Mx_2");
      s.tag("ResponseType","My_2");
      s.tag("ResponseType","Mz_2");

      theResponse = new ElementResponse(this, 1, P);

    // local force -
    }  else if (strcmp(argv[0],"localForce") == 0 || strcmp(argv[0],"localForces") == 0) {

      s.tag("ResponseType","N_ 1");
      s.tag("ResponseType","Vy_1");
      s.tag("ResponseType","Vz_1");
      s.tag("ResponseType","T_1");
      s.tag("ResponseType","My_1");
      s.tag("ResponseType","Tz_1");
      s.tag("ResponseType","N_2");
      s.tag("ResponseType","Py_2");
      s.tag("ResponseType","Pz_2");
      s.tag("ResponseType","T_2");
      s.tag("ResponseType","My_2");
      s.tag("ResponseType","Mz_2");

      theResponse = new ElementResponse(this, 2, P);

    // chord rotation -
    }  else if (strcmp(argv[0],"chordRotation") == 0 || strcmp(argv[0],"chordDeformation") == 0 
	      || strcmp(argv[0],"basicDeformation") == 0) {

      s.tag("ResponseType","eps");
      s.tag("ResponseType","thetaZ_1");
      s.tag("ResponseType","thetaZ_2");
      s.tag("ResponseType","thetaY_1");
      s.tag("ResponseType","thetaY_2");
      s.tag("ResponseType","thetaX");

      theResponse = new ElementResponse(this, 3, Vector(6));

    // plastic rotation -
    } else if (strcmp(argv[0],"plasticRotation") == 0 || strcmp(argv[0],"plasticDeformation") == 0) {

    s.tag("ResponseType","epsP");
    s.tag("ResponseType","thetaZP_1");
    s.tag("ResponseType","thetaZP_2");
    s.tag("ResponseType","thetaYP_1");
    s.tag("ResponseType","thetaYP_2");
    s.tag("ResponseType","thetaXP");

    theResponse = new ElementResponse(this, 4, Vector(6));
  
  // section response -
  } 
    else if (strstr(argv[0],"sectionX") != 0) {
      if (argc > 2) {
	double sectionLoc = atof(argv[1]);
	
	double xi[maxNumSections];
	double L = crdTransf->getInitialLength();
	beamInt->getSectionLocations(numSections, L, xi);
	
	sectionLoc /= L;
	
	double minDistance = fabs(xi[0]-sectionLoc);
	int sectionNum = 0;
	for (int i = 1; i < numSections; i++) {
	  if (fabs(xi[i]-sectionLoc) < minDistance) {
	    minDistance = fabs(xi[i]-sectionLoc);
	    sectionNum = i;
	  }
	}
	
	s.tag("GaussPointOutput");
	s.attr("number",sectionNum+1);
	s.attr("eta",xi[sectionNum]*L);
	
	theResponse = theSections[sectionNum]->setResponse(&argv[2], argc-2, s);
      }
    }
    
    else if (strcmp(argv[0],"section") ==0) { 
      if (argc > 1) {
	
	int sectionNum = atoi(argv[1]);

	if (sectionNum > 0 && sectionNum <= numSections && argc > 2) {
	  
	  double xi[maxNumSections];
	  double L = crdTransf->getInitialLength();
	  beamInt->getSectionLocations(numSections, L, xi);
	  
	  s.tag("GaussPointOutput");
	  s.attr("number",sectionNum);
	  s.attr("eta",xi[sectionNum-1]*L);
	  
	  theResponse =  theSections[sectionNum-1]->setResponse(&argv[2], argc-2, s);
	  
	  s.endTag();
	} else if (sectionNum == 0) { // argv[1] was not an int, we want all sections, 
	
	  CompositeResponse *theCResponse = new CompositeResponse();
	  int numResponse = 0;
	  double xi[maxNumSections];
	  double L = crdTransf->getInitialLength();
	  beamInt->getSectionLocations(numSections, L, xi);
	  
	  for (int i=0; i<numSections; i++) {
	    
	    s.tag("GaussPointOutput");
	    s.attr("number",i+1);
	    s.attr("eta",xi[i]*L);
	    
	    Response *theSectionResponse = theSections[i]->setResponse(&argv[1], argc-1, s);
	    
	    s.endTag();	  
	    
	    if (theSectionResponse != 0) {
	      numResponse = theCResponse->addResponse(theSectionResponse);
	    }
	  }
	  
	  if (numResponse == 0) // no valid responses found
	    delete theCResponse;
	  else
	    theResponse = theCResponse;
	}
      }
    }
 
  s.endTag();
  return theResponse;
}

int 
Timoshenko3d::getResponse(int responseID, Information &eleInfo)		//LN modify
{
  double N, V, M1, M2, T;
  double L = crdTransf->getInitialLength();
  double oneOverL = 1.0/L;

  if (responseID == 1)
    return eleInfo.setVector(this->getResistingForce());
    
  else if (responseID == 2) {
    // Axial
    N = q(0);
    P(6) =  N;
    P(0) = -N+p0[0];
    
    // Torsion
    T = q(5);
    P(9) =  T;
    P(3) = -T;
    
    // Moments about z and shears along y
    M1 = q(1);
    M2 = q(2);
    P(5)  = M1;
    P(11) = M2;
    V = (M1+M2)*oneOverL;
    P(1) =  V+p0[1];
    P(7) = -V+p0[2];
    
    // Moments about y and shears along z
    M1 = q(3);
    M2 = q(4);
    P(4)  = M1;
    P(10) = M2;
    V = -(M1+M2)*oneOverL;
    P(2) = -V+p0[3];
    P(8) =  V+p0[4];

    return eleInfo.setVector(P);
  }

  // Chord rotation
  else if (responseID == 3)
    return eleInfo.setVector(crdTransf->getBasicTrialDisp());

  // Plastic rotation
  else if (responseID == 4) {
    static Vector vp(6);
    static Vector ve(6);
    const Matrix &kb = this->getInitialBasicStiff();
    kb.Solve(q, ve);
    vp = crdTransf->getBasicTrialDisp();
    vp -= ve;
    return eleInfo.setVector(vp);
  }

  else
    return -1;
}

Matrix
Timoshenko3d::getNd(int sec, const Vector &v, double L)
{
  double pts[maxNumSections];
  beamInt->getSectionLocations(numSections, L, pts);
  double x     = L * pts[sec];
  Rslt = theSections[sec]->getStressResultant();
  Defo  = theSections[sec]->getSectionDeformation();

  double OmegaZ, OmegaY;
  if (Rslt(5)*Defo(2) != 0.0) OmegaZ = Rslt(2)*Defo(5)/Rslt(5)/Defo(2)/L/L; 
  else                        OmegaZ = 0.;

  if (Rslt(4)*Defo(3) != 0.0) OmegaY = Rslt(3)*Defo(4)/Rslt(4)/Defo(3)/L/L;
  else                        OmegaY = 0.;

  double muZ    = 1./(1.+12.*OmegaZ);
  double phi1Z  =  muZ*x*(L-x)*(L-x+6.*L*OmegaZ)                      /L/L;
  //double phi1pZ =  muZ*(3.*x*x+L*L*(1+6.*OmegaZ)-4.*L*(x+3.*x*OmegaZ))/L/L;
  double phi2Z  = -muZ*x*(L-x)*(x + 6.*L*OmegaZ)                      /L/L;
  //double phi2pZ =  muZ*(3.*x*x-L*L* 6. *OmegaZ  +2.*L*x*(6.*OmegaZ-1))/L/L;
  double phi3Z  =  muZ*(L-x)*(L-3.*x+12*L*OmegaZ)                     /L/L;
  //double phi3pZ =  muZ*(6.*x - 4.*L * (1+3.*OmegaZ))                  /L/L;
  double phi4Z  =  muZ*x*(  3.*x+2*L*(6*OmegaZ-1))                    /L/L;
  //double phi4pZ =  muZ*2.*(3.*x+L*(6.*OmegaZ-1))                      /L/L;

  double muY    = 1./(1.+12.*OmegaY);
  double phi1Y  =  muY*x*(L-x)*(L-x+6.*L*OmegaY)                      /L/L;
  //double phi1pY =  muY*(3.*x*x+L*L*(1+6.*OmegaY)-4.*L*(x+3.*x*OmegaY))/L/L;
  double phi2Y  = -muY*x*(L-x)*(x + 6.*L*OmegaY)                      /L/L;
  //double phi2pY =  muY*(3.*x*x-L*L* 6. *OmegaY  +2.*L*x*(6.*OmegaY-1))/L/L;
  double phi3Y  =  muY*(L-x)*(L-3.*x+12*L*OmegaY)                     /L/L;
  //double phi3pY =  muY*(6.*x - 4.*L * (1+3.*OmegaY))                  /L/L;
  double phi4Y  =  muY*x*(  3.*x+2*L*(6*OmegaY-1))                    /L/L;
  //double phi4pY =  muY*2.*(3.*x+L*(6.*OmegaY-1))                      /L/L;

  Matrix Nd(6,6);
  Nd.Zero();
  //P, Mz, My, Vy, Vz, T
  Nd(0,0) = 1.;
  Nd(1,1) = phi3Z;
  Nd(1,2) = phi4Z;
  Nd(2,3) = phi3Y;
  Nd(2,4) = phi4Y;
  Nd(3,1) = phi1Y; // shear components
  Nd(3,2) = phi2Y; // shear components
  Nd(4,3) = phi1Z; // shear components 
  Nd(4,4) = phi2Z; // shear components 
  Nd(5,5) = 1.; // torsion components 

  return Nd;
}

Matrix
Timoshenko3d::getBd(int sec, const Vector &v, double L)
{
  double pts[maxNumSections];
  beamInt->getSectionLocations(numSections, L, pts);
  
  double x = L * pts[sec];
  Rslt = theSections[sec]->getStressResultant();
  Defo  = theSections[sec]->getSectionDeformation();

  double OmegaZ, OmegaY;
  if (Rslt(5)*Defo(2) != 0.0) OmegaZ = Rslt(2)*Defo(5)/Rslt(5)/Defo(2)/L/L; 
  else                        OmegaZ = 0.;

  if (Rslt(4)*Defo(3) != 0.0) OmegaY = Rslt(3)*Defo(4)/Rslt(4)/Defo(3)/L/L;
  else                        OmegaY = 0.;

  double muZ    = 1./(1.+12.*OmegaZ);
  //double phi1Z  =  muZ*x*(L-x)*(L-x-6*OmegaZ*L)                    /L/L;
  double phi1pZ =  muZ*(3*x*x-4*L*x*(1-3*OmegaZ)-L*L*(6*OmegaZ-1))/L/L;
  //double phi2Z  =  muZ*x*(L-x)*(6*OmegaZ*L-x)                      /L/L;
  double phi2pZ =  muZ*(6*L*(L-2*x)*OmegaZ-(2*L-3*x)*x)             /L/L;
  double phi3Z  =  (L-x)*muZ*(L-3*x-12*L*OmegaZ)                    /L/L;
  double phi3pZ =  2*muZ*(3*x+L*(6*OmegaZ-2))                       /L/L;
  double phi4Z  =  x*muZ*(3*x-2*L*(1+6*OmegaZ))                     /L/L;
  double phi4pZ =  -2*muZ*(L-3*x+6*L*OmegaZ)                        /L/L;

  double muY    = 1./(1.+12.*OmegaY);
  //double phi1Y  =  muY*x*(L-x)*(L-x-6*OmegaY*L)                    /L/L;
  double phi1pY =  muY*(3*x*x-4*L*x*(1-3*OmegaY)-L*L*(6*OmegaY-1))/L/L;
  //double phi2Y  =  muY*x*(L-x)*(6*OmegaY*L-x)                      /L/L;
  double phi2pY =  muY*(6*L*(L-2*x)*OmegaY-(2*L-3*x)*x)             /L/L;
  double phi3Y  =  (L-x)*muY*(L-3*x-12*L*OmegaY)                    /L/L;
  double phi3pY =  2*muY*(3*x+L*(6*OmegaY-2))                       /L/L;
  double phi4Y  =  x*muY*(3*x-2*L*(1+6*OmegaY))                     /L/L;
  double phi4pY =  -2*muY*(L-3*x+6*L*OmegaY)                        /L/L;

  Matrix Bd(6,6);
  Bd.Zero();
  //P, Mz, My, Vy, Vz, T
  Bd(0,0) = 1./L;
  Bd(1,1) = phi3pZ;
  Bd(1,2) = phi4pZ;
  Bd(2,3) = phi3pY; 
  Bd(2,4) = phi4pY; 
  Bd(3,1) = phi1pY-phi3Y; // shear components
  Bd(3,2) = phi2pY-phi4Y; // shear components
  Bd(4,3) = phi1pZ-phi3Z; // shear components 
  Bd(4,4) = phi2pZ-phi4Z; // shear components 
  Bd(5,5) = 1./L; // torsion components 

  return Bd;
}

// AddingSensitivity:BEGIN ///////////////////////////////////

const Matrix &
Timoshenko3d::getKiSensitivity(int gradNumber)
{
	K.Zero();
	return K;
}

const Matrix &
Timoshenko3d::getMassSensitivity(int gradNumber)
{
	K.Zero();
	return K;
}

const Vector &
Timoshenko3d::getResistingForceSensitivity(int gradNumber)
{
	static Vector dummy(3);		// No distributed loads
	return dummy;
}

// NEW METHOD
int
Timoshenko3d::commitSensitivity(int gradNumber, int numGrads)
{
 	return 0;
}

// AddingSensitivity:END /////////////////////////////////////////////