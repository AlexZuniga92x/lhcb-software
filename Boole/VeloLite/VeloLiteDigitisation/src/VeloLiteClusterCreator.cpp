// Gaudi
#include "GaudiKernel/AlgFactory.h"
// Kernel/LHCbKernel
#include "Kernel/VeloLiteChannelID.h"
#include "Kernel/VeloLiteDataFunctor.h"
// Local
#include "IStripNoiseTool.h"
#include "VeloLiteClusterCreator.h"

using namespace LHCb;

/** @file VeloLiteClusterCreator.cpp
 *
 *  Implementation of class : VeloLiteClusterCreator
 *
 */

DECLARE_ALGORITHM_FACTORY(VeloLiteClusterCreator);

//=============================================================================
/// Constructor
//=============================================================================
VeloLiteClusterCreator::VeloLiteClusterCreator(const std::string& name,
                                               ISvcLocator* pSvcLocator) : 
  GaudiAlgorithm(name, pSvcLocator),
  m_digits(0),
  m_clusters(0),
  m_det(0) {

  declareProperty("MaxClusters", m_maxClusters = 10000);
  declareProperty("InclusionThreshold", m_inclusionThreshold = 0.1);
  declareProperty("SignalToNoiseCut", m_signalToNoiseCut = 3.0);
  declareProperty("ClusterSignalToNoiseCut", m_clusterSignalToNoiseCut = 4.5);
  declareProperty("ClusterAbsoluteADCCut", m_clusterAbsoluteADCCut = 20.);
  declareProperty("HighThreshold", m_highThreshold = 30.0);
  declareProperty("ElectronsPerADC", m_electronsPerADC = 600.);

}

//=============================================================================
/// Initialisation
//=============================================================================
StatusCode VeloLiteClusterCreator::initialize() {

  StatusCode sc = GaudiAlgorithm::initialize();
  if (sc.isFailure()) return sc;
  debug() << " ==> initialize()" << endmsg;
  m_det = getDet<DeVeloLite>(DeVeloLiteLocation::Default);
  m_StripNoiseTool = tool<IStripNoiseTool>("VeloLiteStripNoiseTool",
                                           "StripNoise", this);
  m_debug   = msgLevel(MSG::DEBUG);
  m_verbose = msgLevel(MSG::VERBOSE);
  return StatusCode::SUCCESS;

}

//=========================================================================
///  Main execution
//=========================================================================
StatusCode VeloLiteClusterCreator::execute() {

  if (m_debug) debug() << " ==> execute()" << endmsg;
  m_clusters = new VeloStripClusters;
  // Retrieve digits. 
  m_digits = get<MCVeloLiteDigits>(MCVeloLiteDigitLocation::Default);
  if (m_debug) {
    debug() << m_digits->size() << " digits in " 
            << MCVeloLiteDigitLocation::Default << endmsg;
  }
  // Do the clustering.
  makeClusters();
  return storeClusters();

}

//=============================================================================
/// Finalisation 
//=============================================================================
StatusCode VeloLiteClusterCreator::finalize() {

  return GaudiAlgorithm::finalize();

}


