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

// $Revision: 1.0 $
// $Date: 2011/08/30 23:00:47 $
// $Source: /usr/local/cvs/OpenSees/SRC/material/nD/MCFTRCPlaneStress.cpp,v $
// File: MCFTRCPlaneStress.cpp
//
// Written: Li Ning (Neallee@tju.edu.cn)
// Created: 2011.8
//
// Description: This file contains the class definition for
// MCFTRCPlaneStress
// For Detailed explanation of the model, please refer to the journal paper
// entitled "The modified compression-filed theory for reinforced concrete element
// subjected to shear, ACI Journal. s83-22. 1986. pp. 219-231"

#include <MCFTRCPlaneStress.h>
#include <UniaxialMaterial.h>
#include <Vector.h>
#include <Matrix.h>
#include <math.h>
#include <float.h>
#include <Channel.h>
#include <FEM_ObjectBroker.h>
#include <stdlib.h>

#include <Tensor.h>

#include <DummyStream.h>
#include <MaterialResponse.h>
#include <elementAPI.h>
#include <f2c.h>

using namespace std;

#define OPS_Export 

static int numMCFTRCPlaneStressMaterials = 0;

OPS_Export void *
OPS_NewMCFTRCPlaneStressMaterial()
{
  if (numMCFTRCPlaneStressMaterials == 0) {
    numMCFTRCPlaneStressMaterials++;
    //OPS_Error("MCFTRCPlaneStress material - Written by Lining - Copyright@2011\n", 1);
  }

  NDMaterial *theMaterial = 0;

  int numRemainingArgs = OPS_GetNumRemainingInputArgs();

  if (numRemainingArgs < 16) {
    opserr << "Invalid Args want: NDMaterial MCFTRCPlaneStress matTag? rho? UniaxiaMatTag1? UniaxiaMatTag2? UniaxiaMatTag3? UniaxiaMatTag4? \
               angle1? angle2? rhox? rhoy? dbx? dby? fpc? fyx? fyy? Ec0? epsc0? aggsize? xd? yd?\n";
    return 0;	
  }

  int tag;
  double rho;
  int    iData[4];
  double dData[14];
  int numData = 0;

  numData = 1;
  if (OPS_GetInt(&numData, &tag) != 0) {
    opserr << "WARNING invalid nDMaterial MCFTRCPlaneStress tag" << endln;
    return 0;
  }

  numData = 1;
  if (OPS_GetDouble(&numData, &rho) != 0) {
    opserr << "WARNING invalid parameter rho MCFTRCPlaneStress tag:" << tag << endln;
    return 0;	
  }

  numData = 4;
  if (OPS_GetInt(&numData, iData) != 0) {
    opserr << "WARNING invalid nDMaterial MCFTRCPlaneStress tag: " << tag << endln;
    return 0;
  }

  numData = 14;
  if (OPS_GetDouble(&numData, dData) != 0) {
    opserr << "WARNING invalid data MCFTRCPlaneStress tag: " << tag << endln;
    return 0;
  }

  UniaxialMaterial *theUniaxialMaterial1 = OPS_GetUniaxialMaterial(iData[0]);
    
  if (theUniaxialMaterial1 == 0) {
    opserr << "WARNING material not found\n";
    opserr << "Material: " << iData[0];
    opserr << "\nMCFTRCPlaneStress tag: " << tag << endln;
    return 0;
  }
  
  UniaxialMaterial *theUniaxialMaterial2 = OPS_GetUniaxialMaterial(iData[1]);

  if (theUniaxialMaterial2 == 0) {
    opserr << "WARNING material not found\n";
    opserr << "Material: " << iData[1];
    opserr << "\nMCFTRCPlaneStress tag: " << tag << endln;
    return 0;
  }
  
  UniaxialMaterial *theUniaxialMaterial3 = OPS_GetUniaxialMaterial(iData[2]);
  if (theUniaxialMaterial3 == 0) {
    opserr << "WARNING material not found\n";
    opserr << "Material: " << iData[2];
    opserr << "\nMCFTRCPlaneStress tag: " << tag << endln;
    return 0;
  }

  UniaxialMaterial *theUniaxialMaterial4 = OPS_GetUniaxialMaterial(iData[3]);  
  if (theUniaxialMaterial4 == 0) {
    opserr << "WARNING material not found\n";
    opserr << "Material: " << iData[3];
    opserr << "\nMCFTRCPlaneStress tag: " << tag << endln;
    return 0;
  }

  theMaterial = new MCFTRCPlaneStress (tag, rho,
						     theUniaxialMaterial1, theUniaxialMaterial2, 
						     theUniaxialMaterial3, theUniaxialMaterial4,
						     dData[0], dData[1], dData[2], dData[3], dData[4],
							 dData[5], dData[6], dData[7], dData[8], dData[9],
							 dData[10],dData[11],dData[12],dData[13]);
       
  if (theMaterial == 0) {
    opserr << "WARNING ran out of memory creating material\n";
    opserr << "MCFTRCPlaneStress: " << tag << endln;
    return 0;
  }

  return theMaterial;
}
 
MCFTRCPlaneStress ::MCFTRCPlaneStress (int tag, 
								   double RHO,
								   UniaxialMaterial *s1,
								   UniaxialMaterial *s2,
								   UniaxialMaterial *c1,
								   UniaxialMaterial *c2,
								   double   ANGLE1, double   ANGLE2,
								   double   ROU1,   double   ROU2,
								   double   DB1,    double   DB2,
								   double   FPC,
								   double   FYX,    double   FYY,
								   double   E,
								   double   EPSC0,
								   double   AGGR,
								   double   XD,     double   YD):
  NDMaterial(tag, ND_TAG_MCFTRCPlaneStress),
  rho(RHO), angle1(ANGLE1), angle2(ANGLE2), rhox(ROU1), rhoy(ROU2), db1(DB1), db2(DB2),
  fpc(FPC), fyx(FYX), fyy(FYY), Es(E), epsc0(EPSC0), aggr(AGGR), xd(XD), yd(YD),
  lastStress(3), Tstress(3), stress_vec(3), stress0_vec(3), strain_vec(3),
  secant_matrix(3,3), tangent_matrix(3,3), CepsC12p(2), epsC12p(2), 
  epsC_vec(3), epsCe_vec(3), epsCp_vec(3), epsSlip_vec(3),
  CepsC_vec(3), CepsCe_vec(3), CepsCp_vec(3), CepsSlip_vec(3), 
  epsC12cm_vec(2), epsCcm_vec(3), epsC12tm_vec(2), epsCtm_vec(3),
  CepsC12cm_vec(2), CepsCcm_vec(3), CepsC12tm_vec(2), CepsCtm_vec(3),
  citaE(0.0), citaS(0.0)
{
    CepsC_vec.Zero();
	CepsCe_vec.Zero();
	CepsCp_vec.Zero();
	CepsSlip_vec.Zero();
	CepsCcm_vec.Zero();
	CepsCtm_vec.Zero();
    CepsC12cm_vec.Zero();
	CepsC12tm_vec.Zero();

	fC1 = 0.0; epsC1 = 0.0;
	fC2 = 0.0; epsC2 = 0.0;

    lastStress.Zero();
    
    if ( fpc > 0.0 ) { fpc = -fpc; } // set for fpc < 0
    if ( epsc0 > 0.0 ) { epsc0 = -epsc0; }
    theMaterial = 0;
    
    // Allocate pointers to theSteel1
    theMaterial = new UniaxialMaterial *[4];
    
    if ( theMaterial == 0 ) {
      opserr << " MCFTRCPlaneStress::MCFTRCPlaneStress - failed allocate material array\n";
      exit(-1);
    }
    
    // Get the copy for the Steel1
    theMaterial[S_ONE] = s1->getCopy();
    if ( theMaterial[S_ONE] == 0 ) {
      opserr << " MCFTRCPlaneStress::MCFTRCPlaneStress - failed to get a copy for steel1\n";
      exit(-1);
    }
    
    // Get the copy for the Steel2
    theMaterial[S_TWO] = s2->getCopy();	
    if ( theMaterial[S_TWO] == 0 ) {
      opserr << " MCFTRCPlaneStress::MCFTRCPlaneStress - failed to get a copy for steel2\n";
      exit(-1);
    }
    
    // Get the copy for the Concrete1
    theMaterial[C_ONE] = c1->getCopy();	
    if ( theMaterial[C_ONE] == 0 ) {
      opserr << " MCFTRCPlaneStress::MCFTRCPlaneStress - failed to get a copy for concrete1\n";
      exit(-1);
    }
    
    // Get the copy for the Concrete2
    theMaterial[C_TWO] = c2->getCopy();	
    if ( theMaterial[C_TWO] == 0 ) {
      opserr << " MCFTRCPlaneStress::MCFTRCPlaneStress - failed to get a copy for concrete2\n";
      exit(-1);
    }
    
    theResponses = new Response *[8];  
    if ( theResponses == 0) {
      opserr << " MCFTRCPlaneStress::MCFTRCPlaneStress - failed allocate responses array\n";
      exit(-1);
    }
    
    OPS_Stream *theDummyStream = new DummyStream();
    const char **argv = new const char *[1];
    argv[0] = "setVar";
    theResponses[0] = theMaterial[S_ONE]->setResponse(argv, 1, *theDummyStream);  //s1
    theResponses[1] = theMaterial[S_TWO]->setResponse(argv, 1, *theDummyStream);  //s2
	argv[0] = "getVar";
	theResponses[2] = theMaterial[S_ONE]->setResponse(argv, 1, *theDummyStream);  //s1
	theResponses[3] = theMaterial[S_TWO]->setResponse(argv, 1, *theDummyStream);  //s2
    argv[0] = "setVar";
    theResponses[4] = theMaterial[C_ONE]->setResponse(argv, 1, *theDummyStream);  //c1
    theResponses[5] = theMaterial[C_TWO]->setResponse(argv, 1, *theDummyStream);  //c2
    argv[0] = "getVar";
    theResponses[6] = theMaterial[C_ONE]->setResponse(argv, 1, *theDummyStream);  //c1
    theResponses[7] = theMaterial[C_TWO]->setResponse(argv, 1, *theDummyStream);  //c2

    if ((theResponses[0] == 0) || (theResponses[1] == 0) ||
	(theResponses[2] == 0) || (theResponses[3] == 0) ||
	(theResponses[4] == 0) || (theResponses[5] == 0) ||
	(theResponses[6] == 0) || (theResponses[7] == 0) ) {
      opserr << " MCFTRCPlaneStress::MCFTRCPlaneStress - failed to set appropriate materials tag: " << tag << "\n";
      exit(-1);
    }
    
    delete theDummyStream;
	strain_vec.Zero();
    this->revertToStart();
	determineTrialStress(strain_vec);
    
}

MCFTRCPlaneStress::MCFTRCPlaneStress()
  :NDMaterial(0, ND_TAG_MCFTRCPlaneStress),
  lastStress(3), Tstress(3), stress_vec(3), stress0_vec(3),  strain_vec(3),
  secant_matrix(3,3),tangent_matrix(3,3), CepsC12p(2), epsC12p(2), 
  epsC12cm_vec(2), epsCcm_vec(3), epsC12tm_vec(2), epsCtm_vec(3),
  epsC_vec(3), epsCe_vec(3), epsCp_vec(3), epsSlip_vec(3),
  citaE(0.0), citaS(0.0)
{
  CepsC_vec.Zero();
  CepsCe_vec.Zero();
  CepsCp_vec.Zero();
  CepsSlip_vec.Zero();
  CepsCcm_vec.Zero();
  CepsCtm_vec.Zero();
  CepsC12cm_vec.Zero();
  CepsC12tm_vec.Zero();

  fC1 = 0.0; epsC1 = 0.0;
  fC2 = 0.0; epsC2 = 0.0;

  theMaterial = 0;
  theResponses = 0;

  strain_vec.Zero();

  this->revertToStart();
  determineTrialStress(strain_vec);

}

MCFTRCPlaneStress::~MCFTRCPlaneStress()
{
  if (theMaterial != 0) {
    for (int i=0; i<4; i++) {
	  if (theMaterial[i])
	    delete theMaterial[i];
    }
    delete [] theMaterial;
  }
  if (theResponses != 0) {
    for (int j=0; j<8; j++) {
	  if (theResponses[j] != 0)
	    delete theResponses[j];
    }
    delete [] theResponses;
  }
}

double 
MCFTRCPlaneStress::getRho(void)
{
	return rho;
}

// really used one
int 
MCFTRCPlaneStress::setTrialStrain(const Vector &v)
{
  strain_vec = v;  //strain_vec(2) *= 0.5; //halfGamma
  //Tstress = determineTrialStress(strain_vec);
  if (determineSecantModulus(strain_vec) != 0 )
	opserr << "setTrialStrain error!" << endln;

  return 0;
}

int 
MCFTRCPlaneStress::setTrialStrain(const Vector &v, const Vector &r)
{
  opserr << "error: MCFTRCPlaneStress::setTrialStrain(&v, &r) -- not really responsibility" << endln;
  return 0;
}

int 
MCFTRCPlaneStress::setTrialStrainIncr(const Vector &v)
{
  opserr << "error:MCFTRCPlaneStress::setTrialStrainIncr(&v) -- not really responsibility" << endln;
  return 0;
}

int
MCFTRCPlaneStress::setTrialStrainIncr(const Vector &v, const Vector &r)
{
  opserr << "error:MCFTRCPlaneStress::setTrialStrainIncr(&v, &r) -- not really responsibility" << endln;
  return 0;
}

const Matrix& 
MCFTRCPlaneStress::getTangent(void)
{
  //determineTrialStress(strain_vec);
  //determineTangent(strain_vec);
  return tangent_matrix;
}

const Vector& 
MCFTRCPlaneStress::getStress(void)
{
  return stress_vec;
}

const Vector& 
MCFTRCPlaneStress::getStrain()
{
  return strain_vec;
}

const Vector& 
MCFTRCPlaneStress::getCommittedStress(void)
{
  return stress_vec;
}

const Vector& 
MCFTRCPlaneStress::getCommittedStrain(void)
{
  return strain_vec;
}

int
MCFTRCPlaneStress::commitState(void)
{
  for (int i=0; i<4; i++) {
    theMaterial[i]->commitState();
  }

  CepsC_vec    = epsC_vec; 
  CepsCe_vec   = epsCe_vec; 
  CepsCp_vec   = epsCp_vec;   
  CepsSlip_vec = epsSlip_vec;
  CepsCcm_vec  = epsCcm_vec;  
  CepsCtm_vec  = epsCtm_vec;  
  CepsC12cm_vec = epsC12cm_vec;
  CepsC12tm_vec = epsC12tm_vec;
  CepsC12p = epsC12p;

  lastStress = stress_vec;
  
  return 0;
}

int
MCFTRCPlaneStress::revertToLastCommit(void)
{
  for (int i=0; i<4; i++) {
    theMaterial[i]->revertToLastCommit();
  }

  epsC_vec    = CepsC_vec; 
  epsCe_vec   = CepsCe_vec;
  epsCp_vec   = CepsCp_vec;
  epsSlip_vec = CepsSlip_vec;
  epsCcm_vec = CepsCcm_vec;  
  epsCtm_vec = CepsCtm_vec;  
  epsC12cm_vec = CepsC12cm_vec;
  epsC12tm_vec = CepsC12tm_vec;

  epsC12p = CepsC12p;
  stress_vec = lastStress;

  return 0;
}

int
MCFTRCPlaneStress::revertToStart(void)
{
  for (int i=0; i<4; i++) {
    theMaterial[i]->revertToStart();
  }
  
  Tstress.Zero();
  strain_vec.Zero();
  stress_vec.Zero();

  epsC_vec.Zero();
  epsCe_vec.Zero();
  epsCp_vec.Zero();
  epsSlip_vec.Zero();
  CepsC_vec.Zero();
  CepsCe_vec.Zero();
  CepsCp_vec.Zero();
  CepsSlip_vec.Zero();

  epsC12p.Zero();
  CepsC12p.Zero();

  epsC12cm_vec.Zero();
  epsC12tm_vec.Zero();
  CepsC12cm_vec.Zero();
  CepsC12tm_vec.Zero();

  epsCcm_vec.Zero();
  epsCtm_vec.Zero();
  CepsCcm_vec.Zero();
  CepsCtm_vec.Zero();

  
  return 0;
}

NDMaterial* 
MCFTRCPlaneStress::getCopy(void)
{
  MCFTRCPlaneStress* theCopy =
    new MCFTRCPlaneStress( this->getTag(), 
					 rho, 
					 theMaterial[S_ONE], 
					 theMaterial[S_TWO], 
					 theMaterial[C_ONE], 
					 theMaterial[C_TWO], 
					 angle1, angle2, 
					 rhox, rhoy, 
					 db1, db2, fpc, fyx, fyy,
					 Es, epsc0, aggr, xd, yd);
  theCopy->strain_vec = strain_vec;
  return theCopy;
}

NDMaterial* 
MCFTRCPlaneStress::getCopy(const char *type)
{
	MCFTRCPlaneStress* theModel =
		new MCFTRCPlaneStress( this->getTag(), 
           rho, theMaterial[S_ONE], theMaterial[S_TWO], theMaterial[C_ONE], theMaterial[C_TWO],
		   angle1, angle2, rhox, rhoy, db1, db2, fpc, fyx, fyy, Es, epsc0, aggr, xd, yd );
	theModel->strain_vec = strain_vec;
	theModel->stress_vec = stress_vec;
	return theModel;
}

Response*
MCFTRCPlaneStress::setResponse (const char **argv, int argc, OPS_Stream &output)
{
    //Response *theResponse =0;
	const char *matType = this->getType();

	output.tag("NdMaterialOutput");
	output.attr("matType",this->getClassType());
	output.attr("matTag",this->getTag());

	if (strcmp(argv[0],"stress") == 0 || strcmp(argv[0],"stresses") == 0)
		return new MaterialResponse(this, 1, this->getStress());
	else if (strcmp(argv[0],"strain") == 0 || strcmp(argv[0],"strains") == 0)
		return new MaterialResponse(this, 2, this->getStrain());
	//else if (strcmp(argv[0], "state") == 0)
		//return new MaterialResponse(this, 3, this->getState());
    else if (strcmp(argv[0], "steel1") == 0)
        return theMaterial[S_ONE]->setResponse(argv+1, argc-1, output);
	else if (strcmp(argv[0], "steel2") == 0)
		return theMaterial[S_TWO]->setResponse(argv+1, argc-1, output);
	else if (strcmp(argv[0], "concrete1") == 0)
		return theMaterial[C_ONE]->setResponse(argv+1, argc-1, output);
	else if (strcmp(argv[0], "concrete2") == 0)
		return theMaterial[C_TWO]->setResponse(argv+1, argc-1, output);
	else
		return 0;
}

