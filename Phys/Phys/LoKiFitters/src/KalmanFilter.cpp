// $Id: KalmanFilter.cpp,v 1.4 2010-01-04 16:50:56 ibelyaev Exp $
// ============================================================================
// Include files 
// ============================================================================
// GaudiKernel
// ============================================================================
// #include "GaudiKernel/SystemOfUnits.h"
// ============================================================================
// DaVinciKernel
// ============================================================================
#include "Kernel/IParticleTransporter.h"
// ============================================================================
// LHCbMath
// ============================================================================
#include "LHCbMath/Power.h"
#include "LHCbMath/MatrixUtils.h"
#include "LHCbMath/MatrixTransforms.h"
// ============================================================================
// Local
// ============================================================================
#include "KalmanFilter.h"
// ============================================================================
// CaloUtils 
// ============================================================================
#include "CaloUtils/CaloMomentum.h"
// ============================================================================
// LoKi
// ============================================================================
#include "LoKi/Child.h"
// ============================================================================
/** @file 
 *  Implementation file for functions from namespace LoKi::KalmanFilter
 *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
 *  @date 2008-03-06
 */
// ========================================================================-===
// the purely internal program 
// ============================================================================
namespace 
{
  // ==========================================================================
  /// inverse "large" error in position: used to avoid singularity 
  const double s_ERROR2_i =  1.0 / Gaudi::Math::pow ( 20.0 * Gaudi::Units::cm , 2 ) ;
  // ==========================================================================
  /// "smooth" the singular values 
  template <class T, unsigned int D>
  inline 
  void _smooth 
  ( ROOT::Math::SMatrix<T,D,D,ROOT::Math::MatRepSym<T,D> >& mtrx       , 
    const double                                            error = -1 ) 
  {
    //
    const double error2_i = 
      error <= 0.0 ? 
      s_ERROR2_i   :
      std::min ( 1.0 / Gaudi::Math::pow ( error , 2 ) , s_ERROR2_i ) ;
    //
    for ( unsigned i = 0 ; i < D ; ++i ) { mtrx(i,i) += error2_i ; }
  }
  // ==========================================================================
  /** make Z-projection of the particle 
   *  see the documentation for namespace DaVinciTransporter
   *  projectAndTransport for deltaZ = 0 
   */
  void _project_Z_ (  LoKi::KalmanFilter::Entry& entry ) 
  {
    //
    const Gaudi::LorentzVector& p = entry.m_p.momentum        () ;
    const Gaudi::SymMatrix3x3&  c = entry.m_p.posCovMatrix    () ;
    const Gaudi::Matrix4x3&     d = entry.m_p.posMomCovMatrix () ;
    //
    const double tx = p.Px () / p.Pz() ;
    const double ty = p.Py () / p.Pz() ;
    //
    const double k1 = c ( 0 , 2 ) ;
    const double k2 = c ( 1 , 2 ) ;  
    const double cz = c ( 2 , 2 ) ;
    //
    // dirty trick # 1 !! 
    Gaudi::SymMatrix3x3& _c = const_cast<Gaudi::SymMatrix3x3&>( c ) ;
    //
    // x-y 2x2 subblock
    _c ( 0 , 0 ) +=  tx * tx * cz - ( tx * k1 + tx * k1 ) ;
    _c ( 0 , 1 ) +=  tx * ty * cz - ( tx * k2 + ty * k1 ) ;
    _c ( 1 , 1 ) +=  ty * ty * cz - ( ty * k2 + ty * k2 ) ;
    //
    // z-row/column
    _c ( 0 , 2 ) = 0  ;
    _c ( 1 , 2 ) = 0  ;
    _c ( 2 , 2 ) = 0  ;
    //
    // dirty trick # 2 !!    
    Gaudi::Matrix4x3& _d = const_cast<Gaudi::Matrix4x3&> ( d ) ;
    //
    double _a = _d ( 0 , 2 ) ;
    _d ( 0 , 0 ) -= _a * tx ;
    _d ( 0 , 1 ) -= _a * ty ;
    _d ( 0 , 2 )  = 0 ;
    //
    _a = _d ( 1 , 2 ) ;
    _d ( 1 , 0 ) -= _a * tx ;
    _d ( 1 , 1 ) -= _a * ty ;
    _d ( 1 , 2 )  = 0 ;
    //
    _a = _d ( 2 , 2 ) ;
    _d ( 2 , 0 ) -= _a * tx ;
    _d ( 2 , 1 ) -= _a * ty ;
    _d ( 2 , 2 )  = 0 ;
    //
    _a = _d ( 3 , 2 ) ;
    _d ( 3 , 0 ) -= _a * tx ;
    _d ( 3 , 1 ) -= _a * ty ;
    _d ( 3 , 2 )  = 0 ;
    //
  }
  // ==========================================================================
  /** "update" the entry and get the valid "gain" matrix
   *  
   *   @todo KalmanFilter::_update, add the treatment of Gamma-like particles 
   *
   *   @param entry     the entry to be updated 
   *   @param asFlying  treat the particle as "flying-particle"
   *   @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
   *   @date 2008-03-06
   */
  StatusCode _update ( LoKi::KalmanFilter::Entry&       entry , 
                       LoKi::KalmanFilter::ParticleType type  ) 
  {
    //
    // make the proper projection (if required) 
    if ( LoKi::KalmanFilter::LongLivedParticle == type ) 
    { _project_Z_ ( entry ) ; }
    //
    const Gaudi::SymMatrix3x3& _pmcov = entry.m_p.posCovMatrix() ;
    //
    if ( LoKi::KalmanFilter::GammaLikeParticle  == type ) 
    {
      Gaudi::Math::setToScalar ( entry.m_vxi , 0.0 ) ;
    }
    else if ( LoKi::KalmanFilter::DiGammaLikeParticle  == type ) 
    {
      Gaudi::Math::setToScalar ( entry.m_vxi , 0.0 ) ;      
    }
    else if ( LoKi::KalmanFilter::LongLivedParticle == type  
              || 
              _pmcov ( 2 , 2 ) < 0.25 * ( _pmcov ( 0 , 0 ) + _pmcov ( 1 , 1 ) ) )
    {
      Gaudi::SymMatrix2x2 cixy ;
      // basic particle? : use some tricks to 
      cixy ( 0 , 0 ) = _pmcov ( 0, 0 ) ;
      cixy ( 0 , 1 ) = _pmcov ( 0, 1 ) ;
      cixy ( 1 , 1 ) = _pmcov ( 1, 1 ) ;
      if ( !cixy.Invert() )  
      { return StatusCode 
          ( LoKi::KalmanFilter::ErrorInMatrixInversion1 , true ) ; }  // RETURN 
      //
      // The most tricky part I
      entry.m_vxi ( 0 , 0 ) = cixy ( 0 , 0 ) ;
      entry.m_vxi ( 0 , 1 ) = cixy ( 0 , 1 ) ;
      entry.m_vxi ( 1 , 1 ) = cixy ( 1 , 1 ) ;
      //
      // The most tricky part II 
      const Gaudi::LorentzVector& mom = entry.m_p.momentum() ;
      const Gaudi::Vector2 slopes ( mom.Px() / mom.Pz() , mom.Py() / mom.Pz() ) ;
      const Gaudi::Vector2 cslope ( cixy * slopes )  ;
      entry.m_vxi ( 0 , 2 ) = -1 * cslope ( 0 ) ;
      entry.m_vxi ( 1 , 2 ) = -1 * cslope ( 1 ) ;
      entry.m_vxi ( 2 , 2 ) = ROOT::Math::Similarity ( slopes , cixy ) ;
      //
      // REDEFINE THE PARTICLE TYPE 
      entry.m_type = LoKi::KalmanFilter::LongLivedParticle ;
      //
    }
    else // ShortLived Particle 
    {
      // the regular particle:
      entry.m_vxi = _pmcov ;
      if ( !entry.m_vxi.Invert() )
      {
        /// remove singularities 
        entry.m_vxi = _pmcov ;
        _smooth ( entry.m_vxi ) ;
        if ( !entry.m_vxi.Invert() )
        { return StatusCode 
            ( LoKi::KalmanFilter::ErrorInMatrixInversion2 , true ) ; } // RETURN
      }
      //
    }
    // 
    Gaudi::Math::geo2LA      ( entry.m_p.referencePoint () , entry.m_parx ) ;
    Gaudi::Math::geo2LA      ( entry.m_p.momentum       () , entry.m_parq ) ;
    //
    return StatusCode::SUCCESS ;
  }
  // ==========================================================================
} //                                                 end of anonymous namespace
// ============================================================================
// Load the particle into "entry" representation"
// ============================================================================
StatusCode LoKi::KalmanFilter::load 
( const LHCb::Particle&      particle , 
  LoKi::KalmanFilter::Entry& entry    ) 
{ 
  entry.m_type = LoKi::KalmanFilter::UnspecifiedParticle ;
  entry.m_p0   = &particle ;
  entry.m_p    =  particle ;
  //
  return _update ( entry , entry.m_type ) ;
}
// ============================================================================
// Load the particle into "entry" representation"
// ============================================================================
StatusCode LoKi::KalmanFilter::loadAsFlying
( const LHCb::Particle&      particle , 
  LoKi::KalmanFilter::Entry& entry    ) 
{ 
  entry.m_type = LoKi::KalmanFilter::LongLivedParticle ;
  entry.m_p0   = &particle ;
  entry.m_p    =  particle ;
  //
  return _update ( entry , entry.m_type ) ;
}
// ============================================================================
// Load the particle into "entry" representation"
// ============================================================================
StatusCode LoKi::KalmanFilter::loadAsShortLived
( const LHCb::Particle&      particle , 
  LoKi::KalmanFilter::Entry& entry    ) 
{ 
  entry.m_type = LoKi::KalmanFilter::ShortLivedParticle ;
  entry.m_p0   = &particle ;
  entry.m_p    =  particle ;
  //
  return _update ( entry , entry.m_type ) ;
}
// ============================================================================
// Load the particle into "entry" representation"
// ============================================================================
StatusCode LoKi::KalmanFilter::loadAsGamma
( const LHCb::Particle&      particle , 
  LoKi::KalmanFilter::Entry& entry    ) 
{ 
  entry.m_type = LoKi::KalmanFilter::GammaLikeParticle ;
  entry.m_p0   = &particle ;
  entry.m_p    =  particle ;
  //
  return _update ( entry , entry.m_type ) ;
}
// ============================================================================
// Load the particle into "entry" representation"
// ============================================================================
StatusCode LoKi::KalmanFilter::loadAsDiGamma
( const LHCb::Particle&      particle , 
  LoKi::KalmanFilter::Entry& entry    ) 
{ 
  entry.m_type = LoKi::KalmanFilter::DiGammaLikeParticle ;
  entry.m_p0   = &particle ;
  entry.m_p    =  particle ;
  //
  return _update ( entry , entry.m_type ) ;
}
// ============================================================================
// transport the particle and update the entry
// ============================================================================
StatusCode LoKi::KalmanFilter::transport 
( LoKi::KalmanFilter::Entry& entry    , 
  const double               newZ     , 
  IParticleTransporter*      tool     ) 
{
  if ( 0 == tool ) 
  { return StatusCode ( ErrorFromParticleTransporter , true )  ; }
  // 
  StatusCode sc = tool -> transport ( entry.m_p0 , newZ , entry.m_p ) ;
  if ( sc.isFailure() ) { entry.m_p = *entry.m_p0 ; }   
  //
  return _update ( entry , entry.m_type ) ;
}
// ============================================================================
// make one step of Kalman filter 
// ============================================================================
StatusCode LoKi::KalmanFilter::step 
( LoKi::KalmanFilter::Entry&  entry , 
  const Gaudi::XYZPoint&      x     , 
  const Gaudi::SymMatrix3x3&  ci    , 
  const double                chi2  ) 
{
  const Gaudi::Vector3 xx ( x.X () , x.Y() , x.Z() ) ;
  return step ( entry , xx , ci , chi2 ) ;
}
// ============================================================================
// make one step of Kalman filter q
// ============================================================================
StatusCode LoKi::KalmanFilter::step 
( LoKi::KalmanFilter::Entry&  entry , 
  const Gaudi::Vector3&       x     , 
  const Gaudi::SymMatrix3x3&  ci    , 
  const double                chi2  ) 
{
  // the special case :  gamma and digamma
  if ( entry.special () )
  {
    entry.m_x    = x    ;
    entry.m_ci   = ci   ;
    entry.m_chi2 = chi2 ;
    //
    // OK ! 
    int ifail = 0 ;
    /// \f$ C_k = \left( C^{-1}_{k} \right)^{-1}\f$ 
    entry.m_c  = entry.m_ci.Inverse( ifail ) ; 
    /// try to recover it by "soft" constraint to the photon  
    if ( 0 != ifail ) 
    {
      // try to recover it by "soft" constraint to the photon  
      entry.m_ci = ci ;
      _smooth ( entry.m_ci ) ;
      // invert it : 
      ifail = 0 ;
      entry.m_c  = entry.m_ci.Inverse( ifail ) ; 
      //
      if ( 0 != ifail ) 
      { return StatusCode ( ErrorInMatrixInversion3 , true ) ; } // RETURN 
    }
    //
    return StatusCode::SUCCESS ;
  }
  // regular case: 
  // OK !
  /// \f$ C^{-1}_k=C^{-1}_{k-1}+A^TG_kA =  C^{-1}_{k-1}+ V^{-1}_{k} \f$
  entry.m_ci = ci + entry.m_vxi  ; 
  //
  // OK ! 
  int ifail = 0 ;
  /// \f$ C_k = \left( C^{-1}_{k} \right)^{-1}\f$ 
  entry.m_c  = entry.m_ci.Inverse( ifail ) ; 
  if ( 0 != ifail ) 
  { return StatusCode ( ErrorInMatrixInversion3 , true ) ; }
  // OK ! 
  /// \f$\vec{x}_k\f$
  entry.m_x = entry.m_c * ( ci*x + entry.m_vxi * entry.m_parx  ) ; 
  // OK ! 
  const Gaudi::Vector3 dx = entry.m_parx - entry.m_x ;  
  // OK !
  entry.m_q = entry.m_parq - entry.m_p.posMomCovMatrix() * entry.m_vxi * dx ; 
  // OK ! 
  const double dchi2 = 
    ROOT::Math::Similarity ( entry.m_vxi  , dx            ) + 
    ROOT::Math::Similarity ( ci           , entry.m_x - x ) ;
  //
  // update chi2 
  entry.m_chi2 = chi2 + dchi2 ;
  //
  return StatusCode::SUCCESS ;
}
// ============================================================================
/*  make one step of Kalman filter (similar to seeding)
 *  @param entry1 (update)       measurements to be updated 
 *  @param entry2 (update)       measurements to be updated 
 *  @param chi2  (input)        the initial chi2 
 *  @return status code 
 *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
 *  @date 2008-03-06
 */
