//--------------------------------------------------------------------------
//
// Environment:
//      This software is part of the EvtGen package developed jointly
//      for the BaBar and CLEO collaborations.  If you use all or part
//      of it, please give an appropriate acknowledgement.
//
// Copyright Information: See EvtGen/COPYRIGHT
//      Copyright (C) 1998      Caltech, UCSB
//
// Module: EvtSSSCPT.cc
//
// Description: Routine to decay scalar -> 2 scalars (CPT)
//
// Modification history:
//
//    SHY       April 28, 1997       Module created
//
//------------------------------------------------------------------------
// 
#include <stdlib.h>
#include "EvtGen/EvtParticle.hh"
#include "EvtGen/EvtGenKine.hh"
#include "EvtGen/EvtCPUtil.hh"
#include "EvtGen/EvtPDL.hh"
#include "EvtGen/EvtReport.hh"
#include "EvtGen/EvtSSSCPT.hh"
#include "EvtGen/EvtId.hh"
#include "EvtGen/EvtString.hh"
#include "EvtGen/EvtConst.hh"

EvtSSSCPT::~EvtSSSCPT() {}

void EvtSSSCPT::getName(EvtString& model_name){

  model_name="SSS_CPT";     

}


EvtDecayBase* EvtSSSCPT::clone(){

  return new EvtSSSCPT;

}

void EvtSSSCPT::init(){

  // check that there are 8 arguments
  checkNArg(8);
  checkNDaug(2);

}


void EvtSSSCPT::decay( EvtParticle *p){

  //added by Lange Jan4,2000
  static EvtId B0=EvtPDL::getId("B0");
  static EvtId B0B=EvtPDL::getId("anti-B0");


  double t;
  EvtId other_b;

  EvtCPUtil::OtherB(p,t,other_b);

  p->initializePhaseSpace(getNDaug(),getDaugs());

  EvtComplex amp;

  EvtComplex A,Abar;
  EvtComplex P,Q,D,Im;

  P=EvtComplex(cos(-getArg(0)),sin(-getArg(0)));
  Q=EvtComplex(cos(getArg(0)),sin(getArg(0)));
  D=EvtComplex(getArg(6)*cos(getArg(7)),getArg(6)*sin(getArg(7)));
  Im=EvtComplex(0.0,1.0);
  
  A=EvtComplex(getArg(2)*cos(getArg(3)),getArg(2)*sin(getArg(3)));
  Abar=EvtComplex(getArg(4)*cos(getArg(5)),getArg(4)*sin(getArg(5)));
  
  if (other_b==B0B){
    amp=A*cos(getArg(1)*t/(2*EvtConst::c))+
      Im*sin(getArg(1)*t/(2*EvtConst::c))*
      (Q/P*A + 2.0*D*Abar);
  }
  if (other_b==B0){
    amp=Abar*cos(getArg(1)*t/(2*EvtConst::c))+
      Im*sin(getArg(1)*t/(2*EvtConst::c))*
      (P/Q*A - 2.0*D*Abar);
  }
  
  vertex(amp);
  
  return ;
}

