// $Id: $
// ============================================================================
// Include files 
// ============================================================================
// LoKiTrigger
// ============================================================================
#include "LoKi/VxMakerConf.h"
// ============================================================================
/** @file  
 *  Implementation file for class  LoKi::Hlt1::VxMakerConf
 *  @see LoKi::Hlt1::VxMakerConf
 *  
 *  This file is part of LoKi project: 
 *   ``C++ ToolKit for Smart and Friendly Physics Analysis''
 * 
 *  The package has been designed with the kind help from
 *  Galina PAKHLOVA and Sergey BARSUK.  Many bright ideas, 
 *  contributions and advices from G.Raven, J.van Tilburg, 
 *  A.Golutvin, P.Koppenburg have been used in the design.
 *
 *  By usage of this code one clearly states the disagreement 
 *  with the campain of Dr.O.Callot et al.: 
 *  ``No Vanya's lines are allowed in LHCb/Gaudi software.''
 *  
 *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
 *  @date   2008-11-12
 *
 *                    $Revision: 53561 $
 *  Last Modification $Date$ 
 *                 by $Author$
 */
// ============================================================================
//  consructor from the verter making parameters 
// ============================================================================
LoKi::Hlt1::VxMakerConf::VxMakerConf
( const double docamax , 
  const double chi2max )
//
  : m_track1cut ( LoKi::Constant<const LHCb::Track*     , bool> ( true ) ) 
  , m_track2cut ( LoKi::Constant<const LHCb::Track*     , bool> ( true ) ) 
  , m_vxcut     ( LoKi::Constant<const LHCb::VertexBase*, bool> ( true ) ) 
//
  , m_track1cut_trivial ( true )
  , m_track2cut_trivial ( true ) 
  , m_vxcut_trivial     ( true ) 
//
  , m_docamax ( docamax ) 
  , m_chi2max ( chi2max ) 
{}
// ============================================================================
//  Full constructor form the vertex making parameters & cuts
// ============================================================================
LoKi::Hlt1::VxMakerConf::VxMakerConf
( const double docamax , 
  const double chi2max , 
  const LoKi::Functor<const LHCb::Track*     ,bool>& c1 , 
  const LoKi::Functor<const LHCb::Track*     ,bool>& c2 , 
  const LoKi::Functor<const LHCb::VertexBase*,bool>& c3 ) 
//
  : m_track1cut ( c1 )
  , m_track2cut ( c2 ) 
  , m_vxcut     ( c3 ) 
//
  , m_track1cut_trivial ( false )
  , m_track2cut_trivial ( false ) 
  , m_vxcut_trivial     ( false ) 
//
  , m_docamax ( docamax ) 
  , m_chi2max ( chi2max ) 
{}
// ============================================================================
//  Constructor form the vertex making parameters & cuts
// ============================================================================
LoKi::Hlt1::VxMakerConf::VxMakerConf
( const double docamax , 
  const double chi2max , 
  const LoKi::Functor<const LHCb::Track*     ,bool>& c1 , 
  const LoKi::Functor<const LHCb::Track*     ,bool>& c2 )
//
  : m_track1cut ( c1 ) 
  , m_track2cut ( c2 ) 
  , m_vxcut     ( LoKi::Constant<const LHCb::VertexBase*, bool> ( true ) ) 
//
  , m_track1cut_trivial ( false )
  , m_track2cut_trivial ( false ) 
  , m_vxcut_trivial     ( true  ) 
//
  , m_docamax ( docamax ) 
  , m_chi2max ( chi2max ) 
{}
// ============================================================================
// Constructor form the vertex making parameters & cuts
// ============================================================================
LoKi::Hlt1::VxMakerConf::VxMakerConf
( const double docamax , 
  const double chi2max , 
  const LoKi::Functor<const LHCb::Track*     ,bool>& c1 , 
  const LoKi::Functor<const LHCb::VertexBase*,bool>& c2 ) 
//
  : m_track1cut ( c1 ) 
  , m_track2cut ( c1 ) 
  , m_vxcut     ( c2 ) 
//
  , m_track1cut_trivial ( false )
  , m_track2cut_trivial ( false ) 
  , m_vxcut_trivial     ( false ) 
//
  , m_docamax ( docamax ) 
  , m_chi2max ( chi2max ) 
{}
// ============================================================================
//  Constructor form the vertex making parameters & cuts
// ============================================================================
LoKi::Hlt1::VxMakerConf::VxMakerConf
( const double docamax , 
  const double chi2max , 
  const LoKi::Functor<const LHCb::Track*     ,bool>& c1 )
//
  : m_track1cut ( c1 ) 
  , m_track2cut ( c1 ) 
  , m_vxcut     ( LoKi::Constant<const LHCb::VertexBase*, bool> ( true ) ) 
//
  , m_track1cut_trivial ( false )
  , m_track2cut_trivial ( false ) 
  , m_vxcut_trivial     ( true  ) 
//
  , m_docamax ( docamax ) 
  , m_chi2max ( chi2max ) 
{}
// ============================================================================
// Constructor from the vertex making parameters & cuts
// ============================================================================
LoKi::Hlt1::VxMakerConf::VxMakerConf
( const double docamax , 
  const double chi2max , 
  const LoKi::Functor<const LHCb::VertexBase*,bool>& c1 ) 
//
  : m_track1cut ( LoKi::Constant<const LHCb::Track*     , bool> ( true ) ) 
  , m_track2cut ( LoKi::Constant<const LHCb::Track*     , bool> ( true ) ) 
  , m_vxcut     ( c1 ) 
//
  , m_track1cut_trivial ( true  )
  , m_track2cut_trivial ( true  ) 
  , m_vxcut_trivial     ( false ) 
//
  , m_docamax ( docamax ) 
  , m_chi2max ( chi2max ) 
{}
// ============================================================================
// virtual destructor 
// ============================================================================
LoKi::Hlt1::VxMakerConf::~VxMakerConf(){}
// ============================================================================
// printout 
// ============================================================================
std::ostream& LoKi::Hlt1::VxMakerConf::fillStream 
( std::ostream& s ) const 
{
  s << "LoKi.Hlt1.VxMakerConf(" << docamax() << "," << chi2max() ;
  //
  if ( !track1cut_trivial() ) { s << "," << m_track1cut ; }
  if ( !track2cut_trivial() ) { s << "," << m_track2cut ; }
  if ( !    vxcut_trivial() ) { s << "," << m_vxcut     ; }
  //
  return s << ")" ;
}
// ============================================================================
// conversion to the string 
// ============================================================================
std::string LoKi::Hlt1::VxMakerConf::toString () const 
{
  std::ostringstream s ;
  fillStream ( s ) ;
  return s.str() ;
}
// ============================================================================
// output operator to ostream  
// ============================================================================
std::ostream& operator<<( std::ostream& s ,const LoKi::Hlt1::VxMakerConf& o ) 
{ return o.fillStream ( s ) ; }
// ============================================================================


// ============================================================================
// The END 
// ============================================================================
