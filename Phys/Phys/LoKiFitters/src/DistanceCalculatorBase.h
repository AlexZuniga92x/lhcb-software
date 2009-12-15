// $Id: DistanceCalculatorBase.h,v 1.6 2009-12-15 15:24:26 ibelyaev Exp $
// ============================================================================
#ifndef LOKIFITTERS_DISTANCECALCULATORBASE_H 
#define LOKIFITTERS_DISTANCECALCULATORBASE_H 1
// ============================================================================
// Include files
// ============================================================================
// GaudiAlg
// ============================================================================
#include "GaudiAlg/GaudiTool.h"
// ============================================================================
// Event
// ============================================================================
#include "Event/Particle.h"
#include "Event/VertexBase.h"
// ============================================================================
// LHCbMath
// ============================================================================
#include "LHCbMath/Line.h"
#include "LHCbMath/GeomFun.h"
#include "LHCbMath/MatrixUtils.h"
#include "LHCbMath/MatrixTransforms.h"
// ============================================================================
// DaVinciKernel
// ============================================================================
#include "Kernel/IDistanceCalculator.h"
// ============================================================================
namespace LoKi 
{
  /** @class DistanceCalculatorBase DistanceCalculator.h
   *
   *  It is the simplest implementation of the basic math, 
   *  needed for the real implementation 
   *  on the abstract interface IDistanceCalculator 
   *  Essentially it relies on many nice functions, coded 
   *  
   *  
   *  @see IDistanceCalculator 
   *
   *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
   *  @date   2008-03-05
   */
  class DistanceCalculatorBase : public GaudiTool 
  {
  public:
    // ========================================================================
    /// get the actual type of the line  
    typedef Gaudi::Math::Line<Gaudi::XYZPoint,Gaudi::XYZVector> Line_ ;
    // ========================================================================
  public:
    // ========================================================================
    /// the error codes:
    enum {
      /// Invalid Particle 
      InvalidParticle         = 901 , // Invalid Particle 
      /// Invalid Vertex 
      InvalidVertex           = 902 , // Invalid Vertex 
      /// Invalid Data
      InvalidData             = 903 , // Invalid Data 
      /// Error in Matrix Inversion 
      ErrorInMatrixInversion  = 905 , // Error in Matrix Inversion
      /// Error from transporter 
      ErrorFromTransporter    = 906 , // Error from Particle Transporter
      /// No Convergency
      NoConvergency           = 910 , // No Convergency is detected 
      /// Not Implemented Yet 
      NotImplemented          = 920   // not yet implemented 
    } ;
    // ========================================================================
  protected:
    /// construct the line trajectory from the particle  
    inline Line_  line   ( const LHCb::Particle& particle ) const ;
    // ========================================================================
    /** find the impact parameter ("distance") vector 
     *  of the particle with respect to some vertex 
     *  @param particle (input) the particle 
     *  @param vertex   (input) the vertex 
     *  @param impact   (output) the evaluated impact parameter vector 
     */  
    inline void i_distance 
    ( const LHCb::Particle&   particle , 
      const LHCb::VertexBase& vertex   , 
      Gaudi::XYZVector&       impact   ) const ;
    // =========================================================================
    /** find the impact parameter ("distance") vector 
     *  of the particle with respect to some fixed point 
     *  @param particle (input) the particle 
     *  @param point    (input) the fixed point 
     *  @param impact   (output) the evaluated impact parameter vector 
     */  
    inline void i_distance 
    ( const LHCb::Particle&   particle , 
      const Gaudi::XYZPoint&  point    , 
      Gaudi::XYZVector&       impact   ) const ;
    // ========================================================================
    /** find the points of the closest approach for two  particles
     *  @param particel1 (input) the first particle 
     *  @param particle2 (input) the second particle 
     *  @param point1    (output) the point on the first trajectory 
     *  @param point2    (output) the point on the second trajectory
     */
    inline void i_distance 
    ( const LHCb::Particle&   particle1 ,  // the first particle 
      const LHCb::Particle&   particle2 ,  // the second particle 
      Gaudi::XYZPoint&        point1    ,  // the point on the first trajectory 
      Gaudi::XYZPoint&        point2    ) const ;  
    // ========================================================================
    /**  evaluate the  distance between two vertices 
     *   @param vx1  (input) the first vertex 
     *   @param vx2  (input) the second vertex 
     *   @param dist (output) the distance between vertices 
     *   @param chi2 (output,optional) the chi2 separation significance 
     */
    inline StatusCode  i_distance 
    ( const LHCb::VertexBase& vx1      , 
      const LHCb::VertexBase& vx2      , 
      double&                 dist     , 
      double*                 chi2 = 0 ) const ;
    // ========================================================================
    /**  evaluate the  distance between the vertex and the fixed point point 
     *   @param vertex (input) the vertex 
     *   @param point  (input) the fixed point
     *   @param dist   (output) the distance between vertices 
     *   @param chi2   (output,optional) the chi2 separation significance 
     */
    inline StatusCode i_distance 
    ( const LHCb::VertexBase& vertex   , 
      const Gaudi::XYZPoint&  point    , 
      double&                 dist     , 
      double*                 chi2 = 0 ) const ;
    // ========================================================================
    /* evalute the "projected" distance 
     * 
     *  \f$s=\frac{\left(\vec{\mathbf{v}}\vec{\mathbf{p}}
     *     \right)}{\left|\vec{\mathbf{p}}\right|}\f$
     *  where vector \f$\vec{\mathbf{v}}\f$ is a vector from 
     *  the primary to the secondary vertex: 
     *    \f$\vec{\mathbf{v}}=\vec{\mathbf{x}}_{d}-\vec{\mathbf{x}}_{pv}\f$,
     *
     * @param primary  (input)  the production vertex 
     * @param particle (input)  the particle 
     * @param decay    (input)  the decay particle 
     * @param return the projected distance
     * @return status code 
     */ 
    inline double i_distance 
    ( const LHCb::VertexBase& primary  , 
      const LHCb::Particle&   particle , 
      const LHCb::VertexBase& decay    ) const ;
    // ========================================================================
  protected:
    // ========================================================================
    /** check the validity of the particle
     *  @param p pointer to the particle 
     *  @return StatusCode 
     */
    inline StatusCode check ( const LHCb::Particle*   v ) const ;
    /** check the validity of the vertex 
     *  @param v pointer to the vertex
     *  @return StatusCode 
     */
    inline StatusCode check ( const LHCb::VertexBase* v ) const ;
    /** check the validity of the particles 
     *  @param p1 pointer to the first particle 
     *  @param p2 pointer to the second particle 
     *  @return StatusCode 
     */   
    inline StatusCode check 
    ( const LHCb::Particle*   p1 , 
      const LHCb::Particle*   p2 ) const ;
    /** check the validity of the particle and the vertex 
     *  @param p pointer to the particle 
     *  @param v pointer to the vertex
     *  @return StatusCode 
     */   
    inline StatusCode check 
    ( const LHCb::Particle*   p ,
      const LHCb::VertexBase* v ) const ;
    /** check the validity of the vertices 
     *  @param v1 pointer to the first vertex
     *  @param v2 pointer to the second vertex
     *  @return StatusCode 
     */   
    inline StatusCode check 
    ( const LHCb::VertexBase* v1 ,
      const LHCb::VertexBase* v2 ) const ;
    // ========================================================================
  protected: 
    /** Standard constructor
     *  @param type tool type(?)
     *  @param name tool instance name 
     *  @param parent the pointer to the parent
     */
    DistanceCalculatorBase
    ( const std::string& type   ,    // tool type (?)
      const std::string& name   ,    // tool instance name 
      const IInterface*  parent ) ;  // the parent 
    // virtual and protected desctrustor 
    virtual ~DistanceCalculatorBase () ; // Destructor
  private:
    /// the default constructor is disabled 
    DistanceCalculatorBase () ; // no default constructor 
    /// the copy    constructor is disabled 
    DistanceCalculatorBase ( const DistanceCalculatorBase&) ; // no copy 
    /// the assignement operator is disabled 
    DistanceCalculatorBase& operator=( const DistanceCalculatorBase&) ; // no assignement
  private:
  };
} // end of namespace LoKi
// ========================================================================
// check the validity of the particle
// ========================================================================
inline StatusCode LoKi::DistanceCalculatorBase::check 
( const LHCb::Particle*    p ) const 
{
  if ( 0 == p ) 
  { return Error ( "LHCb::Particle* points to NULL", InvalidParticle ) ; }
  //
  return StatusCode::SUCCESS ;
}
// ========================================================================
// check the validity of the vertex 
// ========================================================================
inline StatusCode LoKi::DistanceCalculatorBase::check 
 ( const LHCb::VertexBase* v ) const 
{
  if ( 0 == v ) 
  { return Error ( "LHCb::VertexBase* points to NULL", InvalidVertex ) ; }
  //
  return StatusCode::SUCCESS ;
}
// ========================================================================
// check the validity of the particles 
// ========================================================================
inline StatusCode LoKi::DistanceCalculatorBase::check 
( const LHCb::Particle*   p1 , 
  const LHCb::Particle*   p2 ) const 
{
  if ( 0 == p1 || 0 == p2  ) 
  { return Error ( "LHCb::Particle* points to NULL", InvalidParticle ) ; }
  //
  return StatusCode::SUCCESS ;
}
// ========================================================================
// check the validity of the particle and the vertex 
// ========================================================================
inline StatusCode LoKi::DistanceCalculatorBase::check 
( const LHCb::Particle*   p ,
  const LHCb::VertexBase* v ) const 
{
  if ( 0 == p ) 
  { return Error ( "LHCb::Particle* points to NULL", InvalidParticle ) ; }
  if ( 0 == v ) 
  { return Error ( "LHCb::VertexBase* points to NULL", InvalidVertex ) ; }
  //
  return StatusCode::SUCCESS ;
}
// ========================================================================
// check the validity of the vertices 
// ========================================================================
inline StatusCode LoKi::DistanceCalculatorBase::check 
( const LHCb::VertexBase* v1 , 
  const LHCb::VertexBase* v2 ) const 
{
  if ( 0 == v1 || 0 == v2 ) 
  { return Error ( "LHCb::VertexBase* points to NULL", InvalidVertex ) ; }
  //
  return StatusCode::SUCCESS ;
}
// ========================================================================
// construct the line trajector from the particle  
// ============================================================================
inline LoKi::DistanceCalculatorBase::Line_
LoKi::DistanceCalculatorBase::line ( const LHCb::Particle& p ) const 
{ return Line_( p.referencePoint() , p.momentum().Vect() ) ; }
// ============================================================================
/*  find the impact parameter ("distance") vector 
 *  of the particle with respect to some vertex 
 */  
// ============================================================================
inline void 
LoKi::DistanceCalculatorBase::i_distance 
( const LHCb::Particle&   particle , 
  const LHCb::VertexBase& vertex   , 
  Gaudi::XYZVector&       impact   ) const 
{ 
  i_distance ( particle , vertex.position() , impact ) ; 
}
// ============================================================================
/*  find the impact parameter ("distance") vector 
 *  of the particle with respect to some fixed point 
 */  
// ============================================================================
inline void 
LoKi::DistanceCalculatorBase::i_distance 
( const LHCb::Particle&   particle , 
  const Gaudi::XYZPoint&  point    , 
  Gaudi::XYZVector&       impact   ) const 
{
  // (re)use the nice functions by Matt&Juan
  impact = Gaudi::Math::closestPoint ( point , line ( particle ) ) - point ;
}
// ============================================================================
// find the points of the closest approach for two  particles
// ============================================================================
inline void LoKi::DistanceCalculatorBase::i_distance 
( const LHCb::Particle&   particle1 ,  // the first particle 
  const LHCb::Particle&   particle2 ,  // the second particle 
  Gaudi::XYZPoint&        point1    ,  // the point on the first trajectory 
  Gaudi::XYZPoint&        point2    ) const 
{
  double mu1 = 0 ;
  double mu2 = 0 ;
  const  Line_ line1 ( line ( particle1 ) ) ;
  const  Line_ line2 ( line ( particle2 ) ) ;
  
  // (re)use the nice functions by Matt&Juan
  Gaudi::Math::closestPointParams ( line1 , line2 , mu1 , mu2 ) ;
  
  point1 = line1 ( mu1 ) ; // the point on the first tarjectory
  point2 = line2 ( mu2 ) ; // the point on the second trajectory  
}
// ============================================================================
//  evaluate the  distance (and chi^2) between two vertices 
// ============================================================================
inline 
StatusCode 
LoKi::DistanceCalculatorBase::i_distance 
( const LHCb::VertexBase& vx1  , 
  const LHCb::VertexBase& vx2  , 
  double&                 dist , 
  double*                 chi2 ) const 
{
  // calculate the distance 
  const Gaudi::XYZVector delta ( vx1.position() - vx2.position() ) ;
  dist = delta.R() ;
  if ( 0 == chi2 ) { return StatusCode::SUCCESS ; } // RETURN
  // evaluate chi2: 
  Gaudi::SymMatrix3x3 cov ( vx1.covMatrix() + vx2.covMatrix() ) ;
  if ( !cov.Invert() ) 
  { return Error ( "Error in matrix inversion" , ErrorInMatrixInversion ); }
  // evaluate the chi2 
  *chi2 = Gaudi::Math::Similarity ( delta , cov ) ; 
  //
  return StatusCode::SUCCESS ;
}
// ============================================================================
//  evaluate the  distance (and chi^2) between 
// ============================================================================
inline 
StatusCode 
LoKi::DistanceCalculatorBase::i_distance 
( const LHCb::VertexBase& v    , 
  const Gaudi::XYZPoint&  p    , 
  double&                 dist ,
  double*                 chi2 ) const 
{
  // calculate the distance 
  const Gaudi::XYZVector delta ( v.position() - p ) ;
  dist = delta.R() ;
  if ( 0 == chi2 ) { return StatusCode::SUCCESS ; } // RETURN
  // evaluate chi2: 
  Gaudi::SymMatrix3x3 cov ( v.covMatrix() ) ;
  if ( !cov.Invert() ) 
  { return Error ( "Error in matrix inversion" , ErrorInMatrixInversion ) ; }  
  // evaluate the chi2 
  *chi2 = Gaudi::Math::Similarity ( delta , cov ) ; 
  //
  return StatusCode::SUCCESS ;
}
// ============================================================================
// evalute the "projected" distance 
// ============================================================================
inline 
double 
LoKi::DistanceCalculatorBase::i_distance 
( const LHCb::VertexBase& primary  , 
  const LHCb::Particle&   particle , 
  const LHCb::VertexBase& decay    ) const 
{
  // decay position
  const Gaudi::XYZPoint&  vd = decay   . position () ;
  // origin position 
  const Gaudi::XYZPoint&  vp = primary . position () ;
  // the unit vector along the momentum 
  const Gaudi::XYZVector   p  = particle.momentum().Vect().Unit() ;
  //
  return ( vd  - vp ).Dot ( p ) ;
}
// ============================================================================
// The END 
// ============================================================================
#endif // LOKIFITTERS_DISTANCECALCULATORBASE_H
// ============================================================================
