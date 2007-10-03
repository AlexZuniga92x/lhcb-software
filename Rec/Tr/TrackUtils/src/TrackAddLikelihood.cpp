// $Id: TrackAddLikelihood.cpp,v 1.2 2007-10-03 14:01:37 mneedham Exp $


// BOOST
#include <boost/assign/list_of.hpp> // for 'map_list_of()


// Gaudi
#include "GaudiKernel/AlgFactory.h"

// track interfaces
#include "Event/Track.h"

#include "TrackInterfaces/ITrackManipulator.h"
#include "TrackAddLikelihood.h"
#include "GaudiKernel/ToStream.h"

using namespace LHCb;

DECLARE_ALGORITHM_FACTORY( TrackAddLikelihood );

TrackAddLikelihood::TrackAddLikelihood(const std::string& name,
                       ISvcLocator* pSvcLocator):
  GaudiAlgorithm(name, pSvcLocator)
{
 
 declareProperty("inputLocation", m_inputLocation = TrackLocation::Default);
 declareProperty("LikelihoodTool", m_likelihoodToolName = "TrackLikelihood");



 declareProperty("types", m_types = boost::assign::list_of(Track::PatVelo )(Track::PatVeloTT )
                                           (Track::PatForward )(Track::TrackMatching )
                                           (Track::PatKShort)(Track::TsaTrack)(Track::PatVeloGeneral));                  
}

TrackAddLikelihood::~TrackAddLikelihood()
{
  // destructor
}


StatusCode TrackAddLikelihood::initialize() {
  
  std::vector<unsigned int>::const_iterator iterT = m_types.begin();
  for (; iterT != m_types.end(); ++iterT){
    Track::History type = Track::History(*iterT);
    std::string aName = Gaudi::Utils::toString(type);
    ITrackManipulator* aTool =  tool<ITrackManipulator>(m_likelihoodToolName,
                                                        aName+"_likTool", this );
   
    m_toolMap[type] = aTool; 
  } // iterT
  return StatusCode::SUCCESS;
}

StatusCode TrackAddLikelihood::execute(){

  Tracks* inCont = get<Tracks>(m_inputLocation);

  // loop 
  for (Tracks::iterator iterT = inCont->begin(); iterT != inCont->end(); ++iterT) {
    unsigned int type = (*iterT)->history();
    if (std::find(m_types.begin(), m_types.end(),type) != m_types.end()){ 
      m_toolMap[type]->execute(**iterT).ignore();
    }
    else {
      return Warning("Likelihood not calculated: Unknown track type",
                     StatusCode::SUCCESS );
    }
  } // iterT
   
  return StatusCode::SUCCESS;
};

