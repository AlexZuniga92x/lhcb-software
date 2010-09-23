// $Id: TriggerTisTosInHlt.cpp,v 1.4 2010-07-21 21:22:17 tskwarni Exp $
// Include files 
#include <algorithm>
#include <vector>
#include <sstream>

// from Gaudi
#include "GaudiKernel/StatusCode.h"
#include "GaudiKernel/StringKey.h"
#include "GaudiKernel/ToolFactory.h" 

// local
#include "TriggerTisTosInHlt.h"

#include "boost/regex.hpp"

#include "Event/HltDecReports.h"


using namespace LHCb;

//-----------------------------------------------------------------------------
// Implementation file for class : TriggerTisTosInHlt
//
// 2007-08-20 : Tomasz Skwarnicki
//-----------------------------------------------------------------------------

// Declaration of the Tool Factory
DECLARE_TOOL_FACTORY( TriggerTisTosInHlt );

const std::vector< std::string > TriggerTisTosInHlt::m_empty_selections = std::vector< std::string >();

//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
TriggerTisTosInHlt::TriggerTisTosInHlt( const std::string& type,
                              const std::string& name,
                              const IInterface* parent )
  : TriggerSelectionTisTosInHlt ( type, name , parent )
  , m_inspector(0)
{
  declareInterface<ITriggerTisTos>(this);

  declareProperty("TriggerInputWarnings", m_trigInputWarn = false );
  declareProperty("AllowIntermediateSelections", m_allowIntermediateSelections = false );

}
//=============================================================================
// Destructor
//=============================================================================
TriggerTisTosInHlt::~TriggerTisTosInHlt() {} 

//=============================================================================
// Initialization
//=============================================================================
StatusCode TriggerTisTosInHlt::initialize() {
  StatusCode sc = TriggerSelectionTisTosInHlt::initialize(); // must be executed first
  if ( sc.isFailure() ) return sc;  // error printed already by GaudiAlgorithm

  debug() << "==> Initialize" << endmsg;

  m_inspector = svc<Hlt::IInspector>("Hlt::Service",true);

  setOfflineInput();
  setTriggerInput();
  
  return StatusCode::SUCCESS;
}

//=============================================================================


void TriggerTisTosInHlt::getTriggerNames()
{
  if( m_newEvent ){
    m_triggerNames.clear(); 
    m_newEvent = false;
  }

  // done before ?
  if( !(m_triggerNames.empty()) ){ return; }


  // get trigger names from HltDecReports and HltSelReports
  if( !m_hltDecReports )getHltSummary();
  //    for the same TCK this should be fixed list for events which passed Hlt
  if( m_hltDecReports ){
    m_triggerNames = m_hltDecReports->decisionNames();
  }

  if( m_allowIntermediateSelections ){    
    Hlt::IInspector::KeyList selIDs;
    m_inspector->selections(selIDs);
    for( Hlt::IInspector::KeyList::const_iterator i =
          selIDs.begin(); i!=selIDs.end(); ++i) {
      if( find( m_triggerNames.begin(), m_triggerNames.end(), *i )
          == m_triggerNames.end() ){
        m_triggerNames.push_back(*i);
      }
    }
  }
  
  if( m_triggerNames.empty() ){
    Warning( "No known trigger names found" , StatusCode::FAILURE, 1 ).setChecked();
  }
  
}

void TriggerTisTosInHlt::setTriggerInput()
{
  m_triggerInput_Selections.clear();
}

void TriggerTisTosInHlt::addToTriggerInput( const std::string & selectionNameWithWildChar)
{
#if 0
  // if selectionNameWithWildChar contains a * without a . in front of it: print warning...
  static boost::regex warn("[^\\.]\\*");
  if ( boost::regex_search( selectionNameWithWildChar, warn )  ) {
        Warning( " addToTriggerInput now does Posix (Perl) regular expression matching instead of globbing;" 
                 " this implies that eg. a '*' should be replaced by '.*'. You've specified "
                 " a selectionName using a '*', without leading '.': '" + selectionNameWithWildChar 
               + "'. Please verify whether this is what you still want\n "
               " For more information on the supported syntax, please check "
               " http://www.boost.org/doc/libs/1_39_0/libs/regex/doc/html/boost_regex/syntax/perl_syntax.html"
                 ,StatusCode::SUCCESS 
                 ).ignore();
  }
#endif
  unsigned int sizeAtEntrance( m_triggerInput_Selections.size() );
  getTriggerNames();
  boost::regex expr(selectionNameWithWildChar);
  for( std::vector< std::string >::const_iterator inpt=m_triggerNames.begin();inpt!=m_triggerNames.end();++inpt){
    if( boost::regex_match( *inpt, expr ) ) {
      if( find( m_triggerInput_Selections.begin(), m_triggerInput_Selections.end(), *inpt ) 
          == m_triggerInput_Selections.end() ){
        m_triggerInput_Selections.push_back(*inpt);
      }
    }
  }
  if( m_trigInputWarn && (m_triggerInput_Selections.size()==sizeAtEntrance) ){
    std::ostringstream mess;
    mess << " addToTriggerInput called with selectionNameWithWildChar=" << selectionNameWithWildChar
         << " added no selection to the Trigger Input, which has size=" << m_triggerInput_Selections.size();
    Warning( mess.str(),StatusCode::SUCCESS, 50 ).setChecked();
  }
}
 

unsigned int TriggerTisTosInHlt::tisTosTrigger()
{
  unsigned int result=0;  
  if( m_triggerInput_Selections.empty() ){
    if( m_trigInputWarn )Warning(" tisTosTrigger called with empty Trigger Input").setChecked();
    return result;
  }
  for( std::vector< std::string >::const_iterator iTriggerSelection=m_triggerInput_Selections.begin();
       iTriggerSelection!=m_triggerInput_Selections.end(); ++iTriggerSelection){
    unsigned int res = tisTosSelection( *iTriggerSelection );
    if( res & kDecision ){      
      result |= res;
    }  
    if( (result & kTOS) && (result & kTIS) && (result & kTPS ) )break;  
  }
  return result;  
}

