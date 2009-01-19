// $Id: DecodeVeloRawBuffer.cpp,v 1.17 2009-01-19 11:24:39 dhcroft Exp $

#include "GaudiKernel/AlgFactory.h"

#include "Event/RawEvent.h"
#include "Event/RawBank.h"
#include "Event/VeloLiteCluster.h"
#include "Event/VeloCluster.h"

#include "VeloDet/DeVelo.h"
#include "VeloDet/DeVeloSensor.h"

#include "DecodeRawBankToLiteClusters.h"
#include "DecodeRawBankToClusters.h"
#include "VeloClusterPtrLessThan.h"

#include "DecodeVeloRawBuffer.h"
#include "VeloRawBankDecoder.h"
#include "VeloRawBankVersions.h"


//-----------------------------------------------------------------------------
// Implementation file for class : DecodeVeloRawBuffer
//
// 2004-02-02 : Chris Parkes
// 2006-02-23 : David Jones, Kurt Rinnert
//              - complete rewrite
//-----------------------------------------------------------------------------

DECLARE_ALGORITHM_FACTORY( DecodeVeloRawBuffer );

//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
DecodeVeloRawBuffer::DecodeVeloRawBuffer( const std::string& name,
                              ISvcLocator* pSvcLocator)
  : GaudiAlgorithm ( name , pSvcLocator ) 
  , m_forcedBankVersion(0) // there is no version 0, so this means bank version is not enforced
{
  declareProperty("DecodeToVeloLiteClusters",m_decodeToVeloLiteClusters=true);
  declareProperty("DecodeToVeloClusters",m_decodeToVeloClusters=false);
  declareProperty("DumpVeloClusters",m_dumpVeloClusters=false);
  declareProperty("RawEventLocation",m_rawEventLocation=LHCb::RawEventLocation::Default);
  declareProperty("VeloLiteClustersLocation",m_veloLiteClusterLocation=LHCb::VeloLiteClusterLocation::Default);
  declareProperty("VeloClusterLocation",m_veloClusterLocation=LHCb::VeloClusterLocation::Default);
  declareProperty("AssumeChipChannelsInRawBuffer",m_assumeChipChannelsInRawBuffer=false);
  declareProperty("ForceBankVersion",m_forcedBankVersion=0);
}


//=============================================================================
// Destructor
//=============================================================================
DecodeVeloRawBuffer::~DecodeVeloRawBuffer() {}; 

//=============================================================================
// Initialisation. Check parameters
//=============================================================================
StatusCode DecodeVeloRawBuffer::initialize() {

  StatusCode sc = GaudiAlgorithm::initialize(); // must be executed first
  if ( sc.isFailure() ) return sc;  // error printed already by GaudiAlgorithm

  debug () << "==> Initialise" << endreq;

  // check whether enforced bank version is supported
  if(m_forcedBankVersion) {
    switch (m_forcedBankVersion) {
      case VeloDAQ::v2: // ok, supported
      case VeloDAQ::v3:
        break;
      default: // not supported, bail out
        error() << "User enforced VELO raw buffer version " 
          << m_forcedBankVersion 
          << " is not supported."
          << endmsg;
        return StatusCode::FAILURE;
    }
  }
  
  
  m_velo = getDet<DeVelo>( DeVeloLocation::Default );

  return StatusCode::SUCCESS;
};

//=============================================================================
// Main execution
//=============================================================================
StatusCode DecodeVeloRawBuffer::execute() {

  // fetch raw bank in any case
  if (!exist<LHCb::RawEvent>(m_rawEventLocation) ) {
    debug() << "Raw Event not found in " << m_rawEventLocation
	    << endmsg;
    createEmptyBanks();
    return StatusCode::SUCCESS;
  }
  LHCb::RawEvent* rawEvent = get<LHCb::RawEvent>(m_rawEventLocation);

  const std::vector<LHCb::RawBank*>& banks = rawEvent->banks(LHCb::RawBank::Velo);

  StatusCode sc;

  // decode to lite clusters, if requested, bail out if something goes wrong
  if (m_decodeToVeloLiteClusters) {
    sc = decodeToVeloLiteClusters(banks);
    if (sc.isFailure()) return sc;
  }

  // decode to clusters, if requested, bail out if something goes wrong
  if (m_decodeToVeloClusters) {
    sc = decodeToVeloClusters(banks);
    if (sc.isFailure()) return sc;
  }

  return StatusCode::SUCCESS;
};

//=============================================================================
// Whatever needs to be done at the end
//=============================================================================
StatusCode DecodeVeloRawBuffer::finalize() {

  debug () << "==> Finalise" << endreq;

  return StatusCode::SUCCESS;
};

//=============================================================================
// Private helpers
//=============================================================================

