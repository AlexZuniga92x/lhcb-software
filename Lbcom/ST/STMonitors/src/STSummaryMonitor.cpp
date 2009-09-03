// $Id: STSummaryMonitor.cpp,v 1.3 2009-09-03 10:23:14 mtobin Exp $

// Gaudi
#include "GaudiKernel/AlgFactory.h"

// LHCbKernel
#include "Kernel/STDetSwitch.h"
#include "Kernel/STDAQDefinitions.h"

// STTELL1Event
#include "Event/STSummary.h"

// AIDA
#include "AIDA/IHistogram1D.h"

// local
#include "STSummaryMonitor.h"

using namespace LHCb;
//using namespace STDAQ;

DECLARE_ALGORITHM_FACTORY( STSummaryMonitor);

//--------------------------------------------------------------------
//
//  STSummaryMonitor
//
//--------------------------------------------------------------------

STSummaryMonitor::STSummaryMonitor( const std::string& name, 
                                    ISvcLocator* pSvcLocator ) :
  ST::HistoAlgBase(name, pSvcLocator),
  c_binIDaSynch(0.5),
  c_binIDmissing(2.5), 
  c_binIDcorrupted(1.5),
  c_binIDrecovered(3.5)
{
  // constructer
  declareSTConfigProperty("InputData" , m_summaryLocation, 
                          STSummaryLocation::TTSummary);
  declareProperty("SuppressMissing",   m_suppressMissing   = false);
  declareProperty("SuppressRecovered", m_suppressRecovered = false);
  declareProperty("PipeLineSize",      m_pipeLineSize      = 187  );
}

StatusCode STSummaryMonitor::initialize()
{
  // Initialize ST::HistoAlgBase
  StatusCode sc = ST::HistoAlgBase::initialize();
  if (sc.isFailure()) return sc;  

  // Book histograms
  m_1d_pcn = book1D("pcn", "PCN distribution", 0, m_pipeLineSize, m_pipeLineSize );
  m_1d_errors = book1D("errors", "Error Info in Summary", 0., c_nErrorBins, c_nErrorBins);
  m_1d_dataSize = book1D("dataSize", "Data size (kB) per event", 0, 500 , 250);

  return StatusCode::SUCCESS;
}


StatusCode STSummaryMonitor::execute()
{ 
  // Skip if there is no Tell1 data
  if( !exist<STSummary>(m_summaryLocation) ) {
    return Warning("No data at given location", StatusCode::SUCCESS, 0);
  }

  // Get the data
  const STSummary* summary = get<STSummary>(m_summaryLocation);
  // debug() << "Found " << data->size() << " boards." << endmsg;
  
  // Filling the histograms
  
  // Fill PCN histogram
  m_1d_pcn->fill( summary->pcn() );
  
  // Fill error summary histogram
  if( !(summary->pcnSynch()) ) {
    m_1d_errors->fill( c_binIDaSynch );
  }
  if( (summary->corruptedBanks()).size() > 0 ) {
    m_1d_errors->fill( c_binIDcorrupted );
  }
  if ( !m_suppressMissing && (summary->missingBanks()).size() > 0 ) {
    m_1d_errors->fill( c_binIDmissing );
  }
  if ( !m_suppressRecovered && (summary->recoveredBanks()).size() > 0 ) {
    m_1d_errors->fill( c_binIDrecovered );
  }

  // Fill data size histogram
  m_1d_dataSize->fill( summary->rawBufferSize()/1024 );
  
  return StatusCode::SUCCESS;
}

StatusCode STSummaryMonitor::finalize()
{
  return ST::HistoAlgBase::finalize();// must be called after all other actions
}
