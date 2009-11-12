// $:  $

// Gaudi
#include "GaudiKernel/AlgFactory.h"

#include "TsaSeed.h"

#include "Event/Track.h"

// boost
#include "boost/lexical_cast.hpp"
#include <boost/lambda/bind.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/foreach.hpp>

// try
#include "TsaKernel/TsaTStationHitManager.h"
#include "TsaKernel/TsaSeedingHit.h"


using namespace Tf::Tsa;
using namespace boost::lambda;

typedef Tf::Tsa::TStationHitManager ITHitMan;
typedef Tf::Tsa::TStationHitManager::HitRange Hits;



// factory defs
DECLARE_ALGORITHM_FACTORY( Seed );

Seed::Seed(const std::string& name,
           ISvcLocator* pSvcLocator):
  BaseAlg(name, pSvcLocator)
{

  declareProperty("addHits", m_addHitsInITOverlap = true);
  declareProperty("calcLikelihood", m_calcLikelihood = true);
  declareProperty("selectorType", m_selectorType = "Tf::Tsa::SeedSelector");
  declareProperty("seedTracksLocation", m_seedTrackLocation = SeedTrackLocation::Default);
  declareProperty("seedHitLocation",  m_seedHitLocation = SeedHitLocation::Default);
  declareProperty("seedStubLocation", m_seedStubLocation = SeedStubLocation::Default);
  declareProperty("OnlyGood", m_onlyGood = false);
  declareProperty("DiscardChi2", m_discardChi2 = 1.5);
  
}

Seed::~Seed(){}

StatusCode Seed::initialize()
{
  // Initialization

  const StatusCode sc = BaseAlg::initialize();
  if (sc.isFailure()){
    return Error("Failed to initialize");
  }

  // init the tools
  for (unsigned int i = 0; i < 3; ++i){
    ITsaSeedStep* xStepIT = tool<ITsaSeedStep>("Tf::Tsa::ITXSearch", "xSearchS" + boost::lexical_cast<std::string>(i) , this);
    ITsaSeedStep* stereoStepIT = tool<ITsaSeedStep>("Tf::Tsa::ITStereoSearch", "stereoS"
                                                    + boost::lexical_cast<std::string>(i) , this);
    m_xSearchStep.push_back(xStepIT);
    m_stereoStep.push_back(stereoStepIT);
  }
  for (unsigned int o = 3; o < 5; ++o){
    ITsaSeedStep* xStepOT = tool<ITsaSeedStep>("Tf::Tsa::OTXSearch", "xSearchS" + boost::lexical_cast<std::string>(o) , this);
    ITsaSeedStep* stereoStepOT = tool<ITsaSeedStep>("Tf::Tsa::OTStereoSearch", "stereoS"
                                                    + boost::lexical_cast<std::string>(o) , this);
    m_xSearchStep.push_back(xStepOT);
    m_stereoStep.push_back(stereoStepOT);
  }

  m_xSelection = tool<ITsaSeedStep>("Tf::Tsa::XProjSelector", "xSelection" , this);
  m_finalSelection = tool<ITsaSeedStep>(m_selectorType, "finalSelection" , this);
  m_likelihood = tool<ITsaSeedStep>("Tf::Tsa::Likelihood", "likelihood" , this);
  m_addHits = tool<ITsaSeedAddHits>("Tf::Tsa::SeedAddHits","SeedAddHits",this);
  m_stubFind = tool<ITsaStubFind>("Tf::Tsa::StubFind","stubFinder" ,this);
  m_stubLinker = tool<ITsaStubLinker>("Tf::Tsa::StubLinker","stubLinker",this);
  m_extendStubs =  tool<ITsaStubExtender>("Tf::Tsa::StubExtender","stubExtender",this);

  
  
  return sc;
}