// analysisReport for Trigger (define Trigger & Offline Input before calling)
std::string TriggerTisTosInHlt::analysisReportTrigger(){
  std::ostringstream report;
  report << offset() 
         << " Trigger #-of-sel " <<   m_triggerInput_Selections.size() << std::endl;
  unsigned int result=0;  
  if( m_triggerInput_Selections.empty() )
    { report << "Trigger Input empty" << std::endl; return report.str();}
  for( std::vector< std::string >::const_iterator iTriggerSelection=m_triggerInput_Selections.begin();
       iTriggerSelection!=m_triggerInput_Selections.end(); ++iTriggerSelection){
    unsigned int res = tisTosSelection( *iTriggerSelection );
    ++m_reportDepth;
    if( res & kDecision ){      
      report << analysisReportSelection( *iTriggerSelection );
      result |= res;
    } else {
      report << offset() << " Selection " + *iTriggerSelection + " decision=false " << std::endl;      
    }
    --m_reportDepth;
    
    //    if( (result & kTOS) && (result & kTIS) && (result & kTPS ) )break;  
  }
  TisTosTob res( result );
  report << offset() 
         << " Trigger #-of-sel " <<   m_triggerInput_Selections.size()
         << " TIS= " << res.tis() << " TOS= " << res.tos() << " TPS= " << res.tps() 
         << " decision= " << res.decision() << std::endl;
  return report.str();
}

// fast check for TOS
bool TriggerTisTosInHlt::tosTrigger(){ 
  if( m_triggerInput_Selections.empty() ){
    if( m_trigInputWarn )Warning(" tosTrigger called with empty Trigger Input").setChecked();
    return false;
  }
  for( std::vector< std::string >::const_iterator iTriggerSelection=m_triggerInput_Selections.begin();
       iTriggerSelection!=m_triggerInput_Selections.end(); ++iTriggerSelection){
    if( tosSelection( *iTriggerSelection ) )return true;
  }
  return false;  
}

// fast check for TIS
bool TriggerTisTosInHlt::tisTrigger(){
  if( m_triggerInput_Selections.empty() ){
    if( m_trigInputWarn )Warning(" tisTrigger called with empty Trigger Input").setChecked();
    return false;
  }
  for( std::vector< std::string >::const_iterator iTriggerSelection=m_triggerInput_Selections.begin();
       iTriggerSelection!=m_triggerInput_Selections.end(); ++iTriggerSelection){
    if( tisSelection( *iTriggerSelection ) )return true;
  }
  return false;  
}

// fast check for TIS
bool TriggerTisTosInHlt::tusTrigger(){
  if( m_triggerInput_Selections.empty() ){
    if( m_trigInputWarn )Warning(" tpsTrigger called with empty Trigger Input").setChecked();
    return false;
  }
  for( std::vector< std::string >::const_iterator iTriggerSelection=m_triggerInput_Selections.begin();
       iTriggerSelection!=m_triggerInput_Selections.end(); ++iTriggerSelection){
    if( tusSelection( *iTriggerSelection ) )return true;
  }
  return false;  
}

std::vector< std::string > TriggerTisTosInHlt::triggerSelectionNames(unsigned int decisionRequirement,
                                                                     unsigned int tisRequirement,
                                                                     unsigned int tosRequirement,
                                                                     unsigned int tpsRequirement)
{ 
  if( (decisionRequirement>=kAnything) && ( tisRequirement>=kAnything) && ( tosRequirement>=kAnything) 
      && ( tpsRequirement>=kAnything) )
  {
    return m_triggerInput_Selections;
  }
  std::vector< std::string > selections;  
  for( std::vector<std::string>::const_iterator iSel = m_triggerInput_Selections.begin();
       iSel != m_triggerInput_Selections.end(); ++iSel ){
    bool decision,tis,tos,tps;
    unsigned int result = tisTosSelection( *iSel);
    decision = result & kDecision;
    tos = result & kTOS;
    tis = result & kTIS;
    tps = result & kTPS;    
    if( ((decisionRequirement>=kAnything)||(decision==decisionRequirement))
        && ((tisRequirement>=kAnything)||(tis==tisRequirement)) 
        && ((tosRequirement>=kAnything)||(tos==tosRequirement)) 
        && ((tpsRequirement>=kAnything)||(tps==tpsRequirement)) ){
      selections.push_back( *iSel );
    }
  }
  return selections;  
}



std::vector<const LHCb::HltObjectSummary*> TriggerTisTosInHlt::hltObjectSummaries( unsigned int tisRequirement,
                                                                                   unsigned int tosRequirement,
                                                                                   unsigned int tpsRequirement)
{
  std::vector<const LHCb::HltObjectSummary*> hosVec;  
  if( m_triggerInput_Selections.empty() ){
    if( m_trigInputWarn )Warning(" hltObjectSummaries called with empty Trigger Input").setChecked();
    return hosVec;
  }
  for( std::vector< std::string >::const_iterator iTriggerSelection=m_triggerInput_Selections.begin();
       iTriggerSelection!=m_triggerInput_Selections.end(); ++iTriggerSelection){
    std::vector<const LHCb::HltObjectSummary*> selHosVec = 
      hltSelectionObjectSummaries(*iTriggerSelection,tisRequirement,tosRequirement,tpsRequirement);
    hosVec.insert(hosVec.end(),selHosVec.begin(),selHosVec.end());
  }
  return hosVec;
}

  
