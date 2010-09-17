#include "GaudiKernel/AlgFactory.h"
#include "VeloMuonBuilder.h"

#include "math.h"
#include <stdlib.h>
#include <stdio.h>
#include "Kernel/LHCbID.h"
#include "Kernel/MuonLayout.h"
#include "Kernel/ILHCbMagnetSvc.h"

#include "Event/TrackFitResult.h"

//-----------------------------------------------------------------------------
// Implementation file for class : VeloMuonBuilder
//
// 2010-09-16 : Paul Seyfert
//-----------------------------------------------------------------------------


DECLARE_ALGORITHM_FACTORY( VeloMuonBuilder );
//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
VeloMuonBuilder::VeloMuonBuilder( const std::string& name, ISvcLocator* pSvcLocator ) :
  GaudiTupleAlg( name , pSvcLocator ) {

  declareProperty( "MuonLocation"    , m_muonpath = "Hlt1/Tracks/MuonSeg" );
  declareProperty( "VeloLocation"    , m_velopath = LHCb::TrackLocation::Velo );
  declareProperty( "OutputLocation"  , m_output = "Rec/Track/VeloMuon" );
  declareProperty( "zmagnet"         , m_zmagnet = 5400. );
  declareProperty( "zmatch"          , m_zmatch = 15000. );
  declareProperty( "chamberhit"      , m_chamberhit = true );
  declareProperty( "distancecut"     , m_distancecut = 3000000 );
  declareProperty( "xscale"          , m_xscale = 0.06);
  declareProperty( "chiweight"       , m_chiweight = 0.1);
  declareProperty( "lhcbids"         , n_lhcbids = 4);
  declareProperty( "cut"       , m_distcutmultiplyer =1);
  declareProperty( "MaxVeloTracks" , m_maxvelos = 1000 );
  declareProperty( "MaxMuonTracks" , m_maxmuons = 30 );

}

//=============================================================================
// Destructor
//=============================================================================
VeloMuonBuilder::~VeloMuonBuilder() {}

//=============================================================================
// Initialization
//=============================================================================
StatusCode VeloMuonBuilder::initialize() {
  StatusCode sc = GaudiTupleAlg::initialize();
  if ( sc.isFailure() ) return sc;

  m_iPosTool=tool<IMuonFastPosTool>( "MuonFastPosTool" );
  m_linearextrapolator = tool<ITrackExtrapolator>( "TrackLinearExtrapolator" );
  m_tracksFitter = tool<ITrackFitter>( "TrackMasterFitter");

  ILHCbMagnetSvc* m_magFieldSvc = svc<ILHCbMagnetSvc>( "MagneticFieldSvc", true );
  m_fieldfactor = - (m_magFieldSvc->signedRelativeCurrent());

  return StatusCode::SUCCESS;
}


//=============================================================================
//  Finalization
//=============================================================================
StatusCode VeloMuonBuilder::finalize() {
  return GaudiTupleAlg::finalize();

}


using namespace LHCb;
//=============================================================================
// Main execution
//=============================================================================
StatusCode VeloMuonBuilder::execute() {

  if (exist<Tracks>(m_velopath))
    m_velotracks = get<Tracks>(m_velopath);
  else {
    return StatusCode::SUCCESS;
  }
  if (m_velotracks->size() > m_maxvelos)
    return StatusCode::SUCCESS;

  if (exist<Tracks>(m_muonpath))
    m_muontracks = get<Tracks>(m_muonpath);
  else {
    return StatusCode::SUCCESS;
  }
  if (m_muontracks->size() > m_maxmuons)
    return StatusCode::SUCCESS;


  //  LinkedTo<MCParticle,Track> veloLink( evtSvc(), msgSvc(), m_velopath );
  bool existed = false;
  Tracks* tracks;
  if (exist<Tracks>(m_output)) {
    existed = true;
    tracks = get<Tracks>(m_output);
  }  else {
    tracks = new Tracks();
  }

  //  tracks = buildVeloMuon(*m_velotracks, *m_muontracks, &veloLink);
  buildVeloMuon(*m_velotracks, *m_muontracks, tracks);
  
  if (!existed) {
    put(tracks,m_output);
  }
  
  return StatusCode::SUCCESS;
}