int
MCFTRCPlaneStress::getResponse (int responseID, Information &matInfo)
{

	switch (responseID) {
	case -1:
		return -1;
	case 1:
		if (matInfo.theVector != 0)
			*(matInfo.theVector) = getStress();
		return 0;
	case 2:
		if (matInfo.theVector != 0)
			*(matInfo.theVector) = getStrain();
		return 0;
	case 3:
		//if (matInfo.theVector != 0)
		//	*(matInfo.theVector) = getState();
		return 0;
	default:
		return -1;
	}
}

void
MCFTRCPlaneStress::Print(OPS_Stream &s, int flag )
{
	s << "\n\tMCFTRCPlaneStress, material id: " << this->getTag() << endln;

	s << "\tRho: " << rho << endln;
	s << "\tangle1: " << angle1 << "\tangle2: " << angle2 << endln;
	s << "\trou1: " << rhox << "\trou2: " << rhoy << endln;
	s << "\tfpc: " << fpc << "\tepsc0: " << epsc0 << "\taggr: " << aggr <<endln;
	s << "\tfyx: " << fyx << "\tfyy: " << fyy << "\tEs: " << Es << endln;

	s << "\tEnd of MCFTPSMatreial print()." << endln << endln;

	s << "\t call the material print() function : "<< endln;
	
	s << "\t the steel 1 information is : " << endln;
	theMaterial[S_ONE]->Print(s,flag);
	s << "\t the steel 2 information is : " << endln;
	theMaterial[S_TWO]->Print(s,flag);
	s << "\t the concrete 1 information is : " << endln;
	theMaterial[C_ONE]->Print(s,flag);
	s << "\t the concrete 2 information is : " << endln;
	theMaterial[C_TWO]->Print(s,flag);

	s << "\tStrain and stress of the uniaxial materials:"<<endln;
	for (int i=0; i<4; i++)
	{
	  s << "\tUniaxial Material " << i+1 << " :" << endln;
	  s << "\t            Strain : " << theMaterial[i]->getStrain() << endln;
	  s << "\t            Stress : " << theMaterial[i]->getStress() << endln;
	  s << "\t  Uniaxial Tangent : " << theMaterial[i]->getTangent() << endln;
	  s << "\t  Uniaxial Secant  : " << theMaterial[i]->getSecant() << endln;
	}
}

int
MCFTRCPlaneStress::sendSelf(int commitTag, Channel &theChannel)
{
  int res = 0;
  
  int dataTag = this->getDbTag();
  
  // Packs its data into a Vector and sends this to theChannel
  static Vector data(16);
  data(0) = this->getTag();
  data(1) = rho;
  data(2) = angle1;
  data(3) = angle2;
  data(4) = rhox;
  data(5) = rhoy;
  data(6) = db1;
  data(7) = db2;
  data(8) = fpc;
  data(9) = fyx;
  data(10) = fyy;
  data(11) = Es;
  data(12) = epsc0;
  data(13) = aggr;
  data(14) = xd;
  data(15) = yd;

  res += theChannel.sendVector(dataTag, commitTag, data);
  if (res < 0) {
    opserr << "WARNING MCFTRCPlaneStress::sendSelf() - " << this->getTag() << " failed to send Vector\n";
    return res;
  }	      
  
  // Now sends the IDs of its materials
  int matDbTag;
  
  static ID idData(8);
  
  // NOTE: to ensure that the material has a database
  // tag if sending to a database channel.
  
  int i;
  for (i=0; i<4; i++) {
    idData(i) = theMaterial[i]->getClassTag();
    matDbTag = theMaterial[i]->getDbTag();
    if (matDbTag == 0) {
	  matDbTag = theChannel.getDbTag();
	  if (matDbTag != 0)
	    theMaterial[i]->setDbTag(matDbTag);
    }
    idData(i+4) = matDbTag;
  }
  
  res += theChannel.sendID(dataTag, commitTag, idData);
  if (res < 0) {
    opserr << "WARNING MCFTRCPlaneStress::sendSelf() - " << this->getTag() << " failed to send ID\n";
    return res;
  }
  
  // Finally, quad asks its material objects to send themselves
  for (i = 0; i < 4; i++) {
    res += theMaterial[i]->sendSelf(commitTag, theChannel);
    if (res < 0) {
      opserr << "MCFTRCPlaneStress::sendSelf() - " << this->getTag() << " failed to send its Material\n";
      return res;
    }
  }	
  
  return res;
}

int
MCFTRCPlaneStress::recvSelf(int commitTag, Channel &theChannel, FEM_ObjectBroker &theBroker)
{
  int res = 0;
  
  int dataTag = this->getDbTag();
  
  // Quad creates a Vector, receives the Vector and then sets the 
  // internal data with the data in the Vector
  static Vector data(16);
  res += theChannel.recvVector(dataTag, commitTag, data);
  if (res < 0) {
    opserr << "WARNING MCFTRCPlaneStress::recvSelf() - failed to receive Vector\n";
    return res;
  }
  
  this->setTag((int)data(0));
  rho = data(1);
  angle1 = data(2);
  angle2 = data(3);
  rhox   = data(4);
  rhoy   = data(5);
  db1    = data(6);
  db2    = data(7);
  fpc    = data(8);
  fyx    = data(9);
  fyy    = data(10);
  Es     = data(11);
  epsc0  = data(12);
  aggr   = data(13);
  xd     = data(14);
  yd     = data(15);

  static ID idData(8);
  
  // now receives the tags of its materials
  res += theChannel.recvID(dataTag, commitTag, idData);
  if (res < 0) {
    opserr << "WARNING MCFTRCPlaneStress::recvSelf() - " << this->getTag() << " failed to receive ID\n";
    return res;
  }

  if (theMaterial == 0) {
    // Allocate new materials
    theMaterial = new UniaxialMaterial *[4];
    if (theMaterial == 0) {
      opserr << "MCFTRCPlaneStress::recvSelf() - Could not allocate UniaxialMaterial* array\n";
      return -1;
    }
    for (int i = 0; i < 4; i++) {
      int matClassTag = idData(i);
      int matDbTag = idData(i+4);
      // Allocate new material with the sent class tag
      theMaterial[i] = theBroker.getNewUniaxialMaterial(matClassTag);
      if (theMaterial[i] == 0) {
	     opserr << "MCFTRCPlaneStress::recvSelf() - Broker could not create NDMaterial of class type " << matClassTag << endln;
	     return -1;
      }
      // Now receive materials into the newly allocated space
      theMaterial[i]->setDbTag(matDbTag);
      res += theMaterial[i]->recvSelf(commitTag, theChannel, theBroker);
      if (res < 0) {
         opserr << "MCFTRCPlaneStress::recvSelf() - material " << i << "failed to recv itself\n";
	     return res;
      }
    }
  }

  // materials exist , ensure materials of correct type and recvSelf on them
  else {
    for (int i = 0; i < 4; i++) {
      int matClassTag = idData(i);
      int matDbTag = idData(i+4);
      // Check that material is of the right type; if not,
      // delete it and create a new one of the right type
      if (theMaterial[i]->getClassTag() != matClassTag) {
	     delete theMaterial[i];
	     theMaterial[i] = theBroker.getNewUniaxialMaterial(matClassTag);
	     if (theMaterial[i] == 0) {
            opserr << "MCFTRCPlaneStress::recvSelf() - material " << i << "failed to create\n";
	        return -1;
		 }
	  }
      // Receive the material
      theMaterial[i]->setDbTag(matDbTag);
      res += theMaterial[i]->recvSelf(commitTag, theChannel, theBroker);
      if (res < 0) {
        opserr << "MCFTRCPlaneStress::recvSelf() - material " << i << "failed to recv itself\n";
	    return res;
	  }
    }
  }

  return res;
}

