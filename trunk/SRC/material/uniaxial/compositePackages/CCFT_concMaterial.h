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
// $Date: 2008-11-06 20:49:37 $
// $Source: /scratch/slocal/chroot/cvsroot/openseescomp/OpenSEESComp/OpenSees/SRC/material/uniaxial/CCFT_concMaterial.h,v $

// Written: MHS
// Created: Aug 2001

// Modified by : Cenk Tort - tort0008@umn.edu
// University of Minnesota - Civil Engineering Department
// Date : Wed Jul 23 17:32:23 EDT 2003
// Description: This file contains the class definition for
// cyclic uniaxial concrete stress-strain relationship for
// RCFT members.
// Further Modified by: Mark Denavit
// University of Illinois at Urbana-Champaign
// for circular CFT (CCFT) members


#ifndef CCFT_concMaterial_h
#define CCFT_concMaterial_h

#include <UniaxialMaterial.h>

class CCFT_concMaterial : public UniaxialMaterial
{
 public:
  CCFT_concMaterial(int tag, double fc, double depth, double thickness, double fy, double es);
  CCFT_concMaterial();    
  ~CCFT_concMaterial();
  
  int setTrialStrain(double strain, double strainRate = 0.0); 
  double getStrain(void);
  double getStress(void);
  double getTangent(void);
  double getInitialTangent(void);
  
  int commitState(void);
  int revertToLastCommit(void);    
  int revertToStart(void);        
  
  UniaxialMaterial *getCopy(void);
  
  int sendSelf(int commitTag, Channel &theChannel);  
  int recvSelf(int commitTag, Channel &theChannel, 
	       FEM_ObjectBroker &theBroker);    
  
  void Print(OPS_Stream &s, int flag =0);
  
 private:
  //Input Parameters
  double Fc; //user input variable: concrete compressive strength	 
  double D, t; //user input variable: outer diameter of the steel tube and thickness of the steel tube 
  double Fy, Es; //user input variable: yield strength of the steel and stiffness of the steel
  
  
  //Backbone Parameters
  double Ec; //initial stiffness of concrete    
  double ec_n; //strain at peak negative stress
  double ec_p; //strain at peak positive stress
  double Fc_n; //negative compressive strength
  double Fc_p; //positive compressive strength  
  double Ec_sft; //stiffness of the post peak softening branch (is a negative value)
  double Fc_res_n; //compressive residual strength
  double ec_res_n; //strain at the constant residual strength branch
  double r_n; //parameter that controls the nonlinear decending branch of the curve = 7
  double r_p; //parameter that controls the nonlinear decending branch of the curve = 7  
  //double ecr_n; //critical point on the negative envelope curve (post peak, so may not be necessary)
  double ecr_p; //critical point on the positive envelope curve
  double ecrk_p; //cracking strain
  double nn; //parameter in the determination of strain at peak negative stress
  double np; //parameter in the determination of strain at peak positive stress
  
  
  //State Variables
  bool isCracked, CisCracked; //true if cracked, false if not cracked
  int Trule, Crule; //current rule
  double Tstrain, Cstrain; //current strain
  double Tstress, Cstress; //current stress
  double Ttangent, Ctangent; //current tangent stiffness
  double eo, Ceo; //tensile curve offset
  double er1, er3, er6, er13, er14; // Strain at last reversal from rule # (where # is from er#)
  double Cer1, Cer3, Cer6, Cer13, Cer14; // Commited version of the strains at last reversal from ...
  double eb, Ceb; // strain at point b Pre-Cracking: (3-196) Post-Cracking: (3-199)
  double Esec_n, CEsec_n; //secent tangent of unloading (rule 3) (3-150)
  double Epl_n, CEpl_n; //final tangent of unloading (rule 3) (3-151)
  double delta_f_n, Cdelta_f_n; //change in stress from unloading point to joining point (3-152)
  double delta_e_n, Cdelta_e_n; //change in strain from unloading point to joining point (3-153)  
  double epl_n, Cepl_n; //strain at zero stress from uloading (3-154)
  double fnew_n, Cfnew_n; //stress at which the second branch of reloading curve begins (3-155)
  double Enew_n, CEnew_n; //tangent at the point at which the second branch of reloading curve begins (3-156)
  double ere_n, Cere_n; //strain at the point reloading joins the envelope  
  double fnew_str_n, Cfnew_str_n; //stress at which the second branch of reloading curve begins (3-155)
  double Enew_str_n, CEnew_str_n; //tangent at the point at which the second branch of reloading curve begins (3-156)
  double ere_str_n, Cere_str_n; //strain at the point reloading joins the envelope    
  double f71target, E71target, e72target; //the points Rule 7 actually uses (either new or new_str)
  double Cf71target, CE71target, Ce72target; //commited versions
  double f3target, Cf3target; // the inital stress that Rule 3 uses (either from Rule 1 or 2nd branch Rule 7)
  double f81target, E81target, e82target; //the points Rule 8 actually uses (either new or new_str)
  double Cf81target, CE81target, Ce82target; //commited versions
  double f4target, Cf4target; // the inital stress that Rule 4 uses (either from Rule 1 or 2nd branch Rule 8)
  double delta_e_p, Cdelta_e_p; //change in strain from unloading point to joining point (3-163)  
  double delta_f_p, Cdelta_f_p; //change in stress from unloading point to joining point (3-162)
  double fnew_p, Cfnew_p; //stress at which the second branch of reloading curve begins (3-165)
  double Enew_p, CEnew_p; //tangent at the point at which the second branch of reloading curve begins (3-166)
  double ere_p, Cere_p; //strain at the point reloading joins the envelope  
  double fnew_str_p, Cfnew_str_p; //stress at which the second branch of reloading curve begins (3-194)
  double Enew_str_p, CEnew_str_p; //tangent at the point at which the second branch of reloading curve begins (3-195)
  double ere_str_p, Cere_str_p; //strain at the point reloading joins the envelope (3-193)  
  double epl_p, Cepl_p; //strain at zero stress from uloading (3-164)
  double Epl_p, CEpl_p; //final tangent of unloading (rule 4) (3-161)
  double er2, er4, er9, er10, ea; 
  double Cer2, Cer4, Cer9, Cer10, Cea;
  double Esec_p, CEsec_p; //secent tangent of unloading (rule 4) (3-160)
  
  
  // Useful Function
  void transitionCurve(double Tstrain, double &Tstress, double &Ttangent, double ei, double fi, double Ei, double ef, double ff, double Ef);
  void positiveEnvelope(double strain, double &stress, double &tangent);
  void negativeEnvelope(double strain, double &stress, double &tangent);
  void tsaiEquation(double x, double r, double n, double &y, double &z);
  void commitStateVar(void);
  void backToCommitStateVar(void);
  
};
#endif



