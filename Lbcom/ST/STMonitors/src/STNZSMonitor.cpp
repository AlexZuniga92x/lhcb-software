// $Id: STNZSMonitor.cpp,v 1.16 2010-04-12 13:13:24 mtobin Exp $

// Gaudi
#include "GaudiKernel/AlgFactory.h"

// LHCbKernel
#include "Kernel/STDAQDefinitions.h"
#include "Kernel/STBoardMapping.h"
#include "Kernel/ISTReadoutTool.h"
#include "Kernel/LHCbConstants.h"

// ODIN..
#include "Event/ODIN.h"

// STTELL1Event
#include "Event/STTELL1Data.h"

// AIDA/root histograms
#include "AIDA/IProfile1D.h"
#include "AIDA/IHistogram2D.h"
#include "GaudiUtils/Aida2ROOT.h"
#include "TProfile.h"
#include "TH2D.h"

// standard
#include "gsl/gsl_math.h"
#include "boost/lexical_cast.hpp"

// local
#include "STNZSMonitor.h"
#include "Kernel/ISTNoiseCalculationTool.h"

using namespace LHCb;
using namespace AIDA;
using namespace STDAQ;
using namespace STBoardMapping;

DECLARE_ALGORITHM_FACTORY( STNZSMonitor);

//--------------------------------------------------------------------
//
//  STNZSMonitor
//
//--------------------------------------------------------------------

STNZSMonitor::STNZSMonitor( const std::string& name, 
                            ISvcLocator* pSvcLocator ) :
  ST::HistoAlgBase(name, pSvcLocator)
{
  // constructer
  declareProperty("UseSourceID", m_useSourceID = true );
  declareProperty("UpdateRate", m_updateRate = -1);  

  // Debugging
  declareProperty("CheckNoiseCalculation", m_checkCalculation=false);
  // Limit calculation to vector of tell1s given in terms of TELLID (eg TTTELL1 = 1) 
  declareProperty("LimitToTell",      m_limitToTell     ); 

  // Use ODIN time in histograms
  declareProperty("UseODINTime", m_useODINTime = false);

  /// Noise calculation tool
  declareProperty("NoiseToolType",m_noiseToolType="ST::STNoiseCalculationTool");
  declareSTConfigProperty("NoiseToolName",m_noiseToolName,"TTNoiseCalculationTool");

  /// Plot RAW noise
  declareProperty("UseRawNoise", m_rawNoise=false);
}

StatusCode STNZSMonitor::initialize() {
  // Initialize ST::HistoAlgBase
  StatusCode sc = ST::HistoAlgBase::initialize();
  if (sc.isFailure()) return sc;

  m_evtNumber = 0;

  m_noiseTool = tool<ST::ISTNoiseCalculationTool>(m_noiseToolType, m_noiseToolName);

  // Read following period, reset rate and skip events from configuration of tool
  m_followingPeriod = m_noiseTool->followPeriod();
  m_resetRate = m_noiseTool->resetRate();
  m_skipEvents = m_noiseTool->skipEvents();

  // Select small number of TELL1s (useful for debugging) then book histograms
  m_nTELL1s = readoutTool()->nBoard();
  m_selectedTells = false;
  if ( m_limitToTell.size() > 0 ) {
    m_selectedTells = true;
    sort(m_limitToTell.begin(), m_limitToTell.end());
  }
 
  bookHistograms();

  return StatusCode::SUCCESS;
}

