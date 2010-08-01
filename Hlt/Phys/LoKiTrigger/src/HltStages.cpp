// $Id: $
// ============================================================================
// Include files 
// ============================================================================
// LoKi
// ============================================================================
#include "LoKi/TriggerTypes.h"
#include "LoKi/HltStages.h"
#include "LoKi/Constants.h"
// ============================================================================
/** @file
 *  Implementation for Hlt-Stages functors 
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
 *
 *  @see LoKi::TriggerTypes 
 *  @see Hlt::Candidate 
 */
// ============================================================================
// MANDATORY: virtual destructor 
// ============================================================================
LoKi::Stages::IsTrack::~IsTrack(){}
// ============================================================================
// MANDATORY: clone method ("virtual constructor")
// ============================================================================
LoKi::Stages::IsTrack*
LoKi::Stages::IsTrack::clone() const 
{ return new LoKi::Stages::IsTrack(*this) ; }
// ============================================================================
// MANDATORY: the only one essential method 
// ============================================================================
LoKi::Stages::IsTrack::result_type 
LoKi::Stages::IsTrack::operator() 
  ( LoKi::Stages::IsTrack::argument a ) const 
{
  if ( 0 == a ) { Error ("Hlt::Stage* points to NULL, return false " ) ; }
  return 0 != a ? a->is<LHCb::Track>() : false ;
}
// ============================================================================
// OPTIONAL: the ince printout 
// ============================================================================
std::ostream& LoKi::Stages::IsTrack::fillStream ( std::ostream& s ) const 
{ return s << "TS_TRACK" ; }


// ============================================================================
// MANDATORY: virtual destructor 
// ============================================================================
LoKi::Stages::IsL0Muon::~IsL0Muon(){}
// ============================================================================
// MANDATORY: clone method ("virtual constructor")
// ============================================================================
LoKi::Stages::IsL0Muon*
LoKi::Stages::IsL0Muon::clone() const 
{ return new LoKi::Stages::IsL0Muon(*this) ; }
// ============================================================================
// MANDATORY: the only one essential method 
// ============================================================================
LoKi::Stages::IsL0Muon::result_type 
LoKi::Stages::IsL0Muon::operator() 
  ( LoKi::Stages::IsL0Muon::argument a ) const 
{
  if ( 0 == a ) { Error ("Hlt::Stage points to NULL, return false " ) ; }
  return 0 != a ? a->is<LHCb::L0MuonCandidate>() : false ;
}
// ============================================================================
// OPTIONAL: the ince printout 
// ============================================================================
std::ostream& LoKi::Stages::IsL0Muon::fillStream ( std::ostream& s ) const 
{ return s << "TS_L0MUON" ; }


// ============================================================================
// MANDATORY: virtual destructor 
// ============================================================================
LoKi::Stages::IsL0DiMuon::~IsL0DiMuon(){}
// ============================================================================
// MANDATORY: clone method ("virtual constructor")
// ============================================================================
LoKi::Stages::IsL0DiMuon*
LoKi::Stages::IsL0DiMuon::clone() const 
{ return new LoKi::Stages::IsL0DiMuon(*this) ; }
// ============================================================================
// MANDATORY: the only one essential method 
// ============================================================================
LoKi::Stages::IsL0DiMuon::result_type 
LoKi::Stages::IsL0DiMuon::operator() 
  ( LoKi::Stages::IsL0DiMuon::argument a ) const 
{
  if ( 0 == a ) { Error ("Hlt::Stage points to NULL, return false " ) ; }
  return 0 != a ? a->is<Hlt::L0DiMuonCandidate>() : false ;
}
// ============================================================================
// OPTIONAL: the ince printout 
// ============================================================================
std::ostream& LoKi::Stages::IsL0DiMuon::fillStream ( std::ostream& s ) const 
{ return s << "TS_L0DIMUON" ; }


