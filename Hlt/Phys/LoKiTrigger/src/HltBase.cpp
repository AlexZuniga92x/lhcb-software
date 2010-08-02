// $Id$
// ============================================================================
// Include files 
// ============================================================================
// Kernel
// ============================================================================
#include "Kernel/IANNSvc.h"
// ============================================================================
// LoKi
// ============================================================================
#include "LoKi/ILoKiSvc.h"
#include "LoKi/HltBase.h"
// ============================================================================
/** @file 
 *  Implementation file for class Hlt::Base
 *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
 *  @date 2009-03-19
 */
// ============================================================================
/*  standard contructor
 *  @param name algorithm instance name 
 *  @param pSvc pointer to Service Locator 
 */
// ============================================================================
Hlt::Base::Base
( const std::string& name ,                     //      algorithm instance name 
  ISvcLocator*       pSvc )                     //   pointer to Service Locator 
  : GaudiHistoAlg( name , pSvc )
  // services 
  , m_regSvc ( 0 ) 
  , m_hltSvc ( 0 )
  , m_annSvc ( 0 )
{
  // 
  Assert ( setProperty ( "RegisterForContextService" , true ).isSuccess() ,
           "Unable to enforce the registration for Algorithm Context Service") ;
  //
}
// ============================================================================
// virtual and protected destructor 
// ============================================================================
Hlt::Base::~Base() {}
// ============================================================================
// initialize the algorithm 
// ============================================================================
StatusCode Hlt::Base::initialize () 
{
  // initialize the base 
  StatusCode sc = GaudiHistoAlg::initialize () ;
  if ( sc.isFailure() ) { return sc ; }                               // RETURN
  // set the Lock 
  
  // locate LoKi Service 
  svc<LoKi::ILoKiSvc>( "LoKiSvc" , true ) ;
  // check other importat services 
  Assert ( 0 != hltSvc() , "Unable to aquire Hlt Data     Service" ) ;
  Assert ( 0 != regSvc() , "Unable to aquire Hlt Register Service" ) ;
  //
  return StatusCode::SUCCESS ;
}
// ============================================================================
// finalize the algorithm 
// ============================================================================
StatusCode Hlt::Base::finalize () 
{
  // disable the services 
  m_regSvc = 0 ;
  m_hltSvc = 0 ;
  // finalize the base class 
  return GaudiHistoAlg::finalize () ;
}
// ============================================================================
// accessor to Hlt Registration Service 
// ============================================================================
Hlt::IRegister* Hlt::Base::regSvc() const 
{
  if ( 0 != m_regSvc ) { return m_regSvc ; }
  m_regSvc = svc<Hlt::IRegister> ( "Hlt::Service" , true ) ;
  return m_regSvc ;
}
// ============================================================================
// accessor to Hlt Data Service 
// ============================================================================
Hlt::IData* Hlt::Base::hltSvc() const 
{
  if ( 0 != m_hltSvc ) { return m_hltSvc ; }
  m_hltSvc = svc<Hlt::IData> ( "Hlt::Service" , true ) ;
  return m_hltSvc ;
}
// ============================================================================
// accessor to Assigned Numbers & Names service 
// ============================================================================
IANNSvc*   Hlt::Base::annSvc() const 
{
  if ( 0 != m_annSvc ) { return m_annSvc ; }
  m_annSvc = svc<IANNSvc>       ( "Hlt::Service" , true ) ;
  return m_annSvc ;
}
// ============================================================================
// name   -> number for "InfoID" using IANNSvc
// ============================================================================
int Hlt::Base::hltInfoID 
( const std::string& name ) const 
{
  boost::optional<IANNSvc::minor_value_type> i =  
    annSvc() -> value ( "InfoID" , name ) ;
  Assert ( i, " request for unknown Info ID" ) ;
  return i->second;
}
// ============================================================================
// number -> name   for "InfoID" using IANNSvc
// ============================================================================
std::string Hlt::Base::hltInfoName 
( const int id ) const  
{
  boost::optional<IANNSvc::minor_value_type> i =  
    annSvc() -> value ( "InfoID" , id );
  Assert( i, " request for unknown Info ID");
  return i->first;
}
// ============================================================================
// get or create the TES-container of Hlt-Candidates 
// ============================================================================
Hlt::Candidate::Container* Hlt::Base::hltCandidates 
( const std::string& location ) const 
{ 
  IDataProviderSvc* s = evtSvc() ;
  return 
    getOrCreate< Hlt::Candidate::Container, 
    Hlt::Candidate::Container>  ( s , location ) ; 
}      
// ============================================================================
// get or create the TES-container of Hlt-Stages 
// ============================================================================
Hlt::Stage::Container*     Hlt::Base::hltStages  
( const std::string& location ) const 
{
  IDataProviderSvc* s = evtSvc() ;
  return 
    getOrCreate< Hlt::Stage::Container, 
    Hlt::Stage::Container>  ( s , location ) ; 
}      
// ============================================================================
// The END 
// ============================================================================