void STNZSMonitor::bookHistograms() {
  // Get the tell1 mapping from source ID to tell1 number
  std::map<unsigned int, unsigned int>::const_iterator itT = (this->readoutTool())->SourceIDToTELLNumberMap().begin();
  for(; itT != (this->readoutTool())->SourceIDToTELLNumberMap().end(); ++itT) {
    unsigned int sourceID = (*itT).first;
    // Limit to selected tell1s
    if ( m_selectedTells ) {
      if (!binary_search(m_limitToTell.begin(), m_limitToTell.end(), (this->readoutTool())->SourceIDToTELLNumber(sourceID))) {
        continue;
      }
    }
    unsigned int tellID = m_useSourceID ? sourceID : (*itT).second;
    // Create a title for the histogram
    std::string strTellID  = boost::lexical_cast<std::string>(tellID);
    HistoID noiseHistoID        = "noise_$tell" + strTellID;
    std::string noiseHistoTitle = "Noise for " + detType() + "TELL" + strTellID;
    m_noiseHistos[sourceID] = bookProfile1D(noiseHistoID, noiseHistoTitle, -0.5, nStripsPerBoard-0.5, nStripsPerBoard);
    HistoID pedHistoID        = "pedestal_$tell" + strTellID;
    std::string pedHistoTitle = "Pedestal for " + detType() + "TELL" + strTellID;
    m_pedestalHistos[sourceID] = bookProfile1D(pedHistoID, pedHistoTitle, -0.5, nStripsPerBoard-0.5, nStripsPerBoard);
  }
  m_2d_NoisePerLinkVsTell1 = 
    Gaudi::Utils::Aida2ROOT::aida2root( book2D("Noise per link vs TELL1", 0.5, m_nTELL1s+0.5, m_nTELL1s, 0., 96., 96) );
  m_1dp_AvNoisePerTell1 = bookProfile1D("Average Noise vs TELL1", 0.5, m_nTELL1s+0.5, m_nTELL1s);
  m_2d_PedestalPerLinkVsTell1 = 
    Gaudi::Utils::Aida2ROOT::aida2root( book2D("Pedestal per link vs TELL1", 0.5, m_nTELL1s+0.5, m_nTELL1s, 0., 96., 96) );
  m_1dp_AvPedestalPerTell1 = bookProfile1D("Average Pedestal vs TELL1", 0.5, m_nTELL1s+0.5, m_nTELL1s);
  m_2d_NormalisationPerLinkVsTell1 = 
    Gaudi::Utils::Aida2ROOT::aida2root( book2D("Normalisation", 0.5, m_nTELL1s+0.5, m_nTELL1s, 0., 96., 96) );

}

StatusCode STNZSMonitor::execute() { 
  m_evtNumber++;

  // Get the time of the first event and convert to a string for the histogram title.
  if(m_evtNumber == 1) {
    if(m_useODINTime) {
      m_ODIN = get<ODIN>(LHCb::ODINLocation::Default); 
      const Gaudi::Time odinTime = m_ODIN->eventTime();
      m_odinEvent  = "(#"+boost::lexical_cast<std::string>(m_ODIN->runNumber());
      m_odinEvent += " on "+boost::lexical_cast<std::string>(odinTime.day(0));
      m_odinEvent += "/"+boost::lexical_cast<std::string>(odinTime.month(0)+1);
      m_odinEvent += "/"+boost::lexical_cast<std::string>(odinTime.year(0));
      m_odinEvent += " @ "+boost::lexical_cast<std::string>(odinTime.hour(0)); 
      m_odinEvent += ":"+boost::lexical_cast<std::string>(odinTime.minute(0)); 
      m_odinEvent += ":"+boost::lexical_cast<std::string>(odinTime.second(0))+")"; 
    }
  }

  // Skip first m_skipEvents. Useful when running over CMS data.
  if( m_evtNumber < m_skipEvents ) {
    return StatusCode::SUCCESS;
  }

  // loop over the boards which contained an NZS bank
  std::vector<unsigned int>::const_iterator itT = m_noiseTool->tell1WithNZSBegin();
  for(; itT != m_noiseTool->tell1WithNZSEnd(); ++itT) {
    // Flag to check if histogram needs to be updated
    bool needToUpdate = false;
    
    unsigned int sourceID = (*itT);

    // Loop over number of events for FPGA-PP and see if the histograms need to be reset
    std::vector<unsigned int>::const_iterator itEvts = m_noiseTool->cmsNEventsPPBegin(sourceID);
    for(; itEvts != m_noiseTool->cmsNEventsPPEnd(sourceID); ++itEvts) {
      
      int nEvt = (*itEvts);

      // Check if at least one of the PPs requires to update the histogram
      if( m_updateRate > 0 && nEvt%m_updateRate == 0 && nEvt != 0 ) {
        needToUpdate = true;
      }
    } // FPGA-PP
    // Update the noise histogram
    if( needToUpdate ) updateNoiseHistogram( sourceID );
    
  } // boards
  return StatusCode::SUCCESS;
}

