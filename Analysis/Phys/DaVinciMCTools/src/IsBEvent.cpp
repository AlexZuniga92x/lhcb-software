// $Id: IsBEvent.cpp,v 1.6 2007-09-07 13:23:48 pkoppenb Exp $
// Include files 

// from Gaudi
#include "GaudiKernel/DeclareFactoryEntries.h"
#include "GaudiKernel/IParticlePropertySvc.h"
#include "GaudiKernel/ParticleProperty.h"
// local
#include "IsBEvent.h"
#include "Event/SelResult.h"

//-----------------------------------------------------------------------------
// Implementation file for class : IsBEvent
//
// 2005-01-12 : Patrick KOPPENBURG
//-----------------------------------------------------------------------------

// Declaration of the Algorithm Factory

DECLARE_ALGORITHM_FACTORY( IsBEvent );


//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
IsBEvent::IsBEvent( const std::string& name,
                    ISvcLocator* pSvcLocator)
  : GaudiAlgorithm ( name , pSvcLocator )
  , m_particles()
    , m_writeTool()
{
  m_required.push_back("b"); // default is to require a b-quark
  declareProperty( "RequiredParticles", m_required );
  declareProperty( "AndMode", m_andMode = false );
  declareProperty( "AvoidSelResult", m_avoidSelResult = false);
}
//=============================================================================
// Destructor
//=============================================================================
IsBEvent::~IsBEvent() {}; 

//=============================================================================
// Initialization
//=============================================================================
StatusCode IsBEvent::initialize() {
  StatusCode sc = GaudiAlgorithm::initialize(); // must be executed first
  if ( sc.isFailure() ) return sc;  // error printed already by GaudiAlgorithm

  debug() << "==> Initialize" << endmsg;

  // get particle codes
  IParticlePropertySvc *ppSvc = svc<IParticlePropertySvc>("ParticlePropertySvc");
  if( !ppSvc ) {
    fatal() << "Unable to locate Particle Property Service" << endmsg;
    return sc;
  }
  for (std::vector<std::string>::const_iterator PN=m_required.begin() ; 
       PN!=m_required.end() ; ++PN ){
    if ( *PN == "b" ) m_particles.push_back(5); // b quark
    else if ( *PN == "c" ) m_particles.push_back(4); // c
    else if ( *PN == "t" ) m_particles.push_back(6); // top
    else {
      ParticleProperty *pp = ppSvc->find(*PN);
      if (!pp) {
        fatal() << " Unable to retrieve particle property for " << *PN << endmsg;
        return StatusCode::FAILURE;
      }
      int pid = pp->jetsetID();
      m_particles.push_back(pid);
    }
  }  
  if (m_particles.empty()) warning() << "Particles list is empty." << endmsg;
  else info() << "Will be looking for events with " << m_particles << endmsg;
  if (m_andMode) info() << "... of which all have to be there!" << endmsg;

  m_writeTool = tool<IWriteSelResult>("WriteSlResult");

  return StatusCode::SUCCESS;
};

//=============================================================================
// Main execution
//=============================================================================
StatusCode IsBEvent::execute() {

  debug() << "==> Execute" << endmsg;

  LHCb::MCParticles* mcparts = get<LHCb::MCParticles>(LHCb::MCParticleLocation::Default );
  if( !mcparts ){
    fatal() << "Unable to find MC particles at '"
        << LHCb::MCParticleLocation::Default << "'" << endreq;
    return StatusCode::FAILURE;
  }
  bool found = goodEvent(mcparts);
  if (found) debug() << "Found required particle(s)" << endreq;
  setFilterPassed(found);
  
  StatusCode sc = StatusCode::SUCCESS;
  if ( !m_avoidSelResult ) sc = m_writeTool->write(name(),filterPassed()) ;

  return StatusCode::SUCCESS;
};

//=============================================================================
//  All there?
//=============================================================================
bool IsBEvent::goodEvent(LHCb::MCParticles* mcparts){
  
  for ( std::vector<int>::const_iterator id = m_particles.begin() ; 
        id != m_particles.end() ; ++id ){
    bool found = false ;
    for ( LHCb::MCParticles::const_iterator MC = mcparts->begin() ; 
          MC != mcparts->end() ; ++MC){
      if ( *id == 5 ) found = (*MC)->particleID().hasBottom() ;
      else if ( *id == 4 ) found = (*MC)->particleID().hasCharm() ;
      else if ( *id == 6 ) found = (*MC)->particleID().hasTop() ;
      else {
        found = ( (*MC)->particleID().pid() == *id ) ;
      }
      if (found) {
        debug() << "Found required particle " << 
          (*MC)->particleID().pid() << endmsg;
        break ; // happy
      }
    }
    if (!found) debug() << "Did not find PID " << *id << endmsg;
    if ( m_andMode && !found ) return false ; // one is missing
    if ( (!m_andMode) && found ) return true ; // at least one found
  }
  return m_andMode ; // in and-mode one is happy. in or-mode unhappy.
}

///=============================================================================
//  Finalize
//=============================================================================
StatusCode IsBEvent::finalize() {

  debug() << "==> Finalize" << endmsg;

  return GaudiAlgorithm::finalize();  // must be called after all other actions
}

//=============================================================================
