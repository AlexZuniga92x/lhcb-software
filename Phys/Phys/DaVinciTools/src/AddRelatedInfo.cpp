// Include files 
#include "GaudiKernel/DeclareFactoryEntries.h"
#include "GaudiKernel/Property.h"
#include "Kernel/IRelatedInfoTool.h"

#include "boost/algorithm/string.hpp"
#include "Event/RelatedInfoMap.h"

// local
#include "AddRelatedInfo.h"

//-----------------------------------------------------------------------
// Implementation file for class : AddRelatedInfo
//
// Author: Anton Poluektov
//-----------------------------------------------------------------------

using namespace LHCb ;
using namespace Gaudi::Units;

// Declaration of the Algorithm Factory
DECLARE_ALGORITHM_FACTORY( AddRelatedInfo )

//=======================================================================
AddRelatedInfo::AddRelatedInfo(const std::string& name,
                             ISvcLocator* pSvcLocator):
    DaVinciAlgorithm(name, pSvcLocator)
{
  declareProperty("Tools", m_toolNames, "Names of RelatedInfoTools" );
  declareProperty("MaxLevel", m_maxLevel = 0, "Maximum recursion level");
  declareProperty("DaughterLocations", m_daughterLocations, "Locations of daughters"); 
}

//=======================================================================
StatusCode AddRelatedInfo::initialize()
{
  const StatusCode sc = DaVinciAlgorithm::initialize();
  if ( sc.isFailure() ) return sc;

  m_tools.clear();

  std::vector<std::string>::iterator iTool;
  for (iTool = m_toolNames.begin(); iTool != m_toolNames.end(); iTool++) {
    IRelatedInfoTool* t = tool<IRelatedInfoTool>(*iTool,this);

    if (t) {
      m_tools.push_back(t);
    } else {
      error() << "Tuple not found, name = " << (*iTool) << endreq;
      return StatusCode::FAILURE;
    }
  }

  return sc;
}

AddRelatedInfo::~AddRelatedInfo() {}

//=======================================================================
// Main execution
//=======================================================================
StatusCode AddRelatedInfo::execute()
{
  setFilterPassed( true ); // Filter always passes

  m_relMap.clear(); 

  // Loop over input locations
  std::vector<std::string>::const_iterator iLoc = inputLocations().begin();
  std::vector<std::string>::const_iterator endLoc = inputLocations().end();
  for ( ; iLoc != endLoc; ++iLoc) 
  {
    const std::string location = (*iLoc) + "/Particles";

    const Particle::Range parts = getIfExists<Particle::Range>( location );
    if( parts.empty() ) 
    {
      if (msgLevel(MSG::VERBOSE)) verbose() << "No particles found at " << location << endreq;
      continue;
    }

    if (msgLevel(MSG::VERBOSE)) verbose() << " Found "<< parts.size() << " particles" <<endreq;

    // Loop over particles in the locations
    Particle::Range::const_iterator icand;
    for ( icand = parts.begin(); icand != parts.end(); icand++) {

      Particle* c = const_cast<Particle*>(*icand);

      fill(c, c, 0);
    }

  }

  return StatusCode::SUCCESS;
}

//==========================================================================

void AddRelatedInfo::fill(const Particle* top, Particle* c, int level) {

  std::string c_location = c && c->parent() && c->parent()->registry() ?
                           c->parent()->registry()->identifier() : "NotInTES"; 

  bool isInDaughters = false; 

  // For particles other than top of the decay tree, 
  // check if they are in the list of daughter locations
  if (c != top) {

    std::vector<std::string>::const_iterator iDaughterLocation = m_daughterLocations.begin(); 
    for (; iDaughterLocation != m_daughterLocations.end(); iDaughterLocation++) {
      if (c_location.compare(*iDaughterLocation) == 0) {
        isInDaughters = true; 
        break;
      }
    }
  }

  if (c == top || isInDaughters) {

    if (msgLevel(MSG::DEBUG)) debug() << "Filling RelatedInfo for particle at " << c_location << endreq; 

    // Calculate extra information using each tool
    std::list<IRelatedInfoTool*>::iterator iTool;
    for (iTool = m_tools.begin(); iTool != m_tools.end(); iTool++) {
    
      std::string relatedInfoPath = (*iTool)->infoPath(); 
      std::string map_location = c_location; 
      boost::replace_all(map_location, "/Particles", "/" + relatedInfoPath); 
    
      ParticleInfoRelation* relation = getOrCreate<ParticleInfoRelation, ParticleInfoRelation>(map_location); 
      
      StatusCode sc = (*iTool)->calculateRelatedInfo(top, c);

      if (sc.isFailure()) {
        warning() << "Error calculating related info" << endreq;
        continue;
      }
      
      RelatedInfoMap* map = (*iTool)->getInfo(); 
      
      if (msgLevel(MSG::DEBUG)) {
        debug() << "Got RelatedInfoMap, contents as follows: " << endreq;
        RelatedInfoMap::iterator i; 
        for (i = map->begin(); i != map->end(); i++) {
          short key = (*i).first;
          float val = (*i).second; 
          debug() << "  Key = " << key << ", val = " << val << endreq; 
        }
      }
      
      relation->i_relate(c, *map); 

    }
    
  } else {
    if (msgLevel(MSG::VERBOSE)) verbose() << "Particle at " << c_location << " not in the list, skipping" << endreq; 
  }

  // If we reached the maximum recursion level, we're done
  if (level >= m_maxLevel) return;
  
  // Otherwise loop over the daughters of the particle
  const SmartRefVector< LHCb::Particle > & daughters = c->daughters();

  for( SmartRefVector< LHCb::Particle >::const_iterator idau = daughters.begin() ; idau != daughters.end() ; ++idau){
    if( !(*idau)->isBasicParticle() ) {
      // -- if it is not stable, call the function recursively
      
      const Particle* const_dau = (*idau);
      Particle* dau = const_cast<Particle*>(const_dau); 
    
      if ( msgLevel(MSG::DEBUG) ) debug() << " Filling RelatedInfo for daughters of ID " << dau->particleID().pid() << endmsg;
      fill( top, dau, level+1 );
    }
  }

}
