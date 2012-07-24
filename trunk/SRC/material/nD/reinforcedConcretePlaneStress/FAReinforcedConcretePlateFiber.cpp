
//
// Description: This file contains the class definition for
// FAReinforcedConcretePlateFiber

#include "FAReinforcedConcretePlateFiber.h"
#include <UniaxialMaterial.h>
#include <Vector.h>
#include <Matrix.h>
#include <math.h>
#include <float.h>
#include <Channel.h>
#include <FEM_ObjectBroker.h>
#include <stdlib.h>

#include "SteelZ01.h"
#include "ConcreteZ01.h"
#include "ConcreteL01.h"

#include <Tensor.h>

//Vector FAReinforcedConcretePlateFiber :: strain_vec(3);
//Vector FAReinforcedConcretePlateFiber :: stress_vec(3);
//Matrix FAReinforcedConcretePlateFiber :: tangent_matrix(3,3);

#include <DummyStream.h>
#include <MaterialResponse.h>
#include <elementAPI.h>
#define OPS_Export 

static int numFAReinforcedConcretePlateFiberMaterials = 0;

OPS_Export void *
OPS_NewFAReinforcedConcretePlateFiberMaterial()
{
  if (numFAReinforcedConcretePlateFiberMaterials == 0) {
    numFAReinforcedConcretePlateFiberMaterials++;
    //OPS_Error("FAReinforcedConcretePlateFiber uniaxial material - Written by J.Zhong, Thomas T.C. Hsu and Y.L. Mo - Copyright@2009\n", 1);
  }

  // Pointer to a uniaxial material that will be returned
  NDMaterial *theMaterial = 0;

  int numRemainingArgs = OPS_GetNumRemainingInputArgs();

  if (numRemainingArgs < 14) {
    opserr << "Invalid Args want: NDMaterial FAReinforcedConcretePlateFiber matTag? rho? UniaxiaMatTag1? UniaxiaMatTag2? UniaxiaMatTag3? UniaxiaMatTag4? angle1? angle2? rou1? rou2? fpc? fy? E0? epsc0?>\n";
    return 0;	
  }

  int tag;
  double rho;
  int    iData[4];
  double dData[8];
  int numData = 0;

  numData = 1;
  if (OPS_GetInt(&numData, &tag) != 0) {
    opserr << "WARNING invalid tag nDMaterial FAReinforcedConcretePlateFiber tag" << endln;
    return 0;
  }

  numData = 1;
  if (OPS_GetDouble(&numData, &rho) != 0) {
    opserr << "Invalid Arg rho: uniaxialMaterial FAReinforcedConcretePlateFiber tag:" << tag << endln;
    return 0;	
  }

  numData = 4;
  if (OPS_GetInt(&numData, iData) != 0) {
    opserr << "WARNING invalid uniaxialMaterial FAReinforcedConcretePlateFiber tag: " << tag << endln;
    return 0;
  }

  numData = 8;
  if (OPS_GetDouble(&numData, dData) != 0) {
    opserr << "WARNING invalid data FAReinforcedConcretePlateFiber tag: " << tag << endln;
    return 0;
  }

  UniaxialMaterial *theUniaxialMaterial1 = OPS_GetUniaxialMaterial(iData[0]);
    
  if (theUniaxialMaterial1 == 0) {
    opserr << "WARNING material not found\n";
    opserr << "Material: " << iData[0];
    opserr << "\nFAReinforcedConcretePlateFiber tag: " << tag << endln;
    return 0;
  }
  
  UniaxialMaterial *theUniaxialMaterial2 = OPS_GetUniaxialMaterial(iData[1]);

  if (theUniaxialMaterial2 == 0) {
    opserr << "WARNING material not found\n";
    opserr << "Material: " << iData[1];
    opserr << "\nFAReinforcedConcretePlateFiber tag: " << tag << endln;
    return 0;
  }
  
  UniaxialMaterial *theUniaxialMaterial3 = OPS_GetUniaxialMaterial(iData[2]);
  if (theUniaxialMaterial3 == 0) {
    opserr << "WARNING material not found\n";
    opserr << "Material: " << iData[2];
    opserr << "\nFAReinforcedConcretePlateFiber tag: " << tag << endln;
    return 0;
  }

  UniaxialMaterial *theUniaxialMaterial4 = OPS_GetUniaxialMaterial(iData[3]);  
  if (theUniaxialMaterial4 == 0) {
    opserr << "WARNING material not found\n";
    opserr << "Material: " << iData[3];
    opserr << "\nFAReinforcedConcretePlateFiber tag: " << tag << endln;
    return 0;
  }

  //now create the FAReinforcedConcretePlateFiber
  theMaterial = new FAReinforcedConcretePlateFiber (tag, 
						     rho,
						     theUniaxialMaterial1, 
						     theUniaxialMaterial2, 
						     theUniaxialMaterial3, 
						     theUniaxialMaterial4,
						     dData[0],
						     dData[1],
						     dData[2],
						     dData[3],
						     dData[4],
						     dData[5],
						     dData[6],
						     dData[7]);
       
  if (theMaterial == 0) {
    opserr << "WARNING ran out of memory creating material\n";
    opserr << "FAReinforcedConcretePlateFiber: " << tag << endln;
    return 0;
  }

  return theMaterial;
}
 
