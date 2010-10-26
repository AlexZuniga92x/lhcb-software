// $Id$
// ============================================================================
// Include files
// ============================================================================
// STD & STL 
// ============================================================================
#include <algorithm>
// ============================================================================
// local
// ============================================================================
#include "LoKi/HltCandidates.h"
#include "LoKi/Constants.h"
// ============================================================================
/** @file
 *  Implementation of Hlt-Candidates functors 
 * 
 *  This file is part of LoKi project: 
 *   ``C++ ToolKit for Smart and Friendly Physics Analysis''
 * 
 *  By usage of this code one clearly states the disagreement 
 *  with the campain of Dr.O.Callot et al.: 
 *  ``No Vanya's lines are allowed in LHCb/Gaudi software.''
 *  
 *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
 *  @date 2010-08-01
 */
// ============================================================================
// MANDATORY: virtual desctructor 
// ============================================================================
LoKi::Candidates::TotalStages::~TotalStages () {}
// ============================================================================
// MANDATORY: clone method ("virtual constructor")
// ============================================================================
LoKi::Candidates::TotalStages*
LoKi::Candidates::TotalStages::clone() const 
{ return new LoKi::Candidates::TotalStages( *this ) ; }
// ============================================================================
// MANDATORY: the only one essential method 
// ============================================================================
LoKi::Candidates::TotalStages::result_type 
LoKi::Candidates::TotalStages::operator() 
  ( LoKi::Candidates::TotalStages::argument a ) const
{
  if ( 0 == a ) 
  {
    Error("Hlt::Candidate* points to NULL, return -1") ;
    return -1 ;
  }
  return a->stages().size();
}
// ============================================================================
// OPTIONAL: the nice printout 
// ============================================================================
std::ostream& LoKi::Candidates::TotalStages::fillStream ( std::ostream& s ) const
{ return s << "TC_TOTSTAGES" ; }


// ============================================================================
// constructor 
// ============================================================================
LoKi::Candidates::NStages::NStages 
( const  LoKi::TriggerTypes::TS_Cuts& cut ) 
  : LoKi::BasicFunctors<const Hlt::Candidate*>::Function () 
  , m_cut ( cut ) 
{}
// ============================================================================
// MANDATORY: virtual desctructor 
// ============================================================================
LoKi::Candidates::NStages::~NStages () {}
// ============================================================================
// MANDATORY: clone method ("virtual constructor")
// ============================================================================
LoKi::Candidates::NStages*
LoKi::Candidates::NStages::clone() const 
{ return new LoKi::Candidates::NStages( *this ) ; }
// ============================================================================
// MANDATORY: the only one essential method 
// ============================================================================
LoKi::Candidates::NStages::result_type 
LoKi::Candidates::NStages::operator() 
  ( LoKi::Candidates::NStages::argument a ) const
{
  if ( 0 == a ) 
  {
    Error("Hlt::Candidate* points to NULL, return -1") ;
    return -1 ;
  }
  const SmartRefVector<Hlt::Stage>& stages = a->stages() ;
  return std::count_if (  stages.begin () ,
                          stages.end   () , 
                          m_cut           ) ;
}
// ============================================================================
// OPTIONAL: the nice printout 
// ============================================================================
std::ostream& LoKi::Candidates::NStages::fillStream ( std::ostream& s ) const
{ return s << " TC_NSTAGES(" << m_cut << ") " ; }


// ============================================================================
// MANDATORY: virtual desctructor 
// ============================================================================
LoKi::Candidates::Branch::~Branch (){}
// ============================================================================
// MANDATORY: clone method ("virtual constructor")
// ============================================================================
LoKi::Candidates::Branch*
LoKi::Candidates::Branch::clone() const 
{ return new LoKi::Candidates::Branch( *this ) ; }
// ============================================================================
// MANDATORY: the only one essential method 
// ============================================================================
LoKi::Candidates::Branch::result_type 
LoKi::Candidates::Branch::operator() 
  ( LoKi::Candidates::Branch::argument a ) const
{
  if ( 0 == a ) 
  {
    Error("Hlt::Candidate* points to NULL, return false") ;
    return false ;
  }
  //
  const Hlt::Stage* s = a->initiatorStage() ;
  if ( 0 == s ) 
  {
    Error("Hlt::Stage* points to NULL, return false") ;
    return false ;
  }
  //
  return 
    s->is<Hlt::Candidate>() || s->is<Hlt::Stage>() ;
}
// ============================================================================
// OPTIONAL: the nice printout 
// ============================================================================
std::ostream& LoKi::Candidates::Branch::fillStream ( std::ostream& s ) const
{ return s << " TC_BRANCH " ; }

