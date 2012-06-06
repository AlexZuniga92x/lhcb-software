// Gaudi
#include "GaudiKernel/AlgFactory.h"
// Local
#include "VeloLiteClusterMonitor.h"

/** @file VeloLiteClusterMonitor.cpp
 *
 *  Implementation of class : VeloLiteClusterMonitor
 *
 */

using namespace LHCb;

DECLARE_ALGORITHM_FACTORY(VeloLiteClusterMonitor);

//=============================================================================
/// Standard constructor
//=============================================================================
VeloLiteClusterMonitor::VeloLiteClusterMonitor(const std::string& name,
                                               ISvcLocator* pSvcLocator): 
    GaudiTupleAlg(name, pSvcLocator),
    m_clusters(0),
    m_nClusters(0.),
    m_nClusters2(0.),
    m_nOneStrip(0.),
    m_nTwoStrip(0.),
    m_nThreeStrip(0.),
    m_nFourStrip(0.),
    m_nEvents(0) {

  declareProperty("PrintInfo", m_printInfo = false);

}

//=============================================================================
/// Destructor
//=============================================================================
VeloLiteClusterMonitor::~VeloLiteClusterMonitor() {

}

//=============================================================================
/// Initialization
//=============================================================================
StatusCode VeloLiteClusterMonitor::initialize() {

  StatusCode sc = GaudiAlgorithm::initialize();
  if (sc.isFailure()) return sc; 
  debug() << " ==> initialize()" << endmsg;
  setHistoTopDir("VeloLite/");
  return StatusCode::SUCCESS;

};

//=============================================================================
/// Main execution
//=============================================================================
StatusCode VeloLiteClusterMonitor::execute() {

  debug() << " ==> execute()" << endmsg;
  ++m_nEvents;
  if (!exist<VeloStripClusters>(VeloStripClusterLocation::Default)) {
    error() << " ==> There are no VeloStripClusters in the TES! " << endmsg;
    return StatusCode::FAILURE;
  } else {
    m_clusters = get<VeloStripClusters>(VeloStripClusterLocation::Default);
  }
  monitor();
  return StatusCode::SUCCESS;

};

//=============================================================================
///  Finalize
//=============================================================================
StatusCode VeloLiteClusterMonitor::finalize() {

  debug() << " ==> finalize()" << endmsg;
  double err = 0.;
  if (0 != m_nEvents) {
    m_nClusters /= m_nEvents;
    m_nClusters2 /= m_nEvents;
    err = sqrt((m_nClusters2 - (m_nClusters * m_nClusters)) / m_nEvents);
    m_nOneStrip /= m_nEvents;
    m_nTwoStrip /= m_nEvents;
    m_nThreeStrip /= m_nEvents;
    m_nFourStrip /= m_nEvents;
  }
  info() << "------------------------------------------------------" << endmsg;
  info() << "            - VeloLiteClusterMonitor -                " << endmsg;
  info() << "------------------------------------------------------" << endmsg;
  if (m_nClusters > 0) {
    info() << " Number of clusters / event: " << m_nClusters << " +/- " 
           << err << endmsg;
    info().precision(4);
    info() << "   1 strip clusters / event:     " << m_nOneStrip 
           << " (" << (m_nOneStrip / m_nClusters) * 100 << "%)"
           << endmsg;
    info().precision(4);
    info() << "   2 strip clusters / event:     " << m_nTwoStrip 
           << " (" << (m_nTwoStrip / m_nClusters) * 100 << "%)"
           << endmsg;
    info().precision(3);
    info() << "   3 strip clusters / event:     " << m_nThreeStrip 
           << " (" << (m_nThreeStrip / m_nClusters) * 100 << "%)"
           << endmsg;
    info().precision(3);
    info() << "   4 strip clusters / event:     " << m_nFourStrip 
           << " (" << (m_nFourStrip / m_nClusters) * 100 << "%)"
           << endmsg;
  } else {
    info() << " ==> No VeloStripClusters found! " << endmsg;
  }
  info() << "------------------------------------------------------" << endmsg;
  return GaudiAlgorithm::finalize();

}

StatusCode VeloLiteClusterMonitor::monitor() {

  debug()<< " ==> monitor()" << endmsg;
  const int size = m_clusters->size();
  m_nClusters += size;
  m_nClusters2 += size * size;
  plot(size, "nClusters", "Number of clusters / event", 0., 3000., 50);

  VeloStripClusters::iterator it;
  for (it = m_clusters->begin(); it != m_clusters->end(); ++it) {
    int nstrips = (*it)->size();
    plot(nstrips, "clusterSize",
         "Number of strips in cluster",
         -0.5, 5.5, 6);
    switch (nstrips) {
      case 1: m_nOneStrip += 1.; break;
      case 2: m_nTwoStrip += 1.; break;
      case 3: m_nThreeStrip += 1.; break;
      case 4: m_nFourStrip += 1.; break;
      default : break;
    }
    VeloLiteChannelID channel = (*it)->channelID();
    unsigned int sensor = channel.sensor();
    // Printout some info about the cluster
    if (m_printInfo) {
      info() << " ==> VeloStripCluster: sensor "
             << channel.sensor() << ", first strip: "
             << (*it)->strip(0) << endmsg;
      for (int i = 0; i < nstrips; ++i) {
        info() << " ==> VeloStripCluster: strip: "
                << (*it)->strip(i) << ", signal on strip: "
                << (*it)->adcValue(i) <<endmsg;
      }
    }
    double adcSum = 0.;
    for (int i = 0; i < nstrips; ++i) {
      adcSum += double((*it)->adcValue(i));
    }
    plot(adcSum, "adcSum",
         "ADC sum",
         -0.5, 255.5, 256);
    plot((*it)->interStripFraction(), "isp",
         "Inter-strip fraction",
         0., 1., 50);
    plot2D(sensor, (*it)->strip(0), "sensorAndStrip",
           "Sensor and first strip number",
           0., 132., 0., 3000., 132, 50);
  }
  return StatusCode::SUCCESS;

}

