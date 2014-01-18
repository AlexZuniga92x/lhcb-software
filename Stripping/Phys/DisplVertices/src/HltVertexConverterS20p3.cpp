// include files
#include <boost/foreach.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>

#include "GaudiKernel/AlgFactory.h"

#include "Event/HltSelReports.h"
#include "Event/Particle.h"

// local
#include "HltVertexConverterS20p3.h"

//-----------------------------------------------------------------------------
// Implementation file for class : HltVertexConverter, Stripping20p3 version
//
// 2014-01-10 : Pieter David
//-----------------------------------------------------------------------------

// Declaration of the Algorithm Factory
DECLARE_ALGORITHM_FACTORY( HltVertexConverterS20p3 )

//==============================================================================
// Constructor
//==============================================================================
HltVertexConverterS20p3::HltVertexConverterS20p3(const std::string& name, ISvcLocator* pSvcLocator)
 : DaVinciAlgorithm(name, pSvcLocator)
{
  declareProperty( "HltSelReports"
                 , m_HltSelReportsLocation = LHCb::HltSelReportsLocation::Default
                 , "Location of the HltObjectSummary in TES" );

  declareProperty( "HltLines"
                 , m_HltLineNames
                 , "Hlt lines whose candidates to use" );

  declareProperty( "Recursive"
                 , m_recursive = true
                 , "Revive the complete decay tree" );

  declareProperty( "MatchTracksToOffline"
                 , m_matchTracksToOffline = false
                 , "Match tracks to offline tracks (in Rec/Track/Best) and refit vertices");

  m_defaultVertexCovMatrix(0,0) =  5.0*Gaudi::Units::micrometer;
  m_defaultVertexCovMatrix(1,1) =  5.0*Gaudi::Units::micrometer;
  m_defaultVertexCovMatrix(2,2) = 50.0*Gaudi::Units::micrometer;
}
//==============================================================================
// Destructor
//==============================================================================
HltVertexConverterS20p3::~HltVertexConverterS20p3()
{
}

//==============================================================================
// Initialize
//==============================================================================
StatusCode HltVertexConverterS20p3::initialize()
{
  if (msgLevel(MSG::DEBUG)) { debug() << "==> Initialize" << endmsg; }

  StatusCode sc = DaVinciAlgorithm::initialize();
  if (sc.isFailure()) { return sc; }

  //

  return StatusCode::SUCCESS;
}

// output containers on-demand
LHCb::ProtoParticles* HltVertexConverterS20p3::protoParticles()
{
  if ( m_protos == 0 ) {
    m_protos = new LHCb::ProtoParticles();
    put( m_protos, outputLocation()+"/ProtoParticles" );
  }
  return m_protos;
}

LHCb::Tracks* HltVertexConverterS20p3::tracks()
{
  if ( m_tracks == 0 ) {
    m_tracks = new LHCb::Tracks();
    put( m_tracks, outputLocation()+"/Tracks" );
  }
  return m_tracks;
}

namespace {
  // helper in the style of python dict.get
  template<typename TYPE>
  inline TYPE getInfo( const LHCb::HltObjectSummary::Info& infos , const LHCb::HltObjectSummary::Info::key_type& key, const TYPE& defaultVal )
  {
    LHCb::HltObjectSummary::Info::iterator it = infos.find(key);
    return ( it != infos.end() ) ? TYPE (it->second) : defaultVal;
  }
}