// ============================================================================
// MANDATORY: virtual destructor 
// ============================================================================
LoKi::Stages::IsL0Calo::~IsL0Calo(){}
// ============================================================================
// MANDATORY: clone method ("virtual constructor")
// ============================================================================
LoKi::Stages::IsL0Calo*
LoKi::Stages::IsL0Calo::clone() const 
{ return new LoKi::Stages::IsL0Calo(*this) ; }
// ============================================================================
// MANDATORY: the only one essential method 
// ============================================================================
LoKi::Stages::IsL0Calo::result_type 
LoKi::Stages::IsL0Calo::operator() 
  ( LoKi::Stages::IsL0Calo::argument a ) const 
{
  if ( 0 == a ) { Error ("Hlt::Stage points to NULL, return false " ) ; }
  return 0 != a ? a->is<LHCb::L0CaloCandidate>() : false ;
}
// ============================================================================
// OPTIONAL: the ince printout 
// ============================================================================
std::ostream& LoKi::Stages::IsL0Calo::fillStream ( std::ostream& s ) const 
{ return s << "TS_L0CALO" ; }
// ============================================================================

// ============================================================================
// MANDATORY: virtual destructor 
// ============================================================================
LoKi::Stages::IsVertex::~IsVertex (){}
// ============================================================================
// MANDATORY: clone method ("virtual constructor")
// ============================================================================
LoKi::Stages::IsVertex*
LoKi::Stages::IsVertex::clone() const 
{ return new LoKi::Stages::IsVertex(*this) ; }
// ============================================================================
// MANDATORY: the only one essential method 
// ============================================================================
LoKi::Stages::IsVertex::result_type 
LoKi::Stages::IsVertex::operator() 
  ( LoKi::Stages::IsVertex::argument a ) const 
{
  if ( 0 == a ) { Error ("Hlt::Stage points to NULL, return false " ) ; }
  return 0 != a ? a->is<LHCb::RecVertex>() : false ;
}
// ============================================================================
// OPTIONAL: the ince printout 
// ============================================================================
std::ostream& LoKi::Stages::IsVertex::fillStream ( std::ostream& s ) const 
{ return s << "TS_VERTEX" ; }
// ============================================================================



// ============================================================================
// MANDATORY: virtual destructor 
// ============================================================================
LoKi::Stages::IsMultiTrack::~IsMultiTrack (){}
// ============================================================================
// MANDATORY: clone method ("virtual constructor")
// ============================================================================
LoKi::Stages::IsMultiTrack*
LoKi::Stages::IsMultiTrack::clone() const 
{ return new LoKi::Stages::IsMultiTrack(*this) ; }
// ============================================================================
// MANDATORY: the only one essential method 
// ============================================================================
LoKi::Stages::IsMultiTrack::result_type 
LoKi::Stages::IsMultiTrack::operator() 
  ( LoKi::Stages::IsMultiTrack::argument a ) const 
{
  if ( 0 == a ) { Error ("Hlt::Stage points to NULL, return false " ) ; }
  return 0 != a ? a->is<Hlt::MultiTrack>() : false ;
}
// ============================================================================
// OPTIONAL: the ince printout 
// ============================================================================
std::ostream& LoKi::Stages::IsMultiTrack::fillStream ( std::ostream& s ) const 
{ return s << "TS_MULTITRACK" ; }
// ============================================================================


// ============================================================================
// MANDATORY: virtual destructor 
// ============================================================================
LoKi::Stages::IsStage::~IsStage (){}
// ============================================================================
// MANDATORY: clone method ("virtual constructor")
// ============================================================================
LoKi::Stages::IsStage*
LoKi::Stages::IsStage::clone() const 
{ return new LoKi::Stages::IsStage(*this) ; }
// ============================================================================
// MANDATORY: the only one essential method 
// ============================================================================
LoKi::Stages::IsStage::result_type 
LoKi::Stages::IsStage::operator() 
  ( LoKi::Stages::IsStage::argument a ) const 
{
  if ( 0 == a ) { Error ("Hlt::Stage points to NULL, return false " ) ; }
  return 0 != a ? a->is<Hlt::Stage>() : false ;
}
// ============================================================================
// OPTIONAL: the ince printout 
// ============================================================================
std::ostream& LoKi::Stages::IsStage::fillStream ( std::ostream& s ) const 
{ return s << "TS_STRAGE" ; }
// ============================================================================

