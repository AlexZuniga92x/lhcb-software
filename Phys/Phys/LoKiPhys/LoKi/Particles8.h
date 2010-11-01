// $Id$
// ============================================================================
#ifndef LOKI_PARTICLES8_H 
#define LOKI_PARTICLES8_H 1
// ============================================================================
// Include files
// ============================================================================
// LoKiCore 
// ============================================================================
#include "LoKi/UniqueKeeper.h"
// ============================================================================
// LoKiPhys 
// ============================================================================
#include "LoKi/PhysTypes.h"
// ============================================================================
namespace LHCb { class ProtoParticle ; }
// ============================================================================
/** @file
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
 *  @date 2006-02-22 
 */
// ============================================================================
namespace LoKi
{
  // ==========================================================================
  namespace Particles 
  {
    // ========================================================================
    /** @class HasProtos
     *  Trivial class which check if the particle has a 
     *  protoparticle from the list 
     *  
     *  @see LHCb::Particle
     *  @see LHCb::Particle::proto
     *  @see LHCb::ProtoParticle
     * 
     *  @see LoKi::Cuts::HASPROTOS
     *  @see LoKi::Cuts::PROTOS
     *
     *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
     *  @date 2006-02-22 
     */     
    class GAUDI_API HasProtos
      : public LoKi::BasicFunctors<const LHCb::Particle*>::Predicate
      , public LoKi::UniqueKeeper<LHCb::ProtoParticle>
    {
    public:
      // ======================================================================
      /// constructor from one protoparticle 
      HasProtos ( const LHCb::ProtoParticle* proto ) ;
      /// constructor from vector of protoparticles 
      HasProtos ( const LHCb::ProtoParticle::ConstVector& ps ) ;
      // ======================================================================
      /** templated constructor from sequence of ptoroparticles 
       *  @param first 'begin'-iterator of the sequence 
       *  @param last  'end'-iterator of the sequence 
       */
      template <class PROTO>
      HasProtos 
      ( PROTO first , 
        PROTO last  ) 
        : LoKi::BasicFunctors<const LHCb::Particle*>::Predicate ()
        , LoKi::UniqueKeeper<LHCb::ProtoParticle> ( first , last ) 
      {} 
      /** templated constructor from sequence of ptoroparticles 
       *  @param first 'begin'-iterator of the sequence 
       *  @param last  'end'-iterator of the sequence 
       */
      template <class PROTO>
      HasProtos 
      ( const LoKi::Keeper<PROTO>& keeper )
        : LoKi::BasicFunctors<const LHCb::Particle*>::Predicate ()
        , LoKi::UniqueKeeper<LHCb::ProtoParticle> ( keeper.begin() , 
                                                    keeper.end  () ) 
      {} 
      /** templated constructor from sequence of ptoroparticles 
       *  @param first 'begin'-iterator of the sequence 
       *  @param last  'end'-iterator of the sequence 
       */
      template <class PROTO>
      HasProtos 
      ( const LoKi::UniqueKeeper<PROTO>& keeper )
        : LoKi::BasicFunctors<const LHCb::Particle*>::Predicate ()
        , LoKi::UniqueKeeper<LHCb::ProtoParticle> ( keeper.begin() , 
                                                    keeper.end  () ) 
      {} 
      /// MANDATORY: virtual destructor
      virtual ~HasProtos() {};
      /// MANDATORY: clone method ("virtual constructor")
      virtual  HasProtos* clone() const { return new HasProtos(*this); }
      /// MANDATORY: the only one essential method
      virtual  result_type operator() ( argument p ) const ;
      /// OPTIONAL: the specific printout 
      virtual std::ostream& fillStream( std::ostream& s ) const ;
      // ======================================================================
    private:
      // ======================================================================
      HasProtos() ;
      // ======================================================================
    } ;
    // ========================================================================    
    /** @class HasProtosInTree
     *  Trivial class which check if the particle (or any of its 
     *  daughter particle) has a 
     *  protoparticle from the list 
     *  
     *  @see LHCb::Particle
     *  @see LHCb::Particle::proto
     *  @see LHCb::ProtoParticle
     * 
     *  @see LoKi::Cuts::HASPROTOSINTREE
     *  @see LoKi::Cuts::PROTOSINTREE
     *
     *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
     *  @date 2006-02-22 
     */
    class GAUDI_API HasProtosInTree
      : public LoKi::BasicFunctors<const LHCb::Particle*>::Predicate
    {
    public:
      // ======================================================================
      /// constructor from one protoparticle 
      HasProtosInTree 
      ( const LHCb::ProtoParticle* proto ) ;
      HasProtosInTree
      ( const LHCb::ProtoParticle::ConstVector& ps ) ;
      // ======================================================================
      /** templated constructor from sequence of ptoroparticles 
       *  @param first 'begin'-iterator of the sequence 
       *  @param last  'end'-iterator of the sequence 
       */
      template <class PROTO>
      HasProtosInTree 
      ( PROTO first , 
        PROTO last  ) 
        : LoKi::BasicFunctors<const LHCb::Particle*>::Predicate ()
        , m_cut ( first , last ) 
      {} 
      /** templated constructor from sequence of ptoroparticles 
       *  @param first 'begin'-iterator of the sequence 
       *  @param last  'end'-iterator of the sequence 
       */
      template <class PROTO>
      HasProtosInTree 
      ( const LoKi::Keeper<PROTO>& keeper )
        : LoKi::BasicFunctors<const LHCb::Particle*>::Predicate ()
        , m_cut ( keeper.begin() , keeper.end() )
      {} 
      /** templated constructor from sequence of ptoroparticles 
       *  @param first 'begin'-iterator of the sequence 
       *  @param last  'end'-iterator of the sequence 
       */
      template <class PROTO>
      HasProtosInTree 
      ( const LoKi::UniqueKeeper<PROTO>& keeper )
        : LoKi::BasicFunctors<const LHCb::Particle*>::Predicate ()
        , m_cut ( keeper.begin() , keeper.end() )
      {} 
      // ======================================================================
      /// MANDATORY: virtual destructor
      virtual ~HasProtosInTree() {};
      /// MANDATORY: clone method ("virtual constructor")
      virtual  HasProtosInTree* clone() const 
      { return new HasProtosInTree(*this); }
      /// MANDATORY: the only one essential method
      virtual  result_type operator() ( argument p ) const ;
      /// OPTIONAL: the specific printout 
      virtual std::ostream& fillStream( std::ostream& s ) const ;
      // ======================================================================
    private: 
      // ======================================================================
      HasProtosInTree() ;
      // ======================================================================
    private: 
      // ======================================================================
      LoKi::Particles::HasProtos m_cut ;
      // ======================================================================
    } ;
    // ========================================================================    
  }  //                                        end of namespace LoKi::Particles
  // ==========================================================================
} //                                                      end of namespace LoKi
// ============================================================================
//                                                                      The END 
// ============================================================================
#endif // LOKI_PARTICLES8_H
// ============================================================================