// ============================================================================
StatusCode LoKi::KalmanFilter::step 
( LoKi::KalmanFilter::Entry&  entry1   ,
  LoKi::KalmanFilter::Entry&  entry2   , 
  const double                chi2     ) 
{
  //
  if ( entry1.m_type != LongLivedParticle  && 
       entry1.m_type != ShortLivedParticle  ) 
  { return StatusCode ( ErrorInInputData , true ) ; }
  //
  if ( entry2.m_type != LongLivedParticle  && 
       entry2.m_type != ShortLivedParticle  ) 
  { return StatusCode ( ErrorInInputData , true ) ; }
  //
  entry1.m_ci = entry1.m_vxi + entry2.m_vxi ; 
  entry2.m_ci = entry1.m_ci ;
  //
  /// \f$ C_k = \left( C^{-1}_{k} \right)^{-1}\f$ 
  int ifail = 0 ;
  entry1.m_c  = entry1.m_ci.Inverse( ifail ) ; 
  if ( 0 != ifail ) { return StatusCode ( ErrorInMatrixInversion3 , true ) ; }
  entry2.m_c  = entry1.m_c ;
  
  /// \f$\vec{x}_k\f$
  entry1.m_x = entry1.m_c * ( entry1.m_vxi * entry1.m_parx + 
                              entry2.m_vxi * entry2.m_parx ) ;
  
  entry2.m_x = entry1.m_x ;
  // OK ! 
  const Gaudi::Vector3 dx1 = entry1.m_parx - entry1.m_x ;  
  entry1.m_q = entry1.m_parq - entry1.m_p.posMomCovMatrix() * entry1.m_vxi * dx1 ; 
  // OK !
  const Gaudi::Vector3 dx2 = entry2.m_parx - entry2.m_x ;  
  entry2.m_q = entry2.m_parq - entry2.m_p.posMomCovMatrix() * entry2.m_vxi * dx2 ; 
  // OK ! 
  const double dchi2_1 = ROOT::Math::Similarity ( entry1.m_vxi , dx1 ) ;
  //
  // update chi2 
  entry1.m_chi2 = chi2          + dchi2_1 ;
  //
  const double dchi2_2 = ROOT::Math::Similarity ( entry2.m_vxi , dx2 ) ;
  //
  entry2.m_chi2 = entry1.m_chi2 + dchi2_2 ;
  //
  return StatusCode::SUCCESS ;
}
// ============================================================================
/*  make one step of Kalman filter (similar to seeding)
 *  @param entry1 (update)       measurements to be updated 
 *  @param entry2 (update)       measurements to be updated 
 *  @param entry3 (update)       measurements to be updated 
 *  @param chi2   (input)        the initial chi2 
 *  @return status code 
 *  @author Vanya BELYAEV Ivan.Belyaev@cern.ch
 *  @date 2010-11-10
 */
