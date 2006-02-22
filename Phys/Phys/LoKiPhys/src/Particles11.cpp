// $Id: Particles11.cpp,v 1.2 2006-02-22 21:02:09 ibelyaev Exp $
// ============================================================================
// CVS tag $Name: not supported by cvs2svn $, version $Revision: 1.2 $
// ============================================================================
// $Log: not supported by cvs2svn $
// Revision 1.1  2006/02/22 20:53:47  ibelyaev
//  add a lot of new functions (without fillStream)
//
// ============================================================================
// Include files 
// ============================================================================
// STD & STL 
// ============================================================================
#include <algorithm>
// ============================================================================
// Event 
// ============================================================================
#include "Event/Particle.h"
#include "Event/Vertex.h"
// ============================================================================
// LoKiCore 
// ============================================================================
#include "LoKi/Constants.h"
#include "LoKi/Print.h"
#include "LoKi/Keeper.h"
#include "LoKi/UniqueKeeper.h"
// ============================================================================
// LoKiPhys
// ============================================================================
#include "LoKi/Particles11.h"
#include "LoKi/PhysAlgs.h"
// ============================================================================

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
 *  By usage of this code one clearly states the disagreement 
 *  with the campain of Dr.O.Callot et al.: 
 *  "No Vanya's lines are allowed in LHCb/Gaudi software."
 *
 *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
 *  @date 2006-02-22
 */
// ============================================================================

// ============================================================================
/// constructor from one particle 
// ============================================================================
LoKi::Particles::IsAParticle::IsAParticle 
( const LHCb::Particle*                 p ) 
  : LoKi::Predicate<const LHCb::Particle*>() 
  , LoKi::UniqueKeeper<LHCb::Particle> ( p ) 
{} ;
// ============================================================================
/// constructor from container of particles
// ============================================================================
LoKi::Particles::IsAParticle::IsAParticle 
( const LHCb::Particles*                p ) 
  : LoKi::Predicate<const LHCb::Particle*>() 
  , LoKi::UniqueKeeper<LHCb::Particle> ()
{ if ( 0 != p ) { addObjects( p->begin() , p->end() ) ; } };
// ============================================================================
/// constructor from container of particles
// ============================================================================
LoKi::Particles::IsAParticle::IsAParticle 
( const SmartRefVector<LHCb::Particle>& p ) 
  : LoKi::Predicate<const LHCb::Particle*>() 
  , LoKi::UniqueKeeper<LHCb::Particle> ( p.begin() , p.end() ) 
{} ;
// ============================================================================
/// constructor from container of particles
// ============================================================================
LoKi::Particles::IsAParticle::IsAParticle 
( const LHCb::Particle::Vector&         p ) 
  : LoKi::Predicate<const LHCb::Particle*>() 
  , LoKi::UniqueKeeper<LHCb::Particle> ( p.begin() , p.end() ) 
{} ;
// ============================================================================
/// constructor from container of particles
// ============================================================================
LoKi::Particles::IsAParticle::IsAParticle 
( const LHCb::Particle::ConstVector&    p ) 
  : LoKi::Predicate<const LHCb::Particle*>() 
  , LoKi::UniqueKeeper<LHCb::Particle> ( p.begin() , p.end() ) 
{} ;
// ============================================================================
/// constructor from container of particles
// ============================================================================
LoKi::Particles::IsAParticle::IsAParticle 
( const LoKi::PhysTypes::Range&         p ) 
  : LoKi::Predicate<const LHCb::Particle*>() 
  , LoKi::UniqueKeeper<LHCb::Particle> ( p.begin() , p.end() ) 
{} ;
// ============================================================================
/// constructor from container of particles
// ============================================================================
LoKi::Particles::IsAParticle::IsAParticle 
( const LoKi::Keeper<LHCb::Particle>&   p ) 
  : LoKi::Predicate<const LHCb::Particle*>() 
    , LoKi::UniqueKeeper<LHCb::Particle> ( p ) 
{} ;
// ============================================================================
/// constructor from container of particles
// ============================================================================
LoKi::Particles::IsAParticle::IsAParticle 
( const LoKi::UniqueKeeper<LHCb::Particle>& p ) 
  : LoKi::Predicate<const LHCb::Particle*>() 
  , LoKi::UniqueKeeper<LHCb::Particle> ( p ) 
{} ;
// ============================================================================
/// copy constructor
// ============================================================================
LoKi::Particles::IsAParticle::IsAParticle 
( const LoKi::Particles::IsAParticle& right ) 
  : LoKi::Predicate<const LHCb::Particle*>( right ) 
  , LoKi::UniqueKeeper<LHCb::Particle>    ( right ) 
{} ;
// ============================================================================
/// MANDATORY: the only one essential method
// ============================================================================
LoKi::Particles::IsAParticle::result_type 
LoKi::Particles::IsAParticle::operator() 
  ( LoKi::Particles::IsAParticle::argument p ) const 
{
  if ( 0 == p ) 
  {
    Error    ( " Argument is invalid! return 'false' " );
    return false ;
  }
  if ( empty() ) 
  {
    Warning  ( " Empty list of particles is specified! return 'false' " );
    return false ;
  }
  // look for the particle 
  return std::binary_search ( begin() , end() , p ) ;
} ;
// ============================================================================


