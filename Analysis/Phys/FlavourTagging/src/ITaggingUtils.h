#ifndef ITAGGINGUTILS_H 
#define ITAGGINGUTILS_H 1

#include <string>
#include "GaudiKernel/ToolFactory.h"
#include "GaudiKernel/IAlgTool.h"
#include "Event/Particle.h"
#include "Event/RecVertex.h"

static const InterfaceID IID_ITaggingUtils("ITaggingUtils", 1 , 0); 

/** @class ITaggingUtils ITaggingUtils.h 
 *  
 *  v1.0
 *  @author Marco Musy (Milano)
 *  @date   2007-02-07
 */

class ITaggingUtils : virtual public IAlgTool {

public:
  
  /// Retrieve interface ID
  static const InterfaceID& interfaceID() { return IID_ITaggingUtils; };
 
  virtual StatusCode calcIP( const LHCb::Particle* , 
                             const LHCb::Vertex* , double&, double&)=0;
  
  virtual StatusCode calcIP( const LHCb::Particle* , 
                             const LHCb::RecVertex* , double&, double&)=0;

  virtual StatusCode calcIP( const LHCb::Particle* ,
                             const LHCb::RecVertex::ConstVector& ,
                             double& , double& )=0;
  virtual int countTracks( LHCb::Particle::ConstVector& )=0;
  virtual bool isinTree( const LHCb::Particle*, 
                         std::vector<const LHCb::Particle*>& , double& )=0;
 
};
#endif // ICOMBINETAGGERSTOOL_H
