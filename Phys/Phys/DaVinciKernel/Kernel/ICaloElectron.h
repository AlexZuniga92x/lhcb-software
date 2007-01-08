// $Id: ICaloElectron.h,v 1.4 2007-01-08 09:47:32 jpalac Exp $
#ifndef KERNEL_ICALOELECTRON_H 
#define KERNEL_ICALOELECTRON_H 1

// Include files
// from STL
#include <string>

// from Gaudi
#include "GaudiKernel/IAlgTool.h"

// from LHCb
#include "Event/CaloHypo.h"
#include "Event/Particle.h"
#include "Event/State.h"

#include "CaloUtils/CaloMomentum.h"
#include "CaloDet/DeCalorimeter.h"

static const InterfaceID IID_ICaloElectron ( "ICaloElectron", 1, 0 );

/** @class ICaloElectron ICaloElectron.h Kernel/ICaloElectron.h
 *  
 *
 *  @author Olivier Deschamps
 *  @date   2006-11-30
 */
class ICaloElectron : virtual public IAlgTool {
public: 

  // Return the interface ID
  static const InterfaceID& interfaceID() { return IID_ICaloElectron; }

  virtual StatusCode         initialize() = 0;
  virtual bool               setParticle(const  LHCb::Particle* particle) = 0;
  virtual double             eOverP()= 0 ;
  virtual double             e() = 0 ;
  virtual const LHCb::CaloHypo*    electron() = 0 ;
  virtual const LHCb::CaloMomentum bremstrahlung() = 0 ;
  virtual LHCb::State        closestState(std::string toWhat = "hypo") =0;
  virtual LHCb::State        caloState(CaloPlane::Plane plane = CaloPlane::ShowerMax , double deltaShower = 0. ) = 0;
  virtual double             showerZ(CaloPlane::Plane refPlane = CaloPlane::ShowerMax,std::string toWhat = "hypo") =0;
  virtual double             showerDepth(CaloPlane::Plane refPlane = CaloPlane::ShowerMax ,std::string toWhat = "hypo") =0 ;

protected:

private:

};
#endif // KERNEL_ICALOELECTRON_H