// ============================================================================
StatusCode LoKi::KalmanFilter::step 
( LoKi::KalmanFilter::Entry&  entry1   ,
  LoKi::KalmanFilter::Entry&  entry2   , 
  LoKi::KalmanFilter::Entry&  entry3   , 
  const double                chi2     ) 
{
  //
  if ( entry1.m_type != LongLivedParticle  && 
       entry1.m_type != ShortLivedParticle  ) 
  { return StatusCode ( ErrorInInputData , true ) ; }
  //
  if ( entry2.m_type != LongLivedParticle  && 
       entry2.m_type != ShortLivedParticle  ) 
  { return StatusCode ( ErrorInInputData , true ) ; }
  //
  if ( entry3.m_type != LongLivedParticle  && 
       entry3.m_type != ShortLivedParticle  ) 
  { return StatusCode ( ErrorInInputData , true ) ; }
  //
  entry1.m_ci  = entry1.m_vxi ;
  entry1.m_ci += entry2.m_vxi ; 
  entry1.m_ci += entry3.m_vxi ; 
  //
  entry2.m_ci = entry1.m_ci ;
  entry3.m_ci = entry1.m_ci ;
  //
  /// \f$ C_k = \left( C^{-1}_{k} \right)^{-1}\f$ 
  int ifail = 0 ;
  entry1.m_c  = entry1.m_ci.Inverse( ifail ) ; 
  if ( 0 != ifail ) { return StatusCode ( ErrorInMatrixInversion3 , true ) ; }
  entry2.m_c  = entry1.m_c ;
  entry3.m_c  = entry1.m_c ;
  
  /// \f$\vec{x}_k\f$
  entry1.m_x = entry1.m_c * ( entry1.m_vxi * entry1.m_parx + 
                              entry2.m_vxi * entry2.m_parx +
                              entry3.m_vxi * entry3.m_parx ) ;
  
  entry2.m_x = entry1.m_x ;
  entry3.m_x = entry1.m_x ;
  // OK ! 
  const Gaudi::Vector3 dx1 = entry1.m_parx - entry1.m_x ;  
  entry1.m_q = entry1.m_parq - entry1.m_p.posMomCovMatrix() * entry1.m_vxi * dx1 ; 
  // OK !
  const Gaudi::Vector3 dx2 = entry2.m_parx - entry2.m_x ;  
  entry2.m_q = entry2.m_parq - entry2.m_p.posMomCovMatrix() * entry2.m_vxi * dx2 ; 
  // OK ! 
  const Gaudi::Vector3 dx3 = entry3.m_parx - entry3.m_x ;  
  entry3.m_q = entry3.m_parq - entry3.m_p.posMomCovMatrix() * entry3.m_vxi * dx3 ; 

  //
  // update chi2 
  //
  const double dchi2_1 = ROOT::Math::Similarity ( entry1.m_vxi , dx1 ) ;
  entry1.m_chi2 = chi2          + dchi2_1 ;
  //
  const double dchi2_2 = ROOT::Math::Similarity ( entry2.m_vxi , dx2 ) ;
  entry2.m_chi2 = entry1.m_chi2 + dchi2_2 ;
  //
  const double dchi2_3 = ROOT::Math::Similarity ( entry3.m_vxi , dx3 ) ;
  entry3.m_chi2 = entry2.m_chi2 + dchi2_3 ;
  //
  return StatusCode::SUCCESS ;
}
// ============================================================================
/*  make one step of Kalman filter (similar to seeding)
 *  @param entry1 (update)       measurements to be updated 
 *  @param entry2 (update)       measurements to be updated 
 *  @param entry3 (update)       measurements to be updated 
 *  @param entry4 (update)       measurements to be updated 
 *  @param chi2   (input)        the initial chi2 
 *  @return status code 
 *  @author Vanya BELYAEV Ivan.Belyaev@cern.ch
 *  @date 2010-11-10
 */
