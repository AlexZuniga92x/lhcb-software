// $Id: VertexFitter.cpp,v 1.8 2010-03-08 11:35:35 ibelyaev Exp $
// ============================================================================
// Include files 
// ============================================================================
// STD & STL 
// ============================================================================
#include <algorithm>
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/DeclareFactoryEntries.h"
// ============================================================================
// GaudiAlg 
// ============================================================================
#include "GaudiAlg/GaudiTool.h"
// ============================================================================
// Event 
// ============================================================================
#include "Event/Particle.h"
#include "Event/Vertex.h"
// ============================================================================
// Kernel
// ============================================================================
#include "Kernel/IVertexFit.h"
#include "Kernel/IParticleTransporter.h"
#include "Kernel/ParticleProperty.h"
#include "Kernel/IParticlePropertySvc.h"
// ============================================================================
// LHCbMath
// ============================================================================
#include "LHCbMath/Kinematics.h"
#include "LHCbMath/MatrixUtils.h"
#include "LHCbMath/MatrixTransforms.h"
// ============================================================================
// LoKi
// ============================================================================
#include "LoKi/Trees.h"
// ============================================================================
// ROOT/Mathlib
// ============================================================================
#include "Math/Functions.h"
// ============================================================================
// Local 
// ============================================================================
#include "KalmanFilter.h"
#include "VertexFitter.h"
// ============================================================================
/// anonymous namespace to hide few technical constants
namespace 
{
  // ==========================================================================
  const double s_scale   = Gaudi::Units::perCent           ;
  const double s_scale2  = s_scale  * s_scale              ;
  const double s_small   = 0.1 * Gaudi::Units::micrometer  ;
  const double s_small2  = s_small  * s_small              ;
  const double s_middle  =  1  * Gaudi::Units::centimeter  ;
  const double s_middle2 = s_middle * s_middle             ;
  const double s_large   = 10  * Gaudi::Units::centimeter  ;
  const double s_large2  = s_large  * s_large              ; 
  // ==========================================================================
}
// ============================================================================
// load the data from the daughter particles into the internal structures 
// ============================================================================
StatusCode LoKi::VertexFitter::_load 
( const LHCb::Particle::ConstVector& ds ) const 
{
  StatusCode sc = StatusCode::SUCCESS ;
  m_entries.clear() ;
  m_entries.resize ( ds.size() ) ;
  LHCb::Particle::ConstVector::const_iterator c = ds.begin()        ;
  Entries::iterator                           e = m_entries.begin() ;
  //
  for ( ; ds.end() != c && sc.isSuccess() ; ++c , ++e ) 
  { sc = _load ( *c , *e ) ; } ;
  if ( sc.isFailure () ) 
  { return _Warning ( "_load(): the error from _load:" , sc          ) ; } // RETURN 
  if ( m_entries.empty() ) 
  { return _Warning ( "_load(): no valid data found"   , InvalidData ) ; } // RETURN 
  //
  if ( !LoKi::KalmanFilter::okForVertex ( m_entries ) ) 
  { return _Error("Input set could not be verticized"  , InvalidData ) ; }
  //
  return StatusCode::SUCCESS ;
} 
// ============================================================================
// load the data into internal representation 
// ============================================================================
StatusCode LoKi::VertexFitter::_load      
( const LHCb::Particle*     particle , 
  LoKi::VertexFitter::Entry& entry    ) const 
{
  if ( 0 == particle ) 
  { return _Warning ( "_load(): invalid particle" , InvalidParticle ) ; } // RETURN 
  //
  switch ( particleType ( particle ) ) 
  {
    //
  case LoKi::KalmanFilter::LongLivedParticle   :
    return LoKi::KalmanFilter::loadAsFlying     ( *particle  , entry ) ; // RETURN 
    //
  case LoKi::KalmanFilter::ShortLivedParticle  : 
    return LoKi::KalmanFilter::loadAsShortLived ( *particle  , entry ) ; // RETURN 
    //
  case LoKi::KalmanFilter::GammaLikeParticle   : 
    return LoKi::KalmanFilter::loadAsGamma      ( *particle  , entry ) ; // RETURN 
    //
  case LoKi::KalmanFilter::DiGammaLikeParticle : 
    return LoKi::KalmanFilter::loadAsDiGamma    ( *particle  , entry ) ; // RETURN 
    //
  default:
    return LoKi::KalmanFilter::load             ( *particle  , entry ) ; // RETURN 
  }
  //
  return LoKi::KalmanFilter::load ( *particle  , entry ) ;  
}
// ============================================================================
// add one particle at the end of the queue
// ============================================================================
StatusCode LoKi::VertexFitter::_add 
( const LHCb::Particle*  child , 
  const Gaudi::XYZPoint& point ) const
{
  m_entries.push_back( Entry() ) ;
  StatusCode sc = _load      ( child , m_entries.back() ) ;
  if ( sc.isFailure() ) 
  { _Warning ("_add(): the error from _add()      , ignore", sc ) ; }
  sc = _transport ( m_entries.back() , point ) ;
  if ( sc.isFailure() ) 
  { _Warning ("_add(): the error from _transport(), ignore", sc ) ; }
  return StatusCode::SUCCESS ;
} 
// ============================================================================
// make few kalman iterations 
// ============================================================================
StatusCode LoKi::VertexFitter::_iterate 
( const size_t               nIterMax , 
  const Gaudi::Vector3&      _x       ) const 
{  
  // initial position
  const Gaudi::Vector3* x = &_x ;
  // chi2 
  const double  _chi2 = 0 ;
  const double*  chi2 = &_chi2 ;
  // inverse covariance matrix for the position  
  const Gaudi::SymMatrix3x3* ci = &m_seedci ;
  //
  for ( size_t iIter = 1 ; iIter <= nIterMax ; ++iIter ) 
  {    
    // make a proper transportation 
    Gaudi::XYZPoint point ;
    Gaudi::Math::la2geo ( *x , point ) ;
    StatusCode sc = _transport ( point ) ;
    if ( sc.isFailure() ) 
    { _Warning ( "_iterate(): problem with transport ", sc ) ; }    
    // initialize the covariance matrix 
    if ( 0 != iIter ) { m_seedci = (*ci) * s_scale2 ; }
    ci   = &m_seedci ;
    // initialize the chi2 
    chi2 = &_chi2 ;
    const Gaudi::Vector3 x0 ( *x ) ;    
    // start the kalman filter 
    bool special = false ;
    // A) the simplest case: 2 body fit 
    if      ( m_use_twobody_branch  && 
              2 == m_entries.size() && 
              2 == LoKi::KalmanFilter::nGood ( m_entries ) )  
    {
      sc = LoKi::KalmanFilter::step ( m_entries[0] , 
                                      m_entries[1] , *chi2 ) ;
      if ( sc.isSuccess() ) { special = true ; }
      else { _Warning ( "Error from three-body Kalman step" , sc ) ; }      
    }
    // B) use three-body branch 
    else if ( m_use_threebody_branch  && 
              3 == m_entries.size()   && 
              3 == LoKi::KalmanFilter::nGood ( m_entries ) )  
    {
      sc = LoKi::KalmanFilter::step ( m_entries[0] , 
                                      m_entries[1] ,
                                      m_entries[2] , *chi2 ) ;
      if ( sc.isSuccess() ) { special = true ; }
      else { _Warning ( "Error from three-body Kalman step" , sc ) ; }      
    }
    // C) use four-body branch 
    else if ( m_use_fourbody_branch  && 
              4 == m_entries.size()  && 
              4 == LoKi::KalmanFilter::nGood ( m_entries ) )  
    {
      sc = LoKi::KalmanFilter::step ( m_entries[0] , 
                                      m_entries[1] ,
                                      m_entries[2] ,
                                      m_entries[3] , *chi2 ) ;
      if ( sc.isSuccess() ) { special = true ; }
      else { _Warning ( "Error from  four-body Kalman step" , sc ) ; }      
    }
    //
    if ( special && sc.isSuccess () ) 
    {
      // update the parameters 
      const LoKi::KalmanFilter::Entry& last = m_entries.back() ;
      ci   = &last.m_ci   ;
      x    = &last.m_x    ;
      chi2 = &last.m_chi2 ;
    }
    //
    // D) general case (of failure of any N-body specialiation) 
    //
    if  ( !special || sc.isFailure()  ) 
    {
      for ( EIT entry = m_entries.begin() ; m_entries.end() != entry ; ++entry ) 
      {
        // make one Kalman step 
        sc = LoKi::KalmanFilter::step ( *entry , *x , *ci , *chi2 ) ;
        // skip on Failure
        if ( sc.isFailure() ) 
        {  
          _Warning ( "Error from Kalman-step, skip" , sc ) ;
          continue ;                                                // CONTINUE 
        }
        // update the parameters 
        ci   = &entry->m_ci   ;
        x    = &entry->m_x    ;
        chi2 = &entry->m_chi2 ;
      }
    }
    //
    // kalman smooth
    sc = LoKi::KalmanFilter::smooth ( m_entries ) ;
    if ( sc.isFailure() ) 
    { _Warning ( "_iterate(): problem with smoother", sc ) ; }
    // distance in the absolute position 
    const double d1 = ROOT::Math::Mag        ( (*x) - x0 ) ;
    // distance in the chi2 units 
    const double d2 = ROOT::Math::Similarity ( (*x) - x0 , *ci ) ;
    if ( d2 < 0 ) 
    { _Warning ( "_iterate: negative chi2 detected, ignore" , sc ) ; }
    // termination conditions:
    //
    //  (1) STOP if the distance is sufficiently small 
    //      - either the absolute distance 
    //      - or chi2 distance (if at least one iteration is performed) 
    //
    if ( d1 < m_DistanceMax || ( 1 < iIter && 0 <= d2 && d2 < m_DistanceChi2 ) )
    {
      sc = LoKi::KalmanFilter::evalCov ( m_entries ) ;
      if ( sc.isFailure() ) 
      { _Warning ( "_iterate(): problems with covariances" , sc ) ; }
      // 
      counter ( "#iterations" ) += iIter ;
      //
      return StatusCode::SUCCESS ;                             // RETURN 
    } 
  } // end of iterations
  //
  return _Warning ( "No convergency has been reached" , NoConvergency ) ; // RETURN 
} 
// ============================================================================
// make a seed 
// ============================================================================
StatusCode LoKi::VertexFitter::_seed ( const LHCb::Vertex* vertex ) const
{
  // check if vertex could be used as a seed 
  const Gaudi::XYZPoint&     p = vertex->position  () ;
  const Gaudi::SymMatrix3x3& c = vertex->covMatrix () ;
  
  if ( m_seedZmin < p.Z()   && 
       m_seedZmax > p.Z()   && 
       m_seedRho  > p.Rho() && 
       s_small2 < Gaudi::Math::min_diagonal ( c ) && 
       s_large2 > Gaudi::Math::max_diagonal ( c )   ) 
  {
    // use the vertex parameters as proper seed 
    Gaudi::Math::geo2LA ( p , m_seed ) ;
    int ifail = 0 ;
    m_seedci = c.Inverse( ifail ) ;
    if ( !ifail ) 
    {
      // properly scale the seed matrix 
      Gaudi::Math::scale ( m_seedci , s_scale2 ) ;
      return StatusCode::SUCCESS ;                            // RETURN 
    } 
  }
  //
  StatusCode sc = LoKi::KalmanFilter::seed 
    ( m_entries , m_seed , m_seedci , s_scale2 ) ;
  
  if ( sc.isFailure() ) 
  { 
    m_seed[0] = 0.0 ; m_seed[1] = 0.0 ; m_seed [2] = 0.0 ;
    Gaudi::Math::setToUnit ( m_seedci , 1.0/s_middle2 ) ;
    m_seedci(2,2) = 1.0/s_large2 ;
    _Warning ( "_seed(): error in matrix inversion" , sc ) ; 
  }
  /// check the validity of the seed 
  Gaudi::XYZPoint pnt ( m_seed[0] , m_seed[1] , m_seed[2] ) ;
  if ( m_seedZmin > pnt.Z() ) 
  { 
    _Warning ("_seed(): Seed is outside of 'Zmin' fiducial volume " , sc ) ; 
    m_seed[2] = 0.5 * ( 0.0 + m_seedZmin ) ; 
    m_seedci(0,2) = 0 ; m_seedci(1,2) = 0 ;
    m_seedci(2,2) = 4.0/m_seedZmin/m_seedZmin;
  }
  if ( m_seedZmax < pnt.Z() ) 
  { 
    _Warning ("_seed(): Seed is outside of 'Zmax' fiducial volume " , sc ) ; 
    m_seed[2] = 0.5 * ( 0.0 + m_seedZmax ) ; 
    m_seedci(0,2) = 0 ; m_seedci(1,2) = 0 ;
    m_seedci(2,2) = 4.0/m_seedZmax/m_seedZmax;
  }
  if ( m_seedRho  < pnt.Rho() ) 
  { 
    _Warning ("_seed(): Seed is outside of 'Rho'  fiducial volume " , sc ) ; 
    m_seed[0]     = 0.0 ; m_seed[1] = 0.0 ; 
    m_seedci(0,1) = 0   ; m_seedci(0,2) = 0 ; 
    m_seedci(1,2) = 0   ; m_seedci(0,2) = 0 ;
    m_seedci(0,0) = 4.0/m_seedRho/m_seedRho ;
    m_seedci(1,1) = m_seedci(0,0);
  }  
  return StatusCode::SUCCESS ;
}  
// ============================================================================
// The vertex fitting method without creation of a Particle
// ============================================================================
StatusCode LoKi::VertexFitter::fit 
( LHCb::Vertex&                      vertex    ,
  const LHCb::Particle::ConstVector& daughters ) const 
{
  // load the data 
  StatusCode sc = _load ( daughters ) ;
  if ( sc.isFailure() ) 
  { return _Error ( "fit(): failure from _load() ", sc ) ; }      // RETURN 
  sc = _seed ( &vertex ) ; 
  if ( sc.isFailure() ) 
  { _Warning  ( "fit(): failure from _seed()"     , sc ) ; }
  // make "m_nIterMax" iterations 
  sc = _iterate ( m_nIterMaxI , m_seed ) ;
  if ( sc.isFailure() ) 
  { return _Error ( "fit(): failure from _iterate()"  , sc ) ; } // RETURN 
  
  // get the data from filter 
  const Entry&               entry = m_entries.back() ;
  const Gaudi::Vector3&      x     = entry.m_x         ;
  const Gaudi::SymMatrix3x3& c     = entry.m_c         ;
  const double               chi2  = entry.m_chi2      ;
  // update the vertex parameters:
  vertex.setPosition  ( Gaudi::XYZPoint ( x(0) , x(1) , x(2) ) ) ;
  vertex.setCovMatrix ( c    ) ;
  vertex.setChi2      ( chi2 ) ;
  vertex.setNDoF      ( LoKi::KalmanFilter::nDoF ( m_entries ) ) ;
  // fill the vertex 
  vertex.clearOutgoingParticles() ;
  for ( Entries::const_iterator ientry = m_entries.begin() ; 
        m_entries.end() != ientry ; ++ientry ) 
  {
    if ( ientry->regular() )  
    { vertex.addToOutgoingParticles( ientry -> m_p0 ) ; }  
  }
  // keep for future tracing
  m_vertex = &vertex ;
  if ( m_seedZmin > vertex.position().Z()  
    || m_seedZmax < vertex.position().Z() 
    || m_seedRho  < vertex.position().Rho() ) 
  { _Warning ( "fit(): Vertex is outside of fiducial volume " ) ; }
  //
  return sc ;
} 
// ============================================================================
// The vertex fitting method with the creation of a Particle
// ============================================================================
StatusCode LoKi::VertexFitter::fit 
( const LHCb::Particle::ConstVector& daughters ,
  LHCb::Vertex&                      vertex    ,
  LHCb::Particle&                    particle  ) const 
{
  using namespace ROOT::Math ;
  //
  // play a bit with extra-info
  if ( particle.hasInfo ( LHCb::Particle::Chi2OfVertexConstrainedFit ) ) 
  { particle.eraseInfo ( LHCb::Particle::Chi2OfVertexConstrainedFit )  ; }
  
  // make a vertex fit 
  StatusCode sc = fit ( vertex , daughters ) ;
  if ( sc.isFailure() ) { return _Error ( "fit(): failure from fit", sc ) ; }
  // links:
  
  particle.clearDaughters() ;
  for ( LHCb::Particle::ConstVector::const_iterator dau = 
          daughters.begin() ; daughters.end() != dau ; ++dau ) 
  { particle.addToDaughters( *dau ) ; } ;
  // 
  particle.setEndVertex( &vertex ) ;  
  //
  // fill the particle with the proper information 
  // 1) reference point and proper covariance matrices
  particle.setReferencePoint ( vertex.position  () ) ;
  particle.setPosCovMatrix   ( vertex.covMatrix () ) ;
  // 2) momentum, its covariance matrix and covariance with position
  // reset existing matrices
  Gaudi::Math::setToScalar ( m_cmom  , 0.0 ) ;
  Gaudi::Math::setToScalar ( m_cmom1 , 0.0 ) ;
  Gaudi::Math::setToScalar ( m_mpcov , 0.0 ) ;
  Gaudi::Math::setToScalar ( m_mm_c  , 0.0 ) ;
  //
  // 1. for true kinematics: 
  Gaudi::LorentzVector       vct   ;
  const Gaudi::SymMatrix3x3& pos = particle.posCovMatrix() ;
  // 2. for "measured-mass":
  Gaudi::LorentzVector       mm_v  ;
  for ( EIT i = m_entries.begin() ; m_entries.end() != i ; ++i ) 
  {
    Gaudi::Math::add ( vct , i->m_parq ) ;
    m_cmom  += i->m_d ;
    // for measured mass: blind sum of 4-momenta for extrapolated daughters 
    mm_v    += i->m_p.momentum     () ;
    m_mm_c  += i->m_p.momCovMatrix () ;
    //
    if ( i->special() ) { continue ; } // gamma & digamma
    //
    m_mpcov += i->m_e ;
    for ( EIT j = i + 1 ; m_entries.end() != j ; ++j ) 
    {	
      m_cmom1 += i->m_f * pos * Transpose( j->m_f ) ;
      m_cmom1 += j->m_f * pos * Transpose( i->m_f ) ; 
    } 
  }
  Gaudi::Math::add ( m_cmom , m_cmom1 ) ;
  particle.setMomentum        ( vct     ) ;
  particle.setMomCovMatrix    ( m_cmom  ) ;
  particle.setPosMomCovMatrix ( m_mpcov ) ;
  //
  // measured mass & error in measured mass 
  //
  const double mmass =  mm_v.M() ;
  if ( 0 >= mmass ) { _Warning ( "fit(): measured mass       is non-positive" ) ; }
  const double mmerr = Gaudi::Math::sigmamass ( mm_v , m_mm_c ) ;
  if ( 0 >= mmerr ) { _Warning ( "fit(): measured mass error is non-positive" ) ; }  
  particle.setMeasuredMass    ( mmass ) ;
  particle.setMeasuredMassErr ( mmerr ) ;
  //
  // play a bit with extra-info:
  particle.addInfo 
    ( LHCb::Particle::Chi2OfVertexConstrainedFit , vertex.chi2() ) ;
  //
  return StatusCode::SUCCESS ;
} 
// ============================================================================
// add the particle to the vertex and refit 
// ============================================================================
StatusCode LoKi::VertexFitter::add
( const LHCb::Particle*  particle , 
  LHCb::Vertex&          vertex   ) const
{
  if ( 0 == particle ) 
  { return _Error ( "add: Particle* point to NULL!" , InvalidParticle ) ; }
  //
  switch ( particleType ( particle ) ) 
  {
  case LoKi::KalmanFilter::GammaLikeParticle   :
    return _Error ( "add: No way to add   Gamma-like" , InvalidParticle ) ; 
  case LoKi::KalmanFilter::DiGammaLikeParticle : 
    return _Error ( "add: No way to add DiGamma-like" , InvalidParticle ) ;
  default: 
    break ;
  }
  //
  if ( &vertex != m_vertex ) 
  {
    // first need to fit it! 
    const IVertexFit* vFit = this ;
    StatusCode sc = 
      vFit->fit ( vertex                              , 
                  vertex.outgoingParticles().begin () , 
                  vertex.outgoingParticles().end   () ) ;
    if ( sc.isFailure() ) 
    { return _Error ( "add: error from 'fit'", sc ) ; }
  }
  StatusCode sc = _add ( particle , vertex.position() ) ;
  if ( sc.isFailure() ) { _Warning ("add(): failure from _add" , sc ) ; }
  //
  Entry& entry    =   m_entries.back()   ;
  const Entry& e0 = *(m_entries.end()-2) ;
  // make one Kalman step using the previos parameters as estimate
  sc = LoKi::KalmanFilter::step  ( entry , e0.m_x , e0.m_ci , e0.m_chi2) ;
  if ( sc.isFailure() ) { _Warning ("add(): failure from _step" , sc ) ; }
  // smooth it!
  sc = LoKi::KalmanFilter::smooth ( m_entries ) ;
  if ( sc.isFailure() ) { _Warning ("add(): failure from _smooth" , sc ) ; }
  // make few more full iterations 
  m_seedci = entry.m_ci ;
  Gaudi::Math::scale ( m_seedci , s_scale2 ) ;
  sc = _iterate ( m_nIterMaxII , entry.m_x ) ;
  if ( sc.isFailure() ) { _Warning ("add(): failure from _iterate" , sc ) ; }
  //
  const Gaudi::Vector3&      x     = entry.m_x         ;
  const Gaudi::SymMatrix3x3& c     = entry.m_c         ;
  const double               chi2  = entry.m_chi2      ;
  // update the vertex parameters:
  vertex.setPosition  ( Gaudi::XYZPoint ( x(0) , x(1) , x(2) ) ) ; 
  vertex.setCovMatrix ( c    ) ;
  vertex.setChi2      ( chi2 ) ;
  // fill the vertex 
  vertex.addToOutgoingParticles( particle ) ;
  // keep for future tracing
  m_vertex = &vertex ;
  // check for positions
  if ( m_seedZmin > vertex.position().Z() ) 
  { _Warning ( "fit():Vertex is outside of 'Zmin' fiducial volume " , sc ) ; }
  if ( m_seedZmax < vertex.position().Z() ) 
  { _Warning ( "fit():Vertex is outside of 'Zmax' fiducial volume " , sc ) ; }
  if ( m_seedRho  < vertex.position().Rho() ) 
  { _Warning ( "fit():Vertex is outside of 'Rho'  fiducial volume " , sc ) ; }
  //
  return StatusCode::SUCCESS ;
} 
// ============================================================================
// remove the particle from  the vertex and refit 
// ============================================================================
StatusCode LoKi::VertexFitter::remove
( const LHCb::Particle*  particle , 
  LHCb::Vertex&          vertex   ) const
{
  return Error ( "remove(): not implemented (yet)!" , NotImplemented ) ; // ATTENTION!!!
  //
  const StatusCode OK = StatusCode::SUCCESS ;
  if ( 0 == particle ) 
  { return _Warning ( "remove: nothing to remove" , OK ) ; }        // RETURN 
  if ( &vertex != m_vertex ) 
  {
    // first need to fit it !
    const IVertexFit* vFit = this ;
    StatusCode sc = 
      vFit->fit ( vertex                             , 
                  vertex.outgoingParticles().begin() , 
                  vertex.outgoingParticles().end  () ) ;
    if ( sc.isFailure() ) 
    { return _Error ( "remove: error from 'fit'", sc ) ;  }           // RETURN 
  }
  // remove the particle from the vertex 
  vertex.removeFromOutgoingParticles ( particle ) ;
  // find the corresponding entry
  
  //   EIT ifind = m_entries.end() ;
  //   for ( EIT entry = m_entries.begin() ; m_entries.end() != entry ; ++entry ) 
  //   { if ( particle == entry->m_p0 ) { ifind = entry ; break ; } }
  //   if ( m_entries.end() == ifind ) 
  //   { return Warning ("remove: particle is not in the fit", OK ) ; } // RETURN
  
  //   const Entry& back = m_entries.back() ;
  //   // FIX ME!!!
  //   const Gaudi::SymMatrix3x3 nci= ( back.m_ci  /* + ... FIX ME !!!! */ ) ;
  //   int iFail = 0 ;
  //   const Gaudi::SymMatrix3x3 nc = nci.Sinverse( iFail ) ;
  //   if ( 0 != iFail ) { iFail  = 0 ; Error ("remove: invaid matrix inversion") ;}
  //   // FIX ME!!!
  //   const Gaudi::Vector3 x     = nc*back.m_ci*back.m_x /** - ... FIX ME */ ;
  //   // FIX ME !!!
  //   const double         nchi2 = back.m_chi2 /** - ... FIX ME!!! */  ;
  
  //   // remove the entry 
  //   m_entries.erase ( ifind ) ;
  
  //   // make few more full iterations 
  //   StatusCode sc  = _iterate ( m_nIterMaxIII , x ) ;
  
  return StatusCode::SUCCESS ;
}
// ============================================================================
// standard constructor 
// ============================================================================
LoKi::VertexFitter::VertexFitter 
( const std::string& type   , 
  const std::string& name   , 
  const IInterface*  parent ) 
  : base_class ( type , name , parent )