Vector
MCFTRCPlaneStress::determineTrialStress(Vector strain)
{ 
  // Get Principal strain direction first // Get strain values from strain of element
  Vector Tstrain(3);     //epslon_x, epslon_y, gamma_xy
  Tstrain(0) = strain(0);
  Tstrain(1) = strain(1);
  Tstrain(2) = 0.5*strain(2);

  double cita, temp_cita, citan1, citan2, citaLag, dCitaE, dCitaS, citaIC; // principal strain direction
  double epsSx, epsSy, epsts;
  double eC1p=0.0, eC2p=0.0, eC1m=0.0, eC2m=0.0, eT1m=0.0, eT2m=0.0, eSlip1=0.0, eSlip2=0.0;
  double fC1a, fC1b, fC1c, fC2a, fC2b, fC2c, fSx, fSy; //
  double halfGammaOneTwo;

  double fcr   = 0.65 * pow(-fpc, 0.33); //0.31*sqrt(-fpc);      ----ftp
  double Ec    = 2.0 * fpc/epsc0;
  double epscr = fcr/Ec;
  double vci, vcimax, s_cita=0.0, w=0.0, delta_S=0.0, gamma_S=0.0; //vcxy, 
  double betaD1, betaD2;
  double Ecx, Ecy;

  static Vector tempStrain(3); // maintain main strain vector 
  int Par=0;

  // Get citaE based on Tstrain, eq. i-9...
  if ( fabs(Tstrain(0)-Tstrain(1)) < SMALL_STRAIN ) {
	//if (fabs(Tstrain(2)) < SMALL_STRAIN) {
	//  citaE = 0;
	//} else {
	  citaE = 0.25*PI;
	//}
  } else {  // Tstrain(0) != Tstrain(1) 
    temp_cita = 0.5 * atan(fabs(2.0e6*Tstrain(2)/(1.0e6*Tstrain(0)-1.0e6*Tstrain(1)))); 
    if ( fabs(Tstrain(2)) < SMALL_STRAIN ) {
      //if (Tstrain(0) > Tstrain(1) ) 
		citaE = 0;
	  //if (Tstrain(1) > Tstrain(0) ) citaE = 0.5*PI;
    } else if ( (Tstrain(0) > Tstrain(1)) && ( Tstrain(2) > 0) ) {
      citaE = temp_cita;
    } else if ( (Tstrain(0) > Tstrain(1)) && ( Tstrain(2) < 0) ) {
      citaE = PI - temp_cita;
    } else if ( (Tstrain(0) < Tstrain(1)) && ( Tstrain(2) > 0) ) {
      citaE = 0.5*PI - temp_cita;
    } else if ( (Tstrain(0) < Tstrain(1)) && ( Tstrain(2) < 0) ) {
      citaE = 0.5*PI + temp_cita;
    } else {
      opserr << "CSMMRCPlaneStress::determineTrialStress: Failure to calculate citaE\n";
      opserr << " Tstrain(0) = " << Tstrain(0) << endln;
      opserr << " Tstrain(1) = " << Tstrain(1) << endln;
      opserr << " Tstrain(2) = " << Tstrain(2) << endln;
    }
  }

  citaIC = citaE;  // initial cracked angle

  static Matrix Tc(3,3);     // T(cita_E)
  static Matrix Ts1(3,3);    // T(angle1)
  static Matrix Ts2(3,3);    // T(angle2)
  static Matrix T_cita(3,3); // T(cita)
  static Matrix Dc(3,3);     //
  static Matrix Ds1(3,3);    //
  static Matrix Ds2(3,3);    //
  static Matrix Dcp(3,3);    //
  static Matrix Ds1p(3,3);	 //
  static Matrix Ds2p(3,3);	 //

  // EbarC1 EbarC2 GbarC, eq.i-5
  Dcp.Zero();  // local stiffness matrix
  Ds1p.Zero();
  Ds2p.Zero();

  Tc(0,0) = pow(cos(citaE), 2.0);// T(cita); first cita=citaE
  Tc(0,1) = pow(sin(citaE), 2.0);
  Tc(0,2) = cos(citaE)*sin(citaE);
  Tc(1,0) = pow(sin(citaE), 2.0);
  Tc(1,1) = pow(cos(citaE), 2.0);
  Tc(1,2) = -cos(citaE)*sin(citaE);
  Tc(2,0) = -2.0*cos(citaE)*sin(citaE);
  Tc(2,1) = 2.0*cos(citaE)*sin(citaE);
  Tc(2,2) = pow(cos(citaE), 2.0)-pow(sin(citaE), 2.0);

  Ts1(0,0) = pow(cos(angle1), 2.0);// T(angle1)
  Ts1(0,1) = pow(sin(angle1), 2.0);
  Ts1(0,2) = cos(angle1)*sin(angle1);
  Ts1(1,0) = pow(sin(angle1), 2.0);
  Ts1(1,1) = pow(cos(angle1), 2.0);
  Ts1(1,2) = -cos(angle1)*sin(angle1);
  Ts1(2,0) = -2.0*cos(angle1)*sin(angle1);
  Ts1(2,1) = 2.0*cos(angle1)*sin(angle1);
  Ts1(2,2) = pow(cos(angle1), 2.0)-pow(sin(angle1), 2.0);

  Ts2(0,0) = pow(cos(angle2), 2.0);// T(angle2)
  Ts2(0,1) = pow(sin(angle2), 2.0);
  Ts2(0,2) = cos(angle2)*sin(angle2);
  Ts2(1,0) = pow(sin(angle2), 2.0);
  Ts2(1,1) = pow(cos(angle2), 2.0);
  Ts2(1,2) = -cos(angle2)*sin(angle2);
  Ts2(2,0) = -2.0*cos(angle2)*sin(angle2);
  Ts2(2,1) = 2.0*cos(angle2)*sin(angle2);
  Ts2(2,2) = pow(cos(angle2), 2.0)-pow(sin(angle2), 2.0);

  Information &S1SetVar = theResponses[S1_SET_V]->getInformation(); // S1 setVar
  Information &S2SetVar = theResponses[S2_SET_V]->getInformation(); // S2 setVar
  Information &S1GetVar = theResponses[S1_GET_V]->getInformation(); // S1 getVar
  Information &S2GetVar = theResponses[S2_GET_V]->getInformation(); // S2 getVar
  Information &C1SetVar = theResponses[C1_SET_V]->getInformation(); // C1 setVar
  Information &C2SetVar = theResponses[C2_SET_V]->getInformation(); // C2 setVar
  Information &C1GetVar = theResponses[C1_GET_V]->getInformation(); // C1 getVar
  Information &C2GetVar = theResponses[C2_GET_V]->getInformation(); // C2 getVar

  //for small strain return quickly
  if (Tstrain.pNorm(-1) <= DBL_EPSILON) {
	double v = 0.22;
	double temp = Ec/(1.0-v*v);

	Dcp(0,0) = temp;
	Dcp(0,1) = v*temp;
	Dcp(1,0) = v*temp;
	Dcp(1,1) = temp;
	Dcp(2,2) = temp*(1.0-v)/2.0;

	Ds1p(0,0)= Es * rhox;
	Ds2p(0,0)= Es * rhoy;

	Dc.addMatrixTripleProduct( 0.0, Tc,  Dcp,  1.0);
	Ds1.addMatrixTripleProduct(0.0, Ts1, Ds1p, 1.0);
	Ds2.addMatrixTripleProduct(0.0, Ts2, Ds2p, 1.0);

	secant_matrix = Dc + Ds1 + Ds2;
	tangent_matrix = secant_matrix;

	//Tstress.addMatrixVector(0.0, tangent_matrix, Tstrain, 1.0);
	Tstress.Zero();

	return Tstress;
  }

  // Get average strains in reinforcement, eq. i-19
  //epsSx = 0.5 * (Tstrain(0)+Tstrain(1))+0.5*(Tstrain(0)-Tstrain(1))*cos(2.0*angle1)
  //      + 0.5 * Tstrain(2)*sin(2.0*angle1);
  //epsSy = 0.5 * (Tstrain(0)+Tstrain(1))+0.5*(Tstrain(0)-Tstrain(1))*cos(2.0*angle2)
  //      + 0.5 * Tstrain(2)*sin(2.0*angle2);
  tempStrain.addMatrixVector(0.0, Ts1, Tstrain, 1.0);
  epsSx = tempStrain(0);
  tempStrain.addMatrixVector(0.0, Ts2, Tstrain, 1.0);
  epsSy = tempStrain(0);

  // Calculate Steel stress incorporate steel offset strains

  //S1SetVar.setVector(theData);
  //S2SetVar.setVector(theData);
  //theResponses[S1_SET_V]->getResponse();  // setVar
  //theResponses[S2_SET_V]->getResponse();  // setVar

  if (theMaterial[S_ONE]->setTrialStrain(epsSx) != 0) {
    opserr << "MCFTRCPlaneStress::determineTrialStress(), S1 setTrialStrain() Error" << endln;  // s1
	return -1;
  }

  if (theMaterial[S_TWO]->setTrialStrain(epsSy) != 0) {
    opserr << "MCFTRCPlaneStress::determineTrialStress(), S2 setTrialStrain() Error" << endln;  // s2
	return -1;
  }

  fSx = theMaterial[S_ONE]->getStress();
  fSy = theMaterial[S_TWO]->getStress();
  
  // initial epsC vector assumptions equal to previous committed value
  epsC_vec  = Tstrain; // epsC_vec = epsCe_vec + epsCp_vec; 
  //epsCp_vec = CepsCp_vec;

  // vCi need to be satisfied several relationships in the concrete crack region
  static Vector errorVec(3); // epsC_vec converge test need a vector to maintain Error
  static Vector theData(8); // data for materials passed in 
  double errNorm;
  double temp, tolNorm = SMALL_STRAIN;
  bool vCiconverged = false;
  int numberDivide = 1, iteration_counter = 0, ret;
  double deltaEpsC1p, deltaEpsC2p, deltaEpsCm1, deltaEpsCm2, deltaEpsTm1, deltaEpsTm2;

  while (vCiconverged == false && numberDivide <=5) {
    
	// Get citaS based on epsCe_vec, eq.i-11    
	// citaS = cita, based on elastic strain part
	// MCFT cita corresponding strain field, DSFM cita corresponding stress field
	epsCe_vec = epsC_vec - epsCp_vec; // - epsSlip_vec
    if ( fabs(epsCe_vec(0)-epsCe_vec(1)) < SMALL_STRAIN) {
      //if (fabs(epsCe_vec(2)) < SMALL_STRAIN) {
	  //  citaS = 0;
	  //} else {
	    citaS = 0.25*PI;
	  //}
    } else {  // epsCe_vec(0) != epsCe_vec(1) 
      temp_cita = 0.5 * atan(fabs(2.0e6*epsCe_vec(2)/(1.0e6*epsCe_vec(0)-1.0e6*epsCe_vec(1)))); 
      if ( fabs(epsCe_vec(2)) < SMALL_STRAIN ) {
  	    //if (epsCe_vec(0) > epsCe_vec(1) ) citaS = 0;
	    //if (epsCe_vec(1) > epsCe_vec(0) ) citaS = 0.5*PI;
		citaS = 0;
  	  } else if ( (epsCe_vec(0) > epsCe_vec(1)) && ( epsCe_vec(2) > 0) ) {
  	    citaS = temp_cita;
  	  } else if ( (epsCe_vec(0) > epsCe_vec(1)) && ( epsCe_vec(2) < 0) ) {
  	    citaS = PI - temp_cita;
  	  } else if ( (epsCe_vec(0) < epsCe_vec(1)) && ( epsCe_vec(2) > 0) ) {
  	    citaS = 0.5*PI - temp_cita;
  	  } else if ( (epsCe_vec(0) < epsCe_vec(1)) && ( epsCe_vec(2) < 0) ) {
  	    citaS = 0.5*PI + temp_cita;
  	  } else {
  	    opserr << "MCFTRCPlaneStress::determineTrialStress: Failure to calculate citaS\n";
  	    opserr << " epsCe_vec(0) = " << epsCe_vec(0) << endln;
  	    opserr << " epsCe_vec(1) = " << epsCe_vec(1) << endln;
  	    opserr << " epsCe_vec(2) = " << epsCe_vec(2) << endln;
  	  }
    }

    cita=citaS;  //eq.i-11    PI/2.0-citaS = citaCrack
    citan1 = citaS - angle1; //angle1-(PI/2.0-citaS); //eq.i-6
	//if (citan1 < 0.0) citan1 += 2.0*PI;
    citan2 = citaS - angle2; //angle2-(PI/2.0-citaS);
    //if (citan2 < 0.0) citan2 += 2.0*PI;

	// Transform matrix for current strain state 
	T_cita(0,0) = pow(cos(cita), 2.0);// T(cita) 
	T_cita(0,1) = pow(sin(cita), 2.0);
	T_cita(0,2) = cos(cita)*sin(cita);
	T_cita(1,0) = pow(sin(cita), 2.0);
	T_cita(1,1) = pow(cos(cita), 2.0);
	T_cita(1,2) = -cos(cita)*sin(cita);
	T_cita(2,0) = -2.0*cos(cita)*sin(cita);
	T_cita(2,1) = 2.0*cos(cita)*sin(cita);
	T_cita(2,2) = pow(cos(cita), 2.0)-pow(sin(cita), 2.0);

	// Get epsC1,epsC2 and citaS based on epsC_vec, eq.i-10
	//epsC1 = 0.5*(epsC_vec(0)+epsC_vec(1)) 
	//	  + 0.5*sqrt(pow(epsC_vec(0)-epsC_vec(1), 2.0)+pow(epsC_vec(2), 2.0));
	//epsC2 = 0.5*(epsC_vec(0)+epsC_vec(1)) 
	//	  - 0.5*sqrt(pow(epsC_vec(0)-epsC_vec(1), 2.0)+pow(epsC_vec(2), 2.0));

	tempStrain.addMatrixVector(0.0, T_cita, epsC_vec, 1.0);
	epsC1 = tempStrain(0);
	epsC2 = tempStrain(1);
	halfGammaOneTwo = tempStrain(2);  // should be a minor value

	// Vecchio: Towards Cyclic Load Modeling of Reinforced Concrete
	// C max
	deltaEpsCm1 = (epsC1 < CepsC12cm_vec(0) ? epsC1-CepsC12cm_vec(0) : 0);
	deltaEpsCm2 = (epsC2 < CepsC12cm_vec(1) ? epsC2-CepsC12cm_vec(1) : 0);

	epsCcm_vec(0) = CepsCcm_vec(0) + (0.5 * deltaEpsCm1 * (1+cos(2.0*cita)) + 0.5 * deltaEpsCm2 * (1-cos(2.0*cita)));
	epsCcm_vec(1) = CepsCcm_vec(1) + (0.5 * deltaEpsCm1 * (1-cos(2.0*cita)) + 0.5 * deltaEpsCm2 * (1+cos(2.0*cita)));
	epsCcm_vec(2) = CepsCcm_vec(2) + (deltaEpsCm1 - deltaEpsCm2) * sin(2.0*cita);

	//eC1m = 0.5*(epsCcm_vec(0)+epsCcm_vec(1))+0.5*((epsCcm_vec(0)-epsCcm_vec(1))*cos(2.0*cita)+epsCcm_vec(2)*sin(2.0*cita));
	//eC2m = 0.5*(epsCcm_vec(0)+epsCcm_vec(1))-0.5*((epsCcm_vec(0)-epsCcm_vec(1))*cos(2.0*cita)+epsCcm_vec(2)*sin(2.0*cita));

	tempStrain.addMatrixVector(0.0, T_cita, CepsCcm_vec, 1.0);
	eC1m = tempStrain(0);
	eC2m = tempStrain(1);
	halfGammaOneTwo = tempStrain(2);

	// T max
	deltaEpsTm1 = (epsC1 > epsC12tm_vec(0) ? epsC1-CepsC12tm_vec(0) : 0);
	deltaEpsTm2 = (epsC2 > epsC12tm_vec(1) ? epsC2-CepsC12tm_vec(1) : 0);

	epsCtm_vec(0) = CepsCtm_vec(0) + (0.5*deltaEpsTm1*(1+cos(2.0*cita)) + 0.5*deltaEpsTm2*(1-cos(2.0*cita)));
	epsCtm_vec(1) = CepsCtm_vec(1) + (0.5*deltaEpsTm1*(1-cos(2.0*cita)) + 0.5*deltaEpsTm2*(1+cos(2.0*cita)));
	epsCtm_vec(2) = CepsCtm_vec(2) + (deltaEpsTm1 - deltaEpsTm2) * sin(2.0*cita);

	//eT1m = 0.5*(epsCtm_vec(0)+epsCtm_vec(1))+0.5*((epsCtm_vec(0)-epsCtm_vec(1))*cos(2.0*cita)+epsCtm_vec(2)*sin(2.0*cita));
	//eT2m = 0.5*(epsCtm_vec(0)+epsCtm_vec(1))-0.5*((epsCtm_vec(0)-epsCtm_vec(1))*cos(2.0*cita)+epsCtm_vec(2)*sin(2.0*cita));

	tempStrain.addMatrixVector(0.0, T_cita, CepsCtm_vec, 1.0);
	eT1m = tempStrain(0);
	eT2m = tempStrain(1);
	halfGammaOneTwo = tempStrain(2);

	int status=0; // status to check if iteration satisfied eq.i-7
	double tolerance = SMALL_STRESS; // tolerance for iteration
	bool fC1converged, fC2converged;
	double error, fScrx, fScry, epsScrx, epsScry, epsIncr;

	if((epsC1 > 0 && epsC2 > 0) || (epsC1 < 0 && epsC2 < 0)) {

      if(epsC1 > 0 && epsC2 > 0) {
		Par = 1;
		//////////////////////////////////////////////////////////////////////////
		// C1 -- tensile for fc1, epsC1  
		//////////////////////////////////////////////////////////////////////////
	    //  case 1: eq.i-33,34
        epsts = 2.0 * 7.5e-3 / fcr / 750.; //eq. i-33
        temp  = (epsC1 - epscr) / (epsts - epscr);
	    
        if      (temp <= 0.0) fC1a = 0.0;
        else if (temp >  1.0) fC1a = fcr;
        else                  fC1a = fcr * (1 - temp);  //eq. i-34
  	    
        //  case 2: eq. i-35, 36
        temp = 4.0*(rhox/db1*fabs(cos(citan1))+rhoy/db2*fabs(cos(citan2)));
        fC1b = fcr/(1+sqrt(2.2/temp*epsC1));
  	    temp = max(fC1a,fC1b); // eq.i-38 /fC1
  	    
        // eq.i-5   // for positive value
        fC1c = fabs(rhox* (fyx - fSx) * pow(cos(citan1), 2.0)
  	         + rhoy* (fyy - fSy) * pow(cos(citan2), 2.0) );
        double fC1max = min(temp, fC1c);
	    
	    theData(0) = eC1m;  // 
	    theData(1) = eT1m;  // 
	    //theData(2) = epsC1;  // 
	    theData(5) = CepsC12p(0);
	    theData(6) = 1.0; //betaD
		theData(7) = 1.0; // K

	    ret = 0;
	    ret += C1SetVar.setVector(theData);  // Information for C tension, material [2], response[4]
	    ret += theResponses[C1_SET_V]->getResponse(); // C1 setVar 
        ret += theMaterial[C_ONE]->setTrialStrain(epsC1);
	    if (ret != 0) {
	      opserr << "MCFTRCPlaneStress::determineTrialStress(), C1 setTrialStrain() Error" << endln;  // C1
	      return ret;
	    }
	    temp = theMaterial[C_ONE]->getStress();
	    fC1 = min(fC1max, temp);

		//////////////////////////////////////////////////////////////////////////
		if(epsC1 > epscr) { //checkAtCrack(); 
		  // Calculate local stress at cracks
	      status         = 0; // status to check if iteration satisfied eq.i-7
	      fC1converged   = false;
	      epsScrx        = epsSx;
	      epsScry        = epsSy;
	      epsIncr        = 0.0;
	      
	      while ( fC1converged == false && status == 0 ) {
	      	if(fabs(epsScrx) > 0.5 || fabs(epsScry) > 0.5) { // for very large strain at cracks
	      	    opserr << "MCFTRCPlaneStress::determineTrialStress(vector) -- large strain at cracks" << endln;
	      	    opserr << "TrialStrainVector: " << Tstrain(0) << "\t"<< Tstrain(1) << "\t"<< Tstrain(2) << endln;
	      	    opserr << "fC1converged false for strain: epsScrx = " << epsScrx << "; epsScry = " << epsScry << endln; 
	      	    status = 1;
	      	}
	      	// eq.i-20
	      	epsScrx = epsSx + epsIncr * pow(cos(citan1), 2.0);
	      	epsScry = epsSy + epsIncr * pow(cos(citan2), 2.0);
	      
	      	/*if( theMaterial[S_ONE]->setTrialStrain(epsScrx) != 0) {
	      	    opserr << "MCFTRCPlaneStress::determineTrialStress(): fail to determine epsScrx" << endln;
	      	    return -1;
	      	}
	      	fScrx = theMaterial[S_ONE]->getStress();
	      	if( theMaterial[S_TWO]->setTrialStrain(epsScry) != 0) {
	      	    opserr << "MCFTRCPlaneStress::determineTrialStress(): fail to determine epsScry" << endln;
	      	    return -1;
	      	}
	      	fScry = theMaterial[S_TWO]->getStress(); */
	      	fScrx = determinefS(epsScrx, fyx, Es, 1.02*Es);
			fScry = determinefS(epsScry, fyy, Es, 1.02*Es);
	      
	      	// eq.i-7
	      	temp = rhox * (fScrx - fSx) * pow(cos(citan1), 2.0) 
	      	     + rhoy * (fScry - fSy) * pow(cos(citan2), 2.0);
	      	error = fC1 - temp;
	      
	      	if (fabs(error) <= tolerance) {
	      	  fC1converged = true;
	      	} else {
	      	  // temp is function of citan1 citan2 and fSx and fSy,
	      	  // the iterative parameter should include citan1/citan2 angle chosen. 
	      	  // like: if (citan1 > PI/2 && citan2 > PI/2)...
	      	  temp = rhox * theMaterial[S_ONE]->getSecant() * pow(cos(citan1), 2.0)
	      	       + rhoy * theMaterial[S_TWO]->getSecant() * pow(cos(citan2), 2.0);
	      	  epsIncr += error/temp;
	      	}
	      }
	      // eq. i-8
	      vci = rhox * (fScrx - fSx) * cos(citan1) * sin(citan1)
	          + rhoy * (fScry - fSy) * cos(citan2) * sin(citan2);
	      //vci = (vci < 0 ? 0 : vci);
	      
	      // Crack slips determine
	      //while ( citaS > 0.5*PI ) {
	      //	citaS -= 0.5*PI;
	      //}
	      
	      // crack spacing // w   
	      s_cita = 1.0/(sin(citaS)/xd+cos(citaS)/yd); // eq.i-21,22
	      w      = epsC1 * s_cita;
	      
	      // optional 1
	      vcimax = sqrt(-fpc)/(0.31+24*w/(aggr+16)); // shear parameter
	      
	      // if vci > vcimax , decrease fC1
	      if (fabs(vci) > vcimax) {
	      	fC1 -= (fabs(vci) - vcimax)*(tan(citaS)+1./tan(citaS));  // MCFT(1986) eq. 13
	      }
	      //fC1 = (fC1 > 0 ? fC1 : 0);
	      //vci = vci * min(fabs(vcimax), fabs(vci)) / fabs(vci);
		}

	    //////////////////////////////////////////////////////////////////////////
		// C2 -- tensile for fc2, epsC2    
		//////////////////////////////////////////////////////////////////////////
        //epsts = 2.0 * 7.5e-3 / fcr / 750.; //eq. i-33
        temp  = (epsC2 - epscr) / (epsts - epscr);
	    
        if      (temp <= 0.0) fC2a = 0.0;
        else if (temp >  1.0) fC2a = fcr;
        else                  fC2a = fcr * (1 - temp);  //eq. i-34
  	    
        //  case 2: eq. i-35, 36
        temp = 4.0*(rhox/db1*fabs(cos(citan1))+rhoy/db2*fabs(cos(citan2)));
        fC2b = fcr/(1+sqrt(2.2/temp*epsC2));
		temp = max(fC2a,fC2b); // eq.i-38 /fC1

		// eq.i-5   // for positive value
		fC2c = fabs(rhox* (fyx - fSx) * pow(cos(citan1), 2.0)
			 + rhoy* (fyy - fSy) * pow(cos(citan2), 2.0) );
		double fC2max = min(temp, fC2c);
	    
	    theData(0) = eC2m;  // 
	    theData(1) = eT2m;  // 
	    //theData(2) = epsC2;  // 
	    theData(5) = CepsC12p(1);
	    theData(6) = 1.0; //betaD
	    theData(7) = 1.0; // K

	    ret = 0;
	    ret += C2SetVar.setVector(theData);  // Information for C2 in tension, material [3], response[5]
	    ret += theResponses[C2_SET_V]->getResponse(); // C2 setVar 
        ret += theMaterial[C_TWO]->setTrialStrain(epsC2);
	    if (ret != 0) {
	      opserr << "MCFTRCPlaneStress::determineTrialStress(), C2 setTrialStrain() Error" << endln;  // C2
	      return ret;
	    }
	    temp = theMaterial[C_TWO]->getStress();
	    fC2 = min(fC2max, temp);
	    
		//////////////////////////////////////////////////////////////////////////
		if(epsC2 > epscr) { //checkAtCrack(); 
		  // Calculate local stress at cracks
	      status         = 0; // status to check if iteration satisfied eq.i-7
	      fC2converged   = false;
	      epsScrx        = epsSx;
	      epsScry        = epsSy;
	      epsIncr        = 0.0;
	      
	      while ( fC2converged == false && status == 0 ) {
	      	if(fabs(epsScrx) > 0.5 || fabs(epsScry) > 0.5) { // for very large strain at cracks
	      	    opserr << "MCFTRCPlaneStress::determineTrialStress(vector) -- large strain at cracks" << endln;
	      	    opserr << "TrialStrainVector: " << Tstrain(0) << "\t"<< Tstrain(1) << "\t"<< Tstrain(2) << endln;
	      	    opserr << "fC1converged false for strain: epsScrx = " << epsScrx << "; epsScry = " << epsScry << endln; 
	      	    status = 1;
	      	}
	      	// eq.i-20
	      	epsScrx = epsSx + epsIncr * pow(cos(citan1), 2.0);
	      	epsScry = epsSy + epsIncr * pow(cos(citan2), 2.0);
	      
	      	/*if( theMaterial[S_ONE]->setTrialStrain(epsScrx) != 0) {
	      	    opserr << "MCFTRCPlaneStress::determineTrialStress(): fail to determine epsScrx" << endln;
	      	    return -1;
	      	}
	      	fScrx = theMaterial[S_ONE]->getStress();
	      	
	      
	      	if( theMaterial[S_TWO]->setTrialStrain(epsScry) != 0) {
	      	    opserr << "MCFTRCPlaneStress::determineTrialStress(): fail to determine epsScry" << endln;
	      	    return -1;
	      	}
			fScry = theMaterial[S_TWO]->getStress(); */
	      	fScrx = determinefS(epsScrx, fyx, Es, 1.02*Es);
	      	fScry = determinefS(epsScry, fyy, Es, 1.02*Es);
	      
	      	// eq.i-7
	      	temp = rhox * (fScrx - fSx) * pow(cos(citan1), 2.0) 
	      	     + rhoy * (fScry - fSy) * pow(cos(citan2), 2.0);
	      	error = fC2 - temp;
	      
	      	if (fabs(error) <= tolerance) {
	      	  fC2converged = true;
	      	} else {
	      	  // temp is function of citan1 citan2 and fSx and fSy,
	      	  // the iterative parameter should include citan1/citan2 angle chosen. 
	      	  // like: if (citan1 > PI/2 && citan2 > PI/2)...
	      	  temp = rhox * theMaterial[S_ONE]->getSecant() * pow(cos(citan1), 2.0)
	      	       + rhoy * theMaterial[S_TWO]->getSecant() * pow(cos(citan2), 2.0);
	      	  epsIncr += error/temp;
	      	}
	      }
	      // eq. i-8
	      vci = rhox * (fScrx - fSx) * cos(citan1) * sin(citan1)
	          + rhoy * (fScry - fSy) * cos(citan2) * sin(citan2);
	      //vci = (vci < 0 ? 0 : vci);
	      
	      // Crack slips determine
	      //while ( citaS > 0.5*PI ) {
	      //	citaS -= 0.5*PI;
	      //}
	      
	      // crack spacing // w   
	      s_cita = 1.0/(sin(citaS)/xd+cos(citaS)/yd); // eq.i-21,22
	      w      = epsC2 * s_cita;
	      
	      // optional 1
	      vcimax = sqrt(-fpc)/(0.31+24*w/(aggr+16)); // shear parameter
	      
	      // if vci > vcimax , decrease fC2
	      if (fabs(vci) > vcimax) {
	      	fC2 -= (fabs(vci) - vcimax)*(tan(citaS)+1./tan(citaS));  // MCFT(1986) eq. 13
	      }
	      //fC2 = (fC2 > 0 ? fC2 : 0);
	      //vci = vci * min(fabs(vcimax), fabs(vci)) / fabs(vci);
		}

		//////////////////////////////////////////////////////////////////////////
		// get C1, C2 's epsilon_p
		//////////////////////////////////////////////////////////////////////////
		theResponses[C1_GET_V]->getResponse();  // C1 getVar
		theResponses[C2_GET_V]->getResponse();  // C2 getVar
		
		epsC12p(0) = (*C1GetVar.theVector)(5); // ecp1 
        epsC12p(1) = (*C2GetVar.theVector)(5); // ecp2 
	    if (epsC12p(0) > 0) epsC12p(0) = 0.0;
        if (epsC12p(1) > 0) epsC12p(1) = 0.0;

	  } 
	  
      else if (epsC1 < 0 && epsC2 < 0) {
		Par = 2;
		//////////////////////////////////////////////////////////////////////////
		// C1 Kupfer envelop for concrete
		//////////////////////////////////////////////////////////////////////////
		opserr << "epsC1 and epsC2 negative, both!" << endln;
		double sig1 = 0.0, sig2 = 0.0, sig_p1, sig_p2, eps_p1, eps_p2;
		double norm = 1.0, K1, K2;
		while (norm > SMALL_STRESS) {
		  K1 = 1 + 0.92 * (sig2/fpc)-0.76*pow(sig2/fpc,2.0);
		  K2 = 1 + 0.92 * (sig1/fpc)-0.76*pow(sig1/fpc,2.0);
		  sig_p1 = K1 * fpc;
		  eps_p1 = K1 * epsc0;
		  sig_p2 = K2 * fpc;
		  eps_p2 = K2 * epsc0;

		  //////////////////////////////////////////////////////////////////////////
		  // C1
		  //////////////////////////////////////////////////////////////////////////
		  theData(0) = eC1m;  // 
	      theData(1) = eT1m;  // 
	      //theData(2) = epsC1;  // 
	      theData(5) = CepsC12p(0);
	      theData(6) = 1.0; //betaD
	      theData(7) = K1;

	      ret = 0;
	      ret += C1SetVar.setVector(theData);  // Information for C tension, material [2], response[4]
	      ret += theResponses[C1_SET_V]->getResponse(); // C1 setVar 
          ret += theMaterial[C_ONE]->setTrialStrain(epsC1);
	      if (ret != 0) {
	        opserr << "MCFTRCPlaneStress::determineTrialStress(), C1 setTrialStrain() Error" << endln;  // C1
	        return ret;
	      }
	      fC1 = theMaterial[C_ONE]->getStress();

		  //////////////////////////////////////////////////////////////////////////
		  // C2
		  //////////////////////////////////////////////////////////////////////////
		  theData(0) = eC2m;  // 
	      theData(1) = eT2m;  // 
	      //theData(2) = epsC2;  // 
	      theData(5) = CepsC12p(1);
	      theData(6) = 1.0;
	      theData(7) = K2;

	      ret = 0;
	      ret += C2SetVar.setVector(theData);  // Information for C compression, material [3], response[5]
	      ret += theResponses[C2_SET_V]->getResponse(); // C2 setVar
	      ret += theMaterial[C_TWO]->setTrialStrain(epsC2); //need further revision
          if (ret != 0) {
	        opserr << "MCFTRCPlaneStress::determineTrialStress(), C2 setTrialStrain() Error" << endln;  // C2
	        return ret;
	      }
	      fC2 = theMaterial[C_TWO]->getStress(); // w==5 ??
	      
		  //////////////////////////////////////////////////////////////////////////
		  norm = sqrt(pow(fC1-sig1,2.0)+pow(fC2-sig2,2.0));
		  sig1 = fC1;
		  sig2 = fC2;
		}

		theResponses[C1_GET_V]->getResponse(); // C1 getVar
		epsC12p(0) = (*C1GetVar.theVector)(5); // ecp1
		theResponses[C2_GET_V]->getResponse(); // C2 getVar
		epsC12p(1) = (*C2GetVar.theVector)(5); // ecp2 
		opserr << "K1 = " << K1 << "\tK2 = " << K2 << endln;
	  } 
	  
	} 
	
    /*else if (fabs(epsC1) <= 0 || fabs(epsC2) <= 0){  //  uniaxal performance
	  opserr << "Uniaxial behavior path!" << endln;
	  if (fabs(epsC1) <= DBL_EPSILON) {
		opserr << "epsC1 is too small!" << endln;
		double v = 0.22;
	    double temp = Ec/(1.0-v*v);
	    
	    Dcp(0,0) = temp;
	    Dcp(0,1) = v*temp;
	    Dcp(1,0) = v*temp;
	    Dcp(1,1) = temp;
	    Dcp(2,2) = temp*(1.0-v)/2.0;
	    
	    Ds1p(0,0)= Es * rhox;
	    Ds2p(0,0)= Es * rhoy;
	    
	    Dc.addMatrixTripleProduct( 0.0, Tc,  Dcp,  1.0);
	    Ds1.addMatrixTripleProduct(0.0, Ts1, Ds1p, 1.0);
	    Ds2.addMatrixTripleProduct(0.0, Ts2, Ds2p, 1.0);
	    
	    secant_matrix = Dc + Ds1 + Ds2;
	    tangent_matrix = secant_matrix;
		Tstress.addMatrixVector(0.0, tangent_matrix, Tstrain, 1.0);
	    
		return Tstress;

	  } 
	  else if (fabs(epsC2) <= DBL_EPSILON) {
		opserr << "epsC2 is too small!" << endln;
		double v = 0.22;
	    double temp = Ec/(1.0-v*v);
	    
	    Dcp(0,0) = temp;
	    Dcp(0,1) = v*temp;
	    Dcp(1,0) = v*temp;
	    Dcp(1,1) = temp;
	    Dcp(2,2) = temp*(1.0-v)/2.0;
	    
	    Ds1p(0,0)= Es * rhox;
	    Ds2p(0,0)= Es * rhoy;
	    
	    Dc.addMatrixTripleProduct( 0.0, Tc,  Dcp,  1.0);
	    Ds1.addMatrixTripleProduct(0.0, Ts1, Ds1p, 1.0);
	    Ds2.addMatrixTripleProduct(0.0, Ts2, Ds2p, 1.0);
	    
	    secant_matrix = Dc + Ds1 + Ds2;
	    tangent_matrix = secant_matrix;
		Tstress.addMatrixVector(0.0, tangent_matrix, Tstrain, 1.0);

	    return Tstress;

	  } 

	}*/
    
    else {
	  // determine the MCFT state
	  //Tstress = determineMCFTstress(epsC1,epsC2);
      if (epsC1 > 0 && epsC2 < 0) {
		Par = 3;
	    //////////////////////////////////////////////////////////////////////////
	    // C1 -- tensile for fc1, epsC1  
	    //////////////////////////////////////////////////////////////////////////
	    //  case 1: eq.i-33,34
        epsts = 2.0 * 7.5e-3 / fcr / 750.; //eq. i-33
        temp  = (epsC1 - epscr) / (epsts - epscr);
	    
        if      (temp <= 0.0) fC1a = 0.0;
        else if (temp >  1.0) fC1a = fcr;
        else                  fC1a = fcr * (1 - temp);  //eq. i-34
  	    
        //  case 2: eq. i-35, 36
        temp = 4.0*(rhox/db1*fabs(cos(citan1))+rhoy/db2*fabs(cos(citan2)));
        fC1b = fcr/(1+sqrt(2.2/temp*epsC1));
	    temp = max(fC1a, fC1b);
	    
		// eq.i-5   // for positive value
		fC1c = fabs(rhox* (fyx - fSx) * pow(cos(citan1), 2.0)
		     + rhoy* (fyy - fSy) * pow(cos(citan2), 2.0) );
		double fC1max = min(temp, fC1c);

	    theData(0) = eC1m;  // 
	    theData(1) = eT1m;  // 
	    //theData(2) = epsC1;  // 
	    theData(5) = CepsC12p(0);
	    theData(6) = 1.0; //betaD
	    theData(7) = 1.0; // K
	    ret = 0;
	    ret += C1SetVar.setVector(theData);  // Information for C tension, material [2], response[4]
	    ret += theResponses[C1_SET_V]->getResponse(); // C1 setVar 
        ret += theMaterial[C_ONE]->setTrialStrain(epsC1);
	    if (ret != 0) {
	      opserr << "MCFTRCPlaneStress::determineTrialStress(), C1 setTrialStrain() Error" << endln;  // C1
	      return ret;
	    }
	    temp = theMaterial[C_ONE]->getStress();
	    fC1 = min(fC1max, temp);

		//////////////////////////////////////////////////////////////////////////
		if(epsC1 > epscr) { //checkAtCrack(); 
		  // Calculate local stress at cracks
	      status         = 0; // status to check if iteration satisfied eq.i-7
	      fC1converged   = false;
	      epsScrx        = epsSx;
	      epsScry        = epsSy;
	      epsIncr        = 0.0;
	      
	      while ( fC1converged == false && status == 0 ) {
	      	if(fabs(epsScrx) > 0.5 || fabs(epsScry) > 0.5) { // for very large strain at cracks
	      	    opserr << "MCFTRCPlaneStress::determineTrialStress(vector) -- large strain at cracks" << endln;
	      	    opserr << "TrialStrainVector: " << Tstrain(0) << "\t"<< Tstrain(1) << "\t"<< Tstrain(2) << endln;
	      	    opserr << "fC1converged false for strain: epsScrx = " << epsScrx << "; epsScry = " << epsScry << endln; 
	      	    status = 1;
	      	}
	      	// eq.i-20
	      	epsScrx = epsSx + epsIncr * pow(cos(citan1), 2.0);
	      	epsScry = epsSy + epsIncr * pow(cos(citan2), 2.0);
	      
	      	/*if( theMaterial[S_ONE]->setTrialStrain(epsScrx) != 0) {
	      	    opserr << "MCFTRCPlaneStress::determineTrialStress(): fail to determine epsScrx" << endln;
	      	    return -1;
	      	}
	      	fScrx = theMaterial[S_ONE]->getStress();
	      	
	      
	      	if( theMaterial[S_TWO]->setTrialStrain(epsScry) != 0) {
	      	    opserr << "MCFTRCPlaneStress::determineTrialStress(): fail to determine epsScry" << endln;
	      	    return -1;
	      	}
			fScry = theMaterial[S_TWO]->getStress(); */
	      	fScrx = determinefS(epsScrx, fyx, Es, 1.02*Es);
	      	fScry = determinefS(epsScry, fyy, Es, 1.02*Es);
	      
	      	// eq.i-7
	      	temp = rhox * (fScrx - fSx) * pow(cos(citan1), 2.0) 
	      	     + rhoy * (fScry - fSy) * pow(cos(citan2), 2.0);
	      	error = fC1 - temp;
	      
	      	if (fabs(error) <= tolerance) {
	      	  fC1converged = true;
	      	} else {
	      	  // temp is function of citan1 citan2 and fSx and fSy,
	      	  // the iterative parameter should include citan1/citan2 angle chosen. 
	      	  // like: if (citan1 > PI/2 && citan2 > PI/2)...
	      	  temp = rhox * theMaterial[S_ONE]->getSecant() * pow(cos(citan1), 2.0)
	      	       + rhoy * theMaterial[S_TWO]->getSecant() * pow(cos(citan2), 2.0);
	      	  epsIncr += error/temp;
	      	}
	      }
	      // eq. i-8
	      vci = rhox * (fScrx - fSx) * cos(citan1) * sin(citan1)
	          + rhoy * (fScry - fSy) * cos(citan2) * sin(citan2);
	      //vci = (vci < 0 ? 0 : vci);
	      
	      // Crack slips determine
	      //while ( citaS > 0.5*PI ) {
	      //	citaS -= 0.5*PI;
	      //}
	      
	      // crack spacing // w   
	      s_cita = 1.0/(sin(citaS)/xd+cos(citaS)/yd); // eq.i-21,22
	      w      = epsC1 * s_cita;
	      
	      // optional 1
	      vcimax = sqrt(-fpc)/(0.31+24*w/(aggr+16)); // shear parameter
	      
	      // if vci > vcimax , decrease fC1
	      if (fabs(vci) > vcimax) {
	      	fC1 -= (fabs(vci) - vcimax)*(tan(citaS)+1./tan(citaS));  // MCFT(1986) eq. 13
	      }
	      //fC1 = (fC1 > 0 ? fC1 : 0);
	      //vci = vci * min(fabs(vcimax), fabs(vci)) / fabs(vci);
		}


	    //////////////////////////////////////////////////////////////////////////
	    // fC2
	    //////////////////////////////////////////////////////////////////////////
	    
        // C2 --compression for fc2, epsC2 incorporate steel offset strains
	    betaD2 = calcBetaD(epsC1, epsC2);  // softened by the orthogonal direction tension
	    theData(0) = eC2m;  // 
	    theData(1) = eT2m;  // 
	    //theData(2) = epsC2;  // 
	    theData(5) = CepsC12p(1);
	    theData(6) = betaD2;
	    theData(7) = 1.0; //K 
	    ret = 0;
	    ret += C2SetVar.setVector(theData);  // Information for C compression, material [3], response[5]
	    ret += theResponses[C2_SET_V]->getResponse(); // C2 setVar
	    ret += theMaterial[C_TWO]->setTrialStrain(epsC2); //need further revision
        if (ret != 0) {
	      opserr << "MCFTRCPlaneStress::determineTrialStress(), C2 setTrialStrain() Error" << endln;  // C2
	      return ret;
	    }
	    fC2 = theMaterial[C_TWO]->getStress(); // w==5 ??
  	    
	    theResponses[C2_GET_V]->getResponse(); // C2 getVar
        epsC12p(1) = (*C2GetVar.theVector)(5); // ecp2 

	  } 
      
      else if (epsC1 < 0 && epsC2 > 0 ) {
		Par = 4;
		//////////////////////////////////////////////////////////////////////////
	    // C1 -- compression 
	    //////////////////////////////////////////////////////////////////////////
	    
        // C1 --compression for fc1, epsC1 incorporate steel offset strains
	    betaD1 = calcBetaD(epsC2, epsC1);  // softened by the orthogonal direction tension
	    theData(0) = eC1m;  // 
	    theData(1) = eT1m;  // 
	    //theData(2) = epsC1;  // 
	    theData(5) = CepsC12p(0);
	    theData(6) = betaD1;
	    theData(7) = 1.0; // K
	    ret = 0;
	    ret += C1SetVar.setVector(theData);  // Information for C compression, material [2], response[4]
	    ret += theResponses[C1_SET_V]->getResponse(); // C1 setVar
	    ret += theMaterial[C_ONE]->setTrialStrain(epsC1); //need further revision
        if (ret != 0) {
	      opserr << "MCFTRCPlaneStress::determineTrialStress(), C1 setTrialStrain() Error" << endln;  // C1
	      return ret;
	    }
	    fC1 = theMaterial[C_ONE]->getStress(); // w==5 ?? 
  	    
	    theResponses[C1_GET_V]->getResponse(); // C1 getVar
        epsC12p(0) = (*C2GetVar.theVector)(5); // ecp1
	    
	    //////////////////////////////////////////////////////////////////////////
	    // fC2 -- tension
	    //////////////////////////////////////////////////////////////////////////
	    //  case 1: eq.i-33,34
        epsts = 2.0 * 7.5e-3 / fcr / 750.; //eq. i-33
        temp  = (epsC2 - epscr) / (epsts - epscr);
	    
        if      (temp <= 0.0) fC2a = 0.0;
        else if (temp >  1.0) fC2a = fcr;
        else                  fC2a = fcr * (1 - temp);  //eq. i-34
  	    
        //  case 2: eq. i-35, 36
        temp = 4.0*(rhox/db1*fabs(cos(citan1))+rhoy/db2*fabs(cos(citan2)));
        fC2b = fcr/(1+sqrt(2.2/temp*epsC2));
		temp = max(fC2a,fC2b); // eq.i-38 /fC1

		// eq.i-5   // for positive value
		fC2c = fabs(rhox* (fyx - fSx) * pow(cos(citan1), 2.0)
		     + rhoy* (fyy - fSy) * pow(cos(citan2), 2.0) );
		double fC2max = min(temp, fC2c);
	    
	    theData(0) = eC2m;  // 
	    theData(1) = eT2m;  // 
	    //theData(2) = epsC2;  // 
	    theData(5) = CepsC12p(1);
	    theData(6) = 1.0; //betaD
	    theData(7) = 1.0; //K

	    ret = 0;
	    ret += C2SetVar.setVector(theData);  // Information for C tension, material [3], response[5]
	    ret += theResponses[C2_SET_V]->getResponse(); // C2 setVar 
        ret += theMaterial[C_TWO]->setTrialStrain(epsC2);
	    if (ret != 0) {
	      opserr << "MCFTRCPlaneStress::determineTrialStress(), C2 setTrialStrain() Error" << endln;  // C2
	      return ret;
	    }
	    temp = theMaterial[C_TWO]->getStress();
	    fC2 = min(fC2max, temp);

		//////////////////////////////////////////////////////////////////////////
		if(epsC2 > epscr) { //checkAtCrack(); 
		  // Calculate local stress at cracks
		  status         = 0; // status to check if iteration satisfied eq.i-7
		  fC2converged   = false;
		  epsScrx        = epsSx;
		  epsScry        = epsSy;
		  epsIncr        = 0.0;
	      
	      while ( fC2converged == false && status == 0 ) {
	      	if(fabs(epsScrx) > 0.5 || fabs(epsScry) > 0.5) { // for very large strain at cracks
	      	    opserr << "MCFTRCPlaneStress::determineTrialStress(vector) -- large strain at cracks" << endln;
	      	    opserr << "TrialStrainVector: " << Tstrain(0) << "\t"<< Tstrain(1) << "\t"<< Tstrain(2) << endln;
	      	    opserr << "fC1converged false for strain: epsScrx = " << epsScrx << "; epsScry = " << epsScry << endln; 
	      	    status = 1;
	      	}
	      	// eq.i-20
	      	epsScrx = epsSx + epsIncr * pow(cos(citan1), 2.0);
	      	epsScry = epsSy + epsIncr * pow(cos(citan2), 2.0);
	      
	      	/*if( theMaterial[S_ONE]->setTrialStrain(epsScrx) != 0) {
	      	    opserr << "MCFTRCPlaneStress::determineTrialStress(): fail to determine epsScrx" << endln;
	      	    return -1;
	      	}
	      	fScrx = theMaterial[S_ONE]->getStress();
	      	
	      
	      	if( theMaterial[S_TWO]->setTrialStrain(epsScry) != 0) {
	      	    opserr << "MCFTRCPlaneStress::determineTrialStress(): fail to determine epsScry" << endln;
	      	    return -1;
	      	}
			fScry = theMaterial[S_TWO]->getStress(); */
	      	fScrx = determinefS(epsScrx, fyx, Es, 1.02*Es);
	      	fScry = determinefS(epsScry, fyy, Es, 1.02*Es);
	      
	      	// eq.i-7
	      	temp = rhox * (fScrx - fSx) * pow(cos(citan1), 2.0) 
	      	     + rhoy * (fScry - fSy) * pow(cos(citan2), 2.0);
	      	error = fC2 - temp;
	      
	      	if (fabs(error) <= tolerance) {
	      	  fC2converged = true;
	      	} else {
	      	  // temp is function of citan1 citan2 and fSx and fSy,
	      	  // the iterative parameter should include citan1/citan2 angle chosen. 
	      	  // like: if (citan1 > PI/2 && citan2 > PI/2)...
	      	  temp = rhox * theMaterial[S_ONE]->getSecant() * pow(cos(citan1), 2.0)
	      	       + rhoy * theMaterial[S_TWO]->getSecant() * pow(cos(citan2), 2.0);
	      	  epsIncr += error/temp;
	      	}
	      }
	      // eq. i-8
	      vci = rhox * (fScrx - fSx) * cos(citan1) * sin(citan1)
	          + rhoy * (fScry - fSy) * cos(citan2) * sin(citan2);
	      //vci = (vci < 0 ? 0 : vci);
	      
	      // Crack slips determine
	      //while ( citaS > 0.5*PI ) {
	      //	citaS -= 0.5*PI;
	      //}
	      
	      // crack spacing // w   
	      s_cita = 1.0/(sin(citaS)/xd+cos(citaS)/yd); // eq.i-21,22
	      w      = epsC2 * s_cita;
	      
	      // optional 1
	      vcimax = sqrt(-fpc)/(0.31+24*w/(aggr+16)); // shear parameter
	      
	      // if vci > vcimax , decrease fC2
	      if (fabs(vci) > vcimax) {
	      	fC2 -= (fabs(vci) - vcimax)*(tan(citaS)+1./tan(citaS));  // MCFT(1986) eq. 13
	      }
	      //fC2 = (fC2 > 0 ? fC2 : 0);
	      //vci = vci * min(fabs(vcimax), fabs(vci)) / fabs(vci);
		}

	  }
	}

	//eC1p = 0.5*(epsCp_vec(0)+epsCp_vec(1)) 
	//	 + 0.5*sqrt(pow(epsCp_vec(0)-epsCp_vec(1), 2.0)+pow(epsCp_vec(2), 2.0));
	//eC2p = 0.5*(epsCp_vec(0)+epsCp_vec(1)) 
	//	 - 0.5*sqrt(pow(epsCp_vec(0)-epsCp_vec(1), 2.0)+pow(epsCp_vec(2), 2.0));
	tempStrain.addMatrixVector(0.0, T_cita, CepsCp_vec, 1.0);
	eC1p = tempStrain(0);
	eC2p = tempStrain(1);
	halfGammaOneTwo = tempStrain(2); // should be somve value need further check

    // deltaEspCp(2),  P. Ceresa 2009, Eq. 4
	//deltaEpsC1p = epsC12p(0) - CepsC12p(0);
	deltaEpsC1p = epsC12p(0) - eC1p;
	//deltaEpsC2p = epsC12p(1) - CepsC12p(1);
    deltaEpsC2p = epsC12p(1) - eC2p;

    epsCp_vec(0) = CepsCp_vec(0) + (0.5 * deltaEpsC1p * (1+cos(2.0*cita)) + 0.5 * deltaEpsC2p * (1-cos(2.0*cita)));
    epsCp_vec(1) = CepsCp_vec(1) + (0.5 * deltaEpsC1p * (1-cos(2.0*cita)) + 0.5 * deltaEpsC2p * (1+cos(2.0*cita)));
    epsCp_vec(2) = CepsCp_vec(2) + (deltaEpsC1p - deltaEpsC2p) * sin(2.0*cita);

    //need further revision for the initial crack direction, citaIC
    if (rhox > 0.0 && rhoy > 0.0) citaLag = 5./180.*PI;    // eq.i-41~43 
    else                          citaLag = 7.5/180.*PI;
  	
	dCitaE = citaE - citaIC;
	dCitaS = (fabs(dCitaE) > citaLag ? dCitaE-citaLag : dCitaE);
	citaS  = citaIC + dCitaS;

	if (w <= 0.0) delta_S = 0.0;           //eq. i-40. 12 
	else          delta_S = vci/(1.8*pow(w,-0.8)+(0.234*pow(w,-0.707)-0.2)*(-fpc*1.25)); // fpc change for fcc = 1.25*fpc
	
	double gammaSa;
	if (s_cita == 0.0 ) {
	  gammaSa = 0.0;
	} 
	else {
	  gammaSa = delta_S/s_cita;
	}
	
	double gammaSb = Tstrain(2)*cos(2.0*citaS) + (Tstrain(1)-Tstrain(0))*sin(2.0*citaS);  // eq. i-18

	gamma_S = max(gammaSa, gammaSb);

	epsSlip_vec(0) = -0.5*gamma_S*sin(2.0*citaS);   // eq.i-13~15 
	epsSlip_vec(1) =  0.5*gamma_S*sin(2.0*citaS);
	epsSlip_vec(2) =  gamma_S*cos(2.0*citaS);

	// eSlip1,2
	//eSlip1 = 0.5*(epsSlip_vec(0)+epsSlip_vec(1))
	//       + 0.5*sqrt(pow(epsSlip_vec(0)-epsSlip_vec(1), 2.0)+pow(epsSlip_vec(2), 2.0));
	//eSlip2 = 0.5*(epsSlip_vec(0)+epsSlip_vec(1))
	//       - 0.5*sqrt(pow(epsSlip_vec(0)-epsSlip_vec(1), 2.0)+pow(epsSlip_vec(2), 2.0));
	tempStrain.addMatrixVector(0.0, T_cita, epsSlip_vec, 1.0);
	eSlip1 = tempStrain(0);
	eSlip2 = tempStrain(1);
	halfGammaOneTwo = tempStrain(2); // should be some value need further check

    if (fabs(epsC1) < SMALL_STRAIN) {
	  Ecx = theMaterial[C_ONE]->getTangent();
	} else {
	  Ecx = fC1/(epsC1-epsC12p(0)); //-eSlip1
	  if (Ecx > Ec) Ecx = Ec;
	}
    if (fabs(epsC2) < SMALL_STRAIN) {
	  Ecy = theMaterial[C_TWO]->getTangent();
	} else {
	  Ecy = fC2/(epsC2-epsC12p(1)); //-eSlip2
	  if (Ecy > Ec) Ecy = Ec;
	}
  
    Dcp(0,0) = Ecx;
    Dcp(1,1) = Ecy;
    Dcp(2,2) = Ecx * Ecy / ( Ecx + Ecy );
  
    // EbarS1, EbarS2, eq.i-5
    if (fabs(epsSx) < SMALL_STRAIN) Ds1p(0,0) = rhox * (theMaterial[S_ONE]->getTangent()); //getSecant());
    else              Ds1p(0,0) = rhox * fSx / epsSx;
    
    if (fabs(epsSy) < SMALL_STRAIN) Ds2p(0,0) = rhoy * (theMaterial[S_TWO]->getTangent()); //getSecant());
    else              Ds2p(0,0) = rhoy * fSy / epsSy;
  
    Dc.addMatrixTripleProduct( 0.0, Tc,  Dcp,  1.0);
    Ds1.addMatrixTripleProduct(0.0, Ts1, Ds1p, 1.0);
    Ds2.addMatrixTripleProduct(0.0, Ts2, Ds2p, 1.0);

	secant_matrix = Dc + Ds1 + Ds2;

	secant_matrix(0,2) = 0.5 * secant_matrix(0,2);
	secant_matrix(1,2) = 0.5 * secant_matrix(1,2);
	secant_matrix(2,2) = 0.5 * secant_matrix(2,2);

    // Calculate pseudo-prestress vector

    //stress0_vec.addMatrixVector(0.0, Dc, epsCp_vec, 1.0);

    //static Matrix DInv(3,3);  // D invert
    //tangent_matrix.Invert(DInv);

	//Vector tempVec(3);
	//tempVec.addMatrixVector(0.0, DInv, stress0_vec, 1.0);

	//epsCp_vec -= tempVec;

	errorVec = Tstrain - epsCe_vec - epsCp_vec - epsSlip_vec;

    errNorm = errorVec.pNorm(-1);
	// determine the norm of matrixnorm = tempMat.Norm();
	// errNorm = norm - tempNorm;
	
  	if ( errNorm <= tolNorm) {
	  vCiconverged = true;
	  //epsC12cm_vec(0) += deltaEpsCm1;
	  //epsC12cm_vec(1) += deltaEpsCm2;
	  //epsC12tm_vec(0) += deltaEpsTm1;
	  //epsC12tm_vec(1) += deltaEpsTm2;
	//  this->Print(opserr,0);
	} else {

      errorVec /= (pow(2.0, iteration_counter/100));
      
	  epsC_vec += errorVec;
	  
	  iteration_counter += 1;
	  numberDivide = iteration_counter/ 100;
	  if (numberDivide == 5) {
		opserr << "errorNorm = " << errNorm << endln;
	  } 
	}
  }

  // Calculate the tangent matrix.
  tangent_matrix = secant_matrix;

  // Calculate total stress from steel and concrete
  /*if ( Par == 3 ){

    vcxy = (fC1-fC2)*sin(2.0*cita)/2.0;
    if (fabs(cita) <= DBL_EPSILON) {
	  Tstress(0) = fC1 + rhox * fSx;
	  Tstress(1) = fC2 + rhoy * fSy;
	  Tstress(2) = vcxy;
    } else {
      Tstress(0) = fC1 - vcxy / tan(cita) + rhox * fSx;
      Tstress(1) = fC1 - vcxy * tan(cita) + rhoy * fSy;
      Tstress(2) = vcxy;
    }

  }
  else if (Par == 4) {

    vcxy = (fC2-fC1)*sin(2.0*cita)/2.0;
    if (fabs(cita) <= DBL_EPSILON) {
	  Tstress(0) = fC2 + rhox * fSx;
	  Tstress(1) = fC1 + rhoy * fSy;
	  Tstress(2) = vcxy;
    } else {
      Tstress(0) = fC2 - vcxy / tan(cita) + rhox * fSx;
      Tstress(1) = fC2 - vcxy * tan(cita) + rhoy * fSy;
      Tstress(2) = vcxy;
    }

  }
  else {

	//Tstress(0) = pow(cos(cita),2)*fC1 + pow(sin(cita),2)*fC2
	//		   + pow(cos(angle1),2)*rhox*fSx + pow(cos(angle2),2)*rhoy*fSy;
	//
	//Tstress(1) = pow(sin(cita),2)*fC1 + pow(cos(cita),2)*fC2
	//		   + pow(sin(angle1),2)*rhox*fSx + pow(sin(angle2),2)*rhoy*fSy;
	//
	//Tstress(2) = cos(cita)*sin(cita)*fC1 - cos(cita)*sin(cita)*fC2
	//		   + cos(angle1)*sin(angle1)*rhox*fSx + cos(angle2)*sin(angle2)*rhoy*fSy;
    if (fC1 < fC2) {
	  vcxy = (fC2-fC1)*sin(2.0*cita)/2.0;
      if (fabs(cita) <= DBL_EPSILON) {
	    Tstress(0) = fC2 + rhox * fSx;
	    Tstress(1) = fC1 + rhoy * fSy;
	    Tstress(2) = vcxy;
      } else {
        Tstress(0) = fC2 - vcxy / tan(cita) + rhox * fSx;
        Tstress(1) = fC2 - vcxy * tan(cita) + rhoy * fSy;
        Tstress(2) = vcxy;
      } 
    }
	else {
	  vcxy = (fC1-fC2)*sin(2.0*cita)/2.0;
      if (fabs(cita) <= DBL_EPSILON) {
	    Tstress(0) = fC1 + rhox * fSx;
	    Tstress(1) = fC2 + rhoy * fSy;
	    Tstress(2) = vcxy;
      } else {
        Tstress(0) = fC1 - vcxy / tan(cita) + rhox * fSx;
        Tstress(1) = fC1 - vcxy * tan(cita) + rhoy * fSy;
        Tstress(2) = vcxy;
      }
    }

  }  */
  Tstress(0) = fC1 * pow(cos(cita),2.0) + fC2 * pow(sin(cita),2.0) + rhox*fSx;
  Tstress(1) = fC1 * pow(sin(cita),2.0) + fC2 * pow(cos(cita),2.0) + rhoy*fSy;
  Tstress(2) = (fC1-fC2)*sin(2.0*cita)/2.0;

  opserr << "cita = " << cita << ";\t citaS = " << citaS << ";\t citaE = " << citaE <<endln;

  //Tstress.addMatrixVector(0.0, tangent_matrix, strain, 1.0);

  stress_vec = Tstress;

  return stress_vec;
}

