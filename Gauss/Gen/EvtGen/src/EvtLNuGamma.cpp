//--------------------------------------------------------------------------
//
// Environment:
//      This software is part of the EvtGen package developed jointly
//      for the BaBar and CLEO collaborations.  If you use all or part
//      of it, please give an appropriate acknowledgement.
//
// Copyright Information: See EvtGen/COPYRIGHT
//      Copyright (C) 2001      Caltech
//
// Module: EvtLNuGamma.cc
//
// Description: B+ -> l+ nu gamma
//             
//
// Modification history:
//
//    Edward Chen   April 24, 2001           Module created
//
//------------------------------------------------------------------------
//
#include <stdlib.h>
#include <iostream.h>
#include "EvtGen/EvtString.hh"
#include "EvtGen/EvtParticle.hh"
#include "EvtGen/EvtPDL.hh"
#include "EvtGen/EvtGenKine.hh"
#include "EvtGen/EvtLNuGamma.hh"
#include "EvtGen/EvtDiracSpinor.hh"
#include "EvtGen/EvtReport.hh"
#include "EvtGen/EvtComplex.hh"
#include "EvtGen/EvtVector4C.hh"
#include "EvtGen/EvtVector4R.hh"

EvtLNuGamma::~EvtLNuGamma() {}

void EvtLNuGamma::getName(EvtString& model_name){

  model_name="LNUGAMMA";     

}


EvtDecayBase* EvtLNuGamma::clone(){

  return new EvtLNuGamma;

}

void EvtLNuGamma::init(){

  // check that there are 0 arguments
  checkNArg(3);
  checkNDaug(3);

  checkSpinParent(EvtSpinType::SCALAR);

  checkSpinDaughter(0,EvtSpinType::DIRAC);
  checkSpinDaughter(1,EvtSpinType::NEUTRINO);
  checkSpinDaughter(2,EvtSpinType::PHOTON);
}

void EvtLNuGamma::initProbMax(){

  setProbMax(7000.0);

}

void EvtLNuGamma::decay(EvtParticle *p){

  static EvtId BM=EvtPDL::getId("B-");
  p->initializePhaseSpace(getNDaug(),getDaugs());

  EvtComplex myI(0,1);

  EvtParticle *lept, *neut,*phot;
  lept=p->getDaug(0);
  neut=p->getDaug(1);
  phot=p->getDaug(2);

  EvtVector4C lept1,lept2,photon1,photon2;

  if (p->getId()==BM) {
    lept1=EvtLeptonVACurrent(lept->spParent(0),neut->spParentNeutrino());
    lept2=EvtLeptonVACurrent(lept->spParent(1),neut->spParentNeutrino());
  }
  else{
    lept1=EvtLeptonVACurrent(neut->spParentNeutrino(),lept->spParent(0));
    lept2=EvtLeptonVACurrent(neut->spParentNeutrino(),lept->spParent(1));
  }



  EvtVector4R photp = phot->getP4();  // Photon 4-momentum in parent rest frame
  double photE = photp.get(0);  // Photon energy in parent rest frame

  EvtVector4C photone1 = phot->epsParentPhoton(0).conj();
  EvtVector4C photone2 = phot->epsParentPhoton(1).conj();

  EvtVector4R parVelocity(1,0,0,0);  // Parent velocity in parent rest-frame

  double fv,fa;

  fv = getFormFactor(photE);
  fa = fv;

  EvtVector4C temp1a = dual(directProd(parVelocity,photp)).cont2(photone1);
  EvtVector4C temp2a = dual(directProd(parVelocity,photp)).cont2(photone2);

  EvtVector4C temp1b = (photone1)*(parVelocity*photp);
  EvtVector4C temp1c = (photp)*(photone1*parVelocity);

  EvtVector4C temp2b = (photone2)*(parVelocity*photp);
  EvtVector4C temp2c = (photp)*(photone2*parVelocity);

  photon1 = (temp1a*fv) + (myI*fa*(temp1b - temp1c));
  photon2 = (temp2a*fv) + (myI*fa*(temp2b - temp2c));

  vertex(0,0,lept1.cont(photon1));
  vertex(0,1,lept1.cont(photon2));
  vertex(1,0,lept2.cont(photon1));
  vertex(1,1,lept2.cont(photon2));
  
  return;

}

double EvtLNuGamma::getFormFactor(double photonEnergy){
  // Arg[0] = photon mass cutoff (GeV)
  // Arg[1] = R (GeV^(-1))
  // Arg[2] = m_b (GeV)
  // Using Korchemsky et al. Phy Rev D 61 (2000) 114510
  // Up to a constant
  
  double formFactor = 0;
  double qu = 2./3.;
  double qb = -1./3.;

  if (photonEnergy > getArg(0)){
    formFactor = (1/photonEnergy) * ((qu*getArg(1)) - (qb/getArg(2)));
  }
  return formFactor;
}