/// maximal number of iteration for vertex fit  
  , m_nIterMaxI           ( 10 ) // maximal number of iteration for vertex fit  
/// maximal number of iteration for "add" 
  , m_nIterMaxII          (  5 ) // maximal number of iteration for "add" 
/// maximal number of iteration for "remove"    
  , m_nIterMaxIII         (  5 ) // maximal number of iteration for "remove"    
  , m_DistanceMax         ( 1.0 * Gaudi::Units::micrometer ) 
  , m_DistanceChi2        ( 1.0 * Gaudi::Units::perCent    ) 
  , m_transporterName     ( "ParticleTransporter:PUBLIC")  
  , m_transporter         ( 0 )
  , m_seedZmin            ( -1.5 * Gaudi::Units::meter      ) 
  , m_seedZmax            (  3.0 * Gaudi::Units::meter      ) 
  , m_seedRho             ( 50.0 * Gaudi::Units::centimeter )
/// Use the special branch for   two-body decays 
  , m_use_twobody_branch   ( false   ) // Use the sepcial branch for   two-body decays?
/// Use the special branch for three-body decays 
  , m_use_threebody_branch ( false   ) // Use the sepcial branch for three-body decays?
/// Use the special branch for  four-body decays 
  , m_use_fourbody_branch  ( false   ) // Use the sepcial branch for  four-body decays?