// ============================================================================
StatusCode LoKi::KalmanFilter::step 
( LoKi::KalmanFilter::Entry&  entry1   ,
  LoKi::KalmanFilter::Entry&  entry2   , 
  LoKi::KalmanFilter::Entry&  entry3   , 
  LoKi::KalmanFilter::Entry&  entry4   , 
  const double                chi2     ) 
{
  //
  if ( entry1.m_type != LongLivedParticle  && 
       entry1.m_type != ShortLivedParticle  ) 
  { return StatusCode ( ErrorInInputData , true ) ; }
  //
  if ( entry2.m_type != LongLivedParticle  && 
       entry2.m_type != ShortLivedParticle  ) 
  { return StatusCode ( ErrorInInputData , true ) ; }
  //
  if ( entry3.m_type != LongLivedParticle  && 
       entry3.m_type != ShortLivedParticle  ) 
  { return StatusCode ( ErrorInInputData , true ) ; }
  //
  if ( entry4.m_type != LongLivedParticle  && 
       entry4.m_type != ShortLivedParticle  ) 
  { return StatusCode ( ErrorInInputData , true ) ; }
  //
  entry1.m_ci  = entry1.m_vxi ;
  entry1.m_ci += entry2.m_vxi ; 
  entry1.m_ci += entry3.m_vxi ; 
  entry1.m_ci += entry4.m_vxi ; 
  //
  entry2.m_ci = entry1.m_ci ;
  entry3.m_ci = entry1.m_ci ;
  entry4.m_ci = entry1.m_ci ;
  //
  /// \f$ C_k = \left( C^{-1}_{k} \right)^{-1}\f$ 
  int ifail = 0 ;
  entry1.m_c  = entry1.m_ci.Inverse( ifail ) ; 
  if ( 0 != ifail ) { return StatusCode ( ErrorInMatrixInversion3 , true ) ; }
  entry2.m_c  = entry1.m_c ;
  entry3.m_c  = entry1.m_c ;
  entry4.m_c  = entry1.m_c ;
  
  /// \f$\vec{x}_k\f$
  entry1.m_x = entry1.m_c * ( entry1.m_vxi * entry1.m_parx + 
                              entry2.m_vxi * entry2.m_parx +
                              entry3.m_vxi * entry3.m_parx ) ;
  
  entry2.m_x = entry1.m_x ;
  entry3.m_x = entry1.m_x ;
  entry4.m_x = entry1.m_x ;
  // OK ! 
  const Gaudi::Vector3 dx1 = entry1.m_parx - entry1.m_x ;  
  entry1.m_q = entry1.m_parq - entry1.m_p.posMomCovMatrix() * entry1.m_vxi * dx1 ; 
  // OK !
  const Gaudi::Vector3 dx2 = entry2.m_parx - entry2.m_x ;  
  entry2.m_q = entry2.m_parq - entry2.m_p.posMomCovMatrix() * entry2.m_vxi * dx2 ; 
  // OK ! 
  const Gaudi::Vector3 dx3 = entry3.m_parx - entry3.m_x ;  
  entry3.m_q = entry3.m_parq - entry3.m_p.posMomCovMatrix() * entry3.m_vxi * dx3 ; 
  // OK ! 
  const Gaudi::Vector3 dx4 = entry4.m_parx - entry4.m_x ;  
  entry4.m_q = entry4.m_parq - entry4.m_p.posMomCovMatrix() * entry4.m_vxi * dx4 ; 
  
  //
  // update chi2 
  //
  const double dchi2_1 = ROOT::Math::Similarity ( entry1.m_vxi , dx1 ) ;
  entry1.m_chi2 = chi2          + dchi2_1 ;
  //
  const double dchi2_2 = ROOT::Math::Similarity ( entry2.m_vxi , dx2 ) ;
  entry2.m_chi2 = entry1.m_chi2 + dchi2_2 ;
  //
  const double dchi2_3 = ROOT::Math::Similarity ( entry3.m_vxi , dx3 ) ;
  entry3.m_chi2 = entry2.m_chi2 + dchi2_3 ;
  //
  const double dchi2_4 = ROOT::Math::Similarity ( entry4.m_vxi , dx4 ) ;
  entry4.m_chi2 = entry3.m_chi2 + dchi2_4 ;
  //
  return StatusCode::SUCCESS ;
}
// ============================================================================
// kalman smoothing  
// ============================================================================
StatusCode LoKi::KalmanFilter::smooth
( LoKi::KalmanFilter::Entries& entries ) 
{
  if ( entries.empty() ) { return StatusCode::FAILURE ; }
  //
  const Entry& last = entries.back() ;
  for ( Entries::iterator entry = entries.begin() ; 
        entries.end() != entry ; ++entry ) 
  {
    /// \f$ \vec{x}^{n}_k = \vec{x}_{n}\f$ 
    entry -> m_x  = last.m_x ;
    //
    // gamma & digamma 
    //
    if ( entry->special() ) 
    {
      // the simplest way to calculate entry->m_q
      StatusCode sc = transportGamma ( *entry , entry->m_x , &last.m_c ) ;    
      if ( sc.isFailure() ) 
      { entry -> m_q = entry->m_parq ; continue ; }               // CONTINUE 
      //
      Gaudi::Math::geo2LA ( entry->m_p.momentum () , entry->m_q ) ;
      //
      continue ;                                                  // CONTINUE 
    }
    //
    // regular case: 
    // 
    const Gaudi::Vector3 dx = entry->m_parx - entry->m_x ;
    /// \f$ \vec{q}^{n}_k = W_kB^T_{k}G_k\left[\vec{p}_k-A_k\vec{x}_{n}\right]\f$ 
    entry -> m_q = entry -> m_parq 
      - entry -> m_p.posMomCovMatrix() * entry -> m_vxi * dx ; 
  }
  return StatusCode::SUCCESS ;
} 
// ============================================================================
// evaluate all covariances properly 
// ============================================================================
StatusCode LoKi::KalmanFilter::evalCov 
( LoKi::KalmanFilter::Entries& entries ) 
{
  if ( entries.empty() ) { return StatusCode::FAILURE ; }
  //
  using namespace ROOT::Math ;
  //
  const Entry& last = entries.back() ;
  for ( Entries::iterator entry = entries.begin() ; 
        entries.end() != entry ; ++entry ) 
  {
    /// \f$ C^n_k = C_n \f$  
    entry -> m_c = last.m_c ;
    // 
    if ( entry->regular() ) 
    {
      /// \f$ F_k = G_{p}^{-1}G^T{xp} = - V^T_{xp}V^{-1}_x \f$ 
      entry -> m_f = -1.0 * entry->m_p.posMomCovMatrix() * entry->m_vxi   ;
      /// \f$ E_k = - F_k C_n \f$ 
      entry -> m_e = -1.0 * entry->m_f * entry->m_c ;
      /// \f$ D_k = W_k - E^{n}_kF^{T}_{k} = V_p - V^T_{xp}V^{-1}_{x}V_{xp} + F_kC_nF_k^T \f$ 
      entry -> m_d = entry->m_p.momCovMatrix() 
        - Similarity ( entry -> m_p.posMomCovMatrix() , entry -> m_vxi ) 
        + Similarity ( entry -> m_f                   , entry -> m_c   ) ;
    }
    else  // gamma & digamma 
    {
      Gaudi::Math::setToScalar ( entry->m_f , 0.0 ) ;
      Gaudi::Math::setToScalar ( entry->m_e , 0.0 ) ;
      entry -> m_d = entry -> m_p.momCovMatrix    ()  ;
      entry -> m_e = entry -> m_p.posMomCovMatrix ()  ;
    }
    //
  }
  //
  return StatusCode::SUCCESS ;
}
// ============================================================================
// construct the seed from the data 
// ============================================================================
StatusCode LoKi::KalmanFilter::seed 
( const LoKi::KalmanFilter::Entries& entries ,
  Gaudi::Vector3&                    x       , 
  Gaudi::SymMatrix3x3&               ci      , 
  const double                       scale   ) 
{
  if ( entries.empty() ) { return StatusCode::FAILURE ; }
  /// construct the seed from the data 
  Gaudi::Math::setToScalar ( ci , 0.0 ) ;
  Gaudi::Vector3 seed ;
  for ( Entries::const_iterator it = entries.begin() ; 
        entries.end() != it ; ++it ) 
  {
    //
    if ( ! it->regular() ) { continue ; } // CONTINUE 
    //
    ci   += it->m_vxi                ;
    seed += it->m_vxi * it -> m_parx ;
  }
  //
  int ifail =  0  ;
  Gaudi::SymMatrix3x3  c = ci.Inverse ( ifail ) ;
  if ( 0 != ifail ) 
  { 
    // try to recover using "soft" constraints 
    _smooth ( ci ) ;
    //
    ifail =  0  ; 
    c = ci.Inverse ( ifail ) ;
    if ( 0 != ifail ) { return StatusCode ( ErrorInMatrixInversion4 , true ) ; } 
  }
  //
  x = c * seed ; 
  //
  Gaudi::Math::scale ( ci , scale ) ; // scale the gain matrix 
  //
  return StatusCode::SUCCESS ;
}
// ========================================================================      
// construct the seed from the entries 
// ========================================================================      
StatusCode LoKi::KalmanFilter::seed 
( const LoKi::KalmanFilter::Entries& entries ,
  Gaudi::XYZPoint&                   x       , 
  Gaudi::SymMatrix3x3&               ci      , 
  const double                       scale   ) 
{
  Gaudi::Vector3      xx ;
  Gaudi::Math::geo2LA (  x , xx ) ; //  3D-point    -> 3-vector-LA
  StatusCode sc = seed ( entries , xx , ci , scale ) ;
  Gaudi::Math::la2geo ( xx ,  x ) ; //  3-vector-LA -> 3D-point
  return sc ;
}  
// ============================================================================
/*  transport the entry into new point 
 *  @param entry     (UPDATE) the entry to be transported 
 *  @param newpoint  (INPUT)  new 
 *  @param tool      (INPUT)  the particle transporter tool
 *  @return status code 
 *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
 *  @date 2008-03-06
 */
