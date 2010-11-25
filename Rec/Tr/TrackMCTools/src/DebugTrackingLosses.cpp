// $Id: DebugTrackingLosses.cpp,v 1.1 2009-04-01 09:03:56 ocallot Exp $
// Include files 

// from Gaudi
#include "GaudiKernel/AlgFactory.h" 
#include "Event/Track.h"
#include "Event/MCParticle.h"
#include "Event/MCTrackInfo.h"
#include "Event/VeloCluster.h"
#include "Event/STCluster.h"
#include "Event/OTTime.h"
#include "Linker/LinkedFrom.h"
#include "Linker/LinkedTo.h"
#include "Event/ProcStatus.h"

// local
#include "DebugTrackingLosses.h"

//-----------------------------------------------------------------------------
// Implementation file for class : DebugTrackingLosses
//
// 2009-03-26 : Olivier Callot
//-----------------------------------------------------------------------------

// Declaration of the Algorithm Factory
DECLARE_ALGORITHM_FACTORY( DebugTrackingLosses );


//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
DebugTrackingLosses::DebugTrackingLosses( const std::string& name,
                                          ISvcLocator* pSvcLocator)
  : GaudiAlgorithm ( name , pSvcLocator )
{
  declareProperty( "Velo",        m_velo        = false );
  declareProperty( "Forward",     m_forward     = false );
  declareProperty( "Ghost",       m_ghost       = false );
  declareProperty( "Clone",       m_clone       = false );
  declareProperty( "FromStrange", m_fromStrange = false );
  declareProperty( "MinMomentum", m_minMomentum = 5000. );
  declareProperty( "SaveList",    m_saveList    = false );
}
//=============================================================================
// Destructor
//=============================================================================
DebugTrackingLosses::~DebugTrackingLosses() {} 

//=============================================================================
// Initialization
//=============================================================================
StatusCode DebugTrackingLosses::initialize() {
  StatusCode sc = GaudiAlgorithm::initialize(); // must be executed first
  if ( sc.isFailure() ) return sc;  // error printed already by GaudiAlgorithm

  if ( msgLevel(MSG::DEBUG) ) debug() << "==> Initialize" << endmsg;

  m_ppSvc = svc<IParticlePropertySvc>( "ParticlePropertySvc", true );
  m_eventNumber = 0;
  return StatusCode::SUCCESS;
}