//==========================================================================
/// Clustering procedure
//==========================================================================
void VeloLiteClusterCreator::makeClusters() {
  
  if (m_debug) debug() << " ==> makeClusters()" << endmsg;
  std::vector<DeVeloLiteSensor*>::const_iterator its; 
  for (its = m_det->sensorsBegin(); its != m_det->sensorsEnd(); ++its) {
    const unsigned int sensor = (*its)->sensorNumber();
    // Tag all channels as unused.
    m_used.clear();
    m_used.resize((*its)->numberOfStrips(), false); 
    // Retrieve the range of channels for this sensor.
    std::pair<MCVeloLiteDigits::iterator, MCVeloLiteDigits::iterator> range;
    getRangeOfSensor(sensor, range);
    if (m_debug) {
      debug() << "Retrieved " << range.second - range.first 
              << " digits for sensor " << sensor << endmsg;
    }
    // Sort by increasing ADC value.         
    std::stable_sort(range.first, range.second,
                     VeloLiteDataFunctor::Less_by_adc<const MCVeloLiteDigit*>());
    // Swap to decreasing ADC.
    std::reverse(range.first, range.second);
    // Loop over digits.
    MCVeloLiteDigits::iterator itd;
    for (itd = range.first; itd != range.second; ++itd) {
      MCVeloLiteDigit* digit = *itd;    
      if (m_verbose) {
        verbose() << "Digit with ADC " << digit->adc() 
                  << " (sensor " << digit->sensor() << ")" << endmsg;
      }
      // Try to make a cluster from this digit.
      double stn = 0.;
      VeloLiteInternalCluster cluster;
      cluster.setSensor(sensor);
      if (!makeCluster(digit, cluster, stn)) continue;
      if (m_verbose) {
        verbose() << "Made cluster with S/N = " << stn << endmsg;
      }
      double adc = 0.;
      for (unsigned int i = 0; i < cluster.size(); ++i) {
        adc += cluster.adcValue(i);
      }
      // Apply cut on signal to noise ratio and ADC value.
      if (stn < m_clusterSignalToNoiseCut || adc < m_clusterAbsoluteADCCut) {
        unmarkCluster(cluster);
        continue;
      }
      // Cluster passed cut. 
      if (m_verbose) {
        verbose() << "Adding cluster with S/N = " << stn 
                  << ", size " << cluster.size()  
                  << ", first strip " << cluster.strip(0) 
                  << ", first ADC " << cluster.adcValue(0) << endmsg;
      }
      // Calculate the interstrip fraction. 
      double sum = 0.;
      double mean = 0.;
      for (unsigned int i = 0; i < cluster.size(); ++i) {
        sum += cluster.adcValue(i);
        mean += cluster.strip(i) * cluster.adcValue(i);
      } 
      mean /= sum;
      double fraction = mean - cluster.strip(0); 
      // Set the key.
      VeloLiteChannelID channelID(cluster.sensor(), cluster.strip(0));
      // Make a lite cluster.
      const VeloStripLiteCluster liteCluster(channelID, 
                                             fraction,
                                             cluster.size(),
                                             cluster.hasHighThreshold());
      VeloStripCluster* newCluster = new VeloStripCluster(liteCluster, 
                                                          cluster.stripSignals());
      m_clusters->insert(newCluster, channelID);
      if (m_clusters->size() >= m_maxClusters) {
        // Too many clusters. Cannot add any more after this one.
        Warning("Too many clusters in this event. Processing incomplete.").ignore();
        info() << "Processing limit is " << m_maxClusters << " clusters." << endmsg;
        info() << "Stopping clusterization while processing sensor " 
               << sensor << endmsg;
        return;
      }
    } 
  } 

}  

//==========================================================================
/// Create cluster from digit
//==========================================================================
bool VeloLiteClusterCreator::makeCluster(MCVeloLiteDigit* digit, 
                                         VeloLiteInternalCluster& cluster, 
                                         double& stn) {
  
  if (m_debug) debug() << " ==> makeCluster()" << endmsg;
  // Add the cluster centre
  if (!addCentralChannel(cluster, stn, digit)) {
    // Failed to make a cluster centre
    if (m_debug) debug() << "Cluster rejected" << endmsg;
    return false;
  }
  if (m_debug) debug() << "Cluster accepted" << endmsg;

  // Try to add neighbouring channel.
  if (addAdjacentChannel(cluster, stn, digit, -1)) {
    if (m_debug) debug() << "Added -1 channel" << endmsg;
    // Try to add channel two strips away
    if (addAdjacentChannel(cluster, stn, digit, -2)) {
      if (m_debug) debug() << "Added -2 channel" << endmsg;
    }
  }
  if (addAdjacentChannel(cluster, stn, digit, +1)) {
    if (m_debug) debug() << "Added +1 channel" << endmsg;
    // Try to add channel two strips away
    if (addAdjacentChannel(cluster, stn, digit, +2)) { 
      if (m_debug) debug() << "Added +2 channel" << endmsg; 
    }
  }
  // Check if cluster total ADC is above the non-spillover threshold.
  double totalADC = 0;
  for (unsigned int i = 0; i < cluster.size(); ++i) {
    totalADC += cluster.adcValue(i);
  }
  if (m_highThreshold <= totalADC) {
    cluster.setHasHighThreshold(true);
  } else {
    cluster.setHasHighThreshold(false); 
  }
  if (m_verbose) {
    verbose() << "Cluster on sensor " 
              << cluster.sensor() 
              << ", first strip " << cluster.strip(0) 
              << ", ADC " <<  cluster.adcValue(0) 
              << " has high threshold " 
              << (cluster.hasHighThreshold() ? 'T' : 'F')
              << endmsg;
  }
  return true;

}  

