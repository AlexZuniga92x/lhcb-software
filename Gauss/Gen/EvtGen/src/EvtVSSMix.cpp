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
// Module: EvtVSSMix.cc
//
// Description: Routine to decay vector-> scalar scalar
//
// Modification history:
//
//    RYD       November 24, 1996       Module created
//
//------------------------------------------------------------------------
// 
#include <stdlib.h>
#include "EvtGen/EvtParticle.hh"
#include "EvtGen/EvtGenKine.hh"
#include "EvtGen/EvtPDL.hh"
#include "EvtGen/EvtReport.hh"
#include "EvtGen/EvtVector4C.hh"
#include "EvtGen/EvtVSSMix.hh"
#include "EvtGen/EvtId.hh"
#include "EvtGen/EvtString.hh"
#include "EvtGen/EvtConst.hh"

EvtVSSMix::~EvtVSSMix() {}

void EvtVSSMix::getName(EvtString& model_name){

  model_name="VSS_MIX";     

}


EvtDecayBase* EvtVSSMix::clone(){

  return new EvtVSSMix;

}

void EvtVSSMix::init(){

  // check that there are 1 arguments
  checkNArg(1);
  checkNDaug(2);

  checkSpinParent(EvtSpinType::VECTOR);
    
  checkSpinDaughter(0,EvtSpinType::SCALAR);
  checkSpinDaughter(1,EvtSpinType::SCALAR);

}

void EvtVSSMix::initProbMax(){

  setProbMax(0.5);

}

void EvtVSSMix::decay( EvtParticle *p ){

  //added by Lange Jan4,2000
  static EvtId B0=EvtPDL::getId("B0");
  static EvtId B0B=EvtPDL::getId("anti-B0");

  p->initializePhaseSpace(getNDaug(),getDaugs());
  EvtParticle *s1,*s2;
  s1 = p->getDaug(0);
  s2 = p->getDaug(1);
  EvtVector4R s1mom = s1->getP4();

  double t1,t2,dm;

  s1->setLifetime();
  s2->setLifetime();

  t1=s1->getLifetime();
  t2=s2->getLifetime();

  //dm should probably be a parameter to this model.

  dm=getArg(0)/EvtConst::c;

  double mix_amp;

  EvtId d1,d2;

  d1=s1->getId();
  d2=s2->getId();

  if (d1==B0&&d2==B0B) mix_amp=cos(0.5*dm*(t1-t2));
  if (d1==B0B&&d2==B0) mix_amp=cos(0.5*dm*(t1-t2));
  if (d1==B0&&d2==B0) mix_amp=sin(0.5*dm*(t1-t2));
  if (d1==B0B&&d2==B0B) mix_amp=sin(0.5*dm*(t1-t2));

  double norm=1.0/s1mom.d3mag();

  vertex(0,norm*mix_amp*s1mom*(p->eps(0)));
  vertex(1,norm*mix_amp*s1mom*(p->eps(1)));
  vertex(2,norm*mix_amp*s1mom*(p->eps(2)));

  return ;
}