/// The transport tolerance  
  , m_transport_tolerance ( 10 * Gaudi::Units::micrometer ) 
/// number of prints 
  , m_prints              ( 10 )
/// pure technical stuff: 
  , m_entries (   ) 
  , m_vertex  ( 0 )
  , m_seed    (   ) 
  , m_seedci  (   )
  , m_cixy    (   ) 
  , m_cmom    (   ) 
  , m_cmom1   (   ) 
  , m_mpcov   (   ) 
  , m_mm_c    (   )
/// particle classification: 
  , m_ppSvc               (  0      ) 
  , m_longLived           (         ) 
  , m_shortLived          (         ) 
  , m_gammaLike           ( "gamma" )
  , m_gammaCLike          ( Decays::Trees::Invalid_<const LHCb::Particle*>() )
  , m_digammaLike         ( Decays::Trees::Invalid_<const LHCb::Particle*>() )
//
  , m_dd_gammaC  (" gamma -> e+ e- ")
  , m_dd_digamma (" [ ( pi0 -> <gamma> <gamma> ) , ( eta -> <gamma> <gamma> ) ] ")
///
  , m_unclassified  ()
  , m_gamma_like    ()
  , m_gammaC_like   ()
  , m_digamma_like  ()
///
{
  // ==========================================================================
  declareProperty 
    ( "MaxIterations"      , 
      m_nIterMaxI          , 
      "Maximal number of iterations"                     ) ;
  declareProperty
    ( "MaxIterForAdd"      , 
      m_nIterMaxII         , 
      "Maximal number of iterations for 'Add'-method"    ) ;
  declareProperty 
    ( "MaxIterForRemove"   , 
      m_nIterMaxIII        , 
      "Maximal number of iterations for 'Remove'-method" ) ;
  declareProperty 
    ( "DeltaDistance"      , 
      m_DistanceMax        , 
      "Delta-distance as convergency criterion"    ) ;
  declareProperty 
    ( "DeltaChi2"          ,  
      m_DistanceChi2       , 
      "Delta-chi2     as convergency criterion"    ) ;
  declareProperty 
    ( "UseTwoBodyBranch"     , 
      m_use_twobody_branch   , 
      "Use the special branch for   two-body decays" ) ;
  declareProperty 
    ( "UseThreeBodyBranch"   , 
      m_use_threebody_branch , 
      "Use the special branch for three-body decays" ) ;
  declareProperty 
    ( "UseFourBodyBranch"    , 
      m_use_fourbody_branch  , 
      "Use the special branch for  four-body decays" ) ;
  declareProperty 
    ( "UseTwoBodyBranch"   , 
      m_use_twobody_branch , 
      "Use the special branch for two-body decays" ) ;
  declareProperty 
    ( "TransportTolerance"  , 
      m_transport_tolerance , 
      "The tolerance for particle transport" ) ;
  // ========================================================================== 
  declareProperty 
    ( "MaxPrints"        , 
      m_prints           , 
      "Maximal number of prints "        ) ;
  // ===========================================================================
  declareProperty 
    ( "SeedZmin"         , 
      m_seedZmin         ,
      "Allowed Z-min   for seed-vertex "       ) ;
  declareProperty 
    ( "SeedZmax"         ,
      m_seedZmax         , 
      "Allowed Z-max   for seed-vertex "       ) ;
  declareProperty 
    ( "SeedRho"          , 
      m_seedRho          ,
      "Allowed Rho-max for seed-vertex "       ) ;
  declareProperty 
    ( "Transporter"      , 
      m_transporterName  , 
      "The typename of tranporter to bee used" ) ;
  // ==========================================================================
  declareProperty 
    ( "GammaCDecays"  , 
      m_dd_gammaC     , 
      "The gammaC-decays" ) ;
  declareProperty 
    ( "DiGammaDecays" , 
      m_dd_digamma    , 
      "The di-gamma-decays" ) ;
  // ==========================================================================
} 
// ============================================================================
// desctructor 
// ============================================================================
LoKi::VertexFitter::~VertexFitter(){}
// ============================================================================
// the standard initialization of the tool 
// ============================================================================
StatusCode LoKi::VertexFitter::initialize() 
{
  StatusCode sc = GaudiTool::initialize() ;
  if ( sc.isFailure() ) { return sc ; }
  svc<IService>( "LoKiSvc" , true ) ;
  
  // get particle property service 
  m_ppSvc = svc<LHCb::IParticlePropertySvc> ( "LHCb::ParticlePropertySvc" , true );
  // validate  
  sc = m_longLived.validate  ( m_ppSvc ) ;
  if ( sc.isFailure() ) 
  { return Error ( "Unable to validate Long-Lived  particles" , sc ) ; }
  sc = m_shortLived.validate ( m_ppSvc ) ;
  if ( sc.isFailure() ) 
  { return Error ( "Unable to validate Short-Lived particles" , sc ) ; }
  sc = m_gammaLike.validate  ( m_ppSvc ) ;
  if ( sc.isFailure() ) 
  { return Error ( "Unable to validate Gamma-Like  particles" , sc ) ; }
  //
  const StatusCode ok = StatusCode( StatusCode::SUCCESS , true ) ;
  //
  // GammaC ? 
  //
  if ( !m_dd_gammaC.empty() )
  { 
    // construct gamma_c descriptor
    Decays::IDecay* decay = tool<Decays::IDecay> ( "LoKi::Decay/Decays" ) ;
    //
    m_gammaCLike =  decay->tree ( m_dd_gammaC ) ;
    if ( !m_gammaCLike.valid() )   
    { return Error ( "Unable to decode Gamma_c: '" + m_dd_gammaC  + "'" ) ; }
    //
    debug () << " Gamma_c  descriptor : " << m_gammaCLike << endmsg ;
    //
    release ( decay ) ;
  }
  else 
  {
    // disable gamma_c treatment
    m_gammaCLike = Decays::Trees::Any_<const LHCb::Particle*>() ;
    m_gammaCLike = Decays::Trees::Not_<const LHCb::Particle*>( m_gammaCLike ) ;
    Warning ( "The special treatment of Gamma_c is disabled" , ok ) ;
  }
  //
  // DiGamma  ? 
  //
  if ( !m_dd_digamma.empty() )
  { // construct digamma descriptors
    Decays::IDecay* decay = tool<Decays::IDecay> ( "LoKi::Decay/Decays" ) ;
    //
    m_digammaLike =  decay->tree ( m_dd_digamma ) ;
    if ( !m_digammaLike.valid() ) 
    { return Error ( "Unable to decode DiGamma: '" + m_dd_digamma + "'" ) ; }
    //
    debug () << " Di-Gamma descriptor : " << m_digammaLike << endmsg ;
    //
    release ( decay ) ;
  }
  else 
  {
    // disable gamma_c treatment
    m_digammaLike = Decays::Trees::Any_<const LHCb::Particle*>() ;
    m_digammaLike = Decays::Trees::Not_<const LHCb::Particle*>( m_digammaLike ) ;
    Warning ( "The special treatment of DiGamma is disabled" , ok ) ;
  }
  //
  if ( msgLevel ( MSG::DEBUG ) &&  0 == m_prints ) 
  {
    m_prints = 10 ;
    warning () << "Redefine 'MaxPrints' property to " << m_prints << endmsg ;
  }
  //
  return StatusCode::SUCCESS ;
}
// ========================================================================
// the standard finalization of the tool 
// ========================================================================
StatusCode LoKi::VertexFitter::finalize() 
{
  // reset particle property service & functors 
  if ( msgLevel ( MSG::DEBUG ) ) 
  {
    MsgStream& log = debug () ;
    if ( !m_shortLived.accepted().empty() ) 
    {
      log << "Short-Lived particles : " << std::endl ;
      LHCb::ParticleProperties::printAsTable ( m_shortLived.accepted () , log , m_ppSvc ) ;
      log << endmsg ;
    }
    if ( !m_longLived.accepted().empty() ) 
    {
      log << "Long-Lived  particles : " << std::endl ;
      LHCb::ParticleProperties::printAsTable ( m_longLived .accepted () , log , m_ppSvc ) ;
      log << endmsg ;
    }
    //
    // Gamma
    if ( !m_gamma_like.empty() ) 
    {
      std::vector<LHCb::ParticleID> parts ( m_gamma_like.begin() , 
                                            m_gamma_like.end  () ) ;
      
      log << "Gamma-like   particles : " << m_gammaLike << std::endl ;
      LHCb::ParticleProperties::printAsTable ( parts , log , m_ppSvc ) ;
      log << endmsg ;
    }
    // GammaC 
    if ( !m_gammaC_like.empty() ) 
    {
      std::vector<LHCb::ParticleID> parts ( m_gammaC_like.begin() , 
                                            m_gammaC_like.end  () ) ;
      
      log << "GammaC-like   particles : " << m_gammaCLike << std::endl ;
      LHCb::ParticleProperties::printAsTable ( parts , log , m_ppSvc ) ;
      log << endmsg ;
    }
    // DiGamma 
    if ( !m_digamma_like.empty() ) 
    {
      std::vector<LHCb::ParticleID> parts ( m_digamma_like.begin() , 
                                            m_digamma_like.end  () ) ;
      
      log << "DiGamma-like  particles : " << m_digammaLike << std::endl ;
      LHCb::ParticleProperties::printAsTable ( parts , log , m_ppSvc ) ;
      log << endmsg ;
    }
    //
  }
  //
  if ( !m_unclassified.empty() ) 
  {
    MsgStream& log = warning() ;
    std::vector<LHCb::ParticleID> tmp ( m_unclassified.begin() , 
                                        m_unclassified.end  () ) ;
    log << "Unclassified particles : " << std::endl ;
    LHCb::ParticleProperties::printAsTable ( tmp , log , m_ppSvc ) ;
    log << endmsg ;      
  }
  // 
  m_ppSvc = 0 ;
  m_shortLived . setService ( m_ppSvc ) ;
  m_longLived  . setService ( m_ppSvc ) ;
  m_gammaCLike   = Decays::Trees::Invalid_<const LHCb::Particle*>() ;
  m_digammaLike  = Decays::Trees::Invalid_<const LHCb::Particle*>() ;
  //
  return GaudiTool::finalize () ;
}
// ============================================================================
// get the particle type 
// ============================================================================
LoKi::KalmanFilter::ParticleType 
LoKi::VertexFitter::particleType ( const LHCb::Particle* p ) const 
{
  //
  if      ( 0 == p ) 
  { return LoKi::KalmanFilter::UnspecifiedParticle ; }  // RETURN 
  else if ( m_gammaCLike ( p ) )
  { 
    m_gammaC_like.insert ( p->particleID () ) ;
    // ATTENTION! GammaC is *LONG_LIVED_PARTICLE*
    return LoKi::KalmanFilter::LongLivedParticle   ;    // RETURN 
  } 
  else if ( m_digammaLike ( p ) )
  { 
    m_digamma_like.insert ( p->particleID () ) ;   
    return LoKi::KalmanFilter::DiGammaLikeParticle ;    // RETURN 
  } 
  else if ( m_gammaLike   ( p ) )
  { 
    m_gamma_like.insert   ( p->particleID () ) ;   
    return LoKi::KalmanFilter::GammaLikeParticle   ;    // RETURN
  }
  else if ( m_longLived  ( p->particleID () ) ) 
  { return LoKi::KalmanFilter::LongLivedParticle   ; }  // RETURN 
  else if ( m_shortLived ( p->particleID () ) )
  { return LoKi::KalmanFilter::ShortLivedParticle  ; }  // RETURN 
  //
  m_unclassified.insert ( p->particleID() ) ;
  //
  return LoKi::KalmanFilter::UnspecifiedParticle ;
}
// ============================================================================
// transport the data to a certain position 
// ============================================================================
StatusCode LoKi::VertexFitter::_transport 
( LoKi::VertexFitter::Entry& entry , 
  const Gaudi::XYZPoint&     point ) const 
{
  const double refPointZ = entry.m_p.referencePoint().Z() ;
  const double parZ      = entry.m_parx [2]               ;
  //
  // for short-lived particles one needs to transport them into their decay vertex
  if ( LoKi::KalmanFilter::ShortLivedParticle == entry.m_type )
  {
    // get the end-vertex 
    const LHCb::VertexBase* vertex = entry.m_p0->endVertex() ;
    if ( 0 != vertex ) 
    {
      const double vertexZ = vertex->position().Z() ;
      // already properly transported ? 
      if ( std::fabs ( refPointZ - vertexZ ) < m_transport_tolerance && 
           std::fabs ( parZ      - vertexZ ) < m_transport_tolerance ) 
      { return StatusCode::SUCCESS ; }                                // RETURN 
      // else : transport them into own vertex 
      return LoKi::KalmanFilter::transport 
        ( entry , vertex->position() , transporter () ) ;             // RETURN
    }
  }
  // no need for transport?
  if ( std::fabs ( refPointZ - point.Z () ) < m_transport_tolerance && 
       std::fabs ( parZ      - point.Z () ) < m_transport_tolerance  ) 
  { return StatusCode::SUCCESS ; }                                    // RETURN 
  // finally: transport it! 
  return LoKi::KalmanFilter::transport ( entry , point , transporter() ) ; 
}  
// ============================================================================
/// the factory needed for instantiation
DECLARE_NAMESPACE_TOOL_FACTORY ( LoKi , VertexFitter ) ;
// ============================================================================

// ============================================================================
// The END 
// ============================================================================