LHCb::Particle* HltVertexConverterS20p3::reviveParticle(const LHCb::HltObjectSummary* summary, bool recursive)
{
  LHCb::HltObjectSummary::Info numericalInfo(summary->numericalInfoFlattened());
  std::auto_ptr<LHCb::Particle> part(new LHCb::Particle());
  LHCb::Particle* ret = 0;
  part->setParticleID(LHCb::ParticleID(getInfo<int>(numericalInfo, "0#Particle.particleID.pid", 0 )));
  part->setReferencePoint(Gaudi::XYZPoint(
                        getInfo<double>(numericalInfo, "3#Particle.referencePoint.x", 0.),
                        getInfo<double>(numericalInfo, "4#Particle.referencePoint.y", 0.),
                        getInfo<double>(numericalInfo, "2#Particle.referencePoint.z", 0.)
                          ));
  double m   (getInfo<double>(numericalInfo, "1#Particle.measuredMass", 0.));
  double tx  (getInfo<double>(numericalInfo, "5#Particle.slopes.x"    , 0.));
  double ty  (getInfo<double>(numericalInfo, "6#Particle.slopes.y"    , 0.));
  double invp(getInfo<double>(numericalInfo, "7#Particle.1/p"         , 0.));
  // px**2+py**2+pz**2=p**2 => pz**2*(tx**2+ty**2+1)=p**2 => pz=p/sqrt(1+tx**2+ty**2)
  double pz = 1./(invp*std::sqrt(1+tx*tx+ty*ty));
  // m**2 = E**2-p**2 => E=sqrt(m**2+1./(ip*ip))
  part->setMeasuredMass(m);
  part->setMomentum(Gaudi::LorentzVector(tx*pz,ty*pz,pz,std::sqrt(m*m+1./(invp*invp))));
  if ( msgLevel(MSG::VERBOSE) ) { verbose() << "Found particle " << *part << "\nLooking for substructure" << endmsg; }

  if ( summary->substructure().size() == 1 ) {
    SmartRef<LHCb::HltObjectSummary> subSum = summary->substructure()[0];
    if ( subSum->summarizedObjectCLID() == LHCb::Track::classID() ) {
      if (msgLevel(MSG::VERBOSE)) { verbose() << "-> Track" << endmsg; }
      LHCb::ProtoParticle* proto = new LHCb::ProtoParticle();
      protoParticles()->insert(proto);
      part->setProto(proto);
      proto->setTrack( reviveTrack(subSum) );
    } else if ( subSum->summarizedObjectCLID() == LHCb::CaloCluster::classID() ) {
      Warning("Neutral particle - not handled yet");
    } else {
      Warning("Classification failed: 1 daughter but not a track and not a Calo neutral");
    }
    ret = part.get();
    markParticle(part.release());
  } else if ( summary->substructure().size() > 1 ) { // not a basic particle: add endvertex
    std::auto_ptr<LHCb::Vertex> vert(new LHCb::Vertex());
    vert->setPosition(part->referencePoint());
    vert->setCovMatrix(m_defaultVertexCovMatrix);
    part->setEndVertex(vert.get());
    if ( recursive ) {
      BOOST_FOREACH( const SmartRef<LHCb::HltObjectSummary>& daugSum, summary->substructure() ) {
        if ( daugSum->summarizedObjectCLID() == LHCb::Particle::classID() ) {
          if (msgLevel(MSG::VERBOSE)) { verbose() << "-> Daughter particle" << endmsg; }
          const LHCb::Particle* daug = reviveParticle(daugSum, recursive);
          part->addToDaughters(daug);
          vert->addToOutgoingParticles(daug);
        } else {
          Warning("Classification failed: >1 substructure entries, but not particles");
        }
      }
      if ( m_matchTracksToOffline ) { vertexFitter("LoKi")->fit(*vert.get(), part->daughtersVector()); }
    }
    vert.release();
    ret = part.get();
    markNewTree(part.release()); // register particle as well as endvertex 
  } else {
    Warning("Classification failed: particle without substructure");
  }

  return ret;
}