int
MCFTRCPlaneStress::determineSecantModulus(Vector strain)
{
  // Get Principal strain direction first // Get strain values from strain of element
  Vector Tstrain(3);     //epslon_x, epslon_y, gamma_xy
  Tstrain(0) = strain(0);
  Tstrain(1) = strain(1);
  Tstrain(2) = 0.5*strain(2);

  double cita, temp_cita, citan1, citan2, citaLag, dCitaE, dCitaS, citaIC; // principal strain direction
  double epsS1, epsS2, epsts;
  double eC1p=0.0, eC2p=0.0, eC1m=0.0, eC2m=0.0, eT1m=0.0, eT2m=0.0, eSlip1=0.0, eSlip2=0.0;
  double fC1a, fC1b, fC1c, fC2a, fC2b, fC2c, fS1, fS2; //
  double halfGammaOneTwo;

  double fcr   = 0.65 * pow(-fpc, 0.33); //0.31*sqrt(-fpc);      ----ftp
  double Ec    = 2.0 * fpc/epsc0;
  double epscr = fcr/Ec;
  double vci, vcimax, s_cita=0.0, w=0.0, delta_S=0.0, gamma_S=0.0; //vcxy, 
  double betaD1, betaD2;
  double Ecx, Ecy;

  static Vector tempStrain(3); // maintain main strain vector 
  int Par=0;

  // Get citaE based on Tstrain, eq. i-9...
  if ( fabs(Tstrain(0)-Tstrain(1)) < SMALL_STRAIN ) {  // Tstrain(0) == Tstrain(1) 
    citaE = 0.25*PI;
  } else {  // else Tstrain(0) != Tstrain(1) 
    temp_cita = 0.5 * atan(fabs(2.0e6*Tstrain(2)/(1.0e6*Tstrain(0)-1.0e6*Tstrain(1)))); 
    if ( fabs(Tstrain(2)) < SMALL_STRAIN ) {
      citaE = 0;
    } else if ( (Tstrain(0) > Tstrain(1)) && ( Tstrain(2) > 0) ) {
      citaE = temp_cita;
    } else if ( (Tstrain(0) > Tstrain(1)) && ( Tstrain(2) < 0) ) {
      citaE = PI - temp_cita;
    } else if ( (Tstrain(0) < Tstrain(1)) && ( Tstrain(2) > 0) ) {
      citaE = 0.5*PI - temp_cita;
    } else if ( (Tstrain(0) < Tstrain(1)) && ( Tstrain(2) < 0) ) {
      citaE = 0.5*PI + temp_cita;
    } else {
      opserr << "CSMMRCPlaneStress::determineTrialStress: Failure to calculate citaE\n";
      opserr << " Tstrain(0) = " << Tstrain(0) << endln;
      opserr << " Tstrain(1) = " << Tstrain(1) << endln;
      opserr << " Tstrain(2) = " << Tstrain(2) << endln;
    }
  }

  citaIC = citaE;  // initial cracked angle

  static Matrix Tc(3,3);     // T(cita_E)
  static Matrix Ts1(3,3);    // T(angle1)
  static Matrix Ts2(3,3);    // T(angle2)
  static Matrix T_cita(3,3); // T(cita)
  static Matrix Dc(3,3);     //
  static Matrix Ds1(3,3);    //
  static Matrix Ds2(3,3);    //
  static Matrix Dcp(3,3);    //
  static Matrix Ds1p(3,3);	 //
  static Matrix Ds2p(3,3);	 //

  // EbarC1 EbarC2 GbarC, eq.i-5
  Dcp.Zero();  // local stiffness matrix
  Ds1p.Zero();
  Ds2p.Zero();

  Tc(0,0) = pow(cos(citaE), 2.0);// T(cita); first cita=citaE
  Tc(0,1) = pow(sin(citaE), 2.0);
  Tc(0,2) = cos(citaE)*sin(citaE);
  Tc(1,0) = pow(sin(citaE), 2.0);
  Tc(1,1) = pow(cos(citaE), 2.0);
  Tc(1,2) = -cos(citaE)*sin(citaE);
  Tc(2,0) = -2.0*cos(citaE)*sin(citaE);
  Tc(2,1) = 2.0*cos(citaE)*sin(citaE);
  Tc(2,2) = pow(cos(citaE), 2.0)-pow(sin(citaE), 2.0);

  Ts1(0,0) = pow(cos(angle1), 2.0);// T(angle1)
  Ts1(0,1) = pow(sin(angle1), 2.0);
  Ts1(0,2) = cos(angle1)*sin(angle1);
  Ts1(1,0) = pow(sin(angle1), 2.0);
  Ts1(1,1) = pow(cos(angle1), 2.0);
  Ts1(1,2) = -cos(angle1)*sin(angle1);
  Ts1(2,0) = -2.0*cos(angle1)*sin(angle1);
  Ts1(2,1) = 2.0*cos(angle1)*sin(angle1);
  Ts1(2,2) = pow(cos(angle1), 2.0)-pow(sin(angle1), 2.0);

  Ts2(0,0) = pow(cos(angle2), 2.0);// T(angle2)
  Ts2(0,1) = pow(sin(angle2), 2.0);
  Ts2(0,2) = cos(angle2)*sin(angle2);
  Ts2(1,0) = pow(sin(angle2), 2.0);
  Ts2(1,1) = pow(cos(angle2), 2.0);
  Ts2(1,2) = -cos(angle2)*sin(angle2);
  Ts2(2,0) = -2.0*cos(angle2)*sin(angle2);
  Ts2(2,1) = 2.0*cos(angle2)*sin(angle2);
  Ts2(2,2) = pow(cos(angle2), 2.0)-pow(sin(angle2), 2.0);

  Information &S1SetVar = theResponses[S1_SET_V]->getInformation(); // S1 setVar
  Information &S2SetVar = theResponses[S2_SET_V]->getInformation(); // S2 setVar
  Information &S1GetVar = theResponses[S1_GET_V]->getInformation(); // S1 getVar
  Information &S2GetVar = theResponses[S2_GET_V]->getInformation(); // S2 getVar
  Information &C1SetVar = theResponses[C1_SET_V]->getInformation(); // C1 setVar
  Information &C2SetVar = theResponses[C2_SET_V]->getInformation(); // C2 setVar
  Information &C1GetVar = theResponses[C1_GET_V]->getInformation(); // C1 getVar
  Information &C2GetVar = theResponses[C2_GET_V]->getInformation(); // C2 getVar

  //for small strain return quickly
  /*if (Tstrain.pNorm(-1) <= DBL_EPSILON) {
	double v = 0.22;
	double temp = Ec/(1.0-v*v);

	Dcp(0,0) = temp;
	Dcp(0,1) = v*temp;
	Dcp(1,0) = v*temp;
	Dcp(1,1) = temp;
	Dcp(2,2) = temp*(1.0-v)/2.0;

	Ds1p(0,0)= Es * rhox;
	Ds2p(0,0)= Es * rhoy;

	Dc.addMatrixTripleProduct( 0.0, Tc,  Dcp,  1.0);
	Ds1.addMatrixTripleProduct(0.0, Ts1, Ds1p, 1.0);
	Ds2.addMatrixTripleProduct(0.0, Ts2, Ds2p, 1.0);

	secant_matrix = Dc + Ds1 + Ds2;
	tangent_matrix = secant_matrix;

	Tstress.Zero();

	return 0;
  } 
  else if (Tstrain.pNorm(-1) > 1.e12) {
	secant_matrix.Zero();
	secant_matrix(0,0) = 1.e-8;
	secant_matrix(1,1) = 1.e-8;
	secant_matrix(2,2) = 1.e-8;
	tangent_matrix = secant_matrix;
	Tstress.Zero();

	return 0;
  } */

  // Get average strains in reinforcement, eq. i-19
  //epsSx = 0.5 * (Tstrain(0)+Tstrain(1))+0.5*(Tstrain(0)-Tstrain(1))*cos(2.0*angle1)
  //      + 0.5 * Tstrain(2)*sin(2.0*angle1);
  //epsSy = 0.5 * (Tstrain(0)+Tstrain(1))+0.5*(Tstrain(0)-Tstrain(1))*cos(2.0*angle2)
  //      + 0.5 * Tstrain(2)*sin(2.0*angle2);
  tempStrain.addMatrixVector(0.0, Ts1, Tstrain, 1.0);
  epsS1 = tempStrain(0);
  tempStrain.addMatrixVector(0.0, Ts2, Tstrain, 1.0);
  epsS2 = tempStrain(0);

  // Calculate Steel stress incorporate steel offset strains

  //S1SetVar.setVector(theData);
  //S2SetVar.setVector(theData);
  //theResponses[S1_SET_V]->getResponse();  // setVar
  //theResponses[S2_SET_V]->getResponse();  // setVar

  if (theMaterial[S_ONE]->setTrialStrain(epsS1) != 0) {
    opserr << "MCFTRCPlaneStress::determineTrialStress(), S1 setTrialStrain() Error" << endln;  // s1
	return -1;
  }

  if (theMaterial[S_TWO]->setTrialStrain(epsS2) != 0) {
    opserr << "MCFTRCPlaneStress::determineTrialStress(), S2 setTrialStrain() Error" << endln;  // s2
	return -1;
  }

  fS1 = theMaterial[S_ONE]->getStress();
  fS2 = theMaterial[S_TWO]->getStress();
  
  // initial epsC vector assumptions equal to previous committed value
  epsC_vec  = Tstrain; // epsC_vec = epsCe_vec + epsCp_vec; 
  //epsCp_vec = CepsCp_vec;

  // vCi need to be satisfied several relationships in the concrete crack region
  static Vector errorVec(3); // epsC_vec converge test need a vector to maintain Error
  static Vector theData(8); // data for materials passed in 
  double errNorm;
  double temp, tolNorm = SMALL_STRAIN;
  bool vCiconverged = false;
  int numberDivide = 1, iteration_counter = 0, ret;
  double deltaEpsC1p, deltaEpsC2p, deltaEpsCm1, deltaEpsCm2, deltaEpsTm1, deltaEpsTm2;

  while (vCiconverged == false && numberDivide <=5) {
    
	// Get citaS based on epsCe_vec, eq.i-11    
	// citaS = cita, based on elastic strain part
	// MCFT cita corresponding strain field, DSFM cita corresponding stress field
	epsCe_vec = epsC_vec - epsCp_vec; // - epsSlip_vec
    if ( fabs(epsCe_vec(0)-epsCe_vec(1)) < SMALL_STRAIN) {
	  citaS = 0.25*PI;
    } else {  // epsCe_vec(0) != epsCe_vec(1) 
      temp_cita = 0.5 * atan(fabs(2.0e6*epsCe_vec(2)/(1.0e6*epsCe_vec(0)-1.0e6*epsCe_vec(1)))); 
      if ( fabs(epsCe_vec(2)) < SMALL_STRAIN ) {
		citaS = 0;
  	  } else if ( (epsCe_vec(0) > epsCe_vec(1)) && ( epsCe_vec(2) > 0) ) {
  	    citaS = temp_cita;
  	  } else if ( (epsCe_vec(0) > epsCe_vec(1)) && ( epsCe_vec(2) < 0) ) {
  	    citaS = PI - temp_cita;
  	  } else if ( (epsCe_vec(0) < epsCe_vec(1)) && ( epsCe_vec(2) > 0) ) {
  	    citaS = 0.5*PI - temp_cita;
  	  } else if ( (epsCe_vec(0) < epsCe_vec(1)) && ( epsCe_vec(2) < 0) ) {
  	    citaS = 0.5*PI + temp_cita;
  	  } else {
  	    opserr << "MCFTRCPlaneStress::determineTrialStress: Failure to calculate citaS\n";
  	    opserr << " epsCe_vec(0) = " << epsCe_vec(0) << endln;
  	    opserr << " epsCe_vec(1) = " << epsCe_vec(1) << endln;
  	    opserr << " epsCe_vec(2) = " << epsCe_vec(2) << endln;
  	  }
    }

    cita=citaS;  //eq.i-11    PI/2.0-citaS = citaCrack
    citan1 = citaS - angle1; //angle1-(PI/2.0-citaS); //eq.i-6
    citan2 = citaS - angle2; //angle2-(PI/2.0-citaS);

	// Transform matrix for current strain state 
	T_cita(0,0) = pow(cos(cita), 2.0);// T(cita) 
	T_cita(0,1) = pow(sin(cita), 2.0);
	T_cita(0,2) = cos(cita)*sin(cita);
	T_cita(1,0) = pow(sin(cita), 2.0);
	T_cita(1,1) = pow(cos(cita), 2.0);
	T_cita(1,2) = -cos(cita)*sin(cita);
	T_cita(2,0) = -2.0*cos(cita)*sin(cita);
	T_cita(2,1) = 2.0*cos(cita)*sin(cita);
	T_cita(2,2) = pow(cos(cita), 2.0)-pow(sin(cita), 2.0);

	// Get epsC1,epsC2 and citaS based on epsC_vec, eq.i-10
	//epsC1 = 0.5*(epsC_vec(0)+epsC_vec(1)) 
	//	  + 0.5*sqrt(pow(epsC_vec(0)-epsC_vec(1), 2.0)+pow(epsC_vec(2), 2.0));
	//epsC2 = 0.5*(epsC_vec(0)+epsC_vec(1)) 
	//	  - 0.5*sqrt(pow(epsC_vec(0)-epsC_vec(1), 2.0)+pow(epsC_vec(2), 2.0));

	tempStrain.addMatrixVector(0.0, T_cita, epsC_vec, 1.0);
	epsC1 = tempStrain(0);
	epsC2 = tempStrain(1);
	halfGammaOneTwo = tempStrain(2);  // should be a minor value

	// Vecchio: Towards Cyclic Load Modeling of Reinforced Concrete
	// C max
	deltaEpsCm1 = (epsC1 < CepsC12cm_vec(0) ? epsC1-CepsC12cm_vec(0) : 0);
	deltaEpsCm2 = (epsC2 < CepsC12cm_vec(1) ? epsC2-CepsC12cm_vec(1) : 0);

	epsCcm_vec(0) = CepsCcm_vec(0) + (0.5 * deltaEpsCm1 * (1+cos(2.0*cita)) + 0.5 * deltaEpsCm2 * (1-cos(2.0*cita)));
	epsCcm_vec(1) = CepsCcm_vec(1) + (0.5 * deltaEpsCm1 * (1-cos(2.0*cita)) + 0.5 * deltaEpsCm2 * (1+cos(2.0*cita)));
	epsCcm_vec(2) = CepsCcm_vec(2) + (deltaEpsCm1 - deltaEpsCm2) * sin(2.0*cita);

	//eC1m = 0.5*(epsCcm_vec(0)+epsCcm_vec(1))+0.5*((epsCcm_vec(0)-epsCcm_vec(1))*cos(2.0*cita)+epsCcm_vec(2)*sin(2.0*cita));
	//eC2m = 0.5*(epsCcm_vec(0)+epsCcm_vec(1))-0.5*((epsCcm_vec(0)-epsCcm_vec(1))*cos(2.0*cita)+epsCcm_vec(2)*sin(2.0*cita));

	tempStrain.addMatrixVector(0.0, T_cita, CepsCcm_vec, 1.0);
	eC1m = tempStrain(0);
	eC2m = tempStrain(1);
	halfGammaOneTwo = tempStrain(2);

	// T max
	deltaEpsTm1 = (epsC1 > epsC12tm_vec(0) ? epsC1-CepsC12tm_vec(0) : 0);
	deltaEpsTm2 = (epsC2 > epsC12tm_vec(1) ? epsC2-CepsC12tm_vec(1) : 0);

	epsCtm_vec(0) = CepsCtm_vec(0) + (0.5*deltaEpsTm1*(1+cos(2.0*cita)) + 0.5*deltaEpsTm2*(1-cos(2.0*cita)));
	epsCtm_vec(1) = CepsCtm_vec(1) + (0.5*deltaEpsTm1*(1-cos(2.0*cita)) + 0.5*deltaEpsTm2*(1+cos(2.0*cita)));
	epsCtm_vec(2) = CepsCtm_vec(2) + (deltaEpsTm1 - deltaEpsTm2) * sin(2.0*cita);

	//eT1m = 0.5*(epsCtm_vec(0)+epsCtm_vec(1))+0.5*((epsCtm_vec(0)-epsCtm_vec(1))*cos(2.0*cita)+epsCtm_vec(2)*sin(2.0*cita));
	//eT2m = 0.5*(epsCtm_vec(0)+epsCtm_vec(1))-0.5*((epsCtm_vec(0)-epsCtm_vec(1))*cos(2.0*cita)+epsCtm_vec(2)*sin(2.0*cita));

	tempStrain.addMatrixVector(0.0, T_cita, CepsCtm_vec, 1.0);
	eT1m = tempStrain(0);
	eT2m = tempStrain(1);
	halfGammaOneTwo = tempStrain(2);

	int status=0; // status to check if iteration satisfied eq.i-7
	double tolerance = SMALL_STRESS; // tolerance for iteration
	bool fC1converged, fC2converged;
	double error, fScrx, fScry, epsScrx, epsScry, epsIncr;

  //if(epsC1 > 0 && epsC2 > 0) {
  //Par = 1;
    //////////////////////////////////////////////////////////////////////////
    // C1 -- tensile for fc1, epsC1  
    //////////////////////////////////////////////////////////////////////////
    //  case 1: eq.i-33,34
    epsts = 2.0 * 7.5e-3 / fcr / 750.; //eq. i-33
    temp  = (epsC1 - epscr) / (epsts - epscr);
    
    if      (temp <= 0.0) fC1a = 0.0;
    else if (temp >  1.0) fC1a = fcr;
    else                  fC1a = fcr * (1 - temp);  //eq. i-34
    
    //  case 2: eq. i-35, 36
    temp = 4.0*(rhox/db1*fabs(cos(citan1))+rhoy/db2*fabs(cos(citan2)));
    fC1b = fcr/(1+sqrt(2.2/temp*epsC1));
    temp = max(fC1a,fC1b); // eq.i-38 /fC1
    
    // eq.i-5   // for positive value
    fC1c = fabs(rhox* (fyx - fS1) * pow(cos(citan1), 2.0)
         + rhoy* (fyy - fS2) * pow(cos(citan2), 2.0) );
    double fC1max = min(temp, fC1c);
    
    theData(0) = eC1m;
    theData(1) = eT1m;
      //theData(2) = epsC1;
    theData(5) = CepsC12p(0);
    theData(6) = 1.0; //betaD
    theData(7) = 1.0; // K
  
    ret = 0;
    ret += C1SetVar.setVector(theData);  // Information for C tension, material [2], response[4]
    ret += theResponses[C1_SET_V]->getResponse(); // C1 setVar 
    ret += theMaterial[C_ONE]->setTrialStrain(epsC1);
    if (ret != 0) {
      opserr << "MCFTRCPlaneStress::determineTrialStress(), C1 setTrialStrain() Error" << endln;  // C1
      return ret;
    }
    temp = theMaterial[C_ONE]->getStress();
    fC1 = min(fC1max, temp);
  
    //////////////////////////////////////////////////////////////////////////
    if(epsC1 > epscr) { //checkAtCrack(); 
      // Calculate local stress at cracks
      status         = 0; // status to check if iteration satisfied eq.i-7
      fC1converged   = false;
      epsScrx        = epsS1;
      epsScry        = epsS2;
      epsIncr        = 0.0;
      
      while ( fC1converged == false && status == 0 ) {
        if(fabs(epsScrx) > 0.5 || fabs(epsScry) > 0.5) { // for very large strain at cracks
      opserr << "MCFTRCPlaneStress::determineTrialStress(vector) -- large strain at cracks" << endln;
      opserr << "TrialStrainVector: " << Tstrain(0) << "\t"<< Tstrain(1) << "\t"<< Tstrain(2) << endln;
      opserr << "fC1converged false for strain: epsScrx = " << epsScrx << "; epsScry = " << epsScry << endln; 
      status = 1;
        }
        // eq.i-20
        epsScrx = epsS1 + epsIncr * pow(cos(citan1), 2.0);
        epsScry = epsS2 + epsIncr * pow(cos(citan2), 2.0);
        
        /*if( theMaterial[S_ONE]->setTrialStrain(epsScrx) != 0) {
      opserr << "MCFTRCPlaneStress::determineTrialStress(): fail to determine epsScrx" << endln;
      return -1;
        }
        fScrx = theMaterial[S_ONE]->getStress();
        if( theMaterial[S_TWO]->setTrialStrain(epsScry) != 0) {
      opserr << "MCFTRCPlaneStress::determineTrialStress(): fail to determine epsScry" << endln;
      return -1;
        }
        fScry = theMaterial[S_TWO]->getStress(); */
        fScrx = determinefS(epsScrx, fyx, Es, 1.02*Es);
        fScry = determinefS(epsScry, fyy, Es, 1.02*Es);
      
        // eq.i-7
        temp = rhox * (fScrx - fS1) * pow(cos(citan1), 2.0) 
             + rhoy * (fScry - fS2) * pow(cos(citan2), 2.0);
        error = fC1 - temp;
        
        if (fabs(error) <= tolerance) {
          fC1converged = true;
        } else {
          // temp is function of citan1 citan2 and fSx and fSy,
          // the iterative parameter should include citan1/citan2 angle chosen. 
          // like: if (citan1 > PI/2 && citan2 > PI/2)...
          temp = rhox * theMaterial[S_ONE]->getSecant() * pow(cos(citan1), 2.0)
               + rhoy * theMaterial[S_TWO]->getSecant() * pow(cos(citan2), 2.0);
          epsIncr += error/temp;
        }
      }
      // eq. i-8
      vci = rhox * (fScrx - fS1) * cos(citan1) * sin(citan1)
          + rhoy * (fScry - fS2) * cos(citan2) * sin(citan2);
      //vci = (vci < 0 ? 0 : vci);
      
      // Crack slips determine
      //while ( citaS > 0.5*PI ) {
      //	citaS -= 0.5*PI;
      //}
      
      // crack spacing // w   
      s_cita = 1.0/(sin(citaS)/xd+cos(citaS)/yd); // eq.i-21,22
      w      = epsC1 * s_cita;
      
      // optional 1
      vcimax = sqrt(-fpc)/(0.31+24*w/(aggr+16)); // shear parameter
      
      // if vci > vcimax , decrease fC1
      if (fabs(vci) > vcimax) {
      	fC1 -= (fabs(vci) - vcimax)*(tan(citaS)+1./tan(citaS));  // MCFT(1986) eq. 13
      }
      //fC1 = (fC1 > 0 ? fC1 : 0);
      //vci = vci * min(fabs(vcimax), fabs(vci)) / fabs(vci);
    }
  
    //////////////////////////////////////////////////////////////////////////
    // C2 -- tensile for fc2, epsC2    
    //////////////////////////////////////////////////////////////////////////
    //epsts = 2.0 * 7.5e-3 / fcr / 750.; //eq. i-33
    temp  = (epsC2 - epscr) / (epsts - epscr);
    
    if      (temp <= 0.0) fC2a = 0.0;
    else if (temp >  1.0) fC2a = fcr;
    else                  fC2a = fcr * (1 - temp);  //eq. i-34
    
    //  case 2: eq. i-35, 36
    temp = 4.0*(rhox/db1*fabs(cos(citan1))+rhoy/db2*fabs(cos(citan2)));
    fC2b = fcr/(1+sqrt(2.2/temp*epsC2));
    temp = max(fC2a,fC2b); // eq.i-38 /fC1
  
    // eq.i-5   // for positive value
    fC2c = fabs(rhox* (fyx - fS1) * pow(cos(citan1), 2.0)
         + rhoy* (fyy - fS2) * pow(cos(citan2), 2.0) );
    double fC2max = min(temp, fC2c);
    
    theData(0) = eC2m; 
    theData(1) = eT2m;
    //theData(2) = epsC2;
    theData(5) = CepsC12p(1);
    theData(6) = 1.0; //betaD
    theData(7) = 1.0; // K
  
    ret = 0;
    ret += C2SetVar.setVector(theData);  // Information for C2 in tension, material [3], response[5]
    ret += theResponses[C2_SET_V]->getResponse(); // C2 setVar 
    ret += theMaterial[C_TWO]->setTrialStrain(epsC2);
    if (ret != 0) {
      opserr << "MCFTRCPlaneStress::determineTrialStress(), C2 setTrialStrain() Error" << endln;  // C2
      return ret;
    }
    temp = theMaterial[C_TWO]->getStress();
    fC2 = min(fC2max, temp);
    
    //////////////////////////////////////////////////////////////////////////
    if(epsC2 > epscr) { //checkAtCrack(); 
      // Calculate local stress at cracks
      status         = 0; // status to check if iteration satisfied eq.i-7
      fC2converged   = false;
      epsScrx        = epsS1;
      epsScry        = epsS2;
      epsIncr        = 0.0;
      
      while ( fC2converged == false && status == 0 ) {
        if(fabs(epsScrx) > 0.5 || fabs(epsScry) > 0.5) { // for very large strain at cracks
      opserr << "MCFTRCPlaneStress::determineTrialStress(vector) -- large strain at cracks" << endln;
      opserr << "TrialStrainVector: " << Tstrain(0) << "\t"<< Tstrain(1) << "\t"<< Tstrain(2) << endln;
      opserr << "fC1converged false for strain: epsScrx = " << epsScrx << "; epsScry = " << epsScry << endln; 
      status = 1;
        }
        // eq.i-20
        epsScrx = epsS1 + epsIncr * pow(cos(citan1), 2.0);
        epsScry = epsS2 + epsIncr * pow(cos(citan2), 2.0);
        
        /*if( theMaterial[S_ONE]->setTrialStrain(epsScrx) != 0) {
      opserr << "MCFTRCPlaneStress::determineTrialStress(): fail to determine epsScrx" << endln;
      return -1;
        }
        fScrx = theMaterial[S_ONE]->getStress();
  
        if( theMaterial[S_TWO]->setTrialStrain(epsScry) != 0) {
      opserr << "MCFTRCPlaneStress::determineTrialStress(): fail to determine epsScry" << endln;
      return -1;
        }
  	  fScry = theMaterial[S_TWO]->getStress(); */
        fScrx = determinefS(epsScrx, fyx, Es, 1.02*Es);
        fScry = determinefS(epsScry, fyy, Es, 1.02*Es);
        
        // eq.i-7
        temp = rhox * (fScrx - fS1) * pow(cos(citan1), 2.0) 
             + rhoy * (fScry - fS2) * pow(cos(citan2), 2.0);
        error = fC2 - temp;
        
        if (fabs(error) <= tolerance) {
          fC2converged = true;
        } else {
          // temp is function of citan1 citan2 and fSx and fSy,
          // the iterative parameter should include citan1/citan2 angle chosen. 
          // like: if (citan1 > PI/2 && citan2 > PI/2)...
          temp = rhox * theMaterial[S_ONE]->getSecant() * pow(cos(citan1), 2.0)
                  + rhoy * theMaterial[S_TWO]->getSecant() * pow(cos(citan2), 2.0);
          epsIncr += error/temp;
        }
      }
      // eq. i-8
      vci = rhox * (fScrx - fS1) * cos(citan1) * sin(citan1)
           + rhoy * (fScry - fS2) * cos(citan2) * sin(citan2);
      //vci = (vci < 0 ? 0 : vci);
      
      // Crack slips determine
      //while ( citaS > 0.5*PI ) {
      //	citaS -= 0.5*PI;
      //}
      
      // crack spacing // w   
      s_cita = 1.0/(sin(citaS)/xd+cos(citaS)/yd); // eq.i-21,22
      w      = epsC2 * s_cita;
      
      // optional 1
      vcimax = sqrt(-fpc)/(0.31+24*w/(aggr+16)); // shear parameter
      
      // if vci > vcimax , decrease fC2
      if (fabs(vci) > vcimax) {
        fC2 -= (fabs(vci) - vcimax)*(tan(citaS)+1./tan(citaS));  // MCFT(1986) eq. 13
      }
      //fC2 = (fC2 > 0 ? fC2 : 0);
      //vci = vci * min(fabs(vcimax), fabs(vci)) / fabs(vci);
    }
  
    //////////////////////////////////////////////////////////////////////////
    // get C1, C2 's epsilon_p
    //////////////////////////////////////////////////////////////////////////
    theResponses[C1_GET_V]->getResponse();  // C1 getVar
    theResponses[C2_GET_V]->getResponse();  // C2 getVar
    
    epsC12p(0) = (*C1GetVar.theVector)(5); // ecp1 
    epsC12p(1) = (*C2GetVar.theVector)(5); // ecp2 
    if (epsC12p(0) > 0) epsC12p(0) = 0.0;
    if (epsC12p(1) > 0) epsC12p(1) = 0.0;
  
    /*else if (epsC1 < 0 && epsC2 < 0) {
      Par = 2;
      //////////////////////////////////////////////////////////////////////////
      // C1 Kupfer envelop for concrete
      //////////////////////////////////////////////////////////////////////////
      opserr << "epsC1 and epsC2 negative, both!" << endln;
      double sig1 = 0.0, sig2 = 0.0, sig_p1, sig_p2, eps_p1, eps_p2;
      double norm = 1.0, K1, K2;
      while (norm > SMALL_STRESS) {
        K1 = 1 + 0.92 * (sig2/fpc)-0.76*pow(sig2/fpc,2.0);
        K2 = 1 + 0.92 * (sig1/fpc)-0.76*pow(sig1/fpc,2.0);
        sig_p1 = K1 * fpc;
        eps_p1 = K1 * epsc0;
        sig_p2 = K2 * fpc;
        eps_p2 = K2 * epsc0;
      
        //////////////////////////////////////////////////////////////////////////
        // C1
        //////////////////////////////////////////////////////////////////////////
        theData(0) = eC1m;
        theData(1) = eT1m;
        //theData(2) = epsC1;
        theData(5) = CepsC12p(0);
        theData(6) = 1.0; //betaD
        theData(7) = K1;
      
        ret = 0;
        ret += C1SetVar.setVector(theData);  // Information for C tension, material [2], response[4]
        ret += theResponses[C1_SET_V]->getResponse(); // C1 setVar 
        ret += theMaterial[C_ONE]->setTrialStrain(epsC1);
        if (ret != 0) {
          opserr << "MCFTRCPlaneStress::determineTrialStress(), C1 setTrialStrain() Error" << endln;  // C1
          return ret;
        }
        fC1 = theMaterial[C_ONE]->getStress();
      
        //////////////////////////////////////////////////////////////////////////
        // C2
        //////////////////////////////////////////////////////////////////////////
        theData(0) = eC2m;
        theData(1) = eT2m;
        //theData(2) = epsC2;
        theData(5) = CepsC12p(1);
        theData(6) = 1.0;
        theData(7) = K2;
      
        ret = 0;
        ret += C2SetVar.setVector(theData);  // Information for C compression, material [3], response[5]
        ret += theResponses[C2_SET_V]->getResponse(); // C2 setVar
        ret += theMaterial[C_TWO]->setTrialStrain(epsC2); //need further revision
        if (ret != 0) {
          opserr << "MCFTRCPlaneStress::determineTrialStress(), C2 setTrialStrain() Error" << endln;  // C2
          return ret;
        }
        fC2 = theMaterial[C_TWO]->getStress(); // w==5 ??
        
        //////////////////////////////////////////////////////////////////////////
        norm = sqrt(pow(fC1-sig1,2.0)+pow(fC2-sig2,2.0));
        sig1 = fC1;
        sig2 = fC2;
      }
        
      theResponses[C1_GET_V]->getResponse(); // C1 getVar
      epsC12p(0) = (*C1GetVar.theVector)(5); // ecp1
      theResponses[C2_GET_V]->getResponse(); // C2 getVar
      epsC12p(1) = (*C2GetVar.theVector)(5); // ecp2 
      opserr << "K1 = " << K1 << "\tK2 = " << K2 << endln;
    } 
    
    if (epsC1 > 0 && epsC2 < 0) {
      Par = 3;
      //////////////////////////////////////////////////////////////////////////
      // C1 -- tensile for fc1, epsC1  
      //////////////////////////////////////////////////////////////////////////
      //  case 1: eq.i-33,34
      epsts = 2.0 * 7.5e-3 / fcr / 750.; //eq. i-33
      temp  = (epsC1 - epscr) / (epsts - epscr);
      
      if      (temp <= 0.0) fC1a = 0.0;
      else if (temp >  1.0) fC1a = fcr;
      else                  fC1a = fcr * (1 - temp);  //eq. i-34
      
      //  case 2: eq. i-35, 36
      temp = 4.0*(rhox/db1*fabs(cos(citan1))+rhoy/db2*fabs(cos(citan2)));
      fC1b = fcr/(1+sqrt(2.2/temp*epsC1));
      temp = max(fC1a, fC1b);
      
      // eq.i-5   // for positive value
      fC1c = fabs(rhox* (fyx - fS1) * pow(cos(citan1), 2.0)
           + rhoy* (fyy - fS2) * pow(cos(citan2), 2.0) );
      double fC1max = min(temp, fC1c);
      
      theData(0) = eC1m;
      theData(1) = eT1m;
      //theData(2) = epsC1;
      theData(5) = CepsC12p(0);
      theData(6) = 1.0; //betaD
      theData(7) = 1.0; // K
      ret = 0;
      ret += C1SetVar.setVector(theData);  // Information for C tension, material [2], response[4]
      ret += theResponses[C1_SET_V]->getResponse(); // C1 setVar 
      ret += theMaterial[C_ONE]->setTrialStrain(epsC1);
      if (ret != 0) {
        opserr << "MCFTRCPlaneStress::determineTrialStress(), C1 setTrialStrain() Error" << endln;  // C1
        return ret;
      }
      temp = theMaterial[C_ONE]->getStress();
      fC1 = min(fC1max, temp);
    
      //////////////////////////////////////////////////////////////////////////
      if(epsC1 > epscr) { //checkAtCrack(); 
      // Calculate local stress at cracks
        status         = 0; // status to check if iteration satisfied eq.i-7
        fC1converged   = false;
        epsScrx        = epsS1;
        epsScry        = epsS2;
        epsIncr        = 0.0;
        
        while ( fC1converged == false && status == 0 ) {
          if(fabs(epsScrx) > 0.5 || fabs(epsScry) > 0.5) { // for very large strain at cracks
            opserr << "MCFTRCPlaneStress::determineTrialStress(vector) -- large strain at cracks" << endln;
            opserr << "TrialStrainVector: " << Tstrain(0) << "\t"<< Tstrain(1) << "\t"<< Tstrain(2) << endln;
            opserr << "fC1converged false for strain: epsScrx = " << epsScrx << "; epsScry = " << epsScry << endln; 
            status = 1;
          }
          // eq.i-20
          epsScrx = epsS1 + epsIncr * pow(cos(citan1), 2.0);
          epsScry = epsS2 + epsIncr * pow(cos(citan2), 2.0);
          
          //if( theMaterial[S_ONE]->setTrialStrain(epsScrx) != 0) {
    	  //  opserr << "MCFTRCPlaneStress::determineTrialStress(): fail to determine epsScrx" << endln;
    	  //  return -1;
    	  //}
    	  //fScrx = theMaterial[S_ONE]->getStress();
    	  //if( theMaterial[S_TWO]->setTrialStrain(epsScry) != 0) {
    	  //  opserr << "MCFTRCPlaneStress::determineTrialStress(): fail to determine epsScry" << endln;
    	  //  return -1;
    	  //}
    	  //fScry = theMaterial[S_TWO]->getStress();
          fScrx = determinefS(epsScrx, fyx, Es, 1.02*Es);
          fScry = determinefS(epsScry, fyy, Es, 1.02*Es);
          
          // eq.i-7
          temp = rhox * (fScrx - fS1) * pow(cos(citan1), 2.0) 
               + rhoy * (fScry - fS2) * pow(cos(citan2), 2.0);
          error = fC1 - temp;
          
          if (fabs(error) <= tolerance) {
            fC1converged = true;
          } else {
            // temp is function of citan1 citan2 and fSx and fSy,
            // the iterative parameter should include citan1/citan2 angle chosen. 
            // like: if (citan1 > PI/2 && citan2 > PI/2)...
            temp = rhox * theMaterial[S_ONE]->getSecant() * pow(cos(citan1), 2.0)
                 + rhoy * theMaterial[S_TWO]->getSecant() * pow(cos(citan2), 2.0);
            epsIncr += error/temp;
          }
        }
        // eq. i-8
        vci = rhox * (fScrx - fS1) * cos(citan1) * sin(citan1)
            + rhoy * (fScry - fS2) * cos(citan2) * sin(citan2);
        //vci = (vci < 0 ? 0 : vci);
        
        // Crack slips determine
        //while ( citaS > 0.5*PI ) {
        //	citaS -= 0.5*PI;
        //}
        
        // crack spacing // w   
        s_cita = 1.0/(sin(citaS)/xd+cos(citaS)/yd); // eq.i-21,22
        w      = epsC1 * s_cita;
        
        // optional 1
        vcimax = sqrt(-fpc)/(0.31+24*w/(aggr+16)); // shear parameter
        
        // if vci > vcimax , decrease fC1
        if (fabs(vci) > vcimax) {
        	fC1 -= (fabs(vci) - vcimax)*(tan(citaS)+1./tan(citaS));  // MCFT(1986) eq. 13
        }
        //fC1 = (fC1 > 0 ? fC1 : 0);
        //vci = vci * min(fabs(vcimax), fabs(vci)) / fabs(vci);
      }
    
      //////////////////////////////////////////////////////////////////////////
      // fC2
      //////////////////////////////////////////////////////////////////////////
      
      // C2 --compression for fc2, epsC2 incorporate steel offset strains
      betaD2 = calcBetaD(epsC1, epsC2);  // softened by the orthogonal direction tension
      theData(0) = eC2m;
      theData(1) = eT2m;
      //theData(2) = epsC2;
      theData(5) = CepsC12p(1);
      theData(6) = betaD2;
      theData(7) = 1.0; //K
      ret = 0;
      ret += C2SetVar.setVector(theData);  // Information for C compression, material [3], response[5]
      ret += theResponses[C2_SET_V]->getResponse(); // C2 setVar
      ret += theMaterial[C_TWO]->setTrialStrain(epsC2); //need further revision
      if (ret != 0) {
        opserr << "MCFTRCPlaneStress::determineTrialStress(), C2 setTrialStrain() Error" << endln;  // C2
        return ret;
      }
      fC2 = theMaterial[C_TWO]->getStress(); // w==5 ??
      
      theResponses[C2_GET_V]->getResponse(); // C2 getVar
      epsC12p(1) = (*C2GetVar.theVector)(5); // ecp2 
    
    } 
    
    else if (epsC1 < 0 && epsC2 > 0 ) {
    Par = 4;
      //////////////////////////////////////////////////////////////////////////
      // C1 -- compression 
      //////////////////////////////////////////////////////////////////////////
      
      // C1 --compression for fc1, epsC1 incorporate steel offset strains
      betaD1 = calcBetaD(epsC2, epsC1);  // softened by the orthogonal direction tension
      theData(0) = eC1m;
      theData(1) = eT1m;
      //theData(2) = epsC1;
      theData(5) = CepsC12p(0);
      theData(6) = betaD1;
      theData(7) = 1.0; // K
      ret = 0;
      ret += C1SetVar.setVector(theData);  // Information for C compression, material [2], response[4]
      ret += theResponses[C1_SET_V]->getResponse(); // C1 setVar
      ret += theMaterial[C_ONE]->setTrialStrain(epsC1); //need further revision
      if (ret != 0) {
        opserr << "MCFTRCPlaneStress::determineTrialStress(), C1 setTrialStrain() Error" << endln;  // C1
        return ret;
      }
      fC1 = theMaterial[C_ONE]->getStress(); // w==5 ?? 
      
      theResponses[C1_GET_V]->getResponse(); // C1 getVar
      epsC12p(0) = (*C2GetVar.theVector)(5); // ecp1
      
      //////////////////////////////////////////////////////////////////////////
      // fC2 -- tension
      //////////////////////////////////////////////////////////////////////////
      //  case 1: eq.i-33,34
      epsts = 2.0 * 7.5e-3 / fcr / 750.; //eq. i-33
      temp  = (epsC2 - epscr) / (epsts - epscr);
      
      if      (temp <= 0.0) fC2a = 0.0;
      else if (temp >  1.0) fC2a = fcr;
      else                  fC2a = fcr * (1 - temp);  //eq. i-34
      
      //  case 2: eq. i-35, 36
      temp = 4.0*(rhox/db1*fabs(cos(citan1))+rhoy/db2*fabs(cos(citan2)));
      fC2b = fcr/(1+sqrt(2.2/temp*epsC2));
      temp = max(fC2a,fC2b); // eq.i-38 /fC1
    
      // eq.i-5   // for positive value
      fC2c = fabs(rhox* (fyx - fS1) * pow(cos(citan1), 2.0)
           + rhoy* (fyy - fS2) * pow(cos(citan2), 2.0) );
      double fC2max = min(temp, fC2c);
      
      theData(0) = eC2m;
      theData(1) = eT2m;
      //theData(2) = epsC2;
      theData(5) = CepsC12p(1);
      theData(6) = 1.0; //betaD
      theData(7) = 1.0; //K
    
      ret = 0;
      ret += C2SetVar.setVector(theData);  // Information for C tension, material [3], response[5]
      ret += theResponses[C2_SET_V]->getResponse(); // C2 setVar 
      ret += theMaterial[C_TWO]->setTrialStrain(epsC2);
      if (ret != 0) {
        opserr << "MCFTRCPlaneStress::determineTrialStress(), C2 setTrialStrain() Error" << endln;  // C2
        return ret;
      }
      temp = theMaterial[C_TWO]->getStress();
      fC2 = min(fC2max, temp);
    
      //////////////////////////////////////////////////////////////////////////
      if(epsC2 > epscr) { //checkAtCrack();    	    // Calculate local stress at cracks
        status         = 0; // status to check if iteration satisfied eq.i-7
        fC2converged   = false;
        epsScrx        = epsS1;
        epsScry        = epsS2;
        epsIncr        = 0.0;
        
        while ( fC2converged == false && status == 0 ) {
          if(fabs(epsScrx) > 0.5 || fabs(epsScry) > 0.5) { // for very large strain at cracks
            opserr << "MCFTRCPlaneStress::determineTrialStress(vector) -- large strain at cracks" << endln;
            opserr << "TrialStrainVector: " << Tstrain(0) << "\t"<< Tstrain(1) << "\t"<< Tstrain(2) << endln;
            opserr << "fC1converged false for strain: epsScrx = " << epsScrx << "; epsScry = " << epsScry << endln; 
            status = 1;
          }
          // eq.i-20
          epsScrx = epsS1 + epsIncr * pow(cos(citan1), 2.0);
          epsScry = epsS2 + epsIncr * pow(cos(citan2), 2.0);
          
          //if( theMaterial[S_ONE]->setTrialStrain(epsScrx) != 0) {
          //  opserr << "MCFTRCPlaneStress::determineTrialStress(): fail to determine epsScrx" << endln;
          //  return -1;
          //}
          //fScrx = theMaterial[S_ONE]->getStress();
          //if( theMaterial[S_TWO]->setTrialStrain(epsScry) != 0) {
          //  opserr << "MCFTRCPlaneStress::determineTrialStress(): fail to determine epsScry" << endln;
          //  return -1;
          //}
          //fScry = theMaterial[S_TWO]->getStress();
          fScrx = determinefS(epsScrx, fyx, Es, 1.02*Es);
          fScry = determinefS(epsScry, fyy, Es, 1.02*Es);
          
          // eq.i-7
          temp = rhox * (fScrx - fS1) * pow(cos(citan1), 2.0) 
               + rhoy * (fScry - fS2) * pow(cos(citan2), 2.0);
          error = fC2 - temp;
          
          if (fabs(error) <= tolerance) {
            fC2converged = true;
          } else {
            // temp is function of citan1 citan2 and fSx and fSy,
            // the iterative parameter should include citan1/citan2 angle chosen. 
            // like: if (citan1 > PI/2 && citan2 > PI/2)...
            temp = rhox * theMaterial[S_ONE]->getSecant() * pow(cos(citan1), 2.0)
                 + rhoy * theMaterial[S_TWO]->getSecant() * pow(cos(citan2), 2.0);
            epsIncr += error/temp;
          }
        }
        // eq. i-8
        vci = rhox * (fScrx - fS1) * cos(citan1) * sin(citan1)
            + rhoy * (fScry - fS2) * cos(citan2) * sin(citan2);
        //vci = (vci < 0 ? 0 : vci);
        
        // Crack slips determine
        //while ( citaS > 0.5*PI ) {
        //	citaS -= 0.5*PI;
        //}
        
        // crack spacing // w   
        s_cita = 1.0/(sin(citaS)/xd+cos(citaS)/yd); // eq.i-21,22
        w      = epsC2 * s_cita;
        
        // optional 1
        vcimax = sqrt(-fpc)/(0.31+24*w/(aggr+16)); // shear parameter
        
        // if vci > vcimax , decrease fC2
        if (fabs(vci) > vcimax) {
          fC2 -= (fabs(vci) - vcimax)*(tan(citaS)+1./tan(citaS));  // MCFT(1986) eq. 13
        }
        //fC2 = (fC2 > 0 ? fC2 : 0);
        //vci = vci * min(fabs(vcimax), fabs(vci)) / fabs(vci);
      }
    } */
    
    //eC1p = 0.5*(epsCp_vec(0)+epsCp_vec(1)) 
    //	 + 0.5*sqrt(pow(epsCp_vec(0)-epsCp_vec(1), 2.0)+pow(epsCp_vec(2), 2.0));
    //eC2p = 0.5*(epsCp_vec(0)+epsCp_vec(1)) 
    //	 - 0.5*sqrt(pow(epsCp_vec(0)-epsCp_vec(1), 2.0)+pow(epsCp_vec(2), 2.0));
    tempStrain.addMatrixVector(0.0, T_cita, CepsCp_vec, 1.0);
    eC1p = tempStrain(0);
    eC2p = tempStrain(1);
    halfGammaOneTwo = tempStrain(2); // should be somve value need further check
    
    // deltaEspCp(2),  P. Ceresa 2009, Eq. 4
    //deltaEpsC1p = epsC12p(0) - CepsC12p(0);
    deltaEpsC1p = epsC12p(0) - eC1p;
    //deltaEpsC2p = epsC12p(1) - CepsC12p(1);
    deltaEpsC2p = epsC12p(1) - eC2p;
    
    epsCp_vec(0) = CepsCp_vec(0) + (0.5 * deltaEpsC1p * (1+cos(2.0*cita)) + 0.5 * deltaEpsC2p * (1-cos(2.0*cita)));
    epsCp_vec(1) = CepsCp_vec(1) + (0.5 * deltaEpsC1p * (1-cos(2.0*cita)) + 0.5 * deltaEpsC2p * (1+cos(2.0*cita)));
    epsCp_vec(2) = CepsCp_vec(2) + (deltaEpsC1p - deltaEpsC2p) * sin(2.0*cita);
    
    //need further revision for the initial crack direction, citaIC
    if (rhox > 0.0 && rhoy > 0.0) citaLag = 5./180.*PI;    // eq.i-41~43 
    else                          citaLag = 7.5/180.*PI;
    
    dCitaE = citaE - citaIC;
    dCitaS = (fabs(dCitaE) > citaLag ? dCitaE-citaLag : dCitaE);
    citaS  = citaIC + dCitaS;
    
    if (w <= 0.0) delta_S = 0.0;           //eq. i-40. 12 
    else          delta_S = vci/(1.8*pow(w,-0.8)+(0.234*pow(w,-0.707)-0.2)*(-fpc*1.25)); // fpc change for fcc = 1.25*fpc
    
    double gammaSa;
    if (s_cita == 0.0 ) {
      gammaSa = 0.0;
    } else {
      gammaSa = delta_S/s_cita;
    }
    
    double gammaSb = Tstrain(2)*cos(2.0*citaS) + (Tstrain(1)-Tstrain(0))*sin(2.0*citaS);  // eq. i-18
    gamma_S = max(gammaSa, gammaSb);
    
    epsSlip_vec(0) = -0.5*gamma_S*sin(2.0*citaS);   // eq.i-13~15 
    epsSlip_vec(1) =  0.5*gamma_S*sin(2.0*citaS);
    epsSlip_vec(2) =  gamma_S*cos(2.0*citaS);
    
    // eSlip1,2
    //eSlip1 = 0.5*(epsSlip_vec(0)+epsSlip_vec(1))
    //       + 0.5*sqrt(pow(epsSlip_vec(0)-epsSlip_vec(1), 2.0)+pow(epsSlip_vec(2), 2.0));
    //eSlip2 = 0.5*(epsSlip_vec(0)+epsSlip_vec(1))
    //       - 0.5*sqrt(pow(epsSlip_vec(0)-epsSlip_vec(1), 2.0)+pow(epsSlip_vec(2), 2.0));
    tempStrain.addMatrixVector(0.0, T_cita, epsSlip_vec, 1.0);
    eSlip1 = tempStrain(0);
    eSlip2 = tempStrain(1);
    halfGammaOneTwo = tempStrain(2); // should be some value need further check
    
    if (fabs(epsC1) < SMALL_STRAIN) {
      Ecx = theMaterial[C_ONE]->getTangent();
    } else {
      Ecx = fC1/(epsC1-epsC12p(0)); //-eSlip1
      if (Ecx > Ec) Ecx = Ec;
    }
    if (fabs(epsC2) < SMALL_STRAIN) {
      Ecy = theMaterial[C_TWO]->getTangent();
    } else {
      Ecy = fC2/(epsC2-epsC12p(1)); //-eSlip2
      if (Ecy > Ec) Ecy = Ec;
    }
    
    Dcp(0,0) = Ecx;
    Dcp(1,1) = Ecy;
    Dcp(2,2) = Ecx * Ecy / ( Ecx + Ecy );
    
    // EbarS1, EbarS2, eq.i-5
    if (fabs(epsS1) < SMALL_STRAIN) Ds1p(0,0) = rhox * (theMaterial[S_ONE]->getTangent()); //getSecant());
    else              Ds1p(0,0) = rhox * fS1 / epsS1;
    
    if (fabs(epsS2) < SMALL_STRAIN) Ds2p(0,0) = rhoy * (theMaterial[S_TWO]->getTangent()); //getSecant());
    else              Ds2p(0,0) = rhoy * fS2 / epsS2;
    
    Dc.addMatrixTripleProduct( 0.0, Tc,  Dcp,  1.0);
    Ds1.addMatrixTripleProduct(0.0, Ts1, Ds1p, 1.0);
    Ds2.addMatrixTripleProduct(0.0, Ts2, Ds2p, 1.0);
    
    secant_matrix = Dc + Ds1 + Ds2;
    
    secant_matrix(0,2) = 0.5 * secant_matrix(0,2);
    secant_matrix(1,2) = 0.5 * secant_matrix(1,2);
    secant_matrix(2,2) = 0.5 * secant_matrix(2,2);
    
    // Calculate pseudo-prestress vector
    //stress0_vec.addMatrixVector(0.0, Dc, epsCp_vec, 1.0);
    //static Matrix DInv(3,3);  // D invert
    //tangent_matrix.Invert(DInv);
    //Vector tempVec(3);
    //tempVec.addMatrixVector(0.0, DInv, stress0_vec, 1.0);
    
    //epsCp_vec -= tempVec;
    errorVec = Tstrain - epsCe_vec - epsCp_vec - epsSlip_vec;
    
    errNorm = errorVec.pNorm(-1);
    // determine the norm of matrixnorm = tempMat.Norm();
    // errNorm = norm - tempNorm;
    
    if ( errNorm <= tolNorm) {
      vCiconverged = true;
    } else {
      errorVec /= (pow(2.0, iteration_counter/100));
      epsC_vec += errorVec;
      iteration_counter += 1;
      numberDivide = iteration_counter/ 100;
      if (numberDivide == 5) {
    	opserr << "errorNorm = " << errNorm << endln;
      } 
    }

  }
  // Calculate the tangent matrix.
  tangent_matrix = secant_matrix;

  Tstress(0) = fC1 * pow(cos(cita),2.0) + fC2 * pow(sin(cita),2.0) + rhox*fS1;
  Tstress(1) = fC1 * pow(sin(cita),2.0) + fC2 * pow(cos(cita),2.0) + rhoy*fS2;
  Tstress(2) = (fC1-fC2)*sin(2.0*cita)/2.0;

  opserr << "cita = " << cita << ";\t citaS = " << citaS << ";\t citaE = " << citaE <<endln;

  stress_vec = Tstress;

  return 0;
}