StatusCode STNZSMonitor::finalize() {
  //printHistos();
  // Update all histograms at the end
  std::map<int, AIDA::IProfile1D*>::const_iterator itH = m_noiseHistos.begin();

  for( ; itH != m_noiseHistos.end(); ++itH ) { 
    // Limit to selected tell1s
    if ( m_selectedTells && 
         !binary_search(m_limitToTell.begin(), m_limitToTell.end(), (this->readoutTool())->SourceIDToTELLNumber((*itH).first))) {
      continue;
    }
    updateNoiseHistogram( (*itH).first, m_useODINTime );
    if(m_checkCalculation) dumpNoiseCalculation( (*itH).first );
  } 

  return ST::HistoAlgBase::finalize();// must be called after all other actions
}

void STNZSMonitor::updateNoiseHistogram(unsigned int sourceID, bool updateTitle) {
  // Get the histogram and reset it in case it is already booked. 
  if( m_noiseHistos.find(sourceID) != m_noiseHistos.end() && m_pedestalHistos.find(sourceID) != m_pedestalHistos.end()) { 

    IProfile1D* noiseHist = m_noiseHistos.find(sourceID)->second;//->second.end();
    noiseHist->reset();
  
    IProfile1D* pedestalHist = m_pedestalHistos.find(sourceID)->second;//->second.end();
    pedestalHist->reset();
  
    // Loop over strips in tell1
    unsigned int strip=0;
    std::vector<double>::const_iterator itPedBegin;
    std::vector<double>::const_iterator itPedEnd;
    
    std::vector<double>::const_iterator itNoiseBegin;
    std::vector<double>::const_iterator itNoiseEnd;
    if(m_rawNoise) {
      itPedBegin = m_noiseTool->pedestalBegin(sourceID);
      itPedEnd = m_noiseTool->pedestalEnd(sourceID);
      itNoiseBegin = m_noiseTool->rawNoiseBegin(sourceID);
      itNoiseEnd = m_noiseTool->rawNoiseEnd(sourceID);
    } else {
      itPedBegin = m_noiseTool->cmsMeanBegin(sourceID);
      itPedEnd = m_noiseTool->cmsMeanEnd(sourceID);
      itNoiseBegin = m_noiseTool->cmsNoiseBegin(sourceID);
      itNoiseEnd = m_noiseTool->cmsNoiseEnd(sourceID);
    }
    unsigned int tell1 = (this->readoutTool())->SourceIDToTELLNumber(sourceID);
    std::vector<double>::const_iterator itNoise = itNoiseBegin;
    std::vector<double>::const_iterator itPed = itPedBegin;
    for(; itNoise != itNoiseEnd; ++itNoise, ++itPed, ++strip) {
      noiseHist->fill( strip, (*itNoise) );
      pedestalHist->fill( strip, (*itPed) );
      if((*itNoise) > 1e-4) {
        m_1dp_AvNoisePerTell1->fill(tell1,(*itNoise));
        m_1dp_AvPedestalPerTell1->fill(tell1,(*itPed));
      }
    }
    if(updateTitle) {
      TProfile* profNoise = Gaudi::Utils::Aida2ROOT::aida2root ( noiseHist );
      std::string title=profNoise->GetTitle();
      title += " "+m_odinEvent;
      profNoise->SetTitle(title.c_str());
    }
    strip = 0;
    for(unsigned int link=0; link < STDAQ::noptlinks*STDAQ::nports; link++) {
      itNoise = itNoiseBegin+link*LHCbConstants::nStripsInPort;
      itNoiseEnd = itNoise + LHCbConstants::nStripsInPort;
      itPed = itPedBegin+link*LHCbConstants::nStripsInPort;
      double sumNoise = 0; double sumPed = 0; int nChan=0;
      for(; itNoise != itNoiseEnd; ++itNoise, ++itPed, ++strip) {
        if((*itNoise) > 1e-4) {
          nChan += 1;
          sumNoise += (*itNoise);
          sumPed += (*itPed);
        }
      }
      if(nChan > 0) {
        int bin = m_2d_NoisePerLinkVsTell1->FindBin(tell1, link);
        m_2d_NoisePerLinkVsTell1->SetBinContent(bin , sumNoise/nChan);
        m_2d_PedestalPerLinkVsTell1->SetBinContent( bin, sumPed/nChan);
        m_2d_NormalisationPerLinkVsTell1->SetBinContent( bin, 1.);
      }
    }
  } else {
    unsigned int tellID = m_useSourceID ? sourceID : (this->readoutTool())->SourceIDToTELLNumber(sourceID);
    Warning("No histogram booked for "+boost::lexical_cast<std::string>(tellID),0,StatusCode::SUCCESS).ignore();
  }
}

