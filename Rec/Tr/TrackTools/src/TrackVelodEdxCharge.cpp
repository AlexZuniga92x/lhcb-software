
//-----------------------------------------------------------------------------
/** @file TrackVelodEdxCharge.cpp
 *
 * Implementation file for tool TrackVelodEdxCharge
 *
 * @author Chris Jones   Christopher.Rob.Jones@cern.ch
 * @date 18/07/2006
 */
//-----------------------------------------------------------------------------

#include "GaudiKernel/ToolFactory.h"
#include "TrackVelodEdxCharge.h"
#include "Event/VeloCluster.h"
#include "Kernel/SiChargeFun.h"

// namespaces
using namespace LHCb;

// Declaration of the Tool Factory
DECLARE_TOOL_FACTORY( TrackVelodEdxCharge )

//-----------------------------------------------------------------------------

TrackVelodEdxCharge::TrackVelodEdxCharge(const std::string& type,
                                         const std::string& name,
                                         const IInterface* parent) :
  GaudiTool ( type, name, parent )
{
  // tool interface
  declareInterface<ITrackVelodEdxCharge>(this);
  // job options
  declareProperty( "Normalisation", m_Normalisation = 47.1 );
  declareProperty( "Ratio",         m_Ratio         = 0.6  );
  declareProperty( "UseConditions", m_useConditions  = true );
}

TrackVelodEdxCharge::~TrackVelodEdxCharge(){
}

StatusCode TrackVelodEdxCharge::initialize()
{
  StatusCode sc = GaudiTool::initialize();
  if ( sc.isFailure() ) return sc;

  if( !existDet<DataObject>(detSvc(),"Conditions/ParticleID/Velo/VelodEdx") ){
    Warning("VELO dE/dx parameters not in conditions DB", 
            StatusCode::SUCCESS).ignore();
    m_useConditions = false;
  }
  if( !m_useConditions ){
    Warning("Using VELO dE/dx parameters from options not conditions", 
            StatusCode::SUCCESS).ignore();
  }else{
    registerCondition("Conditions/ParticleID/Velo/VelodEdx",m_dEdx,
                      &TrackVelodEdxCharge::i_cachedEdx);
    sc = runUpdate();
    if(!sc) return sc;
  }

  info() << "VELO dE/dx charge parameters :"
         << " normalisation = " << m_Normalisation
         << " ratio = " << m_Ratio <<endmsg;

  return StatusCode::SUCCESS;
}

StatusCode TrackVelodEdxCharge::nTracks( const LHCb::Track * track,
                                         double & nTks ) const
{
  // initialise the charge value
  nTks = 0;
  // is track OK
  if ( !track ) return Warning( "Null track pointer passed" );

  if ( msgLevel(MSG::DEBUG) )
    debug() << "Trying Track " << track->key() << endmsg;

  // get the measurements for the track
  typedef LHCb::Track::MeasurementContainer TkMeas ;
  const TkMeas measurements = track->measurements();
  if ( measurements.empty() ) {
    if ( msgLevel(MSG::DEBUG) )
      debug() << " -> Found " << track->lhcbIDs().size() 
	      << " LHCbIDs but no measurements" << endmsg;
    LHCb::VeloClusters *veloClusters = 
      get<LHCb::VeloClusters*>(LHCb::VeloClusterLocation::Default);
    // try to load clusters from lhcbids
    std::vector<const LHCb::VeloCluster*> clusters;
    for( auto id : track->lhcbIDs() ){
      if( id.isVelo() ) clusters.push_back(veloClusters->object(id.veloID()));
    }
    if(!clusters.empty()) {
      nTks = SiChargeFun::truncatedMean(clusters.begin(),clusters.end(),
					m_Ratio)/m_Normalisation;
    }else{
      nTks = 0.;
      if ( msgLevel(MSG::DEBUG) ) 
	debug() << "   -> No VELO clusters found -> no dE/dx charge measured" << endmsg; 
      return StatusCode::FAILURE;
    }
  }else{
    if ( msgLevel(MSG::DEBUG) )
      debug() << " -> Found " << measurements.size() << " Measurements" << endmsg;
    
    // Fill a temp vector with the velo clusters
    std::vector<const LHCb::VeloMeasurement*> veloMeas; veloMeas.reserve(32);
    
    // loop over measurements
    for ( TkMeas::const_iterator iM = measurements.begin();
	  iM != measurements.end(); ++iM ){  
      // is this a velo measurement
      if ( const VeloMeasurement* mVelo =
	   dynamic_cast<const VeloMeasurement*>(*iM) ) {
	  veloMeas.push_back(mVelo);
      }
    } // loop over measurements

    // how many charges where found
    if ( msgLevel(MSG::DEBUG) ){
      debug() << "  -> Found " << veloMeas.size() << endmsg;
    }

    if ( !veloMeas.empty() ){
      nTks = SiChargeFun::truncatedMean(veloMeas.begin(),veloMeas.end(), m_Ratio )/m_Normalisation;
    }else{
      // no velo clusters found
      if ( msgLevel(MSG::DEBUG) )
	debug() << "   -> No VELO clusters found -> no dE/dx charge measured" << endmsg; 
      return StatusCode::FAILURE;
    }
  }
  // if we get here, all is OK
  return StatusCode::SUCCESS;
}
StatusCode TrackVelodEdxCharge::i_cachedEdx(){
  m_Normalisation = m_dEdx->param<double>("Normalisation");
  m_Ratio = m_dEdx->param<double>("Ratio");
  if ( msgLevel(MSG::DEBUG) )
    debug() << "i_cachedEdx : Normalisation " << m_Normalisation
            << " Ratio " << m_Ratio << endmsg;
  
  return StatusCode::SUCCESS;
}