LHCb::Track* HltVertexConverterS20p3::reviveTrack(const LHCb::HltObjectSummary* summary)
{
  LHCb::HltObjectSummary::Info trackInfo(summary->numericalInfo());
  LHCb::Track* track = 0;
  std::vector<LHCb::LHCbID> hits(summary->lhcbIDsFlattened());
  if ( m_matchTracksToOffline ) {
    // get matching track from Rec/Track/Best

    // LHCb::Track::Range bestTracks = get<LHCb::Track::Ragne>(LHCb::TrackLocation::Default);
    Error("Not implemented yet");
  } else {
    // make a simple unfitted track from the info in the summary

    track = new LHCb::Track();
    tracks()->insert(track);
    // track fit info
    double chi2PerDoF = getInfo<double>(trackInfo, "6#Track.chi2PerDoF", 0.);
    int nDoF = getInfo<int>(trackInfo, "7#Track.nDoF", 1);
    track->setChi2AndDoF(chi2PerDoF*nDoF, nDoF);
    // a state
    LHCb::State firstState;
    firstState.setState( getInfo<double>(trackInfo, "1#Track.firstState.x", 0.)
                       , getInfo<double>(trackInfo, "2#Track.firstState.y", 0.)
                       , getInfo<double>(trackInfo, "0#Track.firstState.z", 0.)
                       , getInfo<double>(trackInfo, "3#Track.firstState.tx", 0.)
                       , getInfo<double>(trackInfo, "4#Track.firstState.ty", 0.)
                       , getInfo<double>(trackInfo, "5#Track.firstState.qOverP", 0.)
                       );
    track->addToStates(firstState);
    // LHCbIDs
    track->setLhcbIDs(hits);
    bool hasVelo = std::find_if( hits.begin(), hits.end(), boost::lambda::bind(&LHCb::LHCbID::isVelo, boost::lambda::_1) ) != hits.end();
    bool hasT    = std::find_if( hits.begin(), hits.end(), boost::lambda::bind(&LHCb::LHCbID::isOT, boost::lambda::_1) || boost::lambda::bind(&LHCb::LHCbID::isIT, boost::lambda::_1) ) != hits.end();
    bool hasTT   = std::find_if( hits.begin(), hits.end(), boost::lambda::bind(&LHCb::LHCbID::isTT, boost::lambda::_1) ) != hits.end();
    if ( hasVelo ) {
      if ( hasT ) {
        track->setType(LHCb::Track::Long);
      } else {
        if ( hasTT ) {
          track->setType(LHCb::Track::Upstream);
        } else {
          track->setType(LHCb::Track::Velo);
        }
      }
    } else if ( hasT ) {
      if ( hasTT ) {
        track->setType(LHCb::Track::Downstream);
      } else {
        track->setType(LHCb::Track::Ttrack);
      }
    } else {
      Warning("Track without Velo and T hits");
    }
    if ( msgLevel(MSG::VERBOSE) ) { verbose() << "Created track " << *track << endmsg; }
  }
  return track;
}

//==============================================================================
// Execute
//==============================================================================
StatusCode HltVertexConverterS20p3::execute()
{
  if (msgLevel(MSG::DEBUG)) { debug() << "==> Execute" << endmsg; }
  setFilterPassed(false);

  m_protos = 0; m_tracks = 0; // reset output container pointers to make sure they are created on-demand

  const LHCb::HltSelReports* selReports = get<LHCb::HltSelReports>(m_HltSelReportsLocation);
  if ( ! selReports ) {
    return Warning("No Hlt SelReports found at "+m_HltSelReportsLocation, StatusCode::FAILURE);
  }
  std::size_t nAcc(0);
  BOOST_FOREACH( const std::string& selection, m_HltLineNames ) {
    const LHCb::HltObjectSummary* objectSummary = selReports->selReport(selection);
    if ( objectSummary != 0 ) {
      BOOST_FOREACH( const LHCb::HltObjectSummary* item, objectSummary->substructureFlattened() ) {
        if ( item->summarizedObjectCLID() == LHCb::Particle::classID() ) {
          LHCb::Particle* part = reviveParticle(item, m_recursive);
          if ( part != 0 ) {
            ++counter("# candidates loaded from "+selection);
            ++nAcc;
          }
        }
      }
    }
  }
  if ( nAcc > 0 ) {
    setFilterPassed(true);
    ++counter("# candidates accepted");
  }

  return StatusCode::SUCCESS;
}

//==============================================================================
// Finalize
//==============================================================================
StatusCode HltVertexConverterS20p3::finalize()
{
  if (msgLevel(MSG::DEBUG)) { debug() << "==> Finalize" << endmsg; }

  //

  return DaVinciAlgorithm::finalize();
}