int
MCFTRCPlaneStress::kupferEnvelop(double Tstrain, double sig_p, double eps_p)
{
  //double sig1 = 0.0, sig2 = 0.0, sig_p1, sig_p2, eps_p1, eps_p2;
  //double norm = 1.0, K1, K2;
  //while (norm > SMALL_STRAIN) {
	//K1 = 1 - 0.92 * (sig2/fpc)-0.76*pow(sig2/fpc,2.0);
	//K2 = 1 - 0.92 * (sig1/fpc)-0.76*pow(sig1/fpc,2.0);
	//sig_p1 = K1 * fpc;
	//eps_p1 = K1 * epsc0;
	//sig_p2 = K2 * fpc;
	//eps_p2 = K2 * epsc0;
  //
	////////////////////////////////////////////////////////////////////////////
	//// C1
	////////////////////////////////////////////////////////////////////////////
	//theData(0) = eC1m;  // 
	//theData(1) = eT1m;  // 
	////theData(2) = epsC1;  // 
	//theData(5) = CepsC12p(0);
	//theData(6) = 1.0; //betaD
    //theData(7) = 1.0;
  //
	//int ret = 0;
	//ret += C1SetVar.setVector(theData);  // Information for C tension, material [2], response[4]
	//ret += theResponses[4]->getResponse(); // C1 setVar 
	//ret += theMaterial[C_ONE]->setTrialStrain(epsC1);
	//if (ret != 0) {
	//	opserr << "MCFTRCPlaneStress::determineTrialStress(), C1 setTrialStrain() Error" << endln;  // C1
	//	return ret;
	//	}
	//fC1 = theMaterial[C_ONE]->getStress();
  //
	////////////////////////////////////////////////////////////////////////////
	//// C2
	////////////////////////////////////////////////////////////////////////////
	//theData(0) = eC2m;  // 
	//theData(1) = eT2m;  // 
	////theData(2) = epsC2;  // 
	//theData(5) = CepsC12p(1);
	//theData(6) = 1.0;
    //theData(7) = 1.0;
  //
	//ret = 0;
	//ret += C2SetVar.setVector(theData);  // Information for C compression, material [3], response[5]
	//ret += theResponses[5]->getResponse(); // C2 setVar
	//ret += theMaterial[C_TWO]->setTrialStrain(epsC2); //need further revision
	//if (ret != 0) {
	//	opserr << "MCFTRCPlaneStress::determineTrialStress(), C2 setTrialStrain() Error" << endln;  // C2
	//	return ret;
	//	}
	//fC2 = theMaterial[C_TWO]->getStress(); // w==5 ??
  //
	////////////////////////////////////////////////////////////////////////////
	//norm = sqrt(pow(fC1-sig1,2)+pow(fC2-sig2,2));
	//sig1 = fC1;
	//sig2 = fC2;
  //}
  //
  return 0;
}

