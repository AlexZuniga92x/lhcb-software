// $Id: VeloPixCluster2MCParticleLinker.cpp,v 1.1.1.1 2009-12-04 14:34:46 marcin Exp $
// from Gaudi
#include "GaudiKernel/AlgFactory.h"
// Linker
#include "Linker/LinkerWithKey.h"
#include "Linker/LinkerTool.h"
// Event
#include "Event/VeloPixCluster.h"
#include "Event/MCHit.h"
#include "Event/MCParticle.h"
// Local
#include "VeloPixCluster2MCParticleLinker.h"

using namespace LHCb;

//--------------------------------------------------------------
// Implementation file for class : VeloPixCluster2MCParticleLinker
//
// 01/12/2009 : Marcin Kucharczyk
// Based on ST code
//--------------------------------------------------------------

// Declaration of the Algorithm Factory
DECLARE_ALGORITHM_FACTORY( VeloPixCluster2MCParticleLinker );

VeloPixCluster2MCParticleLinker::VeloPixCluster2MCParticleLinker(
                                 const std::string& name,
                                 ISvcLocator* pSvcLocator)
  : GaudiAlgorithm(name, pSvcLocator),
    m_asctLocation("VeloPix/Clusters2MCHits")
{
  declareProperty("InputData", m_inputData = 
                  "VeloPix/Clusters");
  declareProperty("OutputData", m_outputData = 
                  "VeloPix/Clusters2MCParticles");
  declareProperty("AddSpillOverHits", m_addSpillOverHits = false); 
  declareProperty("MinFraction", m_minFrac = 0.2);
  declareProperty("OneRef", m_oneRef = false);
}

//=============================================================================
// Destructor
//=============================================================================
VeloPixCluster2MCParticleLinker::~VeloPixCluster2MCParticleLinker() {};

//=============================================================================
// Initialisation
//=============================================================================
StatusCode VeloPixCluster2MCParticleLinker::initialize() {
  StatusCode sc = GaudiAlgorithm::initialize();
  if(sc.isFailure()) return sc;
  if(sc) debug() << "==> Initialise" << endmsg;
  return StatusCode::SUCCESS;
};

//=============================================================================
//  Execution
//=============================================================================
StatusCode VeloPixCluster2MCParticleLinker::execute()
{
  // Get VeloPixClusters
  const VeloPixClusters* clusterCont = get<VeloPixClusters>(m_inputData);
  // Get MCParticles
  MCParticles* mcParts = get<MCParticles>(MCParticleLocation::Default);
  // Create a linker
  LinkerWithKey<MCParticle,VeloPixCluster> aLinker(evtSvc(),msgSvc(),
                                                   outputData());
  // Loop and link VeloPixClusters to MC truth
  VeloPixClusters::const_iterator iterClus =  clusterCont->begin();
  for(; iterClus != clusterCont->end(); ++iterClus) {
    // Find all particles
    ParticleMap partMap;
    StatusCode sc = associateToTruth(*iterClus,partMap);
    if(sc.isFailure()) return sc;
    // Select references to add to table
    std::vector<PartPair> selectedRefs;
    refsToRelate(selectedRefs,partMap,mcParts);
    if(selectedRefs.empty() == false) {
      if(m_oneRef == false) {
        double tWeight = 0;
        std::vector<PartPair>::iterator iterPair = selectedRefs.begin();
        while(iterPair != selectedRefs.end()) {
          aLinker.link(*iterClus,iterPair->first,iterPair->second);
          if(iterPair->first != 0) tWeight += iterPair->second;
          ++iterPair;
        }
      }
      else {
        PartPair bestPair = selectedRefs.back();
        aLinker.link(*iterClus,bestPair.first,bestPair.second);
      } 
    }
  }
  return StatusCode::SUCCESS;
};

//============================================================================
// Reference to related MC particles
//============================================================================
void VeloPixCluster2MCParticleLinker::refsToRelate(
                                      std::vector<PartPair>& selRefs,
                                      const ParticleMap& partMap,
                                      MCParticles* particles) const
{
  // Iterate over map
  ParticleMap::const_iterator iterMap = partMap.begin();
  while(iterMap != partMap.end()) {
    const MCParticle* aParticle = iterMap->first;
    if((0 != aParticle) && (iterMap->second > m_minFrac)) {
      if((m_addSpillOverHits == true) || (particles == aParticle->parent())) {
        selRefs.push_back(std::make_pair(aParticle,iterMap->second));
      }
    }
    ++iterMap;
  }
  return;
}


//============================================================================
// Associate to MC truth
//============================================================================
StatusCode VeloPixCluster2MCParticleLinker::associateToTruth(
                                            const VeloPixCluster* aCluster,
                                            ParticleMap& partMap)
{
  // Make link to truth  to MCHit from cluster
  typedef LinkerTool<VeloPixCluster, MCHit> AsctTool;
  typedef AsctTool::DirectType Table;
  typedef Table::Range Range;
  typedef Table::iterator iterator;
  // Use the VeloPixCluster to MCHit association
  AsctTool associator(evtSvc(),m_asctLocation);
  const Table* aTable = associator.direct();
  if(!aTable) return Error("Failed to find table", StatusCode::FAILURE);
  double foundCharge = 0;
  Range hitsCont = aTable->relations(aCluster);
  iterator iterHit = hitsCont.begin();   
  for(; iterHit != hitsCont.end(); ++iterHit) {
    const MCParticle* aParticle = iterHit->to()->mcParticle();
    partMap[aParticle] += iterHit->weight();
    foundCharge += iterHit->weight();
  }
  partMap[0] += 1.0 - foundCharge;
  return StatusCode::SUCCESS;
}


//============================================================================
StatusCode VeloPixCluster2MCParticleLinker::finalize() {

  return GaudiAlgorithm::finalize();
}