//=============================================================================
// Main execution
//=============================================================================
StatusCode DebugTrackingLosses::execute() {
  ++m_eventNumber;
  if ( msgLevel(MSG::DEBUG) ) debug() << "==> Execute" << endmsg;
  const LHCb::MCParticles* partCont = get<LHCb::MCParticles>(LHCb::MCParticleLocation::Default); 
  
  LHCb::ProcStatus* procStat = getOrCreate<LHCb::ProcStatus,LHCb::ProcStatus>( LHCb::ProcStatusLocation::Default);
  
  if ( procStat->aborted() ) {
    debug() << "** Processing aborted. Don't analyse losses! " << endmsg;
    return StatusCode::SUCCESS;
  }

  LinkedFrom<LHCb::Track,LHCb::MCParticle> veloLinker(    evtSvc(), msgSvc(), LHCb::TrackLocation::Velo );
  LinkedFrom<LHCb::Track,LHCb::MCParticle> forwardLinker( evtSvc(), msgSvc(), LHCb::TrackLocation::Forward );

  MCTrackInfo trackInfo( evtSvc(), msgSvc() );

  LHCb::MCParticles::const_iterator itP = partCont->begin();
  for (; itP != partCont->end(); ++itP){
    LHCb::MCParticle* part = *itP;
    if ( 0 == trackInfo.fullInfo( part ) ) continue;
    if ( ! trackInfo.hasVeloAndT( part ) ) continue;
    if ( abs( part->particleID().pid() ) == 11 ) continue; // reject electron    
    if ( m_fromStrange ) {
      bool isStrange = false;
      const LHCb::MCParticle* mother = part;
      while( 0 != mother->originVertex() ) {
        mother = mother->originVertex()->mother();
        if ( 0 == mother ) break;
        if ( mother->particleID().pid() ==   310 ) isStrange = true;
        if ( mother->particleID().pid() ==  3122 ) isStrange = true;
        if ( mother->particleID().pid() == -3122 ) isStrange = true;
      }
      if ( !isStrange ) continue;
    } else {
      if ( m_minMomentum > fabs( part->p() ) ) continue;
    }
    
    
    bool hasVelo = veloLinker.first( part ) != NULL;

    if ( m_velo && !hasVelo ) {
      info() << "Missed Velo for MCParticle " << part->key() << " ";
      printMCParticle( part );
    } 
    if ( m_forward ) {
      LHCb::Track* veloTr = veloLinker.first(part);
      if ( veloTr == NULL ) continue;
      if ( forwardLinker.first(part) == NULL && !m_clone && !m_ghost ) {
        info() << "Missed Forward (Velo " << veloTr->key() <<") for MCParticle " << part->key() << " ";
        printMCParticle( part );
        if ( m_saveList ) {
          std::vector<int> ref;
          ref.push_back( m_eventNumber );
          ref.push_back( veloTr->key() );
          ref.push_back( part->key() );
          m_badGuys.push_back( ref );
        }
      } else if ( m_clone && forwardLinker.next( ) != NULL ) {
        info() << "Forward clone (Velo " << veloTr->key() <<") for MCParticle " << part->key() << " ";
        printMCParticle( part );
      } 
    }
  }

  if ( m_ghost ) {
    LinkedTo<LHCb::MCParticle>       vLink( evtSvc(), msgSvc(), LHCb::VeloClusterLocation::Default );
    LinkedTo<LHCb::MCParticle>      itLink( evtSvc(), msgSvc(), LHCb::STClusterLocation::ITClusters);
    LinkedTo<LHCb::MCParticle>      otLink( evtSvc(), msgSvc(), LHCb::OTTimeLocation::Default);

    std::string   location = LHCb::TrackLocation::Forward;
    if ( m_velo ) location = LHCb::TrackLocation::Velo;
    LinkedTo<LHCb::MCParticle> trackLinker( evtSvc(), msgSvc(), location );
    
    LHCb::Tracks* tracks = get<LHCb::Tracks>( location );
    for ( LHCb::Tracks::const_iterator itT = tracks->begin(); tracks->end() != itT; ++itT ) {
      if ( trackLinker.first( *itT ) == NULL ) {
        info() << "Ghost track, nb " << (*itT)->key();
        for ( SmartRefVector<LHCb::Track>::iterator itA = (*itT)->ancestors().begin();
              (*itT)->ancestors().end() != itA; ++itA ) {
          info() << " from Velo " << (*itA)->key();
        }
        info() << endmsg;
        for ( std::vector<LHCb::LHCbID>::const_iterator itId = (*itT)->lhcbIDs().begin(); 
              (*itT)->lhcbIDs().end() != itId; ++itId ) {
          if ( (*itId).isVelo() ) {
            LHCb::VeloChannelID idV = (*itId).veloID();          
            info() << format( "   Velo Sensor %3d Strip %4d    ", idV.sensor(), idV.strip() );
            LHCb::MCParticle* part = vLink.first( idV );
            while ( 0 != part ) {
              info() << " " << part->key();
              part = vLink.next();
            }
            info() << endmsg;
          } else if ( (*itId).isIT() ) {
            LHCb::STChannelID stID = (*itId).stID();
            info() << format( "    IT St%2d La%2d Se%2d Str%4d    ",
                              stID.station(), stID.layer(), stID.sector(), stID.strip() );
            LHCb::MCParticle* part = itLink.first( stID );
            while ( 0 != part ) {
              info() << " " << part->key();
              part = itLink.next();
            }
            info() << endmsg;
          } else if ( (*itId).isOT() ) {
            LHCb::OTChannelID otID = (*itId).otID();
            info() << format( "    OT St%2d La%2d mo%2d Str%4d    ", 
                              otID.station(), otID.layer(), otID.module(), otID.straw() );
            LHCb::MCParticle* part = otLink.first( otID );
            while ( 0 != part ) {
              info() << " " << part->key();
              part = otLink.next();
            }
            info() << endmsg;
          }
        }
      }
    }
  }
  
  return StatusCode::SUCCESS;
}

//=========================================================================
//  
//=========================================================================
void DebugTrackingLosses::printMCParticle ( const LHCb::MCParticle* part ) {
  const LHCb::MCParticle* mother = part;
  const LHCb::MCVertex*   vert   = part->originVertex();
  ParticleProperty* pp;
  double p = double( int(  part->p() ) /1000. );
  info() << "MC: [" << p << " GeV]";
  while ( 0 != mother ) {
    pp = m_ppSvc->findByStdHepID( mother->particleID().pid() );
    if ( 0 == pp ) {
      info() << mother->key() << "[" << mother->particleID().pid() <<"]";
    } else {
      info() << mother->key() << "[" <<  pp->particle() <<"]";
    }
    vert = mother->originVertex();
    if ( 0 == vert ) {
      mother = 0;
    } else {
      info() << format( " <-(z=%7.2f)", vert->position().z() );
      mother = vert->mother();
    }
  }
  info() << endreq;
}
//=============================================================================
//  Finalize
//=============================================================================
StatusCode DebugTrackingLosses::finalize() {

  if ( msgLevel(MSG::DEBUG) ) debug() << "==> Finalize" << endmsg;

  if ( m_saveList ) {
    for ( std::vector<std::vector<int> >::iterator it = m_badGuys.begin();
          m_badGuys.end() != it; ++it ) {
      info() << format( "Event %4d  Velo %4d  MCParticle %4d",
                        (*it)[0], (*it)[1], (*it)[2] ) << endmsg;
    }
  }

  return GaudiAlgorithm::finalize();  // must be called after all other actions
}

//=============================================================================