void STNZSMonitor::dumpNoiseCalculation(unsigned int sourceID) {
  const unsigned int TELL=(this->readoutTool())->SourceIDToTELLNumber(sourceID);
  unsigned int strip=0;
  std::vector<double> rawMean = m_noiseTool->rawMean(TELL);
  std::vector<double>::iterator rawMeanIt = rawMean.begin();
  std::vector<double> rawMeanSq = m_noiseTool->rawMeanSq(TELL);
  std::vector<double>::iterator rawMeanSqIt = rawMeanSq.begin();
  std::vector<double> rawNoise = m_noiseTool->rawNoise(TELL);
  std::vector<double>::iterator rawNoiseIt = rawNoise.begin();
  std::vector<unsigned int> rawN = m_noiseTool->rawN(TELL);
  std::vector<unsigned int>::iterator rawNIt = rawN.begin();

  std::vector<double> cmsMean = m_noiseTool->cmsMean(TELL);
  std::vector<double>::iterator cmsMeanIt = cmsMean.begin();
  std::vector<double> cmsMeanSq = m_noiseTool->cmsMeanSq(TELL);
  std::vector<double>::iterator cmsMeanSqIt = cmsMeanSq.begin();
  std::vector<double> cmsNoise = m_noiseTool->cmsNoise(TELL);
  std::vector<double>::iterator cmsNoiseIt = cmsNoise.begin();
  std::vector<unsigned int> cmsN = m_noiseTool->cmsN(TELL);
  std::vector<unsigned int>::iterator cmsNIt = cmsN.begin();
  
  std::string idTELL = boost::lexical_cast<std::string>(TELL);
  std::string idRawMean = "Raw mean, TELL "+idTELL;
  std::string idRawMeanSq = "Raw mean squared, TELL "+idTELL;
  std::string idRawNoiseS = "Raw noise stored, TELL "+idTELL;
  std::string idRawNoiseC = "Raw noise calculated, TELL "+idTELL;
  std::string idRawN = "Raw number events, TELL "+idTELL;

  std::string idCMSMean = "CMS mean, TELL "+idTELL;
  std::string idCMSMeanSq = "CMS mean squared, TELL "+idTELL;
  std::string idCMSNoiseS = "CMS noise stored, TELL "+idTELL;
  std::string idCMSNoiseC = "CMS noise calculated, TELL "+idTELL;
  std::string idCMSN = "CMS number events, TELL "+idTELL;
  int pp=0;
  for(; rawNIt != rawN.end(); ++rawNIt, ++cmsNIt, ++pp) {
    profile1D(pp, (*rawNIt), idRawN, idRawN, -0.5, 3.5, 4);
    profile1D(pp, (*cmsNIt), idCMSN, idCMSN, -0.5, 3.5, 4);


  }
  for(; rawMeanIt != rawMean.end(); ++rawMeanIt, ++rawMeanSqIt, ++rawNoiseIt, ++cmsMeanIt, ++cmsMeanSqIt, ++cmsNoiseIt, ++strip) {
    profile1D(strip, (*rawMeanIt), idRawMean, idRawMean, -0.5, 3071.5, 3072); 
    profile1D(strip, (*rawMeanSqIt), idRawMeanSq, idRawMeanSq, -0.5, 3071.5, 3072); 
    profile1D(strip, (*rawNoiseIt), idRawNoiseS, idRawNoiseS, -0.5, 3071.5, 3072); 
    profile1D(strip, sqrt(*rawMeanSqIt - gsl_pow_2(*rawMeanIt)), idRawNoiseC, idRawNoiseC, -0.5, 3071, 3072);

    profile1D(strip, (*cmsMeanIt), idCMSMean, idCMSMean, -0.5, 3071.5, 3072); 
    profile1D(strip, (*cmsMeanSqIt), idCMSMeanSq, idCMSMeanSq, -0.5, 3071.5, 3072); 
    profile1D(strip, (*cmsNoiseIt), idCMSNoiseS, idCMSNoiseS, -0.5, 3071.5, 3072); 
    profile1D(strip, sqrt(*cmsMeanSqIt - gsl_pow_2(*cmsMeanIt)), idCMSNoiseC, idCMSNoiseC, -0.5, 3071.5, 3072);

  }
}
