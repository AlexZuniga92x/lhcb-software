// $Id: TupleToolCPU.cpp,v 1.1 2010-08-19 13:50:45 pkoppenb Exp $
// Include files

// from Gaudi
#include "GaudiKernel/ToolFactory.h"

// local
#include "TupleToolCPU.h"

#include "GaudiAlg/Tuple.h"
#include "GaudiAlg/ITupleTool.h"
#include "Kernel/IOnOffline.h"
#include "GaudiKernel/Memory.h"
#include "GaudiAlg/ISequencerTimerTool.h"


//#include "GaudiAlg/TupleObj.h"
// #include "GaudiAlg/GaudiTupleAlg.h"


#include "GaudiKernel/IRegistry.h" // IOpaqueAddress

//-----------------------------------------------------------------------------
// Implementation file for class : CPUTupleTool
//
// 2010-08-19 Patrick Koppenburg
//-----------------------------------------------------------------------------

// Declaration of the Tool Factory
DECLARE_TOOL_FACTORY( TupleToolCPU );

//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
TupleToolCPU::TupleToolCPU( const std::string& type,
					const std::string& name,
					const IInterface* parent )
  : TupleToolBase ( type, name , parent )
  , m_timerTool(0)
  , m_timer(0)
{
  declareInterface<IEventTupleTool>(this);
}
//=============================================================================

StatusCode TupleToolCPU::initialize() {
  StatusCode sc = TupleToolBase::initialize(); // must be executed first
  m_timerTool = tool<ISequencerTimerTool>( "SequencerTimerTool" ); //global tool
  m_timer = m_timerTool->addTimer( name() );
  m_timerTool->start(m_timer) ; /// start it now
  
  return sc ;
}

//=============================================================================

StatusCode TupleToolCPU::fill( Tuples::Tuple& tuple ) 
{
  const std::string prefix=fullName();

  double t =  m_timerTool->stop(m_timer) ; /// stop
  if (msgLevel(MSG::DEBUG)) debug() << "Time is " << t  << endmsg ;
  m_timerTool->start(m_timer) ; /// start again

  bool test = true;
  test &= tuple->column( prefix+"Memory", (double)System::virtualMemory());
  test &= tuple->column( prefix+"CPUTime", t);
  return StatusCode(test);
}
