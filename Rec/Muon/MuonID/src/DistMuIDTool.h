// $Id: DistMuIDTool.h,v 1.1 2009-07-01 18:27:11 polye Exp $
#ifndef DISTMUIDTOOL_H 
#define DISTMUIDTOOL_H 1

// Include files
// from Gaudi
#include "GaudiAlg/GaudiTool.h"
#include "ImuIDTool.h"            // Interface
#include "Chi2MuIDTool.h"


/** @class DistMuIDTool DistMuIDTool.h
 *  
 *
 *  @author Jose Angel Hernando Morata
 *  @author Xabier Cid Vidal
 *  @date   2009-03-12
 */
class DistMuIDTool : public Chi2MuIDTool {
public: 
  /// Standard constructor
  DistMuIDTool( const std::string& type, 
                const std::string& name,
                const IInterface* parent);

  virtual ~DistMuIDTool( ); ///< Destructor

  StatusCode muonQuality(LHCb::Track& muTrack, double& Quality);

protected:
  StatusCode computeDistance(const LHCb::Track& muTrack,double& dist);

private:

};
#endif // DISTMUIDTOOL_H
