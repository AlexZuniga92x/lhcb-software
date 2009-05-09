// $Id: LoKiGenDict.h,v 1.12 2009-05-09 19:21:38 ibelyaev Exp $
// ============================================================================
#ifndef LOKI_LOKICOREDICT_H 
#define LOKI_LOKICOREDICT_H 1
// ============================================================================
// Include files
// ============================================================================
// HepMC 
// ============================================================================
#include "HepMC/GenVertex.h"
#include "HepMC/GenParticle.h"
// ============================================================================
// LoKi
// ============================================================================
#include "LoKi/LoKiGen.h"
#include "LoKi/Keeper.h"
#include "LoKi/UniqueKeeper.h"
#include "LoKi/Monitoring.h"
#include "LoKi/Operators.h"
#include "LoKi/Trees.h"
#include "LoKi/TreeOps.h"
// ============================================================================
#include "LoKi/Dicts.h"
#include "LoKi/GenExtractDicts.h"
#include "LoKi/GenAlgsDicts.h"
#include "LoKi/GenMoniDicts.h"
#include "LoKi/GenDecays.h"
// ============================================================================
#include "LoKi/IGenHybridTool.h"
#include "LoKi/GenHybridEngine.h"
// ============================================================================
/** @file
 *  The dictionaries for the package Phys/LoKiGen
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
 *  @date 2007-12-01
 */