// ============================================================================
// MANDATORY: virtual destructor 
// ============================================================================
LoKi::Stages::IsCandidate::~IsCandidate (){}
// ============================================================================
// MANDATORY: clone method ("virtual constructor")
// ============================================================================
LoKi::Stages::IsCandidate*
LoKi::Stages::IsCandidate::clone() const 
{ return new LoKi::Stages::IsCandidate(*this) ; }
// ============================================================================
// MANDATORY: the only one essential method 
// ============================================================================
LoKi::Stages::IsCandidate::result_type 
LoKi::Stages::IsCandidate::operator() 
  ( LoKi::Stages::IsCandidate::argument a ) const 
{
  if ( 0 == a ) { Error ("Hlt::Stage points to NULL, return false " ) ; }
  return 0 != a ? a->is<Hlt::Candidate>() : false ;
}
// ============================================================================
// OPTIONAL: the ince printout 
// ============================================================================
std::ostream& LoKi::Stages::IsCandidate::fillStream ( std::ostream& s ) const 
{ return s << "TS_CANDIDATE" ; }
// ============================================================================


// ============================================================================
// MANDATORY: virtual destructor 
// ============================================================================
LoKi::Stages::Type::~Type(){}
// ============================================================================
// MANDATORY: clone method ("virtual constructor")
// ============================================================================
LoKi::Stages::Type*
LoKi::Stages::Type::clone() const 
{ return new LoKi::Stages::Type(*this) ; }
// ============================================================================
// MANDATORY: the only one essential method 
// ============================================================================
LoKi::Stages::Type::result_type 
LoKi::Stages::Type::operator() 
  ( LoKi::Stages::Type::argument a ) const 
{
  if ( 0 == a ) 
  { 
    Error ("Hlt::Stage points to NULL, return Unknown" ) ; 
    return Hlt::Stage::Unknown ;
  }
  return a->stageType() ;
}
// ============================================================================
// OPTIONAL: the ince printout 
// ============================================================================
std::ostream& LoKi::Stages::Type::fillStream ( std::ostream& s ) const 
{ return s << "TS_TYPE" ; }
// ============================================================================



// ============================================================================
// constructor 
// ============================================================================
LoKi::Stages::TrFun::TrFun
( const LoKi::TrackTypes::TrFunc& fun , 
  const double                    bad )
  : LoKi::BasicFunctors<const Hlt::Stage*>::Function () 
  , m_fun ( fun ) 
  , m_bad ( bad ) 
{}
// ============================================================================
// constructor 
// ============================================================================
LoKi::Stages::TrFun::TrFun
( const LoKi::TrackTypes::TrFunc& fun )
  : LoKi::BasicFunctors<const Hlt::Stage*>::Function () 
  , m_fun ( fun ) 
  , m_bad ( LoKi::Constants::NegativeInfinity ) 
{}
// ============================================================================
// MANDATORY: virtual destructor 
// ============================================================================
LoKi::Stages::TrFun::~TrFun(){}
// ============================================================================
// MANDATORY: clone method ("virtual constructor") 
// ============================================================================
LoKi::Stages::TrFun*
LoKi::Stages::TrFun::clone() const 
{ return new LoKi::Stages::TrFun ( *this ) ; }
// ============================================================================
// MANDATORY: the only one essential method 
// ============================================================================
LoKi::Stages::TrFun::result_type 
LoKi::Stages::TrFun::operator() 
  ( LoKi::Stages::TrFun::argument a ) const
{
  //
  if ( 0 == a ) 
  { 
    Error ("Hlt::Stage points to NULL, return 'bad'" ) ; 
    return m_bad ;
  }
  //
  const LHCb::Track* track = a->get<LHCb::Track>() ;  
  if ( 0 == track ) 
  { 
    Error ("LHCb::Track* points to NULL, return 'bad'" ) ; 
    return m_bad ;
  }
  //
  return m_fun ( track ) ;
}
// ============================================================================
// OPTIONAL: the ince printout 
// ============================================================================
std::ostream& LoKi::Stages::TrFun::fillStream ( std::ostream& s ) const 
{
  s << " TS_TrFUN(" << m_fun ;
  if ( LoKi::Constants::NegativeInfinity != m_bad ) 
  { s << "," << m_bad ; }
  return s << ") " ;
}
// ============================================================================