FAReinforcedConcretePlateFiber ::FAReinforcedConcretePlateFiber (int      tag, 
								   double   RHO,
								   UniaxialMaterial *s1,
								   UniaxialMaterial *s2,
								   UniaxialMaterial *c1,
								   UniaxialMaterial *c2,
								   double   ANGLE1,
								   double   ANGLE2,
								   double   ROU1,
								   double   ROU2,
								   double   FPC,
								   double   FY,
								   double   E,
								   double   EPSC0) :
  NDMaterial(tag, ND_TAG_FAReinforcedConcretePlateFiber), 
  rho(RHO), angle1(ANGLE1), angle2(ANGLE2), rou1(ROU1), rou2(ROU2), citaR(0.0),
  fpc(FPC), fy(FY), E0(E), epsc0(EPSC0), strain_vec(5), stress_vec(5),tangent_matrix(5,5)
{
    steelStatus = 0;
    dirStatus = 0;
    G12 = 0;
    citaStrain = 0.0;
    citaStress = 0.0;
    
    TOneReverseStatus = 0;         
    TOneNowMaxComStrain = 0.0;
    TOneLastMaxComStrain = 0.0;
    
    TTwoReverseStatus = 0;         
    TTwoNowMaxComStrain = 0.0;
    TTwoLastMaxComStrain = 0.0;
    
    COneReverseStatus = 0;         
    COneNowMaxComStrain = 0.0;
    COneLastMaxComStrain = 0.0;
    
    CTwoReverseStatus = 0;         
    CTwoNowMaxComStrain = 0.0;
    CTwoLastMaxComStrain = 0.0;
    
    lastStress[0] = 0.0;  // add at 7.28
    lastStress[1] = 0.0;  // add at 7.28
    lastStress[2] = 0.0;  // add at 7.28 
	lastStress[3] = 0.0;  // add at 7.28
	lastStress[4] = 0.0;  // add at 7.28 

    if ( fpc < 0.0 ) { fpc = -fpc; } // set fpc > 0
    
    theMaterial = 0;
    
    // Allocate pointers to theSteel1
    theMaterial = new UniaxialMaterial *[4];
    
    if ( theMaterial == 0 ) {
      opserr << " FAReinforcedConcretePlateFiber::FAReinforcedConcretePlateFiber - failed allocate material array\n";
      exit(-1);
    }
    
    // Get the copy for theSteel1
    theMaterial[0] = s1->getCopy();
    // Check allocation    
    if ( theMaterial[0] == 0 ) {
      opserr << " FAReinforcedConcretePlateFiber::FAReinforcedConcretePlateFiber - failed to get a copy for steel1\n";
      exit(-1);
    }
    
    
    // Get the copy for theSteel2
    theMaterial[1] = s2->getCopy();	
    // Check allocation    
    if ( theMaterial[1] == 0 ) {
      opserr << " FAReinforcedConcretePlateFiber::FAReinforcedConcretePlateFiber - failed to get a copy for steel2\n";
      exit(-1);
    }
    
    // Get the copy for theConcrete1
    theMaterial[2] = c1->getCopy();	
    // Check allocation    
    if ( theMaterial[2] == 0 ) {
      opserr << " FAReinforcedConcretePlateFiber::FAReinforcedConcretePlateFiber - failed to get a copy for concrete1\n";
      exit(-1);
    }
    
    // Get the copy for theConcrete2
    theMaterial[3] = c2->getCopy();	
    // Check allocation    
    if ( theMaterial[3] == 0 ) {
      opserr << " FAReinforcedConcretePlateFiber::FAReinforcedConcretePlateFiber - failed to get a copy for concrete2\n";
      exit(-1);
    }
    
    /* FMK */
    theResponses = new Response *[6];  
    
    if ( theResponses == 0) {
      opserr << " FAReinforcedConcretePlateFiber::FAReinforcedConcretePlateFiber - failed allocate responses  array\n";
      exit(-1);
    }
    
    OPS_Stream *theDummyStream = new DummyStream();
    
    const char **argv = new const char *[1];

    argv[0] = "getCommittedStrain";
    theResponses[0] = theMaterial[0]->setResponse(argv, 1, *theDummyStream);
    theResponses[1] = theMaterial[1]->setResponse(argv, 1, *theDummyStream);
    argv[0] = "setWallVar";
    theResponses[2] = theMaterial[2]->setResponse(argv, 1, *theDummyStream);
    theResponses[3] = theMaterial[3]->setResponse(argv, 1, *theDummyStream);
    argv[0] = "getPD";
    theResponses[4] = theMaterial[2]->setResponse(argv, 1, *theDummyStream);
    theResponses[5] = theMaterial[3]->setResponse(argv, 1, *theDummyStream);    

    if ((theResponses[0] == 0) || (theResponses[1] == 0) ||
	(theResponses[2] == 0) || (theResponses[3] == 0) ||
	(theResponses[4] == 0) || (theResponses[5] == 0)) {

      opserr << " FAReinforcedConcretePlateFiber::FAReinforcedConcretePlateFiber - failed to set appropriate materials tag: " << tag << "\n";
      exit(-1);
    }
    
    delete theDummyStream;
    /* END FMK */
    determineTrialStress();
    this->revertToStart();
}

FAReinforcedConcretePlateFiber::FAReinforcedConcretePlateFiber()
 :NDMaterial(0, ND_TAG_FAReinforcedConcretePlateFiber), strain_vec(5),
  stress_vec(5),tangent_matrix(5,5),citaR(0.0)
{
  theMaterial = 0;
  theResponses = 0;
  determineTrialStress();
}


FAReinforcedConcretePlateFiber::~FAReinforcedConcretePlateFiber()
{
  // Delete the pointers
  if (theMaterial != 0) {
    for (int i=0; i<4; i++)
      {
	if (theMaterial[i])
	  delete theMaterial[i];
    }
    delete [] theMaterial;
  }
  if (theResponses != 0) {
    for (int j=0; j<6; j++)
      {
	if (theResponses[j] != 0)
	  delete theResponses[j];
    }
    delete [] theResponses;
  }
}
								  

double FAReinforcedConcretePlateFiber::getRho(void)
{
	return rho;
}


int FAReinforcedConcretePlateFiber::setTrialStrain(const Vector &v)
{
  
  // Set values for strain_vec
  strain_vec = v;
  
  
  // Set initial values for Tstress
  Tstress[0] = 0.0;
  Tstress[1] = 0.0;
  Tstress[2] = 0.0;
  Tstress[3] = 0.0;
  Tstress[4] = 0.0;


  TOneReverseStatus = COneReverseStatus;         
  TOneNowMaxComStrain = COneNowMaxComStrain;
  TOneLastMaxComStrain = COneLastMaxComStrain;
  
  TTwoReverseStatus = CTwoReverseStatus;         
  TTwoNowMaxComStrain = CTwoNowMaxComStrain;
  TTwoLastMaxComStrain = CTwoLastMaxComStrain;
  
  determineTrialStress();
  
  return 0;
}

int FAReinforcedConcretePlateFiber::setTrialStrain(const Vector &v, const Vector &r)
{
  opserr << "error: FAReinforcedConcretePlateFiber::setTrialStrainIncr(&v, &r) -- not really responsibility" << endln;
  return 0;
}


int FAReinforcedConcretePlateFiber::setTrialStrainIncr(const Vector &v)
{
  opserr << "error: FAReinforcedConcretePlateFiber::setTrialStrainIncr(&v) -- not really responsibility" << endln;
  return 0;
}


int FAReinforcedConcretePlateFiber::setTrialStrainIncr(const Vector &v, const Vector &r)
{
  opserr << "error: FAReinforcedConcretePlateFiber::setTrialStrainIncr(&v, &r) -- not really responsibility" << endln;
  return 0;
}

const Matrix& FAReinforcedConcretePlateFiber::getTangent(void)
{
  return tangent_matrix;
}

const Vector& FAReinforcedConcretePlateFiber::getStress(void)
{
  
  return stress_vec;
}


const Vector& FAReinforcedConcretePlateFiber :: getStrain()
{
  return strain_vec;
}
    

const Vector& FAReinforcedConcretePlateFiber::getCommittedStress(void)
{
  return stress_vec;
}


const Vector& FAReinforcedConcretePlateFiber::getCommittedStrain(void)
{
    return strain_vec;
}
    

int FAReinforcedConcretePlateFiber::commitState(void)
{
  for (int i=0; i<4; i++)
    {
      theMaterial[i]->commitState();
    }
  
  COneReverseStatus = TOneReverseStatus;         
  COneNowMaxComStrain = TOneNowMaxComStrain;
  COneLastMaxComStrain = TOneLastMaxComStrain;
  
  CTwoReverseStatus = TTwoReverseStatus;         
  CTwoNowMaxComStrain = TTwoNowMaxComStrain;
  CTwoLastMaxComStrain = TTwoLastMaxComStrain;
  
  lastStress[0] = stress_vec(0);
  lastStress[1] = stress_vec(1);
  lastStress[2] = stress_vec(2);
  lastStress[3] = stress_vec(3);
  lastStress[4] = stress_vec(4);
  
  return 0;
}