// ============================================================================
namespace LoKi
{
  // ==========================================================================
  namespace Functors 
  {
    // ========================================================================
    // the specialized printout 
    // ========================================================================
    template <>
    inline std::ostream& Empty<const HepMC::GenVertex*>::fillStream  
    ( std::ostream& s ) const { return s << "GVEMPTY" ; }
    // ========================================================================
    // the specialized printout 
    // ========================================================================
    template <>
    inline std::ostream& Empty<const HepMC::GenParticle*>::fillStream
    ( std::ostream& s ) const { return s << "GEMPTY " ; }
    // ========================================================================
    // the specialized printout 
    // ========================================================================
    template <>
    inline std::ostream& Size<const HepMC::GenVertex*>::fillStream  
    ( std::ostream& s ) const { return s << "GVSIZE" ; }
    // ========================================================================
    // the specialized printout 
    // ========================================================================
    template <>
    inline std::ostream& 
    Size<const HepMC::GenParticle*>::fillStream 
    ( std::ostream& s ) const { return s << "GSIZE " ; }
    // ========================================================================
  } // end of namespace LoKi::Functors 
  // ==========================================================================
} // end of namespace LoKi 
// ============================================================================
namespace LoKi
{
  // ==========================================================================
  namespace Dicts 
  {
    // ========================================================================
    template <>
    class FunCalls<HepMC::GenParticle> 
    {
    private:
      // ======================================================================
      typedef HepMC::GenParticle          Type ;
      typedef LoKi::BasicFunctors<const Type*>::Function Fun  ;
      // ======================================================================
    public:
      // ======================================================================
      // __call__ 
      static Fun::result_type              
      __call__    ( const Fun& fun  , const Type*           o ) 
      { return fun ( o ) ; }
      // ======================================================================
    public:
      // ======================================================================
      // __rrshift__ 
      static Fun::result_type              
      __rrshift__ ( const Fun& fun  , const Type*           o ) 
      { return o >> fun  ; }
      // __rrshift__ 
      static std::vector<Fun::result_type> 
      __rrshift__ ( const Fun& fun  , const LoKi::GenTypes::GenContainer& o ) 
      { return o >> fun  ; }      
      // ======================================================================
    public:
      // ======================================================================
      // _rshift_ 
      static LoKi::FunctorFromFunctor<const Type*,double> __rshift__            
      ( const Fun&                          fun  , 
        const LoKi::Functor<double,double>& o    ) { return fun >> o  ; }
      // _rshift_ 
      static LoKi::FunctorFromFunctor<const Type*,bool>   __rshift__            
      ( const Fun&                          fun  , 
        const LoKi::Functor<double,bool>&   o    ) { return fun >> o  ; }
      // ======================================================================
    } ;
    // ========================================================================
    template <>
    class CutCalls<HepMC::GenParticle> 
    {
    private:
      // ======================================================================
      typedef HepMC::GenParticle           Type ;
      typedef LoKi::BasicFunctors<const Type*>::Predicate Fun  ;
      // ======================================================================
    public:
      // ======================================================================
      // __call__
      static Fun::result_type 
      __call__    ( const Fun& fun  , const Type*           o ) 
      { return fun ( o ) ; }
      // ======================================================================
    public:
      // ======================================================================
      // __rrshift__ 
      static Fun::result_type              
      __rrshift__ ( const Fun& fun  , const Type*           o ) 
      { return o >> fun  ; }
      // __rrshift__ 
      static const LoKi::GenTypes::GenContainer 
      __rrshift__ ( const Fun& fun  , const LoKi::GenTypes::GenContainer& o ) 
      { return o >> fun  ; }      
      // ======================================================================
    public:
      // ======================================================================
      // __rshift__ 
      static LoKi::FunctorFromFunctor<const Type*,bool>
      __rshift__ ( const Fun& fun  , const Fun& o ) 
      { return fun >> o   ; }      
      // ======================================================================
    } ;
    // ========================================================================
    template <>
    class FunCalls<HepMC::GenVertex> 
    {
    private:
      // ======================================================================
      typedef HepMC::GenVertex            Type ;
      typedef LoKi::BasicFunctors<const Type*>::Function Fun  ;
      // ======================================================================
    public:
      // ======================================================================
      // __call__
      static Fun::result_type __call__ 
      ( const Fun& fun  , const Type*           o ) { return fun ( o ) ; }
      // ======================================================================
    public:
      // ======================================================================
      // __rrshift__ 
      static Fun::result_type              
      __rrshift__ ( const Fun& fun  , const Type*           o ) 
      { return o >> fun  ; }
      // __rrshift__ 
      static std::vector<Fun::result_type> 
      __rrshift__ ( const Fun& fun  , const LoKi::GenTypes::GenVContainer& o ) 
      { return o >> fun  ; }      
      // ======================================================================
    public:
      // ======================================================================
      // _rshift_ 
      static LoKi::FunctorFromFunctor<const Type*,double> __rshift__            
      ( const Fun&                          fun  , 
        const LoKi::Functor<double,double>& o    ) { return fun >> o  ; }
      // _rshift_ 
      static LoKi::FunctorFromFunctor<const Type*,bool>   __rshift__            
      ( const Fun&                          fun  , 
        const LoKi::Functor<double,bool>&   o    ) { return fun >> o  ; }
      // ======================================================================
    } ;
    // ========================================================================
    template <>
    class CutCalls<HepMC::GenVertex> 
    {
    private:
      // ======================================================================
      typedef HepMC::GenVertex             Type ;
      typedef LoKi::BasicFunctors<const Type*>::Predicate Fun  ;
      // ======================================================================
    public:
      // ======================================================================
      // __call__
      static Fun::result_type 
      __call__    ( const Fun& fun  , const Type*           o ) 
      { return fun ( o ) ; }
      // ======================================================================
    public:
      // ======================================================================
      // __rrshift__ 
      static Fun::result_type              
      __rrshift__ ( const Fun& fun  , const Type*           o ) 
      { return o >> fun  ; }
      // __rrshift__ 
      static const LoKi::GenTypes::GenVContainer 
      __rrshift__ ( const Fun& fun  , const LoKi::GenTypes::GenVContainer& o ) 
      { return o >> fun  ; }      
      // ======================================================================
    public:
      // ======================================================================
      // __rrshift__ 
      static LoKi::FunctorFromFunctor<const Type*,bool>
      __rshift__ ( const Fun& fun  , const Fun& o ) 
      { return fun >> o  ; }      
      // ======================================================================
    } ;
    // ========================================================================
  } 
  // ==========================================================================
}
// ============================================================================
namespace
{
  // ==========================================================================
  struct _Instantiations 
  {    
    // ========================================================================
    // the basic types
    LoKi::Types::GRange                                    m_r1 ;
    LoKi::Types::GVRange                                   m_r2 ;
    // rangelists:
    LoKi::Types::GRangeL                                   m_l1 ;
    LoKi::Types::GVRangeL                                  m_l2 ;
    // keepers:
    LoKi::Keeper<HepMC::GenParticle>                       m_k1 ;
    LoKi::Keeper<HepMC::GenVertex>                         m_k2 ;
    // unique keepers
    LoKi::UniqueKeeper<HepMC::GenParticle>                 m_u1 ;
    LoKi::UniqueKeeper<HepMC::GenVertex>                   m_u2 ;
    // the basic functions 
    LoKi::Dicts::Funcs<const HepMC::GenParticle*>          m_f1 ;
    LoKi::Dicts::Funcs<const HepMC::GenVertex*>            m_f2 ;
    LoKi::Dicts::VFuncs<const HepMC::GenParticle*>         m_f3 ;
    LoKi::Dicts::VFuncs<const HepMC::GenVertex*>           m_f4 ;
    // operators 
    LoKi::Dicts::FuncOps<const HepMC::GenParticle*>        m_o1 ;
    LoKi::Dicts::FuncOps<const HepMC::GenVertex*>          m_o2 ;
    LoKi::Dicts::CutsOps<const HepMC::GenParticle*>        m_o3 ;
    LoKi::Dicts::CutsOps<const HepMC::GenVertex*>          m_o4 ;
    // functional parts:
    LoKi::Dicts::MapsOps<const HepMC::GenParticle*>        m_fo1  ;
    LoKi::Dicts::PipeOps<const HepMC::GenParticle*>        m_fo2  ;
    LoKi::Dicts::FunValOps<const HepMC::GenParticle*>      m_fo3  ;
    LoKi::Dicts::CutValOps<const HepMC::GenParticle*>      m_fo31 ;
    LoKi::Dicts::ElementOps<const HepMC::GenParticle*>     m_fo4  ;
    LoKi::Dicts::MapsOps<const HepMC::GenVertex*>          m_fo5  ;
    LoKi::Dicts::PipeOps<const HepMC::GenVertex*>          m_fo6  ;
    LoKi::Dicts::FunValOps<const HepMC::GenVertex*>        m_fo7  ;
    LoKi::Dicts::CutValOps<const HepMC::GenVertex*>        m_fo71 ;
    LoKi::Dicts::ElementOps<const HepMC::GenVertex*>       m_fo8  ;
    LoKi::Dicts::SourceOps<const HepMC::GenVertex*>        m_fo9  ;
    LoKi::Dicts::SourceOps<const HepMC::GenParticle*>      m_fo10 ;
    /// mathematics:
    LoKi::Dicts::FunCalls<HepMC::GenParticle>              m_c1 ;
    LoKi::Dicts::FunCalls<HepMC::GenVertex>                m_c2 ;
    LoKi::Dicts::CutCalls<HepMC::GenParticle>              m_c3 ;
    LoKi::Dicts::CutCalls<HepMC::GenVertex>                m_c4 ;
    /// the special operators for identifiers 
    LoKi::Dicts::PIDOps<LoKi::GenParticles::Identifier>    m_i1 ;
    LoKi::Dicts::PIDOps<LoKi::GenParticles::AbsIdentifier> m_i2 ;    
    /// same ? 
    LoKi::TheSame<const HepMC::GenParticle*> m_s1 ;
    LoKi::TheSame<const HepMC::GenVertex*>   m_s2 ;    
    /// trivia
    LoKi::Functors::Empty<const HepMC::GenParticle*> m_e1  ;
    LoKi::Functors::Size<const HepMC::GenParticle*>  m_si1 ;
    LoKi::Functors::Empty<const HepMC::GenVertex*>   m_e2  ;
    LoKi::Functors::Size<const HepMC::GenVertex*>    m_si2 ;    
    // decay funders:
    Decays::Tree_<const HepMC::GenParticle*>           m_tree1 ;
    Decays::Trees::Any_<const HepMC::GenParticle*>     m_tree3 ;
    LoKi::Dicts::TreeOps<const HepMC::GenParticle*>   m_trops ;
    /// fictive constructor 
    _Instantiations () ;
    // ========================================================================
  } ;  
  // ==========================================================================
} // end of anonymous namespace 
// ============================================================================
// The END 
// ============================================================================
#endif // LOKI_LOKICOREDICT_H
// ============================================================================