// ============================================================================
// constructor 
// ============================================================================
LoKi::Stages::TrCut::TrCut
( const LoKi::TrackTypes::TrCuts& fun , 
  const bool                      bad )
  : LoKi::BasicFunctors<const Hlt::Stage*>::Predicate () 
  , m_fun ( fun ) 
  , m_bad ( bad ) 
{}
// ============================================================================
// constructor 
// ============================================================================
LoKi::Stages::TrCut::TrCut
( const LoKi::TrackTypes::TrCuts& fun )
  : LoKi::BasicFunctors<const Hlt::Stage*>::Predicate () 
  , m_fun ( fun   ) 
  , m_bad ( false ) 
{}
// ============================================================================
// MANDATORY: virtual destructor 
// ============================================================================
LoKi::Stages::TrCut::~TrCut(){}
// ============================================================================
// MANDATORY: clone method ("virtual constructor") 
// ============================================================================
LoKi::Stages::TrCut*
LoKi::Stages::TrCut::clone() const 
{ return new LoKi::Stages::TrCut ( *this ) ; }
// ============================================================================
// MANDATORY: the only one essential method 
// ============================================================================
LoKi::Stages::TrCut::result_type 
LoKi::Stages::TrCut::operator() 
  ( LoKi::Stages::TrCut::argument a ) const
{
  //
  if ( 0 == a ) 
  { 
    Error ("Hlt::Stage points to NULL, return 'bad'" ) ; 
    return m_bad ;
  }
  //
  const LHCb::Track* track = a->get<LHCb::Track>() ;  
  if ( 0 == track ) 
  { 
    Error ("LHCb::Track* points to NULL, return 'bad'" ) ; 
    return m_bad ;
  }
  //
  return m_fun ( track ) ;
}
// ============================================================================
// OPTIONAL: the ince printout 
// ============================================================================
std::ostream& LoKi::Stages::TrCut::fillStream ( std::ostream& s ) const 
{
  s << " TS_TrCut(" << m_fun ;
  if ( m_bad ) { s << ",True" ; }
  return s << ") " ;
}
// ============================================================================


 

// ============================================================================
// MANDATORY: virtual destructor 
// ============================================================================
LoKi::Stages::Locked::~Locked(){}
// ============================================================================
// MANDATORY: clone method ("virtual constructor")
// ============================================================================
LoKi::Stages::Locked*
LoKi::Stages::Locked::clone() const 
{ return new LoKi::Stages::Locked(*this) ; }
// ============================================================================
// MANDATORY: the only one essential method 
// ============================================================================
LoKi::Stages::Locked::result_type 
LoKi::Stages::Locked::operator() 
  ( LoKi::Stages::IsTrack::argument a ) const 
{
  if ( 0 == a ) 
  {
    Error ("Hlt::Stage* points to NULL, return false " ) ; 
    return false ;
  }
  return 0 != a->locked() ;
}
// ============================================================================
// OPTIONAL: the ince printout 
// ============================================================================
std::ostream& LoKi::Stages::Locked::fillStream ( std::ostream& s ) const 
{ return s << "TS_LOCKED" ; }




// ============================================================================
// The END 
// ============================================================================