// ============================================================================
StatusCode LoKi::KalmanFilter::transport 
( LoKi::KalmanFilter::Entry& entry    , 
  const Gaudi::Vector3&      point    , 
  IParticleTransporter*      tool     ) 
{
  Gaudi::XYZPoint        xxx ;
  //  3D-point-LA    -> 3-vector
  Gaudi::Math::la2geo ( point , xxx ) ;
  //
  return transport ( entry , xxx , tool ) ;
}
// ============================================================================
/*  transport the entry into new point 
 *  @param entry     (UPDATE) the entry to be transported 
 *  @param newpoint  (INPUT)  new 
 *  @param tool      (INPUT)  the particle transporter tool
 *  @return status code 
 *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
 *  @date 2008-03-06
 */
// ============================================================================
StatusCode LoKi::KalmanFilter::transport 
( LoKi::KalmanFilter::Entry& entry    , 
  const Gaudi::XYZPoint&     point    , 
  IParticleTransporter*      tool     ) 
{
  // regular or uknown entry ?
  if ( !entry.special() ) 
  { return transport ( entry , point.Z () , tool ) ; }
  //
  //
  const  Gaudi::SymMatrix3x3* cov = 0 ;
  return transportGamma ( entry , point , cov ) ;
}
// ============================================================================
/*  transport the photon or diphoton  into new point 
 *  @param entry     (UPDATE) the entry to be transported
 *  @param point     (INPUT)  new position 
 *  @param pointCov2 (INPUT)  covariance matrix for new point 
 *  @return status code 
 *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
 *  @date 2008-03-06
 */