int FAReinforcedConcretePlateFiber::revertToLastCommit(void)
{

  for (int i=0; i<4; i++)
    {
      theMaterial[i]->revertToLastCommit();
    }
  
  TOneReverseStatus = COneReverseStatus;         
  TOneNowMaxComStrain = COneNowMaxComStrain;
  TOneLastMaxComStrain = COneLastMaxComStrain;
  
  TTwoReverseStatus = CTwoReverseStatus;         
  TTwoNowMaxComStrain = CTwoNowMaxComStrain;
  TTwoLastMaxComStrain = CTwoLastMaxComStrain;
  
  return 0;
}


int FAReinforcedConcretePlateFiber::revertToStart(void)
{
  int i;
  for (i=0; i<4; i++)
    {
      theMaterial[i]->revertToStart();
    }
  
  for (i=0; i<5; i++)
    {
      Tstress[i] = 0.0;
    }
  
  strain_vec.Zero();
  stress_vec.Zero();
  
  steelStatus = 0;
  dirStatus = 0;
  G12 = 0;
  
  TOneReverseStatus = 0;         
  TOneNowMaxComStrain = 0.0;
  TOneLastMaxComStrain = 0.0;
  
  TTwoReverseStatus = 0;         
  TTwoNowMaxComStrain = 0.0;
  TTwoLastMaxComStrain = 0.0;
  
  COneReverseStatus = 0;         
  COneNowMaxComStrain = 0.0;
  COneLastMaxComStrain = 0.0;
  
  CTwoReverseStatus = 0;         
  CTwoNowMaxComStrain = 0.0;
  CTwoLastMaxComStrain = 0.0;

  //add by lining for shell element
  determineTrialStress();

  return 0;
}

NDMaterial* FAReinforcedConcretePlateFiber::getCopy(void)
{
  /*
    FAReinforcedConcretePlateFiber *clone;
    clone = new FAReinforcedConcretePlateFiber();
    *clone = *this;
    return clone;
  //*/
  
  FAReinforcedConcretePlateFiber* theCopy =
    new FAReinforcedConcretePlateFiber( this->getTag(), 
					 rho, 
					 theMaterial[0], 
					 theMaterial[1], 
					 theMaterial[2], 
					 theMaterial[3], 
					 angle1, 
					 angle2, 
					 rou1, 
					 rou2, 
					 fpc, 
					 fy, 
					 E0, 
					 epsc0 );
  //theCopy->strain_vec = strain_vec;
  return theCopy;
}


NDMaterial* FAReinforcedConcretePlateFiber::getCopy(const char *type)
{
    /*
	FAReinforcedConcretePlateFiber *clone;
	clone = new FAReinforcedConcretePlateFiber();
	*clone = *this;
	return clone;
	//*/
	FAReinforcedConcretePlateFiber* theModel =
		new FAReinforcedConcretePlateFiber( this->getTag(), 
           rho, theMaterial[0], theMaterial[1], theMaterial[2], theMaterial[3], angle1, angle2, rou1, rou2, fpc, fy, E0, epsc0 );
	//theModel->strain_vec = strain_vec;
	//theModel->stress_vec = stress_vec;
	return theModel;
}


void FAReinforcedConcretePlateFiber::Print(OPS_Stream &s, int flag )
{
	s << "\n\tFAReinforcedConcretePlateFiber, material id: " << this->getTag() << endln;
    /*
	s << "\t Rho: " << rho << endln;
	s << "\t angle1: " << angle1 << endln;
	s << "\t angle2: " << angle2 << endln;
	s << "\t rou1: " << rou1 << endln;
	s << "\t rou2: " << rou2 << endln;
	s << "\t fpc: " << fpc << endln;
	s << "\t fy: " << fy << endln;
	s << "\t E0: " << E0 << endln;


	s << "\t Principal Strain: citaStrain = "<< citaStrain/M_PI*180.0 << endln;
	s << "\t Principal Stress: citaStress = "<< citaStress/M_PI*180.0 << endln;
	s << "\t v12 = " << miu12 << " v21 = " << miu21 << endln;	
	s << "\t steelStatus " << steelStatus << endln;
	s << "\t dirStatus " << dirStatus << endln;
	s << "\t Damage DOne = " << DDOne << endln;
	s << "\t Damage DTwo = " << DDTwo << endln;
	
	s << "\t G12 = " << G12 << endln;
	*/

	int i;
    
	s << "\t call the material print() function : "<< endln;
	
	s << "\t the steel 1 information is : " << endln;
	theMaterial[0]->Print(s,flag);
	s << "\t the steel 2 information is : " << endln;
	theMaterial[1]->Print(s,flag);
	s << "\t the concrete 1 information is : " << endln;
	theMaterial[2]->Print(s,flag);
	s << "\t the concrete 2 information is : " << endln;
	theMaterial[3]->Print(s,flag);


	s << "\t Strain and stress of the uniaxial materials:"<<endln;
	for ( i=0; i<4; i++)
	{
		s<< "\t Uniaxial Material "<<i+1<<" :"<<theMaterial[i]->getStrain()<<"   "<<theMaterial[i]->getStress()<< endln;
	}

}
 

int FAReinforcedConcretePlateFiber::sendSelf(int commitTag, Channel &theChannel)
{
  int res = 0;
  
  int dataTag = this->getDbTag();
  
  // Packs its data into a Vector and sends this to theChannel
  static Vector data(9);
  data(0) = this->getTag();
  data(1) = rho;
  data(2) = angle1;
  data(3) = angle2;
  data(4) = rou1;
  data(5) = rou2;
  data(6) = fpc;
  data(7) = fy;
  data(8) = E0;
  
  res += theChannel.sendVector(dataTag, commitTag, data);
  if (res < 0) {
    opserr << "WARNING FAReinforcedConcretePlateFiber::sendSelf() - " << this->getTag() << " failed to send Vector\n";
    return res;
  }	      
  
  
  // Now sends the IDs of its materials
  int matDbTag;
  
  static ID idData(8);
  
  // NOTE: to ensure that the material has a database
  // tag if sending to a database channel.
  
  int i;
  for (i=0; i<4; i++)
    {
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
    opserr << "WARNING FAReinforcedConcretePlateFiber::sendSelf() - " << this->getTag() << " failed to send ID\n";
    return res;
  }
  
  // Finally, quad asks its material objects to send themselves
  for (i = 0; i < 4; i++) {
    res += theMaterial[i]->sendSelf(commitTag, theChannel);
    if (res < 0) {
      opserr << "FAReinforcedConcretePlateFiber::sendSelf() - " << this->getTag() << " failed to send its Material\n";
      return res;
    }
  }	
  
  return res;
}
 

