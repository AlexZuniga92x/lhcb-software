#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE utestTrackingCoder
#include <boost/test/unit_test.hpp>

#include "../HltTrackingCoder.h"
#include <vector>
#include "Event/Track.h"
#include "Event/State.h"
#include "Kernel/LHCbID.h"
#include <iostream>
using namespace LHCb; 
using namespace std;

// Helper function to compare two track containers element wise
// Check for equal LHCbIDs
bool
equalLHCbIDs(Tracks* tracks, Tracks* reftracks){
  LHCb::Tracks::const_iterator  pRefItr = reftracks->begin();
  bool result=true;
  for(LHCb::Tracks::const_iterator  pItr = tracks->begin(); tracks->end() != pItr; ++pItr){
      LHCb::Track* Tr = (*pItr);
      LHCb::Track* refTr = (*pRefItr);
      result &= ((Tr->lhcbIDs()) == (refTr->lhcbIDs()));
      pRefItr++; // don't forget to increment reference iterator 
  }// end loop over tracks
  return result;
}
 
// Check for equal States
bool
equalStates(Tracks* tracks, Tracks* reftracks){
  LHCb::Tracks::const_iterator  pRefItr = reftracks->begin();
  bool result=true;
  for(LHCb::Tracks::const_iterator  pItr = tracks->begin(); tracks->end() != pItr; ++pItr){
      LHCb::Track* Tr = (*pItr);
      LHCb::Track* refTr = (*pRefItr);
      // check the number of states
      unsigned int nstates=Tr->states().size();
      if (nstates != (refTr->states().size())) return false;
      const vector<State*>& states = Tr->states();
      const vector<State*>& refstates = refTr->states();
      for(unsigned int i=0;i<nstates;++i){
	result &= (states[i]->location() == refstates[i]->location());
	result &= (states[i]->stateVector() == refstates[i]->stateVector());
	//std::cout << states[i]->stateVector() << std::endl;
	//std::cout << refstates[i]->stateVector() << std::endl;
      }

      pRefItr++; // don't forget to increment reference iterator 
  }// end loop over tracks
  return result;
}


// The test fixture: Create a container of tracks to encode
// This can be used as a common starting point for all tests
struct ExampleTracks {
  
  Tracks m_tracks;                  // collection of tracks
  vector<unsigned int> m_rawbank;   // corresponding RawBank
 
  ExampleTracks() {
    BOOST_TEST_MESSAGE("setup example tracks");
    Track* tr=new Track();
    tr->addToLhcbIDs(LHCbID(1));
    tr->addToLhcbIDs(LHCbID(3));
    tr->addToLhcbIDs(LHCbID(5));
    m_tracks.add(tr);

    tr=new Track();
    tr->addToLhcbIDs(LHCbID(2));
    tr->addToLhcbIDs(LHCbID(4));
    tr->addToLhcbIDs(LHCbID(6));
    m_tracks.add(tr);
    tr=new Track();
    tr->addToLhcbIDs(LHCbID(7));
    tr->addToLhcbIDs(LHCbID(11));
    tr->addToLhcbIDs(LHCbID(13));
    tr->addToLhcbIDs(LHCbID(17));
    m_tracks.add(tr);
    // add a State to this track
    Gaudi::TrackVector v(1,1,1,1,1);
    Gaudi::TrackSymMatrix cov;

    State newYork(v,cov,200.,State::EndVelo);
    tr->addToStates(newYork);

    State Michigan(v,cov,220.,State::ClosestToBeam);
    tr->addToStates(Michigan);
    
    // format:                 nIDs  IDs    nStates States(Loc)
    vector<unsigned int> bank = {3,  1,3,5, 0,      
				 3,  2,4,6, 0,
				 4,  7,11,13,17, 2,  // 2 states in this track
				 3 , 1072693248,0 , 1072693248,0 , 1072693248,0 , 1072693248,0 , 1072693248,0 , // state at location 3
				 1 , 1072693248,0 , 1072693248,0 , 1072693248,0 , 1072693248,0 , 1072693248,0}; // state at location 1
    m_rawbank = bank;
  }

  ~ExampleTracks() {
    BOOST_TEST_MESSAGE("tear down example tracks");
    m_tracks.clear();
  }

};

// Each test in this suite will use the ExampleTracks fixture
// fixtures are setup and teared down for each test individually
BOOST_FIXTURE_TEST_SUITE(TrackingCoder, ExampleTracks)
 
BOOST_AUTO_TEST_CASE(comparetracks)
{
  BOOST_CHECK(equalLHCbIDs(&m_tracks,&m_tracks));
  BOOST_CHECK(equalStates(&m_tracks,&m_tracks));
}

BOOST_AUTO_TEST_CASE(encode)
{
  vector<unsigned int> rawBank;
  encodeTracks(&m_tracks,rawBank);
  BOOST_CHECK(rawBank.size() == m_rawbank.size()  );
  BOOST_CHECK(rawBank == m_rawbank );
}

BOOST_AUTO_TEST_CASE(decode)
{
  Tracks tracks;
  BOOST_CHECK(decodeTracks(m_rawbank.data(),m_rawbank.size(),&tracks) == m_tracks.size());
  BOOST_CHECK(tracks.size() == m_tracks.size() );
  BOOST_CHECK(equalLHCbIDs(&tracks,&m_tracks));
  BOOST_CHECK(equalStates(&tracks,&m_tracks));

}


BOOST_AUTO_TEST_CASE(en_de_code)
{
  vector<unsigned int> rawBank;
  encodeTracks(&m_tracks,rawBank);
  Tracks tracks;
  decodeTracks(m_rawbank.data(),m_rawbank.size(),&tracks);
  BOOST_CHECK(equalLHCbIDs(&tracks,&m_tracks));
  BOOST_CHECK(equalStates(&tracks,&m_tracks));

}


BOOST_AUTO_TEST_SUITE_END()