//==========================================================================
/// Try to make a seed cluster from a given channel 
//==========================================================================
bool VeloLiteClusterCreator::addCentralChannel(VeloLiteInternalCluster& cluster,
                                               double& stn,
                                               MCVeloLiteDigit* digit) {
  
  if (m_debug) debug() << " ==> addCentralChannel()" << endmsg;
  // Check if the channel is already used
  if (m_used[digit->strip()]) return false;  
  // Check if single channel S/N is ok
  if (signalToNoise(digit) < m_signalToNoiseCut) return false;
  // This seems to be a suitable start for a cluster
  addChannel(cluster, stn, digit, 0);
  // Tag strip as used
  m_used[digit->strip()] = true;
  return true;

}

//==========================================================================
/// Try to add a neighbouring channel with given offset to a cluster
//==========================================================================
bool VeloLiteClusterCreator::addAdjacentChannel(VeloLiteInternalCluster& cluster,
                                                double& stn, 
                                                MCVeloLiteDigit* digit, 
                                                int offset) {
  
  if (m_debug) debug() << " ==> addAdjacentChannel()" << endmsg;
  const DeVeloLiteSensor* sens = m_det->sensor(digit->key().sensor());
  // Get the ID of the neighbour with the given offset (if available). 
  VeloLiteChannelID nearbyStripId;
  StatusCode sc = sens->neighbour(digit->key(), offset, nearbyStripId);
  if (!sc) return false; 
  // Check if the neighbouring channel is already used.
  if (m_used[nearbyStripId.strip()]) return false;  
  // Check if the nearby strip has a hit.
  MCVeloLiteDigit* nearbyDigit = m_digits->object(nearbyStripId);
  if (!nearbyDigit) return false;

  // if (A or B) {
  //   [mark channel used]
  //   if (A) {
  //     [add hit]
  //   }
  // }
  // 
  // A: hitPlusOne->signalToNoise() > centralHit->signalToNoise() * InclusionThreshold
  // B: hitPlusOne->signalToNoise() > SignalToNoiseCut
  //
  // B is nearly redundant!
  //
  // For the outer two strips, we have an extra check:
  //
  // if (C and (A or B)) {
  //   [mark channel used]
  //   if (A) {
  //     [add hit]
  //   }
  // }
  //
  // A: hitMinusTwo->signalToNoise() > centralHit->signalToNoise() * InclusionThreshold
  // B: hitMinusTwo->signalToNoise() > SignalToNoiseCut(detindex)
  // C: hitMinusTwo->signalToNoise() < hitMinusOne->signalToNoise()
  //

  // Condition C
  if (abs(offset) > 1) {
    // Strip is outside the central three - make an extra cut
    // Get the strip one strip closer to the centre than (offset)
    int innerOffset = (offset < 0) ? offset + 1 : offset - 1;
    // Get the ChannelID for that strip
    VeloLiteChannelID innerStripId;
    StatusCode sc = sens->neighbour(digit->key(), innerOffset, innerStripId);
    // Check if the channel is valid.
    if (sc.isFailure()) return false; 
    // Get the Digit corresponding to that strip
    MCVeloLiteDigit* innerDigit = m_digits->object(innerStripId); 
    if (!innerDigit) {
      Warning("Asking for a +2 Digit where +1 Digit does not exist!").ignore();
      return false;
    }
    // Check if the strips are monotonically decreasing in S/N 
    if (signalToNoise(innerDigit) < signalToNoise(nearbyDigit)) return false;
  }
  
  // Condition A
  bool passInclusionThreshold = (signalToNoise(nearbyDigit) > signalToNoise(digit) * m_inclusionThreshold);
  // Condition B
  bool passSNCut = (signalToNoise(nearbyDigit) > m_signalToNoiseCut); 
  if (!passInclusionThreshold && !passSNCut) { 
    // Fails (A or B)
    return false;
  }
  // Tag channel as used
  m_used[nearbyStripId.strip()] = true;
  if (!passInclusionThreshold) { 
    // Fails (A)
    return false;
  }
  // Add the nearby channel to the cluster
  addChannel(cluster, stn, nearbyDigit, offset); 
  if (m_verbose) {
    verbose() << "Added strip " << nearbyStripId.strip() 
              << " with ADC " << nearbyDigit->adc()
              << " and S/N " << stn << endmsg;
  }
  return true;

}

