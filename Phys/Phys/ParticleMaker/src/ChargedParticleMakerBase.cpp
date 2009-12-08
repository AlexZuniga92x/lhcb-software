// $Id: ChargedParticleMakerBase.cpp,v 1.4 2009-12-08 12:55:07 pkoppenb Exp $
// Include files

#include "GaudiKernel/DeclareFactoryEntries.h"

// from EventSys
#include "Event/Particle.h"
#include "Event/Vertex.h"

// local
#include "ChargedParticleMakerBase.h"

//-----------------------------------------------------------------------------
// Implementation file for class : ChargedParticleMakerBase
//
// 2009-04-21 P. Koppenburg
//-----------------------------------------------------------------------------

//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
ChargedParticleMakerBase::ChargedParticleMakerBase( const std::string& name,
                                    ISvcLocator* pSvcLocator)
  : ParticleMakerBase  ( name , pSvcLocator ) 
  , m_p2s()
  ,  m_trSel   ( NULL )
{
}
//=========================================================================
//  
//=========================================================================
StatusCode ChargedParticleMakerBase::initialize ( ) {
  StatusCode sc = ParticleMakerBase::initialize();
  m_p2s = tool<IParticle2State>("Particle2State"); // not private
  // get an instance of the track selector
  m_trSel = tool<ITrackSelector>( "TrackSelector", "TrackSelector", this );

  const std::string Upper ( to_upper( m_pid ) ) ;
  if      ( "PIONS"     == Upper ) { m_pid = "pi+"    ; } 
  else if ( "PI"        == Upper ) { m_pid = "pi+"    ; } 
  else if ( "PION"      == Upper ) { m_pid = "pi+"    ; } 
  else if ( "KAONS"     == Upper ) { m_pid = "K+"     ; } 
  else if ( "KAON"      == Upper ) { m_pid = "K+"     ; } 
  else if ( "K"         == Upper ) { m_pid = "K+"     ; } 
  else if ( "MUONS"     == Upper ) { m_pid = "mu+"    ; }
  else if ( "MUON"      == Upper ) { m_pid = "mu+"    ; }
  else if ( "MU"        == Upper ) { m_pid = "mu+"    ; }
  else if ( "ELECTRONS" == Upper ) { m_pid = "e+"     ; }
  else if ( "ELECTRON"  == Upper ) { m_pid = "e+"     ; }
  else if ( "POSITRON"  == Upper ) { m_pid = "e+"     ; }
  else if ( "E"         == Upper ) { m_pid = "e+"     ; }
  else if ( "PROTONS"   == Upper ) { m_pid = "p+"     ; }
  else if ( "PROTON"    == Upper ) { m_pid = "p+"     ; }
  else if ( "P"         == Upper ) { m_pid = "p+"     ; }

  sc = setPPs( m_pid ) ;
  if ( sc.isFailure() ) 
  { return Error ( "Particle/Antiparticle are unknown for '"+m_pid+"'",sc);}
  if ( 0 == m_pp || 0 == m_app )
  { return Error ( "Particle/Antiparticle are invalid for '"+m_pid+"'"  );}
  if (  m_pp->charge() < m_app->charge() ) { 
    std::swap( m_pp , m_app ) ; 
    if (msgLevel(MSG::DEBUG)) debug() << "swapping" << m_pp->particle() << " and " << m_app->particle() << endmsg ; 
  }

  m_pid  = m_pp  -> particle () ;
  m_apid = m_app -> particle () ;

  info() << " Particle/AntiParticle to be created\t " 
         << "'"   << m_pid << "'/'" << m_apid << "'" << endmsg ;

  return sc;
}
// ============================================================================
/// set particle properties for particle and for antiparticle  
// ============================================================================
StatusCode ChargedParticleMakerBase::setPPs( const std::string& pid )
{
  if ( 0 == ppSvc() ) { return StatusCode ( 110 ) ; }
  // get the properties of the particle 
  m_pp = ppSvc  () -> find( pid ) ;
  if ( 0 == m_pp    ) { return StatusCode ( 111 ) ; }
  // get the the antiparticle 
  m_app = m_pp -> antiParticle () ;
  if ( 0 == m_app   ) { return StatusCode ( 112 ) ; }
  m_apid = m_app -> particle () ;  
  return StatusCode::SUCCESS ;
};
// ============================================================================
/// Select the appropriate state 
// ============================================================================
const LHCb::State* ChargedParticleMakerBase::usedState( const LHCb::Track* track) const{
  if ( 0==track) Exception("NULL track");
  const LHCb::State* uState = 0 ;
  // default: closest to the beam:
  if ( 0 == uState ) { uState = track->stateAt( LHCb::State::ClosestToBeam    ) ; }
  // if not availabel: first measurementr 
  if ( 0 == uState ) { uState = track->stateAt( LHCb::State::FirstMeasurement ) ; }
  // backup 
  if ( 0 == uState ) 
  {
    Warning("No state closest to beam or at first measurement for track. Using first state instead",10,StatusCode::SUCCESS) ;
    uState = &track->firstState() ;
  }
  if (msgLevel(MSG::VERBOSE)) 
  { verbose() << "Using '" << uState->location() << "' state at " << uState->position() << endmsg ; }
  return uState ;
}
//=============================================================================
// Destructor
//=============================================================================
ChargedParticleMakerBase::~ChargedParticleMakerBase() {};

