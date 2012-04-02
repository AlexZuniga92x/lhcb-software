// Include files 
#include "GaudiKernel/DeclareFactoryEntries.h"
#include "GaudiKernel/Property.h"

#include "Kernel/IExtraInfoTool.h"
// local
#include "AddExtraInfo.h"

//-----------------------------------------------------------------------
// Implementation file for class : AddExtraInfo
//
// Author: Anton Poluektov
//-----------------------------------------------------------------------

using namespace LHCb ;
using namespace Gaudi::Units;

// Declaration of the Algorithm Factory
DECLARE_ALGORITHM_FACTORY( AddExtraInfo )

//=======================================================================
  AddExtraInfo::AddExtraInfo(const std::string& name,
                             ISvcLocator* pSvcLocator):
    DVAlgorithm(name, pSvcLocator)
{
  declareProperty("Tools", m_toolNames, "Names of ExtraInfoTools" );
}

//=======================================================================
StatusCode AddExtraInfo::initialize()
{
  const StatusCode sc = DVAlgorithm::initialize();
  if ( sc.isFailure() ) return sc;

  m_tools.clear();

  std::vector<std::string>::iterator iTool;
  for (iTool = m_toolNames.begin(); iTool != m_toolNames.end(); iTool++) {
    IExtraInfoTool* t = tool<IExtraInfoTool>(*iTool,this);

    if (t) {
      m_tools.push_back(t);
    } else {
      error() << "Tuple not found, name = " << (*iTool) << endreq;
      return StatusCode::FAILURE;
    }
  }

  return sc;
}

AddExtraInfo::~AddExtraInfo() {}

//=======================================================================
// Main executio
//=======================================================================
StatusCode AddExtraInfo::execute() {

  setFilterPassed( true ); // Filter always passes

  // Loop over input locations
  std::vector<std::string>::const_iterator iLoc = inputLocations().begin();
  std::vector<std::string>::const_iterator endLoc = inputLocations().end();
  for ( ; iLoc != endLoc; ++iLoc) {

    std::string location = (*iLoc) + "/Particles";

    if(!exist<LHCb::Particle::Range>(location) ) {
      debug()<<("No selection found in "+ location)<<endreq;
      continue;
    }

    const Particle::Range parts = get<Particle::Range>( location );
    if( parts.empty() ) {
      if (msgLevel(MSG::VERBOSE)) verbose() << "No particles found at " << location << endreq;
      continue;
    }

    if (msgLevel(MSG::VERBOSE)) verbose() << " Found "<< parts.size() << " particles" <<endreq;

    // Loop over particles in the locations
    Particle::Range::const_iterator icand;
    for ( icand = parts.begin(); icand != parts.end(); icand++) {

      std::list<IExtraInfoTool*>::iterator iTool;

      Particle* c = const_cast<Particle*>(*icand);

      // Calculate extra information using each tool
      for (iTool = m_tools.begin(); iTool != m_tools.end(); iTool++) {

        StatusCode sc = (*iTool)->calculateExtraInfo(*icand, *icand);
        if (sc.isFailure()) {
          error() << "Error calculating extra info" << endreq;
          return sc;
        }

        int index = (*iTool)->getFirstIndex();

        for (int i=0 ; i < (*iTool)->getNumberOfParameters(); i++ ) {
          std::string name;
          double value;

          (*iTool)->getInfo(index+i, value, name);

          c->addInfo( index+i, value);

          if (msgLevel(MSG::VERBOSE)) verbose() << "Added extra info: " << name << "=" << value << endreq;
        }

      }

    }

  }

  return StatusCode::SUCCESS;
}

//==========================================================================
