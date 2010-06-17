#ifndef USER_TAGGINGUTILS_H 
#define USER_TAGGINGUTILS_H 1

// from Gaudi
#include "GaudiAlg/GaudiTool.h"
#include "GaudiKernel/ToolFactory.h"
// from Event

class IDistanceCalculator;
class IPVReFitter;
class DVAlgorithm;

#include "ITaggingUtils.h"

/** @class TaggingUtils TaggingUtils.h 
 *
 *  Tool to tag the B flavour with a Electron Tagger
 *
 *  @author Marco Musy
 *  @date   30/06/2005
 */

class TaggingUtils : public GaudiTool, 
	virtual public ITaggingUtils {

public: 
  /// Standard constructor
  TaggingUtils( const std::string& type,
		const std::string& name,
		const IInterface* parent );
  virtual ~TaggingUtils( ); ///< Destructor
  StatusCode initialize();    ///<  initialization
  StatusCode finalize  ();    ///<  finalization

  //-------------------------------------------------------------
  StatusCode calcIP( const LHCb::Particle* axp, 
                     const LHCb::VertexBase* v, 
                     double& ip, double& iperr) ;
  
  StatusCode calcIP( const LHCb::Particle*,
                     const LHCb::RecVertex::ConstVector& ,
                     double& , double& );

  StatusCode calcDOCAmin( const LHCb::Particle* axp,
			  const LHCb::Particle* p1,
			  const LHCb::Particle* p2,
			  double& doca, double& docaerr) ;

  int countTracks( LHCb::Particle::ConstVector& );
  bool isinTree( const LHCb::Particle*, 
                 std::vector<const LHCb::Particle*>& , double& );
  //-------------------------------------------------------------

private:

  std::string m_ChoosePV ;
  const IDistanceCalculator *m_Dist;
  DVAlgorithm* m_dva;
  const IPVReFitter* m_pvReFitter;

};

//===============================================================//
#endif // USER_TAGGINGUTILS_H
