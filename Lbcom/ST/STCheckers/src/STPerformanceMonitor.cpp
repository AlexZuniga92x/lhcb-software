// $Id: STPerformanceMonitor.cpp,v 1.5 2008-12-01 16:28:06 mneedham Exp $

// Gaudi
#include "GaudiKernel/AlgFactory.h"

// LHCbKernel
#include "Kernel/STTell1ID.h"
#include "Kernel/ISTReadoutTool.h"
#include "Kernel/SiChargeFun.h"

// STDet
#include "STDet/DeSTDetector.h"
#include "STDet/DeSTSector.h"

// Event
#include "Event/STSummary.h"

// local
#include "STPerformanceMonitor.h"

using namespace LHCb;

DECLARE_ALGORITHM_FACTORY( STPerformanceMonitor );

//--------------------------------------------------------------------
//
//  STPerformanceMonitor : Monitor the STPerformances
//
//--------------------------------------------------------------------

STPerformanceMonitor::STPerformanceMonitor( const std::string& name, 
                                ISvcLocator* pSvcLocator ) :
  ST::HistoAlgBase(name, pSvcLocator),
  m_event(0) 
{
  // constructer

  declareSTConfigProperty("summaryLocation",m_summaryLocation , STSummaryLocation::TTSummary );
  declareSTConfigProperty("InputData", m_clusterLocation , STClusterLocation::TTClusters);
  declareProperty("ExpectedEvents", m_expectedEvents = 100000);

  setForcedInit();
}

STPerformanceMonitor::~STPerformanceMonitor()
{
  // destructer
}

StatusCode STPerformanceMonitor::initialize()
{
  // Set the top directory to IT or TT.
  if( "" == histoTopDir() ) setHistoTopDir(detType()+"/");

  // Initialize GaudiHistoAlg
  StatusCode sc = ST::HistoAlgBase::initialize();
  if (sc.isFailure()) return Error("Failed to initialize", sc);
    
  return StatusCode::SUCCESS;
}

StatusCode STPerformanceMonitor::execute()
{

  ++m_event;

  // retrieve the event summary
  const STSummary* summary = get<STSummary>(m_summaryLocation);

  // get the clusters
  const STClusters* clusterCont = get<STClusters>(m_clusterLocation);

  // get list of missing sectors
  std::vector<DeSTSector*> deadSectors;

  // some are lost because of errors in decoding
  getDeadSectors(deadSectors,summary->banksWithError());
  
  // some were lost [ie tell1 not readout] 
  getDeadSectors(deadSectors,summary->missingBanks());

  // then all we have to do is loop over the sectors + count
  double frac = 0.0;
  const DeSTDetector::Sectors& sectors = tracker()->sectors();
  for (DeSTDetector::Sectors::const_iterator iterS = sectors.begin();
       iterS != sectors.end(); ++iterS){
    std::vector<DeSTSector*>::const_iterator testIter = std::find(deadSectors.begin(),
                                                                  deadSectors.end(),*iterS);
    testIter != deadSectors.end() ? frac += 0 : frac += (*iterS)->fractionActive() ;
  } // iterS
  frac /= sectors.size(); 
  plot(frac, 1,"active fraction", 0., 1., 200); //plot active fraction

  plot2D(m_event, frac, 11,"active fraction versus time", 0., m_expectedEvents, 0., 1., 50, 200);

  // get the occupancy
  const double occ = clusterCont->size()/(tracker()->nStrip()*frac); 
  plot(occ, 2,"occupancy", 0., 1., 200);
  plot2D(m_event, occ, 12,"occ versus time", 0., m_expectedEvents, 0., 0.1, 50, 200);


  // get the modal charge
  //double shorth = SiChargeFun::shorth(clusterCont->begin(),clusterCont->end());
  //plot(shorth, "shorth", 0., 100., 200);

  // get the modal charge
  double tm = SiChargeFun::truncatedMean(clusterCont->begin(),clusterCont->end());
  plot(tm, "tm", 0., 100., 200);
  plot2D(m_event,tm,13,"tms versus time", 0., m_expectedEvents, 0., 100, 200, 200);

  /*
  double lms = SiChargeFun::LMS(clusterCont->begin(), clusterCont->end());
  plot(lms,"lms", 0., 100., 200 );
  */

  double gm = SiChargeFun::generalizedMean(clusterCont->begin(), clusterCont->end());
  plot(gm,"gm", 0., 100., 200);

 
  return StatusCode::SUCCESS;
}

void STPerformanceMonitor::getDeadSectors(std::vector<DeSTSector*>& deadSectors, 
                                          const std::vector<unsigned int>& banks) const {

  for (std::vector<unsigned int>::const_iterator iter= banks.begin(); 
       iter != banks.end(); ++iter){
    // sectors
    std::vector<DeSTSector*> sectors = readoutTool()->sectors(STTell1ID(*iter));  
    deadSectors.insert(deadSectors.begin(),sectors.begin(),sectors.end());
  } // iter
}