StatusCode DecodeVeloRawBuffer::decodeToVeloLiteClusters(const std::vector<LHCb::RawBank*>& banks) const
{
  LHCb::VeloLiteCluster::FastContainer* fastCont = new LHCb::VeloLiteCluster::FastContainer();

  // make sure we have enough capacity in the container
  // to avoid unnecessary relocations
  // yes this is a guessed number based on <4% of signal events 
  // need more than clusters (one length doubling operation required)
  fastCont->reserve(4096); 

  for (std::vector<LHCb::RawBank*>::const_iterator bi = banks.begin(); 
       bi != banks.end();
       ++bi) {

    const LHCb::RawBank* rb = *bi;

    const DeVeloSensor* sensor = m_velo->sensorByTell1Id(static_cast<unsigned int>(rb->sourceID()));
    if (!sensor) {
      error() << "Could not map source ID "          
	      << rb->sourceID()
	      << " to sensor number!"
	      << endmsg;
      continue;
    }

    const SiDAQ::buffer_word* rawBank = static_cast<const SiDAQ::buffer_word*>(rb->data());
   
    VeloDAQ::decodeRawBankToLiteClusters(rawBank,sensor,m_assumeChipChannelsInRawBuffer,fastCont);

  }
  std::sort(fastCont->begin(),fastCont->end(),SiDataFunctor::Less_by_Channel< LHCb::VeloLiteCluster >());
  put(fastCont,m_veloLiteClusterLocation);
 
  return StatusCode::SUCCESS;
}

StatusCode DecodeVeloRawBuffer::decodeToVeloClusters(const std::vector<LHCb::RawBank*>& banks) const 
{
  LHCb::VeloClusters* clusters = new LHCb::VeloClusters();

  // Number of bytes in bank, including 4 byte header but
  // *without* the padding bytes at the end. 
  // This is filled by the decoding function and should always 
  // be the same as RawBank::size().
  int byteCount;

  for (std::vector<LHCb::RawBank*>::const_iterator bi = banks.begin(); 
       bi != banks.end();
       ++bi) {
    
    const LHCb::RawBank* rb = *bi;
    const SiDAQ::buffer_word* rawBank = static_cast<const SiDAQ::buffer_word*>(rb->data());

    const DeVeloSensor* sensor = m_velo->sensorByTell1Id(static_cast<unsigned int>(rb->sourceID()));
    if (!sensor) {
      error() << "Could not map source ID "          
	      << rb->sourceID()
	      << " to sensor number!"
	      << endmsg;
      continue;
    }

    //info() << "Decoding Clusters on sensor " << sensor << endmsg;

    unsigned int oldSize = clusters->size();
    unsigned int bankVersion = m_forcedBankVersion ? m_forcedBankVersion : rb->version();

    std::string errorMsg;
    switch (bankVersion) {
      case VeloDAQ::v2:
        VeloDAQ::decodeRawBankToClustersV2(rawBank,sensor,m_assumeChipChannelsInRawBuffer,clusters,byteCount);
        break;
      case VeloDAQ::v3:
        VeloDAQ::decodeRawBankToClustersV3(rawBank,sensor,m_assumeChipChannelsInRawBuffer,clusters,byteCount,errorMsg);
        if ( !errorMsg.empty() ) {
          error() << errorMsg << endmsg;
        }
        break;
      default: // bank version is not supported
        error() << "VELO raw buffer version "
          << bankVersion
          << " is not supported."
          << endmsg;
    }

    if (rb->size() != byteCount) {
      error() << "Byte count mismatch between RawBank size and decoded bytes." 
	      << " RawBank: " << rb->size() 
	      << " Decoded: " << byteCount 
        << " Will remove clusters for source ID " << rb->sourceID()
	      << endmsg;
      LHCb::VeloClusters::iterator start = clusters->begin();
      std::advance(start,oldSize);
      clusters->erase(start,clusters->end());
    } 

  }

  put(clusters,m_veloClusterLocation);
  if (m_dumpVeloClusters) dumpVeloClusters(clusters);

  return StatusCode::SUCCESS;
}

void DecodeVeloRawBuffer::dumpVeloClusters(const LHCb::VeloClusters* clusters) const
{
  std::vector<float> adcValues;
  
  for (LHCb::VeloClusters::const_iterator ci =  clusters->begin(); 
       ci != clusters->end(); 
       ++ci) {

    const LHCb::VeloCluster* clu = *ci;
    
    unsigned int sensorNumber = clu->channelID().sensor();
    unsigned int centroidStrip = clu->channelID().strip();

    // interstrip position in 1/8 of strip pitch (as it is encoded in raw bank)
//     float interStripPos = static_cast<unsigned int>((clu->interStripFraction())*8.0)/8.0;
    double interStripPos = clu->interStripFraction();

    info() << "DEC::POSDUMP:"
	   << " SN=" << sensorNumber
	   << " CS=" << centroidStrip
	   << " ISP=" << interStripPos
	   << endmsg;

    info() << "DEC::ADCDUMP:"
	   << " SN=" << sensorNumber;

    for (unsigned int adci=0; adci<clu->size(); ++adci) {
      info() << " " << clu->adcValue(adci)
	     << "@" << clu->strip(adci);
    }
    info() << endmsg;
      
  }
  return;
}

void DecodeVeloRawBuffer::createEmptyBanks() {
  if ( m_decodeToVeloLiteClusters ) {
    LHCb::VeloLiteCluster::FastContainer* fastCont = new LHCb::VeloLiteCluster::FastContainer();
    put(fastCont,m_veloLiteClusterLocation);
  }
  if ( m_decodeToVeloClusters ) {
    LHCb::VeloClusters* clusters = new LHCb::VeloClusters();
    put(clusters,m_veloClusterLocation);
  }
}
