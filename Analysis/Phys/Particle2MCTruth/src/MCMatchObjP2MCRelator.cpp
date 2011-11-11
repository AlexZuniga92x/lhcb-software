// $Id: MCMatchObjP2MCRelator.cpp,v 1.15 2009-10-27 14:08:49 ibelyaev Exp $
// Include files

// from Gaudi
#include "GaudiKernel/ToolFactory.h"
// LoKi
#include "LoKi/IReporter.h"
// P2MCP
#include "P2MCP/P2MCPFunctors.h"
// local
#include "MCMatchObjP2MCRelator.h"
// boost
#include "boost/algorithm/string.hpp"

//-----------------------------------------------------------------------------
// Implementation file for class : MCMatchObjP2MCRelator
//
// 2009-03-04 : Juan PALACIOS
//-----------------------------------------------------------------------------

// Declaration of the Tool Factory
DECLARE_TOOL_FACTORY( MCMatchObjP2MCRelator );


//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
MCMatchObjP2MCRelator::MCMatchObjP2MCRelator( const std::string& type,
                                              const std::string& name,
                                              const IInterface* parent )
  : base_class( type, name , parent )
  , m_reporter (0)
  , m_matcher  (0)
  , m_tables   ( )
  , m_loaded   ( false )
{
  m_tables.push_back ( "Relations/" + LHCb::ProtoParticleLocation::Charged  ) ;
  m_tables.push_back ( "Relations/" + LHCb::ProtoParticleLocation::Upstream ) ;
  m_tables.push_back ( "Relations/" + LHCb::ProtoParticleLocation::Neutrals ) ;

  declareProperty    ( "RelTableLocations",  m_tables ) ;
}
//=============================================================================
StatusCode MCMatchObjP2MCRelator::initialize()
{
  StatusCode sc =  base_class::initialize();
  if (sc.isFailure()) return sc;
  m_reporter = tool<LoKi::IReporter>( "LoKi::Reporter", this ) ;

  if (msgLevel(MSG::VERBOSE)) {
    verbose() << "RelTableLocations: " << endmsg;
    for (Addresses::const_iterator iAddr = m_tables.begin(); iAddr!= m_tables.end(); ++iAddr) {
      verbose() << "\t" << *iAddr << endmsg;
    }
  }

  m_incSvc = svc<IIncidentSvc>("IncidentSvc", true);
  if (0!=m_incSvc) m_incSvc->addListener( this, IncidentType::EndEvent);

  // create the new matcher
  m_matcher = new LoKi::MCMatchObj( "P2MCRelator" , m_reporter ) ;

  if (0==m_matcher) return StatusCode::SUCCESS;

  // increment the reference counter
  m_matcher->addRef() ;

  m_loaded = false ;
  m_pLocDoneMap.clear();

  return (0!=m_reporter && 0!=m_matcher && 0!= m_incSvc) ? StatusCode::SUCCESS : StatusCode::FAILURE;
}
//=============================================================================
StatusCode MCMatchObjP2MCRelator::finalize()
{
  m_incSvc=0;

  if ( 0 != m_matcher )
  {
    m_matcher -> clear () ;
    long count = m_matcher->refCount();
    while ( 0 < count ) { count = m_matcher->release() ; }
    m_matcher = 0 ;
  }

  m_loaded = false ;
  m_pLocDoneMap.clear();
  return base_class::finalize();
}
//=============================================================================
bool MCMatchObjP2MCRelator::isMatched(const LHCb::Particle* particle,
                                      const LHCb::MCParticle* mcParticle) const
{
  if ( !particle ) { mcParticle = NULL; return false; }

  if ( !m_loaded ) { addTables() ; m_loaded = true ; }

  bool match = matcher()->match( particle, mcParticle );

  if ( !match )
  {
    // No match found the 'normal' way. Try the Particle location dependent (uDST) way.
    addTable( particle );
    match = matcher()->match( particle, mcParticle );
  }

  if ( msgLevel(MSG::VERBOSE) )
  {
    if ( match )
    {
      verbose() << "match Particle PID " << particle->particleID().pid()
                << " to MCParticle PID " << mcParticle->particleID().pid()
                << endmsg;
    }
    else
    {
      verbose() << "No match for Particle PID "
                << particle->particleID().pid()
                << endmsg;
    }
  }

  return match;
}
//=============================================================================
void MCMatchObjP2MCRelator::addTable(const LHCb::Particle* particle) const
{
  // Try Particle specific locations for relations tables
  std::string pLoc = objectLocation( particle->parent() );
  if ( !m_pLocDoneMap[pLoc] )
  {
    m_pLocDoneMap[pLoc] = true;

    // Form the relations TES location for this Particles location
    boost::replace_all( pLoc, "/Particles", "/P2MCPRelations" );

    // load the table
    loadTable(pLoc);
  }
}
//=============================================================================
void MCMatchObjP2MCRelator::addTables() const
{
  for ( Addresses::const_iterator item = m_tables.begin(); 
        item != m_tables.end(); ++item )
  {
    loadTable( *item );
  }
  m_loaded = true ;
}
//=============================================================================
void MCMatchObjP2MCRelator::loadTable(const std::string & address) const
{

  // First try respecting the RootInTES settings.
  if      (exist<LoKi::Types::TablePP2MC>(address) )
  {
    LoKi::Types::TablePP2MC* table = get<LoKi::Types::TablePP2MC>(address);
    m_matcher->addMatchInfo(table);
  }
  else if (exist<LoKi::Types::TableP2MC>(address) )
  {
    LoKi::Types::TableP2MC* table = get<LoKi::Types::TableP2MC>(address);
    m_matcher->addMatchInfo(table);
  }

  // If that fails, try without RootInTES
  else if (exist<LoKi::Types::TablePP2MC>(address,false) )
  {
    LoKi::Types::TablePP2MC* table = get<LoKi::Types::TablePP2MC>(address,false);
    m_matcher->addMatchInfo(table);
  }
  else if (exist<LoKi::Types::TableP2MC>(address,false) )
  {
    LoKi::Types::TableP2MC* table = get<LoKi::Types::TableP2MC>(address,false);
    m_matcher->addMatchInfo(table);
  }

  // No good ...

  else
  {
    Warning ( " There is no valid data at '" + address + "'", 
              StatusCode::FAILURE, 0 ).ignore() ;
  }

}
//=============================================================================
void MCMatchObjP2MCRelator::handle(const Incident&)
{
  if ( 0 != m_matcher ) { m_matcher->clear(); }
  m_loaded = false ;
  m_pLocDoneMap.clear();
}
//=============================================================================
// Destructor
//=============================================================================
MCMatchObjP2MCRelator::~MCMatchObjP2MCRelator() {}
//=============================================================================
