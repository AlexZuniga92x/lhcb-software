// $Id: $

// local
#include "RecProcessingTimeMoni.h"

//-----------------------------------------------------------------------------
// Implementation file for class : RecProcessingTimeMoni
//
// 2010-07-15 : Chris Jones
//-----------------------------------------------------------------------------

// Declaration of the Algorithm Factory
DECLARE_ALGORITHM_FACTORY( RecProcessingTimeMoni );

//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
RecProcessingTimeMoni::RecProcessingTimeMoni( const std::string& name,
                                              ISvcLocator* pSvcLocator)
  : GaudiHistoAlg ( name , pSvcLocator )
{
  declareProperty( "Algorithms",   m_algNames );
  declareProperty( "LogMinEventTime", m_logMinTime = -3 );
  declareProperty( "LogMaxEventTime", m_logMaxTime =  8 );
}

//=============================================================================
// Destructor
//=============================================================================
RecProcessingTimeMoni::~RecProcessingTimeMoni() {} 

//=============================================================================
// Initialization
//=============================================================================
StatusCode RecProcessingTimeMoni::initialize() 
{
  StatusCode sc = GaudiHistoAlg::initialize(); 
  if ( sc.isFailure() ) return sc;  

  // are we properly configured
  if ( m_algNames.empty() ) { sc = Warning( "No algorithms to time !"); }

  return sc;
}

//=============================================================================
// Main execution
//=============================================================================
StatusCode RecProcessingTimeMoni::execute() 
{

  // Loop over algorithms to include in the timing and add them up
  double time = 0;
  for ( AlgorithmNames::const_iterator name = m_algNames.begin();
        name != m_algNames.end(); ++name )
  {
    time += chronoSvc()->chronoDelta((*name)+":execute",IChronoStatSvc::ELAPSED)/1000;
  }

  // Take the base 10 log of the time (helps show the large tails)
  const double logtime = std::log10(time);
  
  plot1D( logtime, "overallTime", "log10(Event Processing Time / ms)", 
          m_logMinTime, m_logMaxTime, 100 );
  
  return StatusCode::SUCCESS;
}

//=============================================================================
