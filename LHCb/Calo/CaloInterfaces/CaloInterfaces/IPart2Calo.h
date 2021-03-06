#ifndef IPART2CALO_H 
#define IPART2CALO_H 1

// Include files
// from STL
#include <string>

// from CaloInterfaces
#include "CaloInterfaces/ITrack2Calo.h"

namespace LHCb 
{
  class Particle;
  class ProtoParticle;
}


static const InterfaceID IID_IPart2Calo ( "IPart2Calo", 1, 0 );

/** @class IPart2Calo IPart2Calo.h Kernel/IPart2Calo.h
 *  
 *
 *  @author Olivier Deschamps
 *  @date   2007-06-25
 */
class IPart2Calo : virtual public ITrack2Calo {
public: 

  // Return the interface ID
  static const InterfaceID& interfaceID() { return IID_IPart2Calo; }

  /// virtual destructor
  virtual ~IPart2Calo() {}  

  virtual StatusCode         initialize() = 0;

  using ITrack2Calo::match;
  virtual bool  match(const  LHCb::Particle* part,
                      std::string det = DeCalorimeterLocation::Ecal,
                      CaloPlane::Plane plane = CaloPlane::ShowerMax, 
                      double delta = 0.
                      )= 0;
  virtual bool  match(const  LHCb::ProtoParticle* proto,
                      std::string det = DeCalorimeterLocation::Ecal,
                      CaloPlane::Plane plane = CaloPlane::ShowerMax, 
                      double delta = 0.
                      )= 0;

  virtual bool inAcceptance()=0;  


protected:

private:

};
#endif // IPART2CALO_H