//=============================================================================
// Method kept for compatibility
//=============================================================================
Tracks VeloMuonBuilder::buildVeloMuon(Tracks* veloTracks, Tracks* muonTracks) {
  Tracks trackvector;
  Tracks* trackspointer = buildVeloMuon(*veloTracks,*muonTracks);
  Tracks::iterator trIt;
  for (trIt = trackspointer->begin(); trIt!=trackspointer->end(); ++trIt) {
    trackvector.add(*trIt);
  }
  return trackvector;
}

//=============================================================================
// Method kept for compatibility
//=============================================================================
Tracks* VeloMuonBuilder::buildVeloMuon(Tracks& veloTracks, Tracks& muonTracks) {
  Tracks* trackvector = new Tracks();
  StatusCode sc;
  sc = buildVeloMuon(veloTracks, muonTracks, trackvector);
  if (sc.isSuccess())
    return trackvector;
  else 
    return NULL;
}

//=============================================================================
// Reconstruction of VeloMuon tracks
//=============================================================================
StatusCode VeloMuonBuilder::buildVeloMuon(Tracks& veloTracks, Tracks& muonTracks, Tracks* trackvector) {
  //  Tracks* trackvector = new Tracks();
  Track* aCopy=NULL;
  Track* goodCopy=NULL;
  Tracks::const_iterator muonIter, veloIter;//, minveloIter;
  StatusCode sc = StatusCode::SUCCESS;

  for (muonIter = muonTracks.begin(); muonIter != muonTracks.end(); ++muonIter) {

    //    if (m_bMonteCarlo) {
    //      mcPartm = getMCParticlefromMuonTrack(*muonIter);
    //    }


    Gaudi::XYZPoint chamber = (*muonIter)->position();//this is at the first state
    Gaudi::XYZPoint muonpunktx, muonpunkty;
    if (m_chamberhit) {
      muonpunkty=chamber;
    } else {
      sc = m_linearextrapolator->position(*(*muonIter),m_zmatch,muonpunkty,LHCb::ParticleID(13));
    }
    if (sc.isFailure()) continue;
    sc = m_linearextrapolator->position(*(*muonIter),m_zmagnet,muonpunktx,LHCb::ParticleID(13));

    if (sc.isFailure()) continue;

    std::vector<LHCbID> muonTiles= (*muonIter)->lhcbIDs();
    MuonTileID tileM2=muonTiles[2].muonID();
    int reg = tileM2.region();

    //go through the velos
    float minweight=std::numeric_limits<float>::infinity(); 
    float mindist, minchi2;
    for (veloIter = veloTracks.begin(); veloIter != veloTracks.end() ; ++veloIter) {
      if ((*veloIter)->history() != LHCb::Track::PatVelo &&
	  (*veloIter)->history() != LHCb::Track::PatVeloGeneral) continue;

      Gaudi::XYZPoint velopunktx,velopunkty;
      sc = m_linearextrapolator->position(*(*veloIter),m_zmagnet,velopunktx,LHCb::ParticleID(13));
      if (sc.isFailure()) continue;
      if (m_chamberhit)	{
	sc = m_linearextrapolator->position(*(*veloIter),chamber.z(),velopunkty,LHCb::ParticleID(13));
      } else {
	sc = m_linearextrapolator->position(*(*veloIter),m_zmatch,velopunkty,LHCb::ParticleID(13));
      }
      if (sc.isFailure()) continue;


      if (reg == 0)
	m_xscale = 0.06;
      if (reg == 1)
	m_xscale = 0.1;
      if (reg == 2)
	m_xscale = 0.15;
      if (reg == 3)
	m_xscale = 0.15;


      // now calculate distance

      float weighteddistance = (velopunktx.x()-muonpunktx.x())*(velopunktx.x()-muonpunktx.x())*m_xscale+(1-m_xscale)*(velopunkty.y()-muonpunkty.y())*(velopunkty.y()-muonpunkty.y());

      // -- hard coded after determination on private ntuple
      if (reg == 0)
	m_distancecut = 100;
      if (reg == 1)
	m_distancecut = 200;
      if (reg == 2)
	m_distancecut = 9000;
      if (reg == 3)
	m_distancecut = 38000;
      m_distancecut*=m_distcutmultiplyer;

      if (weighteddistance>m_distancecut) continue;


      
      {

	State* monitorstate = ((*veloIter)->firstState()).clone();
	float xkick = chamber.x() - monitorstate->x();//jstefaniak used interpolated value here
	float m_ptkickConstant = 1265;
	float qp = xkick / m_ptkickConstant / (chamber.z() - m_zmagnet);
	qp = -qp * m_fieldfactor;
	aCopy = new LHCb::Track();
	aCopy->addToAncestors(*veloIter);
	aCopy->addToAncestors(*muonIter);
	std::vector< LHCb::State* > velostates = (*veloIter)->states();
	std::vector< LHCb::State* > copiedstates;
	for (std::vector<LHCb::State*>::iterator stateiter = velostates.begin(); stateiter != velostates.end(); ++stateiter) {
	  copiedstates.push_back((*stateiter)->clone());
	  copiedstates.back()->setQOverP(qp);
	  Gaudi::TrackSymMatrix cov;
	  cov(0,0) = 1;
	  cov(1,1) = 1;
	  cov(2,2) = 1;
	  cov(3,3) = 1;
	  cov(4,4) = qp*qp*0.15*0.15;
	  copiedstates.back()->setCovariance(cov);
	}
	aCopy->addToStates(copiedstates);
	aCopy->addToLhcbIDs((*veloIter)->lhcbIDs());
	std::vector< LHCb::State* > muonstates = (*muonIter)->states();
	copiedstates.clear();
	for (std::vector<LHCb::State*>::iterator stateiter = muonstates.begin(); stateiter != muonstates.end(); ++stateiter) {
	  copiedstates.push_back((*stateiter)->clone());
	  copiedstates.back()->setQOverP(qp);
	  Gaudi::TrackSymMatrix cov;
	  cov(0,0) = 1;
	  cov(1,1) = 1;
	  cov(2,2) = 1;
	  cov(3,3) = 1;
	  cov(4,4) = qp*qp*0.15*0.15;
	  copiedstates.back()->setCovariance(cov);
	}

	aCopy->addToStates(copiedstates);
	std::vector< LHCb::LHCbID > muonids = (*muonIter)->lhcbIDs();                        // pointers to the other ids
	aCopy->addToLhcbIDs(muonids);                                                        // to insert them here
	if (sc.isFailure()) {delete aCopy; aCopy=NULL; continue;}
	m_linearextrapolator->propagate(*monitorstate, chamber.z(),LHCb::ParticleID(13));
	
	aCopy->firstState().setQOverP(qp);

	aCopy->setFitStatus(LHCb::Track::FitStatusUnknown);
	aCopy->setFitHistory(LHCb::Track::FitUnknown);

	TrackFitResult * result = new TrackFitResult();
	result->setPScatter(1/qp); // -- needed in fitter

	aCopy->setFitResult(result);


	sc = m_tracksFitter->fit(*aCopy,LHCb::ParticleID(13));
	if (sc.isFailure()) continue;

	delete monitorstate;
	aCopy->clearAncestors();
	// -- unfortunately this will not be stored on DST :(
	aCopy->addToAncestors(*veloIter);
	aCopy->addToAncestors(*muonIter);
      }

      float weight = weighteddistance * (1-m_chiweight) + m_chiweight * aCopy->chi2();
      if (weight<minweight) {
	// -- if we created a Track and want to use another one. then we should delete the old one
	if (goodCopy) delete goodCopy;                        
	minweight=weight;
	mindist = weighteddistance;
	minchi2 = aCopy->chi2();
	goodCopy=aCopy;
	
      }
      if (aCopy && aCopy != goodCopy) delete aCopy;          // clean up      
    } 
    if (minweight==std::numeric_limits<float>::infinity()) {
      continue; // -- nothing was found
    }

    goodCopy->setType(LHCb::Track::Long);
    trackvector->add(goodCopy);
    goodCopy=NULL; // clean up
  }
  return StatusCode::SUCCESS;
}