// ============================================================================
StatusCode LoKi::KalmanFilter::transportGamma
( LoKi::KalmanFilter::Entry& entry     , 
  const Gaudi::Vector3&      point     , 
  const Gaudi::SymMatrix3x3* pointCov2 ) 
{
  const Gaudi::XYZPoint pnt ( point[0] , point[1] , point[2] );
  return transportGamma ( entry , pnt , pointCov2 ) ;
}
// ============================================================================
/*  transport the photon or diphoton  into new point 
 *  @param entry     (UPDATE) the entry to be transported
 *  @param point     (INPUT)  new position 
 *  @param pointCov2 (INPUT)  covariance matrix for new point 
 *  @return status code 
 *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
 *  @date 2008-03-06
 */
// ============================================================================
StatusCode LoKi::KalmanFilter::transportGamma
( LoKi::KalmanFilter::Entry& entry     , 
  const Gaudi::XYZPoint&     point     , 
  const Gaudi::SymMatrix3x3* pointCov2 ) 
{
  // Gamma-like & Digamma-like entry 
  LHCb::CaloMomentum calo ;
  //
  calo.setReferencePoint ( point ) ;
  if ( 0 != pointCov2 ) { calo.setPosCovMatrix ( *pointCov2 ) ; }
  //
  if      ( GammaLikeParticle == entry.m_type ) 
  {
    const LHCb::ProtoParticle* proto = entry.m_p0->proto() ;
    if ( 0 == proto ) { return StatusCode ( ErrorGammaLikeParticle , true ) ; }
    //
    calo.addCaloPosition ( proto ) ;
  }
  else if ( DiGammaLikeParticle == entry.m_type ) 
  {
    // the first gamma  : 
    const LHCb::Particle*      gamma1 = LoKi::Child::child ( entry.m_p0 , 1 ) ;
    if ( 0 == gamma1 ) { return StatusCode ( ErrorDiGammaLikeParticle , true ) ; }
    const LHCb::ProtoParticle* proto1 = gamma1 -> proto() ;
    if ( 0 == proto1 ) { return StatusCode ( ErrorDiGammaLikeParticle , true ) ; }
    // the second gamma :
    const LHCb::Particle*      gamma2 = LoKi::Child::child ( entry.m_p0 , 2 ) ;
    if ( 0 == gamma2 ) { return StatusCode ( ErrorDiGammaLikeParticle , true ) ; }
    const LHCb::ProtoParticle* proto2 = gamma2 -> proto() ;
    if ( 0 == proto2 ) { return StatusCode ( ErrorDiGammaLikeParticle , true ) ; }
    //
    calo.addCaloPosition ( proto1 ) ;
    calo.addCaloPosition ( proto2 ) ;
    //
  }
  else { return StatusCode ( ErrorGammaTransport , true ) ; }  // RETURN
  //
  const bool ok = calo.evaluate() ;
  if ( !ok ) { return StatusCode ( ErrorFromCaloMomentum , true ) ; }
  //
  // extract the values:
  entry.m_p.setReferencePoint  ( point                     ) ;
  entry.m_p.setMomentum        ( calo.momentum          () ) ;
  entry.m_p.setMomCovMatrix    ( calo.momCovMatrix      () ) ;
  //
  if ( 0 != pointCov2 ) 
  {
    entry.m_p.setPosCovMatrix    ( calo.pointCovMatrix    () ) ;
    entry.m_p.setPosMomCovMatrix ( calo.momPointCovMatrix () ) ;
  }
  else 
  {
    Gaudi::Math::setToScalar
      ( const_cast<Gaudi::SymMatrix3x3&>( entry.m_p.posCovMatrix    () ) , 0.0 ) ;
    Gaudi::Math::setToScalar
      ( const_cast<Gaudi::Matrix4x3&>   ( entry.m_p.posMomCovMatrix () ) , 0.0 ) ;
  }
  // update the entry properly:
  return _update ( entry , entry.m_type ) ;
}
// ============================================================================
/*  get number of 'good' entries for verticing 
 *  @param entries   (input) the vector of entries 
 *  @return number of good entries for verticing
 *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
 *  @date 2010-08-24
 */   