StatusCode Seed::execute(){
  //-------------------------------------------------------------------------
  //  Steering routine for track seeding
  //-------------------------------------------------------------------------

  //  startTimer();
  SeedTracks* seedSel = new SeedTracks();    //  Selected seed candidates
  seedSel->reserve(1000);
  std::vector<SeedTrack*> tempSel; tempSel.reserve(1000);

  if (m_onlyGood) {
    // retrieve all TsaSeedingHits
    ITHitMan* m_hitMan = tool<ITHitMan>("Tf::Tsa::TStationHitManager","TsaDataManager");
    Hits allhits = m_hitMan->hits();
    debug() << "All Tsa hits: " << allhits.size() << endreq;
    
    LHCb::Tracks* fwdTracks = get<LHCb::Tracks>( LHCb::TrackLocation::Forward );
    for ( LHCb::Tracks::const_iterator itT = fwdTracks->begin(); fwdTracks->end() != itT; ++itT ) {
      LHCb::Track* tr = *itT;              
      if (tr->fitStatus() == LHCb::Track::Fitted) {
        // if fitted - used chi2pdf from fit
        if (tr->chi2PerDoF() < m_discardChi2) continue; 
      }
      //else {
      // otherwise use PatQuality from patreco
      // maybe we will have good discriminating property from pat reco only
      //  if (tr->info(LHCb::Track::PatQuality, -1.) < 4.0) continue;
      //}
      debug() << "*** Found bad PatFwd track, marking TsaSeedingHits unused. " ;
      int nHits = 0;
      for ( std::vector<LHCb::LHCbID>::const_iterator itId = tr->lhcbIDs().begin();
            tr->lhcbIDs().end() != itId; ++itId ) {
        for ( Hits::const_iterator itIter = allhits.begin(); itIter != allhits.end(); ++itIter) {
          if ( (*itIter)->hit()->lhcbID()==(*itId) ){
            nHits++;
            (*itIter)->hit()->setStatus(Tf::HitBase::UsedByPatForward, false);
            break;
          }
        }
      }
      debug() << nHits << " hits marked." << endreq; 
    }
  }
  

 
  std::vector<SeedStub*> stubs[3];            //  IT stubs per station
  for (unsigned iS = 0; iS < 3; ++iS) stubs[iS].reserve(100);

  SeedStubs* stubsCont = new SeedStubs();
  stubsCont->reserve(200);

  SeedHits* hitsCont = new SeedHits();
  hitsCont->reserve(10000);

  // put output in the store
  put(seedSel,m_seedTrackLocation);
  put(hitsCont, m_seedHitLocation);
  put(stubsCont, m_seedStubLocation);

  StatusCode sc(StatusCode::SUCCESS,true);

  // Loop over sectors of tracker (0-2 are IT, 3-4 are OT)
  for ( int sector = 0; sector < 5; ++sector ) {
    verbose() << "Sector : " << sector << endreq;

    std::vector<SeedHit*> hits[6], sHits[6];  //  Hits per layer in X and stereo

    std::vector<SeedTrack*> seeds;            //  Seed candidates within the sector
    seeds.reserve(1000);

    verbose() << "Starting X Search" << endreq;
    sc = m_xSearchStep[sector]->execute(seeds,hits);  // x search
    if (sc.isFailure()) {
      return Error("x search failed", StatusCode::FAILURE,1);
    }
    verbose() << " -> XSearch found " << seeds.size() << " seeds" << endreq;

    verbose() << "Starting X selection" << endreq;
    if (sector >2 ) {
      sc = m_xSelection->execute(seeds); // x selection
      if (sc.isFailure()) {
        return Error("x selection failed", StatusCode::FAILURE,1);
      }
      verbose() << " -> XSelection found " << seeds.size() << " seeds" << endreq;
    }

    verbose() << "Starting Stereo Search" << endreq;
    sc = m_stereoStep[sector]->execute(seeds,sHits); // add stereo
    if (sc.isFailure()) {
      return Error("stereo search failed", StatusCode::FAILURE,1);
    }
    verbose() << " -> StereoSearch found " << seeds.size() << " seeds" << endreq;

    verbose() << "Starting First Likelihood calculation" << endreq;
    if (m_calcLikelihood) {
      sc = m_likelihood->execute(seeds); // likelihood
      if (sc.isFailure()) {
        return Error("likelihood failed", StatusCode::FAILURE,1);
      }
    }

    verbose() << "Starting Final Selection" << endreq;
    sc = m_finalSelection->execute(seeds); // final selection
    if (sc.isFailure()) {
      return Error("selection failed", StatusCode::FAILURE,1);
    }
    verbose() << " -> Final Selection found " << seeds.size() << " seeds" << endreq;

    //Delete the temporary objects that have been created
    for ( std::vector<SeedTrack*>::iterator it = seeds.begin(); seeds.end() != it; ++it ) {
      seedSel->insert(*it);
      verbose() << "SeedTrack inserted " << **it << endreq;
    }

    if ( sector <= 2 ) {
      sc = m_stubFind->execute( hits, sHits, stubs );
      if (sc.isFailure()) {
        return Error("stub finding failed", StatusCode::FAILURE,1);
      }
    }

    //  After the IT stub finding is finished, try to link the stubs to make seed candidates
    if ( sector == 2 ) {
      verbose() << "Starting 'thing I don't understand' #1" << endreq;
      std::vector<SeedTrack*> linkedSeeds; linkedSeeds.reserve(50);
      sc = m_stubLinker->execute( stubs, linkedSeeds );
      if (sc.isFailure()) {
        return Error("stub linking failed", StatusCode::FAILURE,1);
      }
      if (m_calcLikelihood == true) {
        sc = m_likelihood->execute(linkedSeeds);
        if (sc.isFailure()) {
          return Error("stub likelihood failed", StatusCode::FAILURE,1);
        }
      }
      sc = m_finalSelection->execute(linkedSeeds);
      if (sc.isFailure()) {
        return Error("stub selection failed", StatusCode::FAILURE,1);
      }
      for ( std::vector<SeedTrack*>::iterator itLinked = linkedSeeds.begin(); linkedSeeds.end() != itLinked; ++itLinked ) {
        seedSel->insert( *itLinked);
        verbose() << "SeedTrack inserted " << **itLinked << endreq;
      }  // it
    }

    //  For those IT stubs that remain, try to extend them into the OT
    if ( sector > 2 ) {
      verbose() << "Starting 'thing I don't understand' #2" << endreq;
      std::vector<SeedTrack*> extendedSeeds; extendedSeeds.reserve(50);
      sc = m_extendStubs->execute( sector, stubs, hits, sHits, extendedSeeds );
      if (sc.isFailure()) {
        return Error("stub extension", StatusCode::FAILURE,1);
      }
      if (m_calcLikelihood == true) {
        sc = m_likelihood->execute(extendedSeeds);
        if (sc.isFailure()) {
          return Error("extended stub likelihood failed", StatusCode::FAILURE,1);
        }
      }
      sc = m_finalSelection->execute(extendedSeeds);
      if (sc.isFailure()) {
        return Error("extended stub selection failed", StatusCode::FAILURE,1);
      }
      for ( std::vector<SeedTrack*>::iterator itEx = extendedSeeds.begin(); extendedSeeds.end() != itEx; ++itEx ) {
        seedSel->insert( *itEx);
        verbose() << "SeedTrack inserted " << **itEx << endreq;
      }
    }

    for ( int lay = 0; lay < 6; ++lay ) {
      for ( std::vector<SeedHit*>::iterator it = hits[lay].begin(); hits[lay].end() != it; ++it )
      {
        hitsCont->insert(*it);
        verbose() << "SeedHit inserted " << **it << endreq;
      }
      for ( std::vector<SeedHit*>::iterator it = sHits[lay].begin(); sHits[lay].end() != it; ++it )
      {
        hitsCont->insert(*it);
        verbose() << "SeedHit inserted " << **it << endreq;
      }
    }
  }

  // clean up the stubs...
  for ( int stn = 0; stn < 3; ++stn ) {
    for ( std::vector<SeedStub*>::iterator it = stubs[stn].begin(); stubs[stn].end() != it; ++it ) {
      stubsCont->insert(*it);
      verbose() << "SeedStub inserted " << **it << endreq;
    }
  }

  // add hits in IT overlap region
  if (m_addHitsInITOverlap == true) {
    verbose() << "Adding Overlap Hits" << endreq;
    sc = m_addHits->execute(seedSel, hitsCont);
    if (sc.isFailure()) {
      return Error("failed to add hits", StatusCode::FAILURE,1);
    }

    SeedTracks::iterator iterT = seedSel->begin();
    for (; iterT != seedSel->end(); ++iterT){
      std::vector<SeedPnt> clusVector = (*iterT)->usedPnts();
      std::for_each(clusVector.begin(),clusVector.end(), 
                  bind(&SeedPnt::setOnTrack,_1, false));
      if ((*iterT)->select() == false) (*iterT)->setLive(false) ;
      (*iterT)->setSelect(false);
      tempSel.push_back(*iterT);
    } // iterT

    sc = m_finalSelection->execute(tempSel);
    if (sc.isFailure()) {
      return Error("failed to make final selection", StatusCode::FAILURE,1);
    }

  }

  debug() << "Created " << seedSel->size() << " SeedTracks at " << m_seedTrackLocation << endreq;

  return StatusCode::SUCCESS;
}