//==========================================================================
/// Add a strip to an existing cluster
//==========================================================================
void VeloLiteClusterCreator::addChannel(VeloLiteInternalCluster& cluster,
                                      double& stn,
                                      MCVeloLiteDigit* digit,
                                      int offset) {

  if (m_debug) debug() << " ==> addChannel()" << endmsg;
  std::vector<std::pair<int, unsigned int> > channels = cluster.stripSignals();
  std::pair<int, unsigned int> newChannel(digit->strip(), digit->adc()); 
  if (offset > 0) { 
    // Add ADC value to end of cluster.
    channels.push_back(newChannel);
  } else {
    // New ADC value becomes first in cluster.
    channels.insert(channels.begin(), newChannel); 
    // Update S/N.
    stn += signalToNoise(digit);
  }
  cluster.setStripSignals(channels);

}  

//==========================================================================
/// Re-tag channels as unused
//==========================================================================
void VeloLiteClusterCreator::unmarkCluster(VeloLiteInternalCluster& cluster) {
  
  if (m_debug) debug() << " ==> unmarkCluster()" << endmsg;
  for (unsigned int i = 0; i < cluster.size(); ++i) {
    m_used[cluster.strip(i)] = false;
  }

}

//=========================================================================
/// Add clusters to TDS
//=========================================================================
StatusCode VeloLiteClusterCreator::storeClusters() {
  
  if (m_debug) debug() << " ==> storeClusters()" << endmsg;
  // Sort clusters in order of ascending sensor + strip
  std::stable_sort(m_clusters->begin(), m_clusters->end(),
                   VeloLiteDataFunctor::Less_by_key<const VeloStripCluster*>());
  put(m_clusters, VeloStripClusterLocation::Default);  
  if (m_debug) {
    debug() << "Stored " << m_clusters->size() << " clusters at " 
            << VeloStripClusterLocation::Default << endmsg;
  }
  return StatusCode::SUCCESS;

}  

//=========================================================================
/// Get range of digits for the specified sensor
//=========================================================================
void VeloLiteClusterCreator::getRangeOfSensor(const unsigned int sensorId,
    std::pair<MCVeloLiteDigits::iterator, MCVeloLiteDigits::iterator>& range) {

  if (m_debug) debug() << " ==> getRangeOfSensor()" << endmsg;
  MCVeloLiteDigit sel(VeloLiteChannelID(sensorId, 0, VeloLiteChannelID::Null));
  // Make sure the digits are sorted by sensor number.
  std::stable_sort(m_digits->begin(), m_digits->end(),
      VeloLiteDataFunctor::Less_by_sensor<const MCVeloLiteDigit*>());
  // Get range with required sensor number.
  range = std::equal_range(m_digits->begin(), m_digits->end(), &sel,
      VeloLiteDataFunctor::Less_by_sensor<const MCVeloLiteDigit*>());

}

//=========================================================================
/// Calculate signal to noise ratio
//=========================================================================
double VeloLiteClusterCreator::signalToNoise(MCVeloLiteDigit* digit) {

  double noise = m_StripNoiseTool->noise(digit->sensor(), digit->strip());
  noise /= m_electronsPerADC;
  if (fabs(noise) < 0.00001) noise = 0.00001;
  return static_cast<double>(digit->adc()) / noise;

}