// ============================================================================
unsigned short LoKi::KalmanFilter::nGood 
( const LoKi::KalmanFilter::Entries& entries ) 
{
  //
  unsigned short result = 0 ;
  for ( Entries::const_iterator ientry = entries.begin() ; 
        entries.end() != ientry ; ++ientry ) 
  {
    switch ( ientry->m_type ) 
    {
    case ShortLivedParticle : ++result    ; break ;
    case LongLivedParticle  : ++result    ; break ;
    default                 :               break ;
    } 
  }
  //
  return result ;
}
// ============================================================================
/*  check if the collection of entries is OK for vertex:
 *   - either at least one short-lived particle 
 *   - or at least two long-lived particles 
 *  @return true of colelction of entries is OK 
 *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
 *  @date 2010-08-24
 */   
// ============================================================================
bool LoKi::KalmanFilter::okForVertex 
( const LoKi::KalmanFilter::Entries& entries ) 
{
  unsigned short nLong = 0 ;
  for ( Entries::const_iterator ientry = entries.begin() ; 
        entries.end() != ientry ; ++ientry ) 
  {
    //
    switch ( ientry->m_type ) 
    {
    case ShortLivedParticle : return true ;           // RETURN 
    case LongLivedParticle  : ++nLong ; break ;
    default                 :           break ;
    } 
    //
    if ( 2 <= nLong )       { return true ; }        //  RETURN 
  }
  //
  return 2 <= nLong ;
}
// ============================================================================
/*  calculate number degrees of freedom for vertex fit 
 *  @thanks Yuehong Xie 
 *  @thanks Wouter Hulsbergen
 *  @thanks Fred Blanc 
 *  @param entries (input) vector of entries 
 *  @return true of colelction of entries is OK 
 *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
 *  @date 2010-09-26
 */
// ============================================================================
int LoKi::KalmanFilter::nDoF 
( const LoKi::KalmanFilter::Entries& entries ) 
{
  //
  int result = -3 ;
  //
  for ( Entries::const_iterator ientry = entries.begin() ; 
        entries.end() != ientry ; ++ientry ) 
  {
    // 3 DoFs per vertex 
    if      ( ientry->m_type == ShortLivedParticle ) { result += 3 ; }
    // 2 DoFs for track 
    else if ( ientry->m_type == LongLivedParticle  ) { result += 2 ; }
  }
  //
  return result ;
}
// ========================================================================

// ============================================================================
// The END 
// ============================================================================
