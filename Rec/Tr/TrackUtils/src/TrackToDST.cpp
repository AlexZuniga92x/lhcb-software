// $Id: TrackToDST.cpp,v 1.10 2007-07-04 11:10:55 dhcroft Exp $
//
// This File contains the implementation of the TsaEff
// C++ code for 'LHCb Tracking package(s)'
//


// BOOST
#include <boost/assign/list_of.hpp> // for 'map_list_of()
// Gaudi
#include "GaudiKernel/AlgFactory.h"

// track interfaces
#include "Event/Track.h"

#include "TrackToDST.h"
using namespace LHCb;
using namespace std;
using namespace boost::assign; // bring 'map_list_of()' into scope

DECLARE_ALGORITHM_FACTORY( TrackToDST );

TrackToDST::TrackToDST(const std::string& name,
                       ISvcLocator* pSvcLocator):
  GaudiAlgorithm(name, pSvcLocator)
{
  // constructor
  this->declareProperty( "TracksInContainer",
                         m_inputLocation = TrackLocation::Default );
  this->declareProperty( "StoreAllStates", m_storeAllStates = false );

  m_theMap = map_list_of("ClosestToBeam", State::ClosestToBeam) 
                        ("FirstMeasurement", State::FirstMeasurement )
                        ("EndVelo",State::EndVelo)
                        ("AtTT",State::AtTT)
                        ("AtT",State::AtT)
                        ("BegRich1",State::BegRich1)
                        ("EndRich1",State::EndRich1)
                        ("BegRich2",State::BegRich2)
                        ("EndRich2",State::EndRich2)
                        ("Spd",State::Spd)
                        ("Prs",State::Prs)
                        ("BegECal", State::BegECal)
                        ("ECalShowerMax", State::ECalShowerMax)
                        ("EndECal",State::EndECal)
                        ("BegHCal", State::BegHCal)
                        ("MidHCal",State::MidHCal)
                        ("EndHCal",State::EndHCal)
                        ("Muon",State::Muon);

  declareProperty("veloStates", m_veloStrings);
  declareProperty("longStates", m_longStrings); 
  declareProperty("TTrackStates", m_tStrings);
  declareProperty("downstreamStates", m_downstreamStrings);
  declareProperty("upstreamStates", m_upstreamStrings);

  m_veloStrings = list_of("ClosestToBeam");
  m_longStrings =  list_of("ClosestToBeam")("BegRich1")("BegRich2");
  m_tStrings = list_of("FirstMeasurement")( "BegRich2");
  m_downstreamStrings = list_of("BegRich1")("FirstMeasurement")("BegRich2");
  m_upstreamStrings = list_of("ClosestToBeam")("BegRich1");

}

TrackToDST::~TrackToDST()
{
  // destructor
}

StatusCode TrackToDST::initialize()
{
  // Initializes TsaInitialization at the begin of program execution.

  StatusCode sc = GaudiAlgorithm::initialize();
  if (sc.isFailure()){
     return Error("Failed to initialize");
  }

  stringToLoc(m_veloStrings,m_veloStates);
  stringToLoc(m_longStrings,m_longStates);
  stringToLoc(m_tStrings,m_tStates);
  stringToLoc(m_downstreamStrings,m_downstreamStates);
  stringToLoc(m_upstreamStrings,m_upstreamStates);

  return StatusCode::SUCCESS;
}

StatusCode TrackToDST::execute(){

  Tracks* inCont = get<Tracks>(m_inputLocation);

  // loop 
  for (Tracks::iterator iterT = inCont->begin(); 
       iterT != inCont->end(); ++iterT){
 
    // remove the necessary States on the Track
    if (m_storeAllStates == false){
      // done in an ugly way for now - will be easier with the new 
      // jobOptions parser
      unsigned int type = (*iterT)->type();
      switch (type){
      case Track::Velo:
      case Track::VeloR:
        cleanStates(*iterT, m_veloStates);
        break;
      case Track::Long:
        cleanStates(*iterT, m_longStates);
        break;
      case Track::Upstream:
        cleanStates(*iterT, m_upstreamStates);
        break;
      case Track::Downstream:
        cleanStates(*iterT, m_downstreamStates);
        break;
      case Track::Ttrack:
        cleanStates(*iterT, m_tStates);
        break;
      default:
        Warning( format("Unknown track type %i",type) , StatusCode::SUCCESS, 1 );
        break;
      } // switch
    } // if

  // remove all the Measurements on the Track
  const std::vector<Measurement*> allmeas = (*iterT) -> measurements();
  for ( std::vector<Measurement*>::const_iterator it = allmeas.begin();
        it != allmeas.end(); ++it)
    (*iterT) -> removeFromMeasurements( *it );
  // set the appropriate flag!
  (*iterT) -> setPatRecStatus( Track::PatRecIDs );

  } // iterT
   
  return StatusCode::SUCCESS;
};


void TrackToDST::cleanStates(Track* aTrack, const SLocations& loc) const{

  std::vector<State*> tempCont;
  SLocations::const_iterator iterL = loc.begin();
  for (; iterL != loc.end(); ++iterL){
    if (aTrack->hasStateAt(*iterL) == true){
      State& tState = aTrack->stateAt(*iterL);
      tempCont.push_back(tState.clone());
    }
    else {
      Warning("Failed to find state - more info in DEBUG",StatusCode::SUCCESS,1);
      debug() << "Missing state at " << *iterL << " on track " << aTrack->key() 
              << " of type " << aTrack->type() << endmsg;
    } 
  } // loca

  aTrack->clearStates();

  std::vector<State*>::iterator iterS = tempCont.begin(); 
  for (; iterS != tempCont.end(); ++iterS){ 
    aTrack->addToStates(**iterS);
    delete *iterS;
  }
} 

void TrackToDST::stringToLoc(const Strings& sCont, SLocations& loc) const{

  loc.reserve(sCont.size());

  Strings::const_iterator iterS = sCont.begin();
  for (; iterS != sCont.end(); ++iterS){
    StateMap::const_iterator mIter = m_theMap.find(*iterS); 
    if (mIter != m_theMap.end()){
      loc.push_back(mIter->second);
    }
    else {
      error() << "attempt to convert unknown state" << endmsg;
    }
  } // iter   

}