int FAReinforcedConcretePlateFiber::recvSelf(int commitTag, Channel &theChannel, FEM_ObjectBroker &theBroker)
{
  int res = 0;
  
  int dataTag = this->getDbTag();
  
  // Quad creates a Vector, receives the Vector and then sets the 
  // internal data with the data in the Vector
  static Vector data(9);
  res += theChannel.recvVector(dataTag, commitTag, data);
  if (res < 0) {
    opserr << "WARNING FAReinforcedConcretePlateFiber::recvSelf() - failed to receive Vector\n";
    return res;
  }
  
  this->setTag((int)data(0));
  rho = data(1);
  angle1 = data(2);
  angle2 = data(3);
  rou1 = data(4);
  rou2 = data(5);
  fpc = data(6);
  fy = data(7);
  E0 = data(8);

  static ID idData(8);
  
  // now receives the tags of its materials
  res += theChannel.recvID(dataTag, commitTag, idData);
  if (res < 0) {
    opserr << "WARNING FAReinforcedConcretePlateFiber::recvSelf() - " << this->getTag() << " failed to receive ID\n";
    return res;
  }


  if (theMaterial == 0) {
    // Allocate new materials
    theMaterial = new UniaxialMaterial *[4];
    if (theMaterial == 0) {
      opserr << "FAReinforcedConcretePlateFiber::recvSelf() - Could not allocate UniaxialMaterial* array\n";
      return -1;
    }
    for (int i = 0; i < 4; i++) {
      int matClassTag = idData(i);
      int matDbTag = idData(i+4);
      // Allocate new material with the sent class tag
      theMaterial[i] = theBroker.getNewUniaxialMaterial(matClassTag);
      if (theMaterial[i] == 0) {
	     opserr << "FAReinforcedConcretePlateFiber::recvSelf() - Broker could not create NDMaterial of class type " << matClassTag << endln;
	     return -1;
      }
      // Now receive materials into the newly allocated space
      theMaterial[i]->setDbTag(matDbTag);
      res += theMaterial[i]->recvSelf(commitTag, theChannel, theBroker);
      if (res < 0) {
         opserr << "FAReinforcedConcretePlateFiber::recvSelf() - material " << i << "failed to recv itself\n";
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
            opserr << "FAReinforcedConcretePlateFiber::recvSelf() - material " << i << "failed to create\n";
	        return -1;
		 }
	  }
      // Receive the material
      theMaterial[i]->setDbTag(matDbTag);
      res += theMaterial[i]->recvSelf(commitTag, theChannel, theBroker);
      if (res < 0) {
           opserr << "FAReinforcedConcretePlateFiber::recvSelf() - material " << i << "failed to recv itself\n";
	       return res;
	  }
    }
  }

  
  return res;

}



int FAReinforcedConcretePlateFiber::determineTrialStress(void)
{ 
  // Get Principal strain direction first
  
  double Tstrain[5];     //epslonx,epslony,0.5*gammaxy	
  // Get strain values from strain of element
  Tstrain[0] = strain_vec(0);
  Tstrain[1] = strain_vec(1);
  Tstrain[2] = 0.5*strain_vec(2);
  Tstrain[3] = strain_vec(3);
  Tstrain[4] = strain_vec(4);
  
  // Get citaR based on Tstrain
  double temp_citaR;
  
  if ( fabs(Tstrain[0]-Tstrain[1]) < 1e-7 )
  {
    citaR = 0.25*M_PI;	
  }
  else // Tstrain[0] != Tstrain[1]
  {
    temp_citaR = 0.5 * atan(fabs(2.0*1.0e6*Tstrain[2]/(1.0e6*Tstrain[0]-1.0e6*Tstrain[1]))); 
    if ( fabs(Tstrain[2]) < 1e-7 ) 
    {
     citaR = 0;
    } 
    else if ( (Tstrain[0] > Tstrain[1]) && ( Tstrain[2] > 0) )
    {			
     citaR = temp_citaR;
    }
    else if ( (Tstrain[0] > Tstrain[1]) && ( Tstrain[2] < 0) )
    {
     citaR = M_PI - temp_citaR;
    }
    else if ( (Tstrain[0] < Tstrain[1]) && ( Tstrain[2] > 0) )
    {
     citaR = 0.5*M_PI - temp_citaR;
    }
    else if ( (Tstrain[0] < Tstrain[1]) && ( Tstrain[2] < 0) )
    {
     citaR = 0.5*M_PI + temp_citaR;
    }
    else
    {
     opserr << "FAReinforcedConcretePlateFiber::determineTrialStress: Failure to calculate citaR\n";
     opserr << " Tstrain[0] = " << Tstrain[0] << endln;
     opserr << " Tstrain[1] = " << Tstrain[1] << endln;
     opserr << " Tstrain[2] = " << Tstrain[2] << endln;
	 opserr << " Tstrain[3] = " << Tstrain[3] << endln;
	 opserr << " Tstrain[4] = " << Tstrain[4] << endln;
    }
  }
  if ( citaR > 0.5 * M_PI ) {
	dirStatus = 1;  // assign value for output in the screen
  }

  while (  (citaR - 0.5*M_PI) > DBL_EPSILON ) {
	  citaR = citaR-0.5*M_PI;
	  dirStatus = 1;  // assign value for output in the screen
  }

  citaStrain = citaR; // assign value for output in the screen
  
  int status = 0; // status to check if iteration for principal stress direction
  double tolerance = 0.0088; // tolerance for iteration, equals to 0.5 degree
  int iteration_counter = 0;
  double error;
  
  error = getAngleError(citaR); // first try citaR
  if (error < tolerance)
    status = 1;
  
  double citaOne = citaR;
  double citaTwo = citaR;
  double minError = 100;
  double citaFinal = 100;
  
  while ( (status == 0) && ( citaOne>0 || citaTwo<0.5*M_PI ) )
  {
    citaOne = citaOne - M_PI/360.0;
    citaTwo = citaTwo + M_PI/360.0;
    
    if ( citaOne >0 )
    {
      error = getAngleError(citaOne);
      if ( minError > error )
      {
         minError = error;
         citaFinal = citaOne;
      }
      if ( error < tolerance )
      {
         status = 1;
         citaFinal = citaOne;
      }
    }
    
    if ( citaTwo < 0.5*M_PI )
    {
      error = getAngleError(citaTwo);
      if ( minError > error )
      {
        minError = error;
        citaFinal = citaTwo;
      }
      if ( error < tolerance )
      {
        status = 1;
        citaFinal = citaTwo;
      }
    }
    
    iteration_counter++;
  }
  
  
  if ( status == 0 ) // does not get converged after iteration
  {
    getAngleError(citaFinal);
    //if ( minError > 0.05 )
    //  opserr << "FAReinforcedConcretePlateFiber::determineTrialStress: Warning, failure to get converged principal stress direction\n";
  } 
  
  //citaStress = citaFinal;  // assign value for output in the screen
  
  return 0;
}



double FAReinforcedConcretePlateFiber::getAngleError(double inputCita)
{
	double outputCita;
	outputCita = getPrincipalStressAngle(inputCita);

	double error;
	double error1, error2, error3;
	error1 = fabs ( inputCita - outputCita);
	error2 = fabs ( inputCita - outputCita + 0.5*M_PI);
	error3 = fabs ( -inputCita + outputCita + 0.5*M_PI);

	if (error1 > error2)
		error = error2;
	else
		error = error1;

	if (error > error3)
		error = error3;

	return error;
}