// ============================================================================
/// constructor from one particle 
// ============================================================================
LoKi::Particles::IsAParticleInTree::IsAParticleInTree 
( const LHCb::Particle*                 p )
  : LoKi::Predicate<const LHCb::Particle*> () 
  , m_cut ( p )
{};  
// ============================================================================
/// constructor from container of particles
// ============================================================================
LoKi::Particles::IsAParticleInTree::IsAParticleInTree
( const LHCb::Particles*                p ) 
  : LoKi::Predicate<const LHCb::Particle*> () 
  , m_cut ( p )
{};
// ============================================================================
/// constructor from container of particles
// ============================================================================
LoKi::Particles::IsAParticleInTree::IsAParticleInTree
( const SmartRefVector<LHCb::Particle>& p ) 
  : LoKi::Predicate<const LHCb::Particle*> () 
  , m_cut ( p )
{};  
// ============================================================================
/// constructor from vector of particles 
// ============================================================================
LoKi::Particles::IsAParticleInTree::IsAParticleInTree
( const LHCb::Particle::Vector&         p ) 
  : LoKi::Predicate<const LHCb::Particle*> () 
  , m_cut ( p )
{};  
// ============================================================================
/// constructor from vector of particles 
// ============================================================================
LoKi::Particles::IsAParticleInTree::IsAParticleInTree
( const LHCb::Particle::ConstVector&    p ) 
  : LoKi::Predicate<const LHCb::Particle*> () 
  , m_cut ( p )
{};  
// ============================================================================
/// constructor from container of particle 
// ============================================================================
LoKi::Particles::IsAParticleInTree::IsAParticleInTree
( const LoKi::PhysTypes::Range&         p ) 
  : LoKi::Predicate<const LHCb::Particle*> () 
  , m_cut ( p )
{};  
// ============================================================================
/// constructor from container of particle 
// ============================================================================
LoKi::Particles::IsAParticleInTree::IsAParticleInTree
( const LoKi::Keeper<LHCb::Particle>&   p ) 
  : LoKi::Predicate<const LHCb::Particle*> () 
  , m_cut ( p )
{};  
// ============================================================================
/// constructor from container of particle 
// ============================================================================
LoKi::Particles::IsAParticleInTree::IsAParticleInTree
( const LoKi::UniqueKeeper<LHCb::Particle>& p )       
  : LoKi::Predicate<const LHCb::Particle*> () 
  , m_cut ( p )
{};  
// ============================================================================
/// copy constructor 
// ============================================================================
LoKi::Particles::IsAParticleInTree::IsAParticleInTree
( const LoKi::Particles::IsAParticleInTree&  right ) 
  : LoKi::Predicate<const LHCb::Particle*> ( right ) 
  , m_cut                                  ( right.m_cut  )
{};  
// ============================================================================
/// MANDATORY: the only one essential method
// ============================================================================
LoKi::Particles::IsAParticleInTree::result_type 
LoKi::Particles::IsAParticleInTree::operator() 
  ( LoKi::Particles::IsAParticleInTree::argument p ) const
{
  if ( 0 == p ) 
  {
    Error    ( " Argument is invalid! return 'false' " );
    return false ;
  }
  if ( empty() ) 
  {
    Warning  ( " Empty list of particles is specified! return 'false' " );
    return false ;
  }
  // scan the tree 
  return LoKi::PhysAlgs::found ( p , m_cut ) ;
} ;
// ============================================================================



