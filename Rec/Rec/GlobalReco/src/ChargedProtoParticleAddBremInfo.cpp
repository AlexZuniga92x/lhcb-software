
//-----------------------------------------------------------------------------
/** @file ChargedProtoParticleAddBremInfo.cpp
 *
 * Implementation file for algorithm ChargedProtoParticleAddBremInfo
 *
 * CVS Log :-
 * $Id: ChargedProtoParticleAddBremInfo.cpp,v 1.1 2009-08-29 20:37:18 jonrob Exp $
 *
 * @author Chris Jones   Christopher.Rob.Jones@cern.ch
 * @date 28/08/2009
 */
//-----------------------------------------------------------------------------

// from Gaudi
#include "GaudiKernel/AlgFactory.h"

// local
#include "ChargedProtoParticleAddBremInfo.h"

//-----------------------------------------------------------------------------

// Declaration of the Algorithm Factory
DECLARE_ALGORITHM_FACTORY( ChargedProtoParticleAddBremInfo );

//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
ChargedProtoParticleAddBremInfo::
ChargedProtoParticleAddBremInfo( const std::string& name,
                                 ISvcLocator* pSvcLocator)
  : ChargedProtoParticleCALOBaseAlg ( name , pSvcLocator ),
    m_InBremTable    ( NULL ),
    m_bremTrTable    ( NULL ),
    m_BremChi2Table  ( NULL ),
    m_dlleBremTable  ( NULL )
{
  // default locations from context()

  using namespace LHCb::Calo2Track;
  using namespace LHCb::CaloIdLocation;
  using namespace LHCb::CaloAlgUtils;

  m_inBremPath      = PathFromContext( context() , InBrem     , InBremHlt     );
  m_bremMatchPath   = PathFromContext( context() , BremMatch  , BremMatchHlt  );
  m_bremChi2Path    = PathFromContext( context() , BremChi2   , BremChi2Hlt   );
  m_bremPIDePath    = PathFromContext( context() , BremPIDe   , BremPIDeHlt   );
  m_protoPath       = PathFromContext( context() , 
                                       LHCb::ProtoParticleLocation::Charged ,
                                       LHCb::ProtoParticleLocation::HltCharged );  

  declareProperty("InputInBremLocation"        , m_inBremPath       );
  declareProperty("InputBremMatchLocation"     , m_bremMatchPath    );
  declareProperty("InputBremChi2Location"      , m_bremChi2Path     );
  declareProperty("InputBremPIDeLocation"      , m_bremPIDePath     );
  declareProperty("ProtoParticleLocation"      , m_protoPath        );

}

//=============================================================================
// Destructor
//=============================================================================
ChargedProtoParticleAddBremInfo::~ChargedProtoParticleAddBremInfo() { }

//=============================================================================
// Main execution
//=============================================================================
StatusCode ChargedProtoParticleAddBremInfo::execute()
{
  // Load the Brem data
  const bool sc = getBremData();
  if ( !sc )
  {
    return Warning( "No BREM data -> ProtoParticles will not be changed.", StatusCode::SUCCESS );
  }

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
    addBrem(*iProto);
  }

  return StatusCode::SUCCESS;
}

//=============================================================================

//=============================================================================
// Loads the Calo Brem data
//=============================================================================
bool ChargedProtoParticleAddBremInfo::getBremData()
{
  const bool sc1 = loadCaloTable(m_InBremTable  , m_inBremPath);
  const bool sc2 = loadCaloTable(m_bremTrTable  , m_bremMatchPath);
  const bool sc3 = loadCaloTable(m_BremChi2Table, m_bremChi2Path);
  const bool sc4 = loadCaloTable(m_dlleBremTable, m_bremPIDePath);

  const bool sc  = sc1 && sc2 && sc3 && sc4;
  if ( sc ) debug() << "BREM PID SUCCESSFULLY LOADED" << endmsg;

  return sc;
}

//=============================================================================
// Add Calo Brem info to the protoparticle
//=============================================================================
bool ChargedProtoParticleAddBremInfo::addBrem( LHCb::ProtoParticle * proto ) const
{
  // First remove existing BREM info
  proto->removeCaloBremInfo();

  // Add new info

  LHCb::Calo2Track::ITrAccTable::Range      aRange;
  LHCb::Calo2Track::ITrHypoTable2D::Range   hRange;
  LHCb::Calo2Track::ITrEvalTable::Range     vRange;

  bool hasBremPID = false;

  aRange = m_InBremTable -> relations ( proto->track() ) ;

  if ( !aRange.empty() )
  {
    hasBremPID = aRange.front().to();
    if ( hasBremPID )
    {
      if ( msgLevel(MSG::VERBOSE) ) verbose() << " -> The Brem. extrapolated line is in Ecal acceptance"  << endmsg;
      proto->addInfo(LHCb::ProtoParticle::InAccBrem , true );

      // Get the highest weight associated brem. CaloHypo (3D matching)
      hRange = m_bremTrTable ->relations ( proto->track() ) ;
      if ( !hRange.empty() )
      {
        proto->addToCalo ( hRange.front().to() );
        proto->addInfo(LHCb::ProtoParticle::CaloNeutralSpd, CaloSpd( hRange.front().to() ));
        proto->addInfo(LHCb::ProtoParticle::CaloNeutralPrs, CaloPrs( hRange.front().to() ));
        proto->addInfo(LHCb::ProtoParticle::CaloNeutralEcal, CaloEcal( hRange.front().to() ));
        proto->addInfo(LHCb::ProtoParticle::CaloBremMatch , hRange.front().weight() );
      }

      // Get the BremChi2 (intermediate) estimator
      vRange = m_BremChi2Table -> relations ( proto->track() ) ;
      if ( !vRange.empty() ) { proto->addInfo(LHCb::ProtoParticle::CaloBremChi2,  vRange.front().to() ); }

      // Get the Brem DLL(e)
      vRange = m_dlleBremTable -> relations ( proto->track() ) ;
      if ( !vRange.empty() ) { proto->addInfo(LHCb::ProtoParticle::BremPIDe , vRange.front().to() ); }

      if ( msgLevel(MSG::VERBOSE) )
        verbose() << " -> BremStrahlung PID : "
                  << " Chi2-Brem  =" <<  proto->info(LHCb::ProtoParticle::CaloBremMatch, -999.)
                  << " BremChi2   =" <<  proto->info(LHCb::ProtoParticle::CaloBremChi2, -999.)
                  << " Dlle (Brem) =" <<  proto->info(LHCb::ProtoParticle::BremPIDe, -999.)
                  << " Spd Digits " <<  proto->info(LHCb::ProtoParticle::CaloNeutralSpd, 0.)
                  << " Prs Digits " <<  proto->info(LHCb::ProtoParticle::CaloNeutralPrs, 0.)
                  << " Ecal Cluster " <<  proto->info(LHCb::ProtoParticle::CaloNeutralEcal, 0.)
                  << endmsg;
    }
    else
    {
      if ( msgLevel(MSG::VERBOSE) ) verbose() << " -> The Brem. extrapolated line is NOT in Ecal acceptance"  << endmsg;
    }
  }
  else
  {
    if ( msgLevel(MSG::VERBOSE) ) verbose() << " ->  No entry for that track in the Brem acceptance table"  << endmsg;
  }

  return hasBremPID;
}