int
MCFTRCPlaneStress::determineTangent(Vector strain)
{
  Vector Tstrain;
  Vector Tstress1(3), Tstress2(3);
  double dEps = 1.0e-9;

  for (int i=0; i<3; i++) {
	Tstrain = strain;

	Tstrain(i) = strain(i)+dEps;
	Tstress1 = determineTrialStress(Tstrain);
	
	Tstrain(i) = strain(i)-dEps;
	Tstress2 = determineTrialStress(Tstrain);

	if (Tstrain.pNorm(-1) == 0.0) {
	  determineTrialStress(strain);
	} else {
	  for (int j=0; j<3; j++){
		tangent_matrix(i,j)=(Tstress1(j)-Tstress2(j))/(2*dEps);
	  }
	}
	
  }
  return 0;
}

double 
MCFTRCPlaneStress::determinefS(double strain, double fy, double E, double Esh)
{
  double epsy  = fy/E;
  double epssh = 0.008;
  double epsu  = 0.15;
  double fS;

  if ( strain <= 0.0 ) {
	if (strain <= -epssh ){
	  if (strain <= -epsu) fS = -0.00001 * fy; // not fracture
	  else                 fS = -fy+(strain+epssh)*Esh;
	}
	else {
	  if (strain <= -epsy) fS = -fy;
	  else                 fS = strain*E;
	}
  } else {
    if (strain >= epssh ){
	  if (strain >= epsu) fS = 0.00001 * fy; 
	  else                fS = fy+(strain-epssh)*Esh;
	}
	else {
	  if (strain >= epsy) fS = fy;
	  else                fS = strain*E;
	}
  }

  return fS;
}

double 
MCFTRCPlaneStress::calcBetaD(double epsC1, double epsC2)
{
    double Cd;
	double Cs = 0.55;
	double temp = epsC1/epsC2+0.28;
	if (temp <= 0.0) Cd = 0.35*pow(-temp, 0.8);
	else             Cd = 0.0;

	double betaD = 1.0/(1.0+Cs*Cd);
	if (betaD > 1.0) return 1.0;
    else             return betaD;
}

int
MCFTRCPlaneStress::checkAtCrack()
{
    opserr << "check tension stress at cracks" << endln;
    return 0;

}

Vector
MCFTRCPlaneStress::determineMCFTStress(double epsC1, double epsC2)
{
    // assuming epsC1 > 0, epsC2 < 0;
    Tstress.Zero();

	return Tstress;
}