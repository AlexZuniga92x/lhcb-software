// $Id$
// ============================================================================
// Include files 
// ============================================================================
// GaudiKernel
// ============================================================================
// Event 
// ============================================================================
#include "Event/RecVertex.h"
// ============================================================================
// LoKiCore 
// ============================================================================
#include "LoKi/Constants.h"
#include "LoKi/Kinematics.h"
// ============================================================================
// LoKiPhys
// ============================================================================
#include "LoKi/Particles1.h"
// ============================================================================
/** @file
 *
 *  Implementation file for functions from namespace  LoKi::Particles
 *
 *  This file is a part of LoKi project - 
 *    "C++ ToolKit  for Smart and Friendly Physics Analysis"
 *
 *  The package has been designed with the kind help from
 *  Galina PAKHLOVA and Sergey BARSUK.  Many bright ideas, 
 *  contributions and advices from G.Raven, J.van Tilburg, 
 *  A.Golutvin, P.Koppenburg have been used in the design.
 *
 *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
 *  @date 2006-02-10 
 */
// ============================================================================
//  constructor 
// ============================================================================
LoKi::Particles::VertexDistance::VertexDistance
( const LHCb::VertexBase* vertex ) 
  : LoKi::BasicFunctors<const LHCb::Particle*>::Function() 
  , m_fun ( vertex )
{}
// ============================================================================
//  constructor 
// ============================================================================
LoKi::Particles::VertexDistance::VertexDistance
( const LoKi::Point3D& vertex ) 
  : LoKi::AuxFunBase ( std::tie ( vertex ) ) 
  , LoKi::BasicFunctors<const LHCb::Particle*>::Function() 
  , m_fun ( vertex )
{}
// ============================================================================
//  constructor 
// ============================================================================
LoKi::Particles::VertexDistance::VertexDistance
( const LoKi::Vertices::VertexHolder& vertex ) 
  : LoKi::BasicFunctors<const LHCb::Particle*>::Function() 
  , m_fun ( vertex )
{}
// ============================================================================
//  constructor 
// ============================================================================
LoKi::Particles::VertexDistance::VertexDistance
( const LoKi::Vertices::VertexDistance& vertex ) 
  : LoKi::BasicFunctors<const LHCb::Particle*>::Function() 
  , m_fun ( vertex )
{}
// ============================================================================
//  constructor 
// ============================================================================
LoKi::Particles::VertexDistance::VertexDistance
( const LoKi::Particles::VertexDistance& vertex ) 
  : LoKi::AuxFunBase                     ( vertex )
  , LoKi::BasicFunctors<const LHCb::Particle*>::Function( vertex ) 
  , m_fun ( vertex.m_fun )
{}
// ============================================================================
//  MANDATORY: virtual destructor 
// ============================================================================
LoKi::Particles::VertexDistance::~VertexDistance(){}
// ============================================================================
//  MANDATORY: clone method ("virtual constructor")
// ============================================================================
LoKi::Particles::VertexDistance*
LoKi::Particles::VertexDistance::clone() const
{ return new LoKi::Particles::VertexDistance(*this) ; }
// ============================================================================
//  MANDATORY: the only one essential method 
// ============================================================================
LoKi::Particles::VertexDistance::result_type 
LoKi::Particles::VertexDistance::operator() 
  ( LoKi::Particles::VertexDistance::argument p ) const
{ return distance ( p ) ; }
// ============================================================================
//  the actual computation 
// ============================================================================
LoKi::Particles::VertexDistance::result_type 
LoKi::Particles::VertexDistance::distance 
( LoKi::Particles::VertexDistance::argument p ) const
{ 
  if ( 0 ==p ) 
  {
    Error ( " Invalid Particle, return 'InvalidDistance'" ) ;
    return LoKi::Constants::InvalidDistance;                   // RETURN 
  }
  // call for actual evalautor 
  return m_fun.distance( p->endVertex() ) ;
}
// ============================================================================
//  OPTIONAL: the specific printout 
// ============================================================================
std::ostream& 
LoKi::Particles::VertexDistance::fillStream ( std::ostream & s ) const 
{ return s << "VD" ; }
// ============================================================================

// ============================================================================
//  constructor 
// ============================================================================
LoKi::Particles::VertexSignedDistance::VertexSignedDistance
( const LHCb::VertexBase* vertex ) 
  : LoKi::BasicFunctors<const LHCb::Particle*>::Function() 
  , m_fun ( vertex )
{}
// ============================================================================
//  constructor 
// ============================================================================
LoKi::Particles::VertexSignedDistance::VertexSignedDistance
( const LoKi::Point3D& vertex ) 
  : LoKi::AuxFunBase ( std::tie ( vertex ) ) 
  , LoKi::BasicFunctors<const LHCb::Particle*>::Function() 
  , m_fun ( vertex )
{}
// ============================================================================
//  constructor 
// ============================================================================
LoKi::Particles::VertexSignedDistance::VertexSignedDistance
( const LoKi::Vertices::VertexHolder& vertex ) 
  : LoKi::BasicFunctors<const LHCb::Particle*>::Function() 
  , m_fun ( vertex )
{}
// ============================================================================
//  constructor 
// ============================================================================
LoKi::Particles::VertexSignedDistance::VertexSignedDistance
( const LoKi::Vertices::VertexDistance& vertex ) 
  : LoKi::BasicFunctors<const LHCb::Particle*>::Function() 
  , m_fun ( vertex )
{}
// ============================================================================
//  constructor 
// ============================================================================
LoKi::Particles::VertexSignedDistance::VertexSignedDistance
( const LoKi::Vertices::VertexSignedDistance& vertex ) 
  : LoKi::BasicFunctors<const LHCb::Particle*>::Function() 
  , m_fun ( vertex )
{}
// ============================================================================
//  constructor 
// ============================================================================
LoKi::Particles::VertexSignedDistance::VertexSignedDistance
( const LoKi::Particles::VertexSignedDistance& vertex ) 
  : LoKi::AuxFunBase                     ( vertex )
  , LoKi::BasicFunctors<const LHCb::Particle*>::Function( vertex ) 
  , m_fun ( vertex.m_fun )
{}
// ============================================================================
//  MANDATORY: virtual destructor 
// ============================================================================
LoKi::Particles::VertexSignedDistance::~VertexSignedDistance(){}
// ============================================================================
//  MANDATORY: clone method ("virtual constructor")
// ============================================================================
LoKi::Particles::VertexSignedDistance*
LoKi::Particles::VertexSignedDistance::clone() const
{ return new LoKi::Particles::VertexSignedDistance(*this) ; }
// ============================================================================
//  the actual computation 
// ============================================================================
LoKi::Particles::VertexSignedDistance::result_type 
LoKi::Particles::VertexSignedDistance::signedDistance 
( LoKi::Particles::VertexSignedDistance::argument p ) const
{ 
  if ( 0 == p )
  {
    Error ( " Invalid Particle, return 'InvalidDistance'" ) ;
    return LoKi::Constants::InvalidDistance;                   // RETURN 
  }
  // call for actual evalautor 
  return m_fun.signedDistance ( p->endVertex() ) ;
} 
// ============================================================================
//  OPTIONAL: the specific printout 
// ============================================================================
std::ostream& 
LoKi::Particles::VertexSignedDistance::fillStream ( std::ostream & s ) const 
{ return s << "VDSIGN" ; }
// ============================================================================


// ============================================================================
//  constructor 
// ============================================================================
LoKi::Particles::VertexDotDistance::VertexDotDistance
( const LHCb::VertexBase* vertex ) 
  : LoKi::BasicFunctors<const LHCb::Particle*>::Function() 
  , LoKi::Vertices::VertexHolder ( vertex )
{}
// ============================================================================
//  constructor 
// ============================================================================
LoKi::Particles::VertexDotDistance::VertexDotDistance
( const LoKi::Point3D& vertex ) 
  : LoKi::AuxFunBase ( std::tie ( vertex ) ) 
  , LoKi::BasicFunctors<const LHCb::Particle*>::Function() 
  , LoKi::Vertices::VertexHolder ( vertex )
{}
// ============================================================================
//  constructor 
// ============================================================================
LoKi::Particles::VertexDotDistance::VertexDotDistance
( const LoKi::Vertices::VertexHolder& vertex ) 
  : LoKi::BasicFunctors<const LHCb::Particle*>::Function() 
  , LoKi::Vertices::VertexHolder ( vertex )
{}
// ============================================================================
//  constructor 
// ============================================================================
LoKi::Particles::VertexDotDistance::VertexDotDistance
( const LoKi::Particles::VertexDotDistance& vertex ) 
  : LoKi::AuxFunBase                     ( vertex )
  , LoKi::BasicFunctors<const LHCb::Particle*>::Function( vertex ) 
  , LoKi::Vertices::VertexHolder ( vertex )
{}
// ============================================================================
//  MANDATORY: virtual destructor 
// ============================================================================
LoKi::Particles::VertexDotDistance::~VertexDotDistance(){}
// ============================================================================
//  MANDATORY: clone method ("virtual constructor")
// ============================================================================
LoKi::Particles::VertexDotDistance*
LoKi::Particles::VertexDotDistance::clone() const
{ return new LoKi::Particles::VertexDotDistance(*this) ; }
// ============================================================================
//  MANDATORY: the only one essential method 
// ============================================================================
LoKi::Particles::VertexDotDistance::result_type 
LoKi::Particles::VertexDotDistance::operator() 
  ( LoKi::Particles::VertexDotDistance::argument p ) const
{ return distance ( p ) ; }
// ============================================================================
//  the actual computation 
// ============================================================================
LoKi::Particles::VertexDotDistance::result_type 
LoKi::Particles::VertexDotDistance::distance 
( LoKi::Particles::VertexDotDistance::argument p ) const
{
  if ( 0 == p      ) 
  { 
    Error("Invalid particle! return 'InvalidDistance' ");
    return  LoKi::Constants::InvalidDistance ;             // RETURN 
  }
  if ( !valid()   )
  { 
    Error("Invalid vertex! return 'InvalidDistance' ");
    return  LoKi::Constants::InvalidDistance ;             // RETURN 
  }
  const LHCb::Vertex* vx = p->endVertex() ;
  if( 0 == vx     ) 
  { 
    Error("Invalid 'endVertex'! return 'InvalidDistance' ");
    return  LoKi::Constants::InvalidDistance ;              // RETURN 
  }
  // momentum direction
  const LoKi::Vector3D direction 
    ( p->momentum().Px() , 
      p->momentum().Py() , 
      p->momentum().Pz() ) ;
  // vertex distance 
  const LoKi::Vector3D dist = vx->position() - position() ;
  //
  const double dirMag = direction.R()  ;
  if ( 0 >= dirMag ) 
  { 
    Warning("Invalid momentum direction") ;
    return dist.R() ;                                       // RETURN 
  }
  //
  return dist.Dot( direction ) / dirMag ;                   // RETURN 
}
// ============================================================================
//  OPTIONAL: the specific printout 
// ============================================================================
std::ostream& 
LoKi::Particles::VertexDotDistance::fillStream ( std::ostream& s ) const 
{ return s << "VDDOT" ; }
// ============================================================================

// ============================================================================
//  constructor 
// ============================================================================
LoKi::Particles::VertexChi2Distance::VertexChi2Distance
( const LHCb::VertexBase* vertex ) 
  : LoKi::BasicFunctors<const LHCb::Particle*>::Function() 
  , m_fun ( vertex )
{}
// ============================================================================
//  constructor 
// ============================================================================
LoKi::Particles::VertexChi2Distance::VertexChi2Distance
( const LoKi::Point3D& vertex ) 
  : LoKi::AuxFunBase ( std::tie ( vertex ) ) 
  , LoKi::BasicFunctors<const LHCb::Particle*>::Function() 
  , m_fun ( vertex )
{}
// ============================================================================
//  constructor 
// ============================================================================
LoKi::Particles::VertexChi2Distance::VertexChi2Distance
( const LoKi::Vertices::VertexHolder& vertex ) 
  : LoKi::BasicFunctors<const LHCb::Particle*>::Function() 
  , m_fun ( vertex )
{}
// ============================================================================
//  constructor 
// ============================================================================
LoKi::Particles::VertexChi2Distance::VertexChi2Distance
( const LoKi::Vertices::VertexChi2Distance& vertex ) 
  : LoKi::BasicFunctors<const LHCb::Particle*>::Function() 
  , m_fun ( vertex )
{}
// ============================================================================
//  constructor 
// ============================================================================
LoKi::Particles::VertexChi2Distance::VertexChi2Distance
( const LoKi::Particles::VertexChi2Distance& vertex ) 
  : LoKi::AuxFunBase                     ( vertex )
  , LoKi::BasicFunctors<const LHCb::Particle*>::Function( vertex ) 
  , m_fun ( vertex.m_fun )
{}
// ============================================================================
//  MANDATORY: virtual destructor 
// ============================================================================
LoKi::Particles::VertexChi2Distance::~VertexChi2Distance(){}
// ============================================================================
//  MANDATORY: clone method ("virtual constructor")
// ============================================================================
LoKi::Particles::VertexChi2Distance*
LoKi::Particles::VertexChi2Distance::clone() const
{ return new LoKi::Particles::VertexChi2Distance(*this) ; }
// ============================================================================
//  MANDATORY: the only one essential method 
// ============================================================================
LoKi::Particles::VertexChi2Distance::result_type 
LoKi::Particles::VertexChi2Distance::operator() 
  ( LoKi::Particles::VertexChi2Distance::argument p ) const
{ return chi2 ( p ) ; }
// ============================================================================
//  the actual computation 
// ============================================================================
LoKi::Particles::VertexChi2Distance::result_type 
LoKi::Particles::VertexChi2Distance::chi2
( LoKi::Particles::VertexChi2Distance::argument p ) const
{ 
  if ( 0 ==p ) 
  {
    Error ( " Invalid Particle, return 'InvalidDistance'" ) ;
    return LoKi::Constants::InvalidDistance;                   // RETURN 
  }
  // call for actual evalautor 
  return m_fun.chi2( p->endVertex() ) ;
}
// ============================================================================
//  OPTIONAL: the specific printout 
// ============================================================================
std::ostream& 
LoKi::Particles::VertexChi2Distance::fillStream ( std::ostream& s ) const 
{ return s << "VDCHI2" ; }
// ============================================================================


// ============================================================================
/*  constructor from container of vertices 
 *  @param vs container of vertices 
 */
// ============================================================================
LoKi::Particles::MinVertexDistance::MinVertexDistance 
( const LHCb::VertexBase::ConstVector& vs ) 
  : LoKi::BasicFunctors<const LHCb::Particle*>::Function () 
  , m_fun ( vs ) 
{}
// ============================================================================
/*  constructor from container of vertices 
 *  @param vs container of vertices 
 */
// ============================================================================
LoKi::Particles::MinVertexDistance::MinVertexDistance 
( const LHCb::Vertex::ConstVector& vs ) 
  : LoKi::BasicFunctors<const LHCb::Particle*>::Function () 
  , m_fun ( vs ) 
{}
// ============================================================================
/*  constructor from container of vertices 
 *  @param vs container of primary vertices 
 */
// ============================================================================
LoKi::Particles::MinVertexDistance::MinVertexDistance 
( const LHCb::RecVertex::ConstVector& vs ) 
  : LoKi::BasicFunctors<const LHCb::Particle*>::Function () 
  , m_fun ( vs ) 
{}
// ============================================================================
/*  constructor from container of vertices 
 *  @param vs container of primary vertices 
 */
// ============================================================================
LoKi::Particles::MinVertexDistance::MinVertexDistance 
( const LHCb::RecVertex::Container*   vs  ) 
  : LoKi::BasicFunctors<const LHCb::Particle*>::Function () 
  , m_fun ( vs ) 
{}
// ============================================================================
/*  constructor from container of vertices 
 *  @param vs container of primary vertices 
 */
// ============================================================================
LoKi::Particles::MinVertexDistance::MinVertexDistance 
( const LoKi::PhysTypes::VRange& vs ) 
  : LoKi::BasicFunctors<const LHCb::Particle*>::Function () 
  , m_fun ( vs ) 
{}
// ============================================================================
//  MANDATORY: the only one essential method 
// ============================================================================
LoKi::Particles::MinVertexDistance::result_type 
LoKi::Particles::MinVertexDistance::operator() 
  ( LoKi::Particles::MinVertexDistance::argument p ) const 
{
  if ( 0 == p ) 
  {
    Error ( " Argument is invalid! return InvalidDistance " ) ;
    return LoKi::Constants::InvalidDistance ;
  }
  // use the actual evaluator 
  return m_fun ( p->endVertex()  ) ;
} 
// ============================================================================
//  OPTIONAL: the specific printout 
// ============================================================================
std::ostream& 
LoKi::Particles::MinVertexDistance::fillStream ( std::ostream& s ) const 
{ return s << "MINVD" ; }
// ============================================================================

// ============================================================================
/*  constructor from container of vertices 
 *  @param vs container of vertices 
 */
// ============================================================================
LoKi::Particles::MinVertexChi2Distance::MinVertexChi2Distance 
( const LHCb::VertexBase::ConstVector& vs ) 
  : LoKi::BasicFunctors<const LHCb::Particle*>::Function () 
  , m_fun ( vs ) 
{}
// ============================================================================
/*  constructor from container of vertices 
 *  @param vs container of vertices 
 */
// ============================================================================
LoKi::Particles::MinVertexChi2Distance::MinVertexChi2Distance 
( const LHCb::Vertex::ConstVector& vs ) 
  : LoKi::BasicFunctors<const LHCb::Particle*>::Function () 
  , m_fun ( vs ) 
{}
// ============================================================================
/*  constructor from container of vertices 
 *  @param vs container of primary vertices 
 */
// ============================================================================
LoKi::Particles::MinVertexChi2Distance::MinVertexChi2Distance 
( const LHCb::RecVertex::ConstVector& vs ) 
  : LoKi::BasicFunctors<const LHCb::Particle*>::Function () 
  , m_fun ( vs ) 
{}
// ============================================================================
/*  constructor from container of vertices 
 *  @param vs container of primary vertices 
 */
// ============================================================================
LoKi::Particles::MinVertexChi2Distance::MinVertexChi2Distance 
( const LHCb::RecVertex::Container*   vs  ) 
  : LoKi::BasicFunctors<const LHCb::Particle*>::Function () 
  , m_fun ( vs ) 
{}
// ============================================================================
/*  constructor from container of vertices 
 *  @param vs container of primary vertices 
 */
// ============================================================================
LoKi::Particles::MinVertexChi2Distance::MinVertexChi2Distance 
( const LoKi::PhysTypes::VRange& vs ) 
  : LoKi::BasicFunctors<const LHCb::Particle*>::Function () 
  , m_fun ( vs ) 
{}
// ============================================================================
//  MANDATORY: the only one essential method 
// ============================================================================
LoKi::Particles::MinVertexChi2Distance::result_type 
LoKi::Particles::MinVertexChi2Distance::operator() 
  ( LoKi::Particles::MinVertexChi2Distance::argument p ) const 
{
  if ( 0 == p ) 
  {
    Error ( " Argument is invalid! return InvalidChi2" ) ;
    return LoKi::Constants::InvalidChi2;
  }
  // use the actual evaluator 
  return m_fun ( p->endVertex() ) ;
}
// ============================================================================
//  OPTIONAL: the specific printout 
// ============================================================================
std::ostream& 
LoKi::Particles::MinVertexChi2Distance::fillStream ( std::ostream& s ) const 
{ return s << "MINVD" ; }
// ============================================================================





// ============================================================================
// The END 
// ============================================================================