double FAReinforcedConcretePlateFiber::getPrincipalStressAngle(double inputAngle)
{   
    double citaIn; // Trial principal stress direction, obtained from input
    citaIn = inputAngle;

	double citaOut; // outputAngle, obtained from stresses based on citaIn

	// Define i, j, k, for loop use
	int i, j, k;

    // Definition of the variables and matrix
	//Transformation matrix
	double TOne[3][3];     // T(citaOne)
	double TMOne[3][3];    // T(minus citaOne)
	double TMSL[3][3];     // T(minus citaSL)
	double TSL_One[3][3];  // T(citaSL minus citaOne)
	double TMST[3][3];     // T(minus citaST)
	double TST_One[3][3];  // T(citaST minus citaOne)

	double V[3][3];        //Matrix considering Hsu/Zhu ratios

	//Strain and stress
	double Tstrain[5];     //epslonx,epslony,0.5*gammaxy	
	double tempStrain[5]; //temp values of strains
	double stressSL, stressST; // stress of steel layers in L and T direction

	//stiffness of element
	double D[3][3];      //tangent stiffness matrix
	double DC[3][3];     //concrete part
	double DSL[3][3];    //steel part in L direction
	double DST[3][3];    //steel part in T direction
	double DC_bar[3][3]; //partial differentiation matrix of concrete part Eq.(49)	
	double tempD[3][3];  //temp matrix for data transfer

	double epsy = fy/E0;
	double fcr = 0.31*sqrt(fpc);
	double rou;
	if ( rou1 < rou2)
	{
		rou = rou1;
	}
	else
	{
		rou = rou2;
	}
	if ( rou < 0.0025 ) rou = 0.0025;
	double B = pow((fcr/fy),1.5)/rou;
	//double fn = fy * (0.91-2.0*B) / (0.98-0.25*B);

	double citaL = angle1; // angle for direction of steel one
	double citaT = angle2; // angle for direction of steel two


    //Set values for matrix TMSL, TMST
	TMSL[0][0] = pow(cos(citaL),2);
	TMSL[0][1] = pow(sin(citaL),2);
	TMSL[0][2] = -2.0*cos(citaL)*sin(citaL);
	TMSL[1][0] = pow(sin(citaL),2);
	TMSL[1][1] = pow(cos(citaL),2);
	TMSL[1][2] = 2.0*cos(citaL)*sin(citaL);
	TMSL[2][0] = cos(citaL)*sin(citaL);
	TMSL[2][1] = -cos(citaL)*sin(citaL);
	TMSL[2][2] = pow(cos(citaL),2)-pow(sin(citaL),2);

	TMST[0][0] = pow(cos(citaT),2);
	TMST[0][1] = pow(sin(citaT),2);
	TMST[0][2] = -2.0*cos(citaT)*sin(citaT);
	TMST[1][0] = pow(sin(citaT),2);
	TMST[1][1] = pow(cos(citaT),2);
	TMST[1][2] = 2.0*cos(citaT)*sin(citaT);
	TMST[2][0] = cos(citaT)*sin(citaT);
	TMST[2][1] = -cos(citaT)*sin(citaT);
	TMST[2][2] = pow(cos(citaT),2)-pow(sin(citaT),2);
    

    //Set values for transformation matrix TOne[3][3],TMOne[3][3],TSL_One[3][3],TST_One[3][3]
	TOne[0][0] = pow(cos(citaIn),2);
	TOne[0][1] = pow(sin(citaIn),2);
	TOne[0][2] = 2.0*cos(citaIn)*sin(citaIn);
	TOne[1][0] = pow(sin(citaIn),2);
	TOne[1][1] = pow(cos(citaIn),2);
	TOne[1][2] = -2.0*cos(citaIn)*sin(citaIn);
	TOne[2][0] = -cos(citaIn)*sin(citaIn);
	TOne[2][1] = cos(citaIn)*sin(citaIn);
	TOne[2][2] = pow(cos(citaIn),2)-pow(sin(citaIn),2);

    TMOne[0][0] = pow(cos(citaIn),2);
	TMOne[0][1] = pow(sin(citaIn),2);
	TMOne[0][2] = -2.0*cos(citaIn)*sin(citaIn);
	TMOne[1][0] = pow(sin(citaIn),2);
	TMOne[1][1] = pow(cos(citaIn),2);
	TMOne[1][2] = 2.0*cos(citaIn)*sin(citaIn);
	TMOne[2][0] = cos(citaIn)*sin(citaIn);
	TMOne[2][1] = -cos(citaIn)*sin(citaIn);
	TMOne[2][2] = pow(cos(citaIn),2)-pow(sin(citaIn),2);

    TSL_One[0][0] = pow(cos(citaL-citaIn),2);
	TSL_One[0][1] = pow(sin(citaL-citaIn),2);
	TSL_One[0][2] = 2.0*cos(citaL-citaIn)*sin(citaL-citaIn);
	TSL_One[1][0] = pow(sin(citaL-citaIn),2);
	TSL_One[1][1] = pow(cos(citaL-citaIn),2);
	TSL_One[1][2] = -2.0*cos(citaL-citaIn)*sin(citaL-citaIn);
	TSL_One[2][0] = -cos(citaL-citaIn)*sin(citaL-citaIn);
	TSL_One[2][1] = cos(citaL-citaIn)*sin(citaL-citaIn);
	TSL_One[2][2] = pow(cos(citaL-citaIn),2)-pow(sin(citaL-citaIn),2);

    TST_One[0][0] = pow(cos(citaT-citaIn),2);
	TST_One[0][1] = pow(sin(citaT-citaIn),2);
	TST_One[0][2] = 2.0*cos(citaT-citaIn)*sin(citaT-citaIn);
	TST_One[1][0] = pow(sin(citaT-citaIn),2);
	TST_One[1][1] = pow(cos(citaT-citaIn),2);
	TST_One[1][2] = -2.0*cos(citaT-citaIn)*sin(citaT-citaIn);
	TST_One[2][0] = -cos(citaT-citaIn)*sin(citaT-citaIn);
	TST_One[2][1] = cos(citaT-citaIn)*sin(citaT-citaIn);
	TST_One[2][2] = pow(cos(citaT-citaIn),2)-pow(sin(citaT-citaIn),2);


    // Get strain values from strain of element in x y directions
	Tstrain[0] = strain_vec(0);
	Tstrain[1] = strain_vec(1);
	Tstrain[2] = 0.5*strain_vec(2);
	Tstrain[3] = strain_vec(3);
	Tstrain[4] = strain_vec(4);
	
	//calculate tempStrain: epslon1,epslon2, 0.5*gamma12 in trial principal stress direction
    for (i=0;i<3;i++)
	{
		tempStrain[i] = 0.0;
		for (k=0;k<3;k++)
			tempStrain[i] += TOne[i][k]*Tstrain[k];	
	}

    double epslonOne, epslonTwo, halfGammaOneTwo;
	epslonOne = tempStrain[0];
	epslonTwo = tempStrain[1];
	halfGammaOneTwo = tempStrain[2];

    double epsn = epsy*(0.91-2.0*B)/(0.98-0.25*B); 

    /* FMK
       double CSL = theMaterial[0]->getCommittedStrain();
       double CST = theMaterial[1]->getCommittedStrain();
    */
    
    theResponses[0]->getResponse();
    theResponses[1]->getResponse();
    Information &theInfoL = theResponses[0]->getInformation();
    Information &theInfoT = theResponses[1]->getInformation();
    double CSL = theInfoL.theDouble;
    double CST = theInfoT.theDouble;
    
    /* END FMK */
    
    //double CSL = ((SteelZ01*)theMaterial[0])->getCommittedStrain();
    //double CST = ((SteelZ01*)theMaterial[1])->getCommittedStrain();
	
    if (( CSL > epsn ) || ( CST > epsn )) {
      steelStatus = 1;
    }
    


	
    //set v12 and v21 obtained from strain
    double strainSL, strainST; //Biaxial strain of steel in L, T
    double strainSF;           //larger one of strainSL, strainST
    
    strainSL= pow(cos(citaL),2)*Tstrain[0] + pow(sin(citaL),2)*Tstrain[1] + 2.0*sin(citaL)*cos(citaL)*Tstrain[2];
    strainST= pow(cos(citaT),2)*Tstrain[0] + pow(sin(citaT),2)*Tstrain[1] + 2.0*sin(citaT)*cos(citaT)*Tstrain[2];
    
    
	if (strainSL > strainST)
	{
		strainSF = strainSL;
	}
	else
	{
		strainSF = strainST;
	}
    
    double v12 = 0.2;
    double v21 = 0.2; //initial values for Hsu/Zhu ratios
    
    
	if ( (epslonOne > 0.0) && (epslonTwo > 0.0) )
	{
		v12 = 0.0;
		v21 = 0.0;
	}
	else if ( (epslonOne > 0.0) && (epslonTwo <= 0.0) )
	{
		v21 = 0.0;
		if (strainSF > 0.002 )
		{
			v12 = 1.0;
			//v12 = 1.9;
		}
		else if (strainSF < 0)
		{
			v12 = 0.2;
		}
		else
		{
			v12 = 0.2 + 400.0*strainSF;
			//v12 = 0.2 + 850.0*strainSF;
		}

		if (steelStatus == 1)
			v12 = 1.0;
		//v12 = 1.9;
	
	}
    else if ( (epslonOne <= 0.0) && (epslonTwo > 0.0) )
      {
	v12 = 0.0;
        if (strainSF > 0.002)
	  {
	    v21 = 1.0;
	    //v21 = 1.9;
	  }
	else if (strainSF < 0)
	  {
	    v21 = 0.2;
	  }
	else
	  {
	    v21 = 0.2 + 400.0*strainSF;
	    //v21 = 0.2 + 850.0*strainSF;
	  }
	
	if (steelStatus == 1)
	  v21 = 1.0;
	//v21 = 1.9;
      }
    else if ( (epslonOne <= 0.0) && (epslonTwo <= 0.0) )
      {
	if (strainSF > 0.002)
	  {
	    v21 = 0.95;
	    v12 = 0.95;
	    //v21 = 1.9;
	    //v12 = 1.9;
	  }
	else if (strainSF < 0)
	  {
	    v21 = 0.2;
	    v12 = 0.2;
	  }
	else
	  {
	    //v21 = 0.2 + 850.0*strainSF;
	    //v12 = 0.2 + 850.0*strainSF;
	    v21 = 0.2 + 375.0*strainSF;
	    v12 = 0.2 + 375.0*strainSF;
	  }
	if (steelStatus == 1)
	  {
	    //v21 = 1.9;
	    //v12 = 1.9;
	    v21 = 0.95;
	    v12 = 0.95;
	  }
      }
    else
      {
    	opserr << "FAReinforcedConcretePlateFiber::getPrincipalStressAngle: failure to get Hsu/Zhu ratio!\n";
      }
    
    
    miu12 = v12; // record the value for output in screen
    miu21 = v21; // record the value for output in screen
    
    
    //set values of matrix V[3][3]
    if ( v12*v21==1.0 )
      {
	opserr << "FAReinforcedConcretePlateFiber::getPrincipalStressAngle: failure to get matrix [V]!\n";
	opserr << "v12= "<<v12 << endln;
	opserr << "v21= "<<v21 << endln;
	V[0][0] = 1.0;
	V[0][1] = 0.0;
	V[0][2] = 0.0;
	V[1][0] = 0.0;
	V[1][1] = 1.0;
	V[1][2] = 0.0;
	V[2][0] = 0.0;
	V[2][1] = 0.0;
	V[2][2] = 1.0;
	
      }
    else
      {
	V[0][0] = 1.0/(1.0-v12*v21);
	V[0][1] = v12/(1.0-v12*v21);
	V[0][2] = 0.0;
	V[1][0] = v21/(1.0-v12*v21);
	V[1][1] = 1.0/(1.0-v12*v21);
	V[1][2] = 0.0;
	V[2][0] = 0.0;
	V[2][1] = 0.0;
	V[2][2] = 1.0;
      }
    
    
    
    //********** get [DC]**************
    //set temp[DC]=[V]*[TOne]
    for (i=0;i<3;i++)
      for (j=0;j<3;j++)
	{
	  tempD[i][j] = 0.0;
	  for (k=0;k<3;k++)
	    tempD[i][j] += V[i][k]*TOne[k][j];			
	}
    for (i=0;i<3;i++)
      for (j=0;j<3;j++)
	{
	  DC[i][j] = tempD[i][j];			
	}
    
    //calculate epslon1_bar, epslon2_bar, 0.5*gamma12
	double temp0 = tempStrain[0];
	double temp1 = tempStrain[1];
	tempStrain[0] = V[0][0]*temp0 + V[0][1]*temp1; //epslon1_bar
	tempStrain[1] = V[1][0]*temp0 + V[1][1]*temp1; //epslon2_bar
    
    
    //get stiffness of uniaxial strain of concrete in 12 direction
    double cigmaOneC; //stress of concrete in 12 direction
    double cigmaTwoC;
    double GOneTwoC; //shear modulus of concrete in 12 direction
    
    double ita = 1.0; // set ita to 1.0 for simplification
    
    
    // get Damage factor: DOne, DTwo
    if ( tempStrain[0] < 0 )
      {
	TOneReverseStatus = 0;
	if ( TOneNowMaxComStrain > tempStrain[0] )
	  TOneNowMaxComStrain = tempStrain[0];
      } 
    else // tempStrain[0] > 0
      {
	if ( TOneReverseStatus == 0 ) // first reverse from compressive strain
	  {
	    TOneReverseStatus = 1;
	    TOneLastMaxComStrain = COneNowMaxComStrain;
	    TOneNowMaxComStrain = 0.0;
	  }
      }
    
    if ( tempStrain[1] < 0 )
      {
	TTwoReverseStatus = 0;
	if ( TTwoNowMaxComStrain > tempStrain[1] )
	  TTwoNowMaxComStrain = tempStrain[1];
      } 
    else // tempStrain[1] > 0
      {
	if ( TTwoReverseStatus == 0 ) // first reverse from compressive strain
	  {
	    TTwoReverseStatus = 1;
	    TTwoLastMaxComStrain = CTwoNowMaxComStrain;
	    TTwoNowMaxComStrain = 0.0;
	  }
      }
    
    double DOne = 1.0;
    double DTwo = 1.0;
    if ( tempStrain[0] < 0 ) 
      {
	DOne = 1 - fabs(0.4*TTwoLastMaxComStrain/epsc0);
	if ( DOne < 0.2 )	DOne = 0.2;
      }
    if ( tempStrain[1] < 0 ) 
      {
	DTwo = 1 - fabs(0.4*TOneLastMaxComStrain/epsc0);
	if ( DTwo < 0.2 )	DTwo = 0.2;
      }
    
    DOne = 1.0;
    DTwo = 1.0;
    
    
    DDOne = DOne;  // assign values for screen output
    DDTwo = DTwo;
    
    //for xx, kk, m, xx=n, kk=delta, keci
    double xx,kk;
    if ( ((fabs(lastStress[0])+fabs(lastStress[0])+fabs(lastStress[0]))==0.0) || (fabs(lastStress[0])==0.0) ) {
      xx = 2.0;
      kk = 1.0;
    } else {
      if ( ( lastStress[0]<0.0 ) && ( lastStress[1]<0.0 ) )
	{
	  if ( lastStress[2] == 0.0 ) 
	    {
	      xx = 2.0;
	      kk = 1.0;
	      //kk = 0;
	    } else {
	    
	    
	    
	    double keci = sqrt( (fabs(lastStress[0])/fabs(lastStress[2])+1) * (fabs(lastStress[1])/fabs(lastStress[2])+1) );
	    
	    xx = 2/pow(keci,3.0);
	    if ( xx < 0.6 ) xx = 0.6;
	    
	    ///*
	    double a, b;
	    if ( keci < 1.5 ) {
	      a = 0;
	      b = 1.0;
	    } else {
	      a = ( 1.3 - 1.0 ) / (1.9 - 1.5);
	      b = 1.0 - a*1.5;
	    }
	    
	    kk = a*keci + b;
	    //*/
	    //kk = 0.105*keci+1;
	    
	    
	  }
	} 
      else if ( ( lastStress[0]>0.0 ) && ( lastStress[1]>0.0 ) )
	{
	  kk = 1.0;		
	  xx = 2.0;
	}
      else // under tension and compression
	{
	  kk = 1.0;
  	  
	  double keciN;
	  if ( lastStress[0]<0 ) {
	    keciN= sqrt(fabs(lastStress[0])/fabs(lastStress[2])+1);
	  } else {
            keciN= sqrt(fabs(lastStress[1])/fabs(lastStress[2])+1);
	  }
	  xx = 2/pow(keciN,3.0);
	  if ( xx < 0.6 ) xx = 0.6;
	  
	}
    }
    
    xx = 2.0; // for normal cases without axial loads
    kk = 1.0; 
    
    //for ConcreteZ01, ConcreteZ01
    //for ConcreteZ01, ConcreteZ01

    // FMK
    //theMaterial[2]->setTrialStrain(xx, kk, DOne,ita,tempStrain[1],tempStrain[0]); 
    //theMaterial[3]->setTrialStrain(xx, kk, DTwo,ita,tempStrain[0],tempStrain[1]); 

    Information &theInfoC02 = theResponses[2]->getInformation();
    Information &theInfoC03 = theResponses[3]->getInformation();
    
    static Vector theData(5);
    theData(0) = xx;
    theData(1) = kk;
    theData(2) = DOne;
    theData(3) = ita;
    theData(4) = tempStrain[1];
    theInfoC02.setVector(theData);
    theResponses[2]->getResponse();
    theData(2) = DTwo;
    theData(4) = tempStrain[0];
    theInfoC03.setVector(theData);
    theResponses[3]->getResponse();

    theMaterial[2]->setTrialStrain(tempStrain[0]); 
    theMaterial[3]->setTrialStrain(tempStrain[1]); 
    
    /* END FMK */


    cigmaOneC = theMaterial[2]->getStress();
    cigmaTwoC = theMaterial[3]->getStress();

    // set GOneTwoC = 1.0;
	if (epslonOne == epslonTwo)
	{
		GOneTwoC = 10000.0; // max value for GOneTwoC
	}
	else
	{
		// change at 03/19/04
		//GOneTwoC = fabs((cigmaOneC-cigmaTwoC)/(epslonOne-epslonTwo));
		// Neallee@tju.edu.cn
		GOneTwoC = fabs((cigmaOneC-cigmaTwoC)/(epslonOne-epslonTwo));
		if (GOneTwoC > 10000.0) // if larger than max value
		GOneTwoC = 10000.0;
	}
    
    G12 = GOneTwoC; // record the value for output in screen


    DC_bar[0][0] = theMaterial[2]->getTangent();
    
    // FMK    
    // DC_bar[0][1] = theMaterial[2]->getPD();
    
    theResponses[4]->getResponse();
    Information &theInfoC1 = theResponses[4]->getInformation();
    DC_bar[0][1] = theInfoC1.theDouble;
    // end 
    
    DC_bar[0][2] = 0.0;
    
    // FMK
    // DC_bar[1][0] = theMaterial[3]->getPD();

    theResponses[5]->getResponse();
    Information &theInfoC2 = theResponses[5]->getInformation();
    DC_bar[1][0] = theInfoC2.theDouble;
    // end FMK 
    
    //DC_bar[1][0] = 0.0;
    DC_bar[1][1] = theMaterial[3]->getTangent();
    DC_bar[1][2] = 0.0;
    
    DC_bar[2][0] = 0.0;
    DC_bar[2][1] = 0.0;	
    DC_bar[2][2] = GOneTwoC;
    
    //before [DC]=[v]*[TOne], now update [DC]=[Dc_bar]*[V]*[TOne]
	for (i=0;i<3;i++)
		for (j=0;j<3;j++)
		{
			tempD[i][j] = 0.0;
			for (k=0;k<3;k++)
				tempD[i][j] += DC_bar[i][k]*DC[k][j];
		}

	for (i=0;i<3;i++)
		for (j=0;j<3;j++)
		{
			DC[i][j] = tempD[i][j];			
		}

    //update [DC]=[TMOne]*[Dc_bar]*[V]*[TOne]
	  for (i=0;i<3;i++)
		  for (j=0;j<3;j++)
		  {
			  tempD[i][j] = 0.0;
			  for (k=0;k<3;k++)
				  tempD[i][j] += TMOne[i][k]*DC[k][j];
		  }

	  for (i=0;i<3;i++)
		  for (j=0;j<3;j++)
		  {
			  DC[i][j] = tempD[i][j];			
		  }
    
    
    //***************** get [DSL] ******************
    //get [DSL]=[V][TOne]
	for (i=0;i<3;i++)
		for (j=0;j<3;j++)
		{
			tempD[i][j] = 0.0;
			for (k=0;k<3;k++)
				tempD[i][j] += V[i][k]*TOne[k][j];
		}

	for (i=0;i<3;i++)
		for (j=0;j<3;j++)
		{
			DSL[i][j] = tempD[i][j];			
		}
    
    //get [DSL]=[TLMOne][V][TOne]
    for (i=0;i<3;i++)
      for (j=0;j<3;j++)
	{
	  tempD[i][j] = 0.0;
	  for (k=0;k<3;k++)
	    tempD[i][j] += TSL_One[i][k]*DSL[k][j];
	}   
    for (i=0;i<3;i++)
      for (j=0;j<3;j++)
		{
			DSL[i][j] = tempD[i][j];			
		}
    
	double strainSL_b; //uniaxial strain of steel in L direction
	strainSL_b = DSL[0][0]*Tstrain[0] + DSL[0][1]*Tstrain[1] + DSL[0][2]*Tstrain[2];
     

	//get stiffness
	double tangentSL;
	theMaterial[0]->setTrialStrain( strainSL_b );
	tangentSL = theMaterial[0]->getTangent();
	stressSL = theMaterial[0]->getStress();

	for (j=0;j<3;j++)
	{
		DSL[0][j] = rou1*tangentSL*DSL[0][j];
		DSL[1][j] = 0.0;
		DSL[2][j] = 0.0;
	}

    //get [DSL]=[TML][Dsl][TLMOne][V][TOne]
	for (i=0;i<3;i++)
		for (j=0;j<3;j++)
		{
			tempD[i][j] = 0.0;
			for (k=0;k<3;k++)
				tempD[i][j] += TMSL[i][k]*DSL[k][j];
		}   
	
	for (i=0;i<3;i++)
		for (j=0;j<3;j++)
		{
			DSL[i][j] = tempD[i][j];			
		}

    //**************** get [DST] ****************     
	//get [DST]=[V][TOne]
	for (i=0;i<3;i++)
		for (j=0;j<3;j++)
		{
			tempD[i][j] = 0.0;
			for (k=0;k<3;k++)
				tempD[i][j] += V[i][k]*TOne[k][j];
		}

	for (i=0;i<3;i++)
		for (j=0;j<3;j++)
		{
			DST[i][j] = tempD[i][j];			
		}

	//get [DST]=[TST_One][V][TOne]
	for (i=0;i<3;i++)
		for (j=0;j<3;j++)
		{
			tempD[i][j] = 0.0;
			for (k=0;k<3;k++)
				tempD[i][j] += TST_One[i][k]*DST[k][j];
			}   
	
	for (i=0;i<3;i++)
		for (j=0;j<3;j++)
		{
			DST[i][j] = tempD[i][j];
		}
    
	double strainST_b; //uniaxial strain of steel in L direction
	strainST_b = DST[0][0]*Tstrain[0] + DST[0][1]*Tstrain[1] + DST[0][2]*Tstrain[2];

	double tangentST;
	theMaterial[1]->setTrialStrain( strainST_b );
	tangentST = theMaterial[1]->getTangent();
	stressST = theMaterial[1]->getStress();

	for (j=0;j<3;j++)
	{
		DST[0][j] = rou2*tangentST*DST[0][j];
		DST[1][j] = 0.0;
		DST[2][j] = 0.0;
	}
     
	//get [DST]=[TMT][Dst][TTMOne][V][TOne]
	for (i=0;i<3;i++)
		for (j=0;j<3;j++)
		{
			tempD[i][j] = 0.0;
			for (k=0;k<3;k++)
				tempD[i][j] += TMST[i][k]*DST[k][j];
		}
     
	for (i=0;i<3;i++)
		for (j=0;j<3;j++)
		{
			DST[i][j] = tempD[i][j];			
		}
  
     
    //****************** get tangent_matrix  ****************    
	// Get tangent_matrix
	tangent_matrix.Zero();
	for (i=0;i<3;i++)
	{
		for (j=0;j<3;j++)
		{
			D[i][j] = 0.0;
			D[i][j] = DC[i][j] + DSL[i][j] + DST[i][j];
			tangent_matrix(i,j) = D[i][j];
		}
	}
    tangent_matrix(0,2) = 0.5*D[0][2];
	tangent_matrix(2,0) = 0.5*D[2][0];
	tangent_matrix(1,2) = 0.5*D[1][2];
	tangent_matrix(2,1) = 0.5*D[2][1];
	tangent_matrix(2,2) = 0.5*D[2][2];
    
	//added by neallee@tju.edu.cn for membrane out of plane stress
	tangent_matrix(3,3) = 0.5*D[2][2];
	tangent_matrix(4,4) = 0.5*D[2][2];
	//end by neallee@tju.edu.cn

    //**************** get Tstress and stress_vec ****************
    Tstress[0] = pow(cos(citaIn),2)*cigmaOneC + pow(sin(citaIn),2)*cigmaTwoC
		         - 2*sin(citaIn)*cos(citaIn)*halfGammaOneTwo*GOneTwoC
			     + pow(cos(citaL),2)*rou1*stressSL + pow(cos(citaT),2)*rou2*stressST;

	Tstress[1] = pow(sin(citaIn),2)*cigmaOneC + pow(cos(citaIn),2)*cigmaTwoC
		         + 2*sin(citaIn)*cos(citaIn)*halfGammaOneTwo*GOneTwoC
			     + pow(sin(citaL),2)*rou1*stressSL + pow(sin(citaT),2)*rou2*stressST;

	Tstress[2] = cos(citaIn)*sin(citaIn)*cigmaOneC - cos(citaIn)*sin(citaIn)*cigmaTwoC
		         + (pow(cos(citaIn),2)-pow(sin(citaIn),2))*halfGammaOneTwo*GOneTwoC
			     + cos(citaL)*sin(citaL)*rou1*stressSL + cos(citaT)*sin(citaT)*rou2*stressST;

    stress_vec(0) = Tstress[0];
	stress_vec(1) = Tstress[1];
	stress_vec(2) = Tstress[2];

	//added by neallee@tju.edu.cn
    stress_vec(3) = tangent_matrix(3,3)*Tstrain[3];
	stress_vec(4) = tangent_matrix(4,4)*Tstrain[4];
	//end by neallee@tju.edu.cn

	// get calculated principal stress direction citaOut
	double temp_citaOut;

    if ( fabs(Tstress[0]-Tstress[1]) < DBL_EPSILON )
	{
	   citaOut = 0.25*M_PI;	
	}
	else // Tstress[0] != Tstress[1]
	{
	  temp_citaOut = 0.5 * atan(fabs(2.0*1.0e6*Tstress[2]/(1.0e6*Tstress[0]-1.0e6*Tstress[1]))); 
	  if ( fabs(Tstress[2]) < 1e-7 ) 
	  {
	  	citaOut = 0;
	  }
	  else if ( (Tstress[0] > Tstress[1]) && ( Tstress[2] > 0) )
	  {			
	  	citaOut = temp_citaOut;
	  }
	  else if ( (Tstress[0] > Tstress[1]) && ( Tstress[2] < 0) )
	  {
	  	citaOut = M_PI - temp_citaOut;
	  }
	  else if ( (Tstress[0] < Tstress[1]) && ( Tstress[2] > 0) )
	  {
	  	citaOut = 0.5*M_PI - temp_citaOut;
	  }
	  else if ( (Tstress[0] < Tstress[1]) && ( Tstress[2] < 0) )
	  {
	  	citaOut = 0.5*M_PI + temp_citaOut;
	  }
	  else
	  {
	  		opserr << "FAReinforcedConcretePlateFiber::getPrincipalStressAngle: Failure to calculate principal stress direction\n";
	  		opserr << " Tstress[0] = " << Tstress[0] << endln;
	  		opserr << " Tstress[1] = " << Tstress[1] << endln;
	  		opserr << " Tstress[2] = " << Tstress[2] << endln;
	  }
	}

	while (  (citaOut - 0.5*M_PI) > DBL_EPSILON ) {
		citaOut = citaOut-0.5*M_PI;		
	}

	citaStress = citaOut; // assign value for screen output 

	return citaOut;
}
