// $Id: NoPIDsParticleMaker.cpp,v 1.18 2009-12-08 12:55:08 pkoppenb Exp $
// Include files 

// from Gaudi
#include "GaudiKernel/DeclareFactoryEntries.h" 

// local
#include "NoPIDsParticleMaker.h"
using namespace Gaudi::Units;

//-----------------------------------------------------------------------------
// Implementation file for class : NoPIDsParticleMaker
//
// 2006-01-23 : I. Belyaev
//-----------------------------------------------------------------------------

// Declaration of the Tool Factory
DECLARE_ALGORITHM_FACTORY( NoPIDsParticleMaker );


//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
NoPIDsParticleMaker::NoPIDsParticleMaker(  const std::string& name,ISvcLocator* pSvcLocator )
  : ChargedParticleMakerBase ( name , pSvcLocator )
  , m_CL     ( 50 * perCent  ) 
  , m_calls  ( 0 ) 
  , m_sum    ( 0 ) 
  , m_sum2   ( 0 )
{
  declareProperty ( "CL"       , m_CL     ) ; 

}
//=============================================================================
// Destructor
//=============================================================================
NoPIDsParticleMaker::~NoPIDsParticleMaker() {} 

//=============================================================================
// Destructor
//=============================================================================
StatusCode NoPIDsParticleMaker::initialize() {
  StatusCode sc = ChargedParticleMakerBase::initialize();

  return StatusCode::SUCCESS;
  
} 

// ============================================================================
StatusCode NoPIDsParticleMaker::finalize() 
{
  const std::string stars ( 80 , '*' );
  info() << stars << endmsg ;
  
  if( 0 != m_ppSvc    ) { m_ppSvc  = 0 ; }
  
  double mean  = m_sum ;
  if ( 0 != m_calls  ) { mean  = m_sum  / m_calls                ; }
  double sigma = 0     ;
  if ( 0 != m_calls  ) { sigma = m_sum2 / m_calls - mean * mean  ; }
  if ( sigma > 0     ) { sigma = sqrt( sigma )                   ; }
  
  always()
    << " created '"    << m_pid 
    << "' and '"       << m_apid  
    << " : "           << m_sum
    << " per "         << m_calls
    << " calls ("      << mean 
    << "+-"            << sigma 
    << ")/event"       << endmsg ;
  // finalize the base 
  return ChargedParticleMakerBase::finalize ();
};
//=============================================================================
// Dispatch the making of particles 
//=============================================================================
StatusCode NoPIDsParticleMaker::makeParticles
( LHCb::Particle::Vector & particles ){
  
  // increase the counter 
  ++m_calls ; 
  
  size_t number = 0 ;
  // get all input data
  const LHCb::ProtoParticles* pps = protos() ;
  // loop over all protoparticles 
  for(LHCb:: ProtoParticles::const_iterator ipp = pps->begin() ; 
      pps->end() != ipp ; ++ipp ) 
  {
    const LHCb::ProtoParticle* pp = *ipp ;
    if ( 0 == pp                ) { continue ; }              // CONTINUE
      if ( 0 == pp -> charge ()   ) { continue ; }              // CONTINUE
      
      const LHCb::Track* ptrack = pp->track();
      if ( 0==ptrack ) {
        Warning("Charged protoparticle with no Track found. Ignoring.");
        continue ;
      }
      // Select tracks
      if (msgLevel(MSG::VERBOSE)) verbose() << "Trying Track " << ptrack->key() << endmsg;
      if ( !trSel()->accept(*ptrack) ) continue;
      if (msgLevel(MSG::VERBOSE)) {
        verbose() << " -> Track selected " << ptrack->key()  
                  << " " << ptrack->firstState().momentum() << endmsg;
      }
      LHCb::Particle* particle = new LHCb::Particle();
      
      StatusCode sc = StatusCode::FAILURE ;
      if      ( m_pp  -> charge() == pp -> charge () ) 
      { sc = fillParticle( pp , m_pp , particle ) ; }
      else if ( m_app -> charge() == pp -> charge () ) 
      { sc = fillParticle( pp , m_app , particle ) ; }
      
      if( sc.isFailure() ) 
      {
        Warning("Error from 'fillParticle'", sc );
        delete particle ; particle = 0 ; continue ;           // CONTINUE
      }
      
      // put particle into the output container 
      particles.push_back( particle );
      ++number ;
      
  } // loop over protoparticles
  
  m_sum   +=          number ;
  m_sum2  += number * number ;

  return StatusCode::SUCCESS ;  
}

// ============================================================================
/** Fill the particle from protoparticle using ID  
 *  @param proto    pointer to ProtoParticle
 *  @param property particle property information
 *  @param particle particle (output) 
 *  @return status code 
 */
// ============================================================================
StatusCode NoPIDsParticleMaker::fillParticle
( const LHCb::ProtoParticle*    proto    ,
  const LHCb::ParticleProperty* property , 
  LHCb::Particle*               particle ) const 
{
  if ( 0 == proto    ) 
  { return Error ( "fillParticle: ProtoParticle*    is NULL" , 120 ) ; }
  if ( 0 == property ) 
  { return Error ( "fillParticle: ParticleProperty* is NULL" , 121 ) ; }
  if ( 0 == particle ) 
  { return Error ( "fillParticle: Particle*         is NULL" , 122 ) ; }
  
  const double mass = property -> mass() ;
  
  particle -> setParticleID   ( property -> particleID () ) ;
  particle -> setConfLevel    ( m_CL ) ;
  particle -> setMeasuredMass ( mass ) ;
  
  particle -> setProto( proto ) ;
  //
  const LHCb::Track* track = proto->track() ;  
  //
  const LHCb::State* state   = usedState( track ) ;
  
  return p2s()->state2Particle(*state,*particle);
};
// ============================================================================