// ============================================================================
/// constructor from one particle 
// ============================================================================
LoKi::Particles::IsAParticleFromTree::IsAParticleFromTree 
( const LHCb::Particle*                 p ) 
  : LoKi::Predicate<const LHCb::Particle*>() 
  , LoKi::UniqueKeeper<LHCb::Particle> ( p ) 
{} ;
// ============================================================================
/// constructor from container of particles
// ============================================================================
LoKi::Particles::IsAParticleFromTree::IsAParticleFromTree 
( const LHCb::Particles*                p ) 
  : LoKi::Predicate<const LHCb::Particle*>() 
  , LoKi::UniqueKeeper<LHCb::Particle> ()
{ if ( 0 != p ) { addObjects( p->begin() , p->end() ) ; } };
// ============================================================================
/// constructor from container of particles
// ============================================================================
LoKi::Particles::IsAParticleFromTree::IsAParticleFromTree
( const SmartRefVector<LHCb::Particle>& p ) 
  : LoKi::Predicate<const LHCb::Particle*>() 
  , LoKi::UniqueKeeper<LHCb::Particle> ( p.begin() , p.end() ) 
{} ;
// ============================================================================
/// constructor from container of particles
// ============================================================================
LoKi::Particles::IsAParticleFromTree::IsAParticleFromTree
( const LHCb::Particle::Vector&         p ) 
  : LoKi::Predicate<const LHCb::Particle*>() 
  , LoKi::UniqueKeeper<LHCb::Particle> ( p.begin() , p.end() ) 
{} ;
// ============================================================================
/// constructor from container of particles
// ============================================================================
LoKi::Particles::IsAParticleFromTree::IsAParticleFromTree
( const LHCb::Particle::ConstVector&    p ) 
  : LoKi::Predicate<const LHCb::Particle*>() 
  , LoKi::UniqueKeeper<LHCb::Particle> ( p.begin() , p.end() ) 
{} ;
// ============================================================================
/// constructor from container of particles
// ============================================================================
LoKi::Particles::IsAParticleFromTree::IsAParticleFromTree 
( const LoKi::PhysTypes::Range&         p ) 
  : LoKi::Predicate<const LHCb::Particle*>() 
  , LoKi::UniqueKeeper<LHCb::Particle> ( p.begin() , p.end() ) 
{} ;
// ============================================================================
/// constructor from container of particles
// ============================================================================
LoKi::Particles::IsAParticleFromTree::IsAParticleFromTree
( const LoKi::Keeper<LHCb::Particle>&   p ) 
  : LoKi::Predicate<const LHCb::Particle*>() 
    , LoKi::UniqueKeeper<LHCb::Particle> ( p ) 
{} ;
// ============================================================================
/// constructor from container of particles
// ============================================================================
LoKi::Particles::IsAParticleFromTree::IsAParticleFromTree
( const LoKi::UniqueKeeper<LHCb::Particle>& p ) 
  : LoKi::Predicate<const LHCb::Particle*>() 
  , LoKi::UniqueKeeper<LHCb::Particle> ( p ) 
{} ;
// ============================================================================
/// copy constructor
// ============================================================================
LoKi::Particles::IsAParticleFromTree::IsAParticleFromTree 
( const LoKi::Particles::IsAParticleFromTree& right ) 
  : LoKi::Predicate<const LHCb::Particle*>( right ) 
  , LoKi::UniqueKeeper<LHCb::Particle>    ( right ) 
{} ;
// ============================================================================
/// MANDATORY: the only one essential method
// ============================================================================
LoKi::Particles::IsAParticleFromTree::result_type 
LoKi::Particles::IsAParticleFromTree::operator() 
  ( LoKi::Particles::IsAParticleFromTree::argument p ) const
{
  if ( 0 == p ) 
  {
    Error    ( " Argument is invalid! return 'false' " );
    return false ;                                             // RETURN 
  }
  if ( empty() ) 
  {
    Warning  ( " Empty list of particles is specified! return 'false' " );
    return false ;                                             // RETURN 
  }
  
  // find the particle by scanning of the decay trees of the other particles 
  for ( iterator ip = begin() ; end() != ip ; ++ip ) 
  {
    // find the particle by scanning of the decay tree of another particle  
    const bool found = LoKi::PhysAlgs::found 
      ( *ip , std::bind2nd ( std::equal_to<const LHCb::Particle*>() , p ) ) ;
    // OK ?
    if ( found ) { return true ; }                             // RETURN
  }
  
  return false ;                                               // RETURN 
} ;
// ============================================================================

// ============================================================================
// The END 
// ============================================================================