// ============================================================================
// constructor 
// ============================================================================
LoKi::Candidates::StageFun::StageFun
( const  LoKi::TriggerTypes::TS_Func& cut  , 
  const  int                          slot ) 
  : LoKi::BasicFunctors<const Hlt::Candidate*>::Function () 
  , m_cut  ( cut  ) 
  , m_slot ( slot ) 
{}
// ============================================================================
// MANDATORY: virtual desctructor 
// ============================================================================
LoKi::Candidates::StageFun::~StageFun () {}
// ============================================================================
// MANDATORY: clone method ("virtual constructor")
// ============================================================================
LoKi::Candidates::StageFun*
LoKi::Candidates::StageFun::clone() const 
{ return new LoKi::Candidates::StageFun( *this ) ; }
// ============================================================================
// MANDATORY: the only one essential method 
// ============================================================================
LoKi::Candidates::StageFun::result_type 
LoKi::Candidates::StageFun::operator() 
  ( LoKi::Candidates::StageFun::argument a ) const
{
  if ( 0 == a ) 
  {
    Error("Hlt::Candidate* points to NULL, return NegativeInfinity") ;
    return LoKi::Constants::NegativeInfinity ;
  }
  // 
  const Hlt::Stage* stage = a->get<Hlt::Stage> ( m_slot ) ;
  if ( 0 == stage ) 
  { 
    Error("Hlt::Stage* points to NULL, return NegativeInfinity") ;
    return LoKi::Constants::NegativeInfinity ;
  }
  //
  return m_cut ( stage ) ;
}
// ============================================================================
// OPTIONAL: the nice printout 
// ============================================================================
std::ostream& LoKi::Candidates::StageFun::fillStream ( std::ostream& s ) const
{ 
  s << " TC_StFUN(" << m_cut ;
  if ( 0 != m_slot ) { s << "," << m_slot ; }
  return s << ") ";
}




// ============================================================================
// constructor 
// ============================================================================
LoKi::Candidates::StageCut::StageCut
( const  LoKi::TriggerTypes::TS_Cuts& cut  , 
  const  int                          slot ) 
  : LoKi::BasicFunctors<const Hlt::Candidate*>::Predicate () 
  , m_cut  ( cut  ) 
  , m_slot ( slot ) 
{}
// ============================================================================
// MANDATORY: virtual desctructor 
// ============================================================================
LoKi::Candidates::StageCut::~StageCut () {}
// ============================================================================
// MANDATORY: clone method ("virtual constructor")
// ============================================================================
LoKi::Candidates::StageCut*
LoKi::Candidates::StageCut::clone() const 
{ return new LoKi::Candidates::StageCut ( *this ) ; }
// ============================================================================
// MANDATORY: the only one essential method 
// ============================================================================
LoKi::Candidates::StageCut::result_type 
LoKi::Candidates::StageCut::operator() 
  ( LoKi::Candidates::StageCut::argument a ) const
{
  if ( 0 == a ) 
  {
    Error ( "Hlt::Candidate* points to NULL, return false" ) ;
    return false ;
  }
  //
  const Hlt::Stage* stage = a->get<Hlt::Stage> ( m_slot ) ;
  if ( 0 == stage ) 
  { 
    Error ( "Hlt::Stage* points to NULL, return false" ) ;
    return false ;
  }
  //
  return m_cut ( stage ) ;
}
// ============================================================================
// OPTIONAL: the nice printout 
// ============================================================================
std::ostream& LoKi::Candidates::StageCut::fillStream ( std::ostream& s ) const
{ 
  s << " TC_StCUT(" << m_cut ;
  if ( 0 != m_slot ) { s << "," << m_slot ; }
  return s << ") ";
}

// ============================================================================
// The END 
// ============================================================================
