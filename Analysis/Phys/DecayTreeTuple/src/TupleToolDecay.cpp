// $Id: TupleToolDecay.cpp,v 1.4 2010-01-26 15:39:26 rlambert Exp $
// Include files

// from Gaudi
#include "GaudiKernel/ToolFactory.h"

// local
#include "TupleToolDecay.h"
#include "Kernel/IParticleTupleTool.h"
#include "Kernel/IDecayFinder.h"
#include "MCInterfaces//IMCDecayFinder.h"

#include "GaudiAlg/Tuple.h"
#include "GaudiAlg/TupleObj.h"

#include "Kernel/DVAlgorithm.h"

#include <boost/lexical_cast.hpp>

//-----------------------------------------------------------------------------
// Implementation file for class : TupleTool
//
// 2007-11-02 : Jeremie Borel
//-----------------------------------------------------------------------------

// Declaration of the Tool Factory
DECLARE_TOOL_FACTORY( TupleToolDecay );


using namespace Gaudi;
using namespace LHCb;


//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
TupleToolDecay::TupleToolDecay( const std::string& type,
                                const std::string& name,
                                const IInterface* parent )
  : GaudiTool ( type, name , parent )
  , m_hasMatched( false )
  , m_myName(name)
  , m_dkFinder(0)
  , m_mcdkFinder(0)
  , m_isMC(false)
{
  declareInterface<TupleToolDecay>(this);

  declareProperty( "ToolList", m_stufferList );
  declareProperty( "InheritTools", m_inheritTools = true );
}

//=============================================================================
// Destructor
//=============================================================================
TupleToolDecay::~TupleToolDecay() {}

//=============================================================================

StatusCode TupleToolDecay::initialize( const std::string& dcy, bool isMC ){
  if( ! GaudiTool::initialize() ) return StatusCode::FAILURE;

  m_isMC = isMC ;

  if (msgLevel(MSG::DEBUG)) debug() << "TupleToolDecay::initialize " << dcy << " " << isMC << endmsg ;

  std::string pname; // sets the name to "ToolName" and not to "AlgoParent.ToolName"
  const Algorithm* alg = dynamic_cast<const Algorithm*>( parent() );
  if( alg ) pname = alg->name();

  if( pname.empty() )
    Warning("Did someone change the name structure ???, your printout will not be nice..." );
  else {
    m_myName = m_myName.substr( pname.size()+1 );
  }

  bool test = true ;
  if (m_isMC){
    m_mcdkFinder = tool<IMCDecayFinder>("MCDecayFinder", this );
    test &= m_mcdkFinder->setDecay( dcy );
  } else {
    m_dkFinder = tool<IDecayFinder>("DecayFinder", this );
    test &= m_dkFinder->setDecay( dcy );
  }

  if (msgLevel(MSG::DEBUG)) debug() << "Initialized " << name()
                                    << " with decay " << decay() << endreq;

  std::sort( m_stufferList.begin(), m_stufferList.end() );
  m_stufferList.erase(std::unique(m_stufferList.begin(),m_stufferList.end()),
                      m_stufferList.end());

  return StatusCode(test);
}

//=============================================================================
// get decay
//=============================================================================
std::string TupleToolDecay::decay() const
{
  return ( m_isMC ? m_mcdkFinder->decay() : m_dkFinder->decay() );
}
