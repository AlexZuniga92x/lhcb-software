
//-----------------------------------------------------------------------------
/** @file ChargedProtoParticleAddVeloInfo.cpp
 *
 * Implementation file for algorithm ChargedProtoParticleAddVeloInfo
 *
 * CVS Log :-
 * $Id: ChargedProtoParticleAddVeloInfo.cpp,v 1.2 2009-09-03 11:09:22 jonrob Exp $
 *
 * @author Chris Jones   Christopher.Rob.Jones@cern.ch
 * @date 28/08/2009
 */
//-----------------------------------------------------------------------------

// from Gaudi
#include "GaudiKernel/AlgFactory.h"

// local
#include "ChargedProtoParticleAddVeloInfo.h"

//-----------------------------------------------------------------------------

// Declaration of the Algorithm Factory
DECLARE_ALGORITHM_FACTORY( ChargedProtoParticleAddVeloInfo );

//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
ChargedProtoParticleAddVeloInfo::
ChargedProtoParticleAddVeloInfo( const std::string& name,
                                 ISvcLocator* pSvcLocator)
  : GaudiAlgorithm ( name , pSvcLocator )
{
  // default locations from context()
  if ( context() == "HLT" || context() == "Hlt" )
  {
    m_protoPath  = LHCb::ProtoParticleLocation::HltCharged;
  }
  else
  {
    m_protoPath  = LHCb::ProtoParticleLocation::Charged;
  }
  declareProperty( "ProtoParticleLocation", m_protoPath );
}

//=============================================================================
// Destructor
//=============================================================================
ChargedProtoParticleAddVeloInfo::~ChargedProtoParticleAddVeloInfo() { }

//=============================================================================
// Initialization
//=============================================================================
StatusCode ChargedProtoParticleAddVeloInfo::initialize()
{
  const StatusCode sc = GaudiAlgorithm::initialize();
  if ( sc.isFailure() ) return sc;

  m_velodEdx = tool<ITrackVelodEdxCharge>( "TrackVelodEdxCharge", "VeloCharge", this );

  return sc;
}

//=============================================================================
// Main execution
//=============================================================================
StatusCode ChargedProtoParticleAddVeloInfo::execute()
{

  // ProtoParticle container
  if ( !exist<LHCb::ProtoParticles>(m_protoPath) )
  {
    return Warning( "No existing ProtoParticle container at " +  m_protoPath + " thus do nothing.",
                    StatusCode::SUCCESS );
  }
  LHCb::ProtoParticles * protos = get<LHCb::ProtoParticles>(m_protoPath);

  // Loop over proto particles
  for ( LHCb::ProtoParticles::iterator iProto = protos->begin();
        iProto != protos->end(); ++iProto )
  {
    // replace the muon information
    addVelodEdx(*iProto);
  }

  return StatusCode::SUCCESS;
}

//=============================================================================

//=============================================================================
// Add VELO dE/dx info to the protoparticle
//=============================================================================
bool ChargedProtoParticleAddVeloInfo::addVelodEdx( LHCb::ProtoParticle * proto ) const
{
  // clear current Velo info
  proto->removeVeloInfo();

  // get velo charge
  double veloNtks(0);
  bool OK(false);
  const StatusCode sc = m_velodEdx->nTracks( proto->track(), veloNtks );
  if ( sc.isSuccess() )
  {
    proto->addInfo ( LHCb::ProtoParticle::VeloCharge, veloNtks );
    OK = true;
  }
  // return status
  return OK;
}
