
// Include files
#include <cmath>

// from Gaudi
#include "GaudiKernel/ToolFactory.h"
#include "GaudiKernel/IRegistry.h"
#include "GaudiKernel/SystemOfUnits.h"

#include "TfKernel/HitExtension.h"
#include "TfKernel/RecoFuncs.h"
#include "TfKernel/RegionID.h"

#include "OTDet/DeOTDetector.h"

// local
#include "PatForwardTool.h"
#include "PatFwdPlaneCounter.h"
#include "PatFwdRegionCounter.h"

//-----------------------------------------------------------------------------
// Implementation file for class : PatForwardTool
//
// 2005-10-04 : Olivier Callot
//-----------------------------------------------------------------------------

static const unsigned int nSta = Tf::RegionID::OTIndex::kNStations;
static const unsigned int nLay = Tf::RegionID::OTIndex::kNLayers;
static const unsigned int nReg = Tf::RegionID::OTIndex::kNRegions + Tf::RegionID::ITIndex::kNRegions;
static const unsigned int nOTReg = Tf::RegionID::OTIndex::kNRegions;
static const unsigned int nITReg = Tf::RegionID::ITIndex::kNRegions;


DECLARE_TOOL_FACTORY( PatForwardTool )

constexpr struct sortQuality {
  bool operator()( const PatFwdTrackCandidate& first, 
                   const PatFwdTrackCandidate& second )
  {
    bool sortDecision = ( first.quality() < second.quality());
    if(first.quality() == second.quality()){
      sortDecision = ( first.chi2PerDoF() < second.chi2PerDoF());
      if(first.chi2PerDoF() == second.chi2PerDoF()){
        sortDecision = ( first.qOverP() < second.qOverP());
      }
    }
    return sortDecision;
  }
} sortQuality_{};

struct MaxSpread {
    double spread;
    MaxSpread(double spread_) : spread { spread_ } {}
    bool operator()(const PatForwardHit* first, const PatForwardHit *last) const {
            return last->projection() < first->projection() + spread;
    }
};


 
//== Count how many weighted hits

template <typename C>
int nbT(const C& c) { return 2*c.nbIT()+c.nbOT(); }

int nT(const PatFwdTrackCandidate& c) {
    return nbT( PatFwdRegionCounter{  std::begin(c), std::end(c) } );
}

template <typename Iterator>
int nbDifferent( Iterator first, Iterator last)  {
    return PatFwdPlaneCounter{ first, last }.nbDifferent();
}
int nbDifferent( const PatFwdTrackCandidate &c) {
    return nbDifferent( std::begin(c), std::end(c) );
}


class PlaneCounter {
    PatFwdPlaneCounter m_counter;
    int m_minPlanes;
public:
    PlaneCounter( PatFwdPlaneCounter counter, int minPlane ) : m_counter{ counter }, m_minPlanes(minPlane) { }
    template <typename Iterator>
    PlaneCounter( Iterator&& first, Iterator&& last, int minPlane ) 
         : m_counter{ std::forward<Iterator>(first) , std::forward<Iterator>(last) }
         , m_minPlanes{minPlane} { }

    bool operator()() const { return m_counter.nbDifferent() >= m_minPlanes; }

    //== Try to make a single zone, by removing the first and adding other as
    //   long as the spread and minPlanes conditions are met.
    template <typename Iterator, typename Predicate>
    Iterator make_single_zone(Iterator first, Iterator mid, Iterator last,
                              Predicate predicate) 
    {
        auto i = mid;
        while ( first + (m_minPlanes-1) != mid  && i != last ) {
          m_counter.removeHit(*first);
          ++first;
          for (; i!=last && predicate(*first,*i) ; ++i ) m_counter.addHit(*i);
          if ( (*this)() ) mid = i;
        }
        return mid;
    }
};

template <typename  Iterator,typename Predicate> 
std::pair<Iterator,Iterator> make_single_zone(Iterator first, Iterator mid, Iterator last, int minPlanes, Predicate predicate) 
{
    PlaneCounter planeCount{ first, mid,  minPlanes };
    if ( !planeCount() ) { //== Enough different planes?
      ++first;
      return { first, first };
    }
    return { first, planeCount.make_single_zone( first, mid, last, predicate ) };
}

template <typename  Iterator,typename Predicate> 
std::pair<Iterator,Iterator> find_range(Iterator first, Iterator mid, Iterator last, Predicate predicate) 
{
    using reference = typename std::iterator_traits<Iterator>::reference;
    //== If not enough hits in the maximum spread, skip
    if ( !predicate(*first,*mid)  ) {
      // first increment first, as we KNOW the current first fails predicate..
      first = std::find_if(++first, mid, [&](reference i) { return predicate(i,*mid);});
      return { first, first };
    }
    //== Add all hits inside the maximum spread. 
    // first increment mid, as we KNOW the current mid passes predicate..
    return { first, std::find_if( ++mid, last, [&](reference i) { return !predicate(*first,i); } ) };
}



//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
PatForwardTool::PatForwardTool( const std::string& type,
                                const std::string& name,
                                const IInterface* parent )
    : base_class( type, name , parent ),
      m_nnSwitch(false)
{
  declareInterface<IPatForwardTool>(this);
  declareInterface<ITracksFromTrack>(this);

  declareProperty( "SecondLoop"            , m_secondLoop            = true );
  declareProperty( "ZAfterVelo"            , m_zAfterVelo            = 1640. * Gaudi::Units::mm );
  declareProperty( "YCompatibleTol"        , m_yCompatibleTol        =   10. * Gaudi::Units::mm );
  declareProperty( "YCompatibleTolFinal"   , m_yCompatibleTolFinal   =    1. * Gaudi::Units::mm );
  declareProperty( "MinXPlanes"            , m_minXPlanes            =   5   );
  declareProperty( "MinPlanes"             , m_minPlanes             =   9   );
  declareProperty( "MinOTDrift"            , m_minOTDrift            = -0.3 * Gaudi::Units::mm  );
  declareProperty( "MaxOTDrift"            , m_maxOTDrift            =  2.5 * Gaudi::Units::mm  );
  declareProperty( "MaxSpreadX"            , m_maxSpreadX            =  0.6  );
  declareProperty( "MaxSpreadSlopeX"       , m_maxSpreadSlopeX       = .011  );
  declareProperty( "MaxSpreadY"            , m_maxSpreadY            =  1.5  );
  declareProperty( "MaxSpreadSlopeY"       , m_maxSpreadSlopeY       = 70.   );
  declareProperty( "MinPt"                 , m_minPt                 = 80. * Gaudi::Units::MeV );
  declareProperty( "MinMomentum"           , m_minMomentum           =  1. * Gaudi::Units::GeV );
  declareProperty( "MaxChi2"               , m_maxChi2               =  40       );
  declareProperty( "MaxChi2Track"          , m_maxChi2Track          =  40       );
  declareProperty( "MinHits"               , m_minHits               =  12       );
  declareProperty( "MinOTHits"             , m_minOTHits             =  14       );
  declareProperty( "CenterOTYSize"         , m_centerOTYSize         =  100 * Gaudi::Units::mm );
  declareProperty( "MaxDeltaY"             , m_maxDeltaY             =  30.      );
  declareProperty( "MaxDeltaYSlope"        , m_maxDeltaYSlope        =  300.      );
  declareProperty( "MaxXCandidateSize"     , m_maxXCandidateSize     =  50       );

  declareProperty( "MagnetKickParams"      , m_magnetKickParams      =  { 1255 * Gaudi::Units::MeV, 175 * Gaudi::Units::MeV } );

  declareProperty( "UseMomentumEstimate", m_useMomentumEstimate = false );
  declareProperty( "MomentumEstimateError"    , m_momentumEstimateError    =  0.5 );
  declareProperty( "MinRange"              , m_minRange              =   300. * Gaudi::Units::mm  );

  declareProperty("StateErrorX2",  m_stateErrorX2  =   4.0);
  declareProperty("StateErrorY2",  m_stateErrorY2  = 400.);
  declareProperty("StateErrorTX2", m_stateErrorTX2 =  6.e-5);
  declareProperty("StateErrorTY2", m_stateErrorTY2 =  1.e-4);
  declareProperty("StateErrorP",   m_stateErrorP   =  0.15);

  declareProperty("AddTTClusterName", m_addTtToolName = "" );

  declareProperty( "WithoutBField"         , m_withoutBField         = false);

  declareProperty("Preselection" , m_Preselection = false);
  declareProperty("PreselectionPT",m_PreselectionPT = 400.*Gaudi::Units::MeV);
  declareProperty("UseWrongSignWindow",m_UseWrongSignWindow = false);
  declareProperty("WrongSignPT",m_WrongSignPT = 2000.*Gaudi::Units::MeV);
  declareProperty("FlagUsedSeeds",m_FlagUsedSeeds = false);

}

//=========================================================================
//  Initialization, get the tools
//=========================================================================
StatusCode PatForwardTool::initialize ( ) {
  StatusCode sc = GaudiTool::initialize(); // must be executed first
  if ( sc.isFailure() ) return sc;  // error printed already by GaudiAlgorithm

  if( UNLIKELY( msgLevel(MSG::DEBUG) ) ) debug() << "==> Initialize" << endmsg;

  m_tHitManager  = tool<Tf::TStationHitManager<PatForwardHit> >("PatTStationHitManager");
  m_fwdTool      = tool<PatFwdTool>( "PatFwdTool", this);

  if ( "" != m_addTtToolName  ) {
    m_addTTClusterTool = tool<IAddTTClusterTool>( m_addTtToolName, this );
  } else {
    m_addTTClusterTool = nullptr;
  }

  return StatusCode::SUCCESS;
}

//=========================================================================
//  Main execution: Extend a track.
//=========================================================================
void PatForwardTool::forwardTrack( const LHCb::Track* tr, LHCb::Tracks* output ) {

  std::vector<LHCb::Track*> outvec;
  tracksFromTrack(*tr,outvec).ignore();

  if (outvec.empty() && m_secondLoop) {
      
      int minXPlanes = m_minXPlanes;
      m_minXPlanes = minXPlanes - 1;

      int minPlanes = m_minPlanes;
      m_minPlanes =  minPlanes - 1;
      
      tracksFromTrack(*tr,outvec).ignore();

      m_minXPlanes = minXPlanes;
      m_minPlanes = minPlanes;
      
  }
  output->reserve(output->size()+outvec.size());
  for ( const auto& i : outvec ) output->insert(i);
}

StatusCode PatForwardTool::tracksFromTrack( const LHCb::Track& seed,
                                            std::vector<LHCb::Track*>& output ){
  bool isDebug = msgLevel( MSG::DEBUG );

  if ( seed.checkFlag( LHCb::Track::Invalid  ) ) return StatusCode::SUCCESS;
  if ( seed.checkFlag( LHCb::Track::Backward ) ) return StatusCode::SUCCESS;
  if ( seed.checkFlag( LHCb::Track::Used ) ) return StatusCode::SUCCESS; // indicates this has already successfully been upgraded

  PatFwdTrackCandidate track( &seed );

  if(m_Preselection && seed.pt() < m_PreselectionPT && 0 != track.qOverP()) return StatusCode::SUCCESS;

  if ( isDebug ) {
    debug() << format( "**** Velo track %3d  x%8.2f  y%8.2f  tx%9.5f  ty%9.5f q/p = %8.6f",
                       seed.key(), track.xStraight( m_zAfterVelo ),
                       track.yStraight( m_zAfterVelo ),
                       track.slX(), track.slY(), 1000. * track.qOverP() ) << endmsg;
  }

  //== Build the initial list of X candidates
  buildXCandidatesList( track );

  int minPlanes = m_minPlanes;  //== Initial value, can be updated later...

  std::vector<PatFwdTrackCandidate> xCandidates;

  int minOTX = int( 1.5 * m_minXPlanes );

  for( auto& icand : m_candidates ) {

    //== Fit the candidate, and store them
    while ( m_fwdTool->fitXCandidate( icand, m_maxChi2, m_minXPlanes ) ) {

      // Copy the whole stuff, keep only used ones and store it.
      PatFwdTrackCandidate temp = icand;
      temp.cleanCoords();
      m_fwdTool->chi2PerDoF( temp );  // Compute and store chi2/dof
      if ( isDebug ) {
        debug() << "Chi2/nDof = " << temp.chi2PerDoF() << " nDoF " << temp.nDoF()
                << " dist at center " << m_fwdTool->distAtMagnetCenter( temp )
                << endmsg;
        debugFwdHits( temp );
      }

      //== Check the chi2 with magnet center constraint.
      if ( m_maxChi2Track > temp.chi2PerDoF()  &&
           (!m_withoutBField || fabs(temp.slX()-temp.xSlope(0))<0.005)){
        //== Count how many weighted hits
        int nbHit = nT( temp ) ; 

        if ( minOTX <= nbHit || inCenter(temp) ) {
          
          //== reject if below threshold
          if (passMomentum( temp, track.sinTrack() )) xCandidates.push_back( temp );

          if( UNLIKELY( msgLevel(MSG::DEBUG) ) ) 
            debug() << "+++ Store candidate " << xCandidates.size()-1 << endmsg;
        } else {
          if( UNLIKELY( msgLevel(MSG::DEBUG) ) ) 
            debug() << " --- not enough hits " << nbHit << endmsg;
        }
      }

      //== tag these coordinates in the original, so that we don't find the same track again
      for ( auto* hit :  icand ) {
        if ( hit->isSelected() ) hit->setIgnored( true );
      }
    }
  }

  if ( isDebug ) {
    debug() << "************ List of X candidates , N = " << xCandidates.size() << endmsg;
    unsigned nCand=0;
    for ( const auto& itL : xCandidates ) {
      debug() << "Candidate " << nCand++
              << " Chi2/nDof = " << itL.chi2PerDoF() << endmsg;
      debugFwdHits( itL );
    }
    if ( !xCandidates.empty() ) debug() << "---- Now get the stereo hits on these ---" << endmsg;
  }

  //== Now try to get space track from these X track.
  std::vector<PatFwdTrackCandidate> goodCandidates;
  int maxPlanes = 0;

  unsigned nCand = 0;
  for ( PatFwdTrackCandidate& temp :  xCandidates ) { //TODO: make this one big 'remove_if'?
    if( UNLIKELY( msgLevel(MSG::DEBUG) ) )
      debug() << "--- Candidate " << ++nCand
              << "  X cord size " << temp.coords().size() 
              << endmsg;

    for ( auto *hit : temp.coords() ) hit->setIgnored( false);  // restore normal flag.
    temp.setSelectedCoords( );

    double qOverP = computeQOverP(temp);  // in 1/GeV
    if ( !fillStereoList( temp, computeStereoTol(qOverP) ) ) continue; // Get the stereo coordinates

    if ( isDebug )  debugFwdHits( temp );
    temp.setSelectedCoords( );

    //== Fit and reject if not good enough
    if ( !m_fwdTool->fitStereoCandidate( temp, m_maxChi2, minPlanes ) ) continue;

    temp.cleanCoords();
    m_fwdTool->chi2PerDoF( temp );  //== compute and store Chi2PerDoF
    if ( isDebug ) {
      debug() << "  ... fit OK  chi2 " << temp.chi2PerDoF() << " nDoF " << temp.nDoF()
              << " dist at center " << m_fwdTool->distAtMagnetCenter( temp )
              << endmsg;
      debugFwdHits( temp );
    }

    //== Remove stereo coordinates incompatible in Y
    if ( !m_fwdTool->removeYIncompatible( temp, m_yCompatibleTolFinal, minPlanes ) ) continue;
    temp.cleanCoords();
    if( UNLIKELY( msgLevel(MSG::DEBUG) ) ) 
      debug() << "  ... Y is compatible" << endmsg;

     // Enough stereo planes
    PatFwdPlaneCounter fullCount( std::begin(temp), std::end(temp) );
    int nbY = fullCount.nbStereo();
    if ( nbY < 4 ) {
      if( UNLIKELY( msgLevel(MSG::DEBUG) ) ) 
        debug() << "Not enough Y planes : " << nbY << endmsg;
      continue;
    }

    if ( m_maxDeltaY + m_maxDeltaYSlope * qOverP *qOverP < fabs(  m_fwdTool->changeInY( temp ) ))  {
      if( UNLIKELY( msgLevel(MSG::DEBUG) ) ) 
        debug() << "  --- Too big change in Y : " <<  m_fwdTool->changeInY( temp ) << endmsg;
      continue;
    }

    temp.setQuality( computeQuality(temp, qOverP) );

    //== Verify if enough OT measurements, counting IT for 2/plane
    //== Ignore the y central region, OT inefficient there.

    PatFwdRegionCounter regions( std::begin(temp), std::end(temp) );
    temp.setNbIT( regions.nbIT() );
    temp.setNbOT( regions.nbOT() );

    if ( !inCenter(temp) ) {
      if ( m_minHits > nbT(temp) ){
        if( UNLIKELY( msgLevel(MSG::DEBUG) ) ) 
          debug() << "  --- Not enough hits : " << nbT(temp) << endmsg;
        continue;
      }
      if ( temp.nbIT() == 0 && temp.nbOT() < m_minOTHits ) {
        if( UNLIKELY( msgLevel(MSG::DEBUG) ) ) 
          debug() << " Too few OT for OT only track : " << temp.nbOT() << endmsg;
        continue;
      }
    }
    maxPlanes = std::max(maxPlanes, fullCount.nbDifferent());

    //== reject if below threshold
    if (!passMomentum( temp, track.sinTrack())) continue;

    //== Update requirement according to already found good solutions...
    minPlanes = std::max( minPlanes,   nbDifferent(temp)-1 );
    goodCandidates.push_back( std::move(temp) );
  }


  // added for Tr/NNTools -- sort all candidates with respect to PatQuality
  if( this->nnSwitch()){
    std::stable_sort( std::begin(goodCandidates), std::end(goodCandidates), sortQuality_);
    // loop over all candidates
    for( auto& all : goodCandidates ) {
	  all.setCand1stQuality(all.quality());
      if(goodCandidates.size() == 1) {
	    all.setCand2ndQuality(0.);
      } else {
        auto i2 = std::find_if( std::begin(goodCandidates), std::end(goodCandidates),
                                [&](const PatFwdTrackCandidate& c) { return c.quality() != all.quality(); } );
        if (i2!=std::end(goodCandidates)) { all.setCand2ndQuality( i2->quality() ); }
      }
    }
  }
  // end of NNTools loop
  //================================================================================
  //  Now some filtering of tracks, in case of multiple candidates.
  //================================================================================

  if ( goodCandidates.size() > 1  ) {

    // remove track with sensibly lower number of planes
    int minPlanes = maxPlanes - 1;
    auto last = std::remove_if( std::begin(goodCandidates), std::end(goodCandidates), 
                                [=](const PatFwdTrackCandidate& t) { return nbDifferent(t) < minPlanes; } );
    // remove worst quality  
    // TODO: could fold this accumulate into the lambda above by capturing bestQuality by reference
    auto bestQuality = std::accumulate( std::begin(goodCandidates), last, 1000.,
                                  [](double m, const PatFwdTrackCandidate& t) { return std::min(m,t.quality()); } );
    bestQuality += 1.0;
    last = std::remove_if( std::begin(goodCandidates), last,
                           [=](const PatFwdTrackCandidate& t) { return t.quality()  >= bestQuality; }  );
    // remove if sensibly less OT 
    // TODO: could fold this accumulate into the lambda above by capturing maxOT by reference
    auto maxOT = std::accumulate( std::begin(goodCandidates), last, 0,
                                  [](int m, const PatFwdTrackCandidate& t) { return std::max(m,nbT(t)); } );
    maxOT = std::min(24,maxOT)-3;
    last = std::remove_if( std::begin(goodCandidates), last,
                           [=](const PatFwdTrackCandidate& t) { return nbT(t)  <= maxOT; }  );
    if( UNLIKELY( msgLevel(MSG::DEBUG) ) ) {
        debug() << "Ignoring " << std::distance(last,std::end(goodCandidates)) 
                << " candidates -- either not enough planes, or bad quality, or not enough OT" << endmsg;
    }
    goodCandidates.erase(last,std::end(goodCandidates));
  }

  if( UNLIKELY( msgLevel(MSG::DEBUG) ) ) 
    debug() << "Storing " << goodCandidates.size() << " good tracks " << endmsg;
  //=== Store tracks...
  for (const auto&  cand : goodCandidates ) {
    output.push_back( seed.clone() );
    LHCb::Track* fwTra = output.back();
    //fwTra->clearAncestors();
    fwTra->addToAncestors( seed );  // Set the Velo track as only ancestor of the Forward track
    fwTra->setType( LHCb::Track::Long );
    fwTra->setHistory( LHCb::Track::PatForward );

    //== Add a new state in the T stations ...
    
    double qOverP = m_fwdTool->qOverP( cand );
    // set q/p in all of the existing states
    std::for_each( std::begin( fwTra->states() ), std::end( fwTra->states() ),
                   [qOverP]( LHCb::State* state ) {
      state->setQOverP(qOverP);
      state->setErrQOverP2(qOverP*qOverP*0.012*0.012);
    } );
    
    Gaudi::TrackSymMatrix cov;
    cov(0,0) = m_stateErrorX2;
    cov(1,1) = m_stateErrorY2;
    cov(2,2) = m_stateErrorTX2;
    cov(3,3) = m_stateErrorTY2;
    double errQOverP = m_stateErrorP*qOverP;
    cov(4,4) = errQOverP * errQOverP;


    for ( const auto& z : m_fwdTool->zOutputs() ) {
      double dz = z - m_fwdTool->zReference();
      LHCb::State temp;
      temp.setLocation( LHCb::State::AtT );
      temp.setState( cand.x( dz ), cand.y( dz ), z,
		             cand.xSlope( dz ), cand.ySlope( dz ), qOverP );
      //== overestimated covariance matrix, as input to the Kalman fit
      temp.setCovariance( cov );
      fwTra->addToStates( std::move(temp) );
    }
    //== New information, on the track fit.
    fwTra->setChi2PerDoF( cand.chi2PerDoF() );
    fwTra->setNDoF(       cand.nDoF() );
    fwTra->addInfo(LHCb::Track::PatQuality, cand.quality());
    // added for NNTools
    fwTra->addInfo(LHCb::Track::Cand1stQPat, cand.cand1stquality());
    fwTra->addInfo(LHCb::Track::Cand2ndQPat, cand.cand2ndquality());

    //== Add reference to the used clusters/, T stations
    std::for_each( std::begin(cand), std::end(cand), 
                   [&fwTra](PatForwardHit *myHit) {
           fwTra->addToLhcbIDs( myHit->hit()->lhcbID() );
           myHit->hit()->setStatus(Tf::HitBase::UsedByPatForward);
           myHit->setIsUsed(true);
    } );
    fwTra -> setPatRecStatus( LHCb::Track::PatRecIDs );

    if ( m_addTTClusterTool ) {
      StatusCode sc = m_addTTClusterTool->addTTClusters( *fwTra );
      if (sc.isFailure())
        if( UNLIKELY( msgLevel(MSG::DEBUG) ) ) 
          debug()<<" Failure in adding TT clusters to track"<<endmsg;
    }
  }
  // mark the input tracks and all ancestors as used
  if(m_FlagUsedSeeds && output.size()>0){
    const_cast<LHCb::Track&>(seed).setFlag(LHCb::Track::Used,true);
    std::for_each( std::begin(seed.ancestors()), std::end(seed.ancestors()),
                   [](const LHCb::Track* anc) { const_cast<LHCb::Track*>(anc)->setFlag(LHCb::Track::Used,true); } );
  }

  if( UNLIKELY( msgLevel(MSG::DEBUG) ) ) debug() << "Finished track" << endmsg;
  return StatusCode::SUCCESS;
}
//=========================================================================
//  Fill the vector of hit pointer, sorted by projection.
//=========================================================================

void PatForwardTool::fillXList ( PatFwdTrackCandidate& track, 
                                 double kickRange, double maxRangeRef, double zMagnet ,float dir) {
  double xExtrapRef = track.xStraight( m_fwdTool->zReference() );
  
  double xMin = 0;
  double xMax = 0;
  
  if(m_useMomentumEstimate && 0 != track.qOverP() && !m_withoutBField){
    if(dir > 0){
      xMin = xExtrapRef - kickRange;
      xMax = xExtrapRef + maxRangeRef;
    } else{
      xMin = xExtrapRef - maxRangeRef;
      xMax = xExtrapRef + kickRange;
    }
  } else{
    xMin = xExtrapRef - maxRangeRef;
    xMax = xExtrapRef + maxRangeRef;
  }
  
  if( UNLIKELY( msgLevel(MSG::DEBUG) ) ){ 
    debug()<< "xMax: " << xMax <<endmsg;
    debug()<< "xMin: " << xMin <<endmsg;
  }
  
  for (unsigned int sta = 0; sta < nSta; sta ++){
    for (unsigned int lay = 0; lay< nLay; lay++){
      if (lay == 1 || lay == 2) continue;  // X layers
      for (unsigned int region = 0; region <nReg; region ++){
        const Tf::EnvelopeBase* regionB = m_tHitManager->region(sta,lay,region);

        double yCompat = m_yCompatibleTol + 50 * fabs(track.slY());
        double yRegion = track.yStraight( regionB->z() );
        
        double xExtrapStation  = track.xStraight( regionB->z() );
        const double deltaX = maxRangeRef * ( regionB->z() - zMagnet ) / ( m_fwdTool->zReference() - zMagnet );
        double xHitMin = xExtrapStation - deltaX;
        xHitMin        = xHitMin - fabs( yRegion * regionB->sinT() ) - 20.;
        double ty = track.slY();
        double y0 = track.yStraight( 0. );

        if ( yRegion+yCompat < regionB->ymin() ||
             yRegion-yCompat > regionB->ymax() ) {
          continue;
        }

        for ( PatFwdHit *hit : m_tHitManager->hitsWithMinX(xHitMin, sta, lay, region)) {
          if (hit->hit()->ignore()) continue;

          updateHitForTrack( hit, y0, ty);

          if ( !hit->hit()->isYCompatible( yRegion, yCompat ) ) continue;

          hit->setIgnored( false );
          hit->setRlAmb( 0 );
          
          if (!driftOK( *hit ) ) {
              hit->setSelected( false );
              continue;
          }
          hit->setSelected( true );

          double xRef = m_fwdTool->xAtReferencePlane( track, hit );
          hit->setProjection( xRef );

          if ( xMax < xRef ) break;
          if ( xMin <= xRef ) m_xHitsAtReference.push_back( hit );
        }
      }
    }
  }
  std::sort( std::begin(m_xHitsAtReference),
             std::end(m_xHitsAtReference),
             Tf::increasingByProjection<PatForwardHit>() );
}

//=========================================================================
//  Fill the vector of hit pointer, sorted by projection.
//=========================================================================
bool PatForwardTool::fillStereoList ( PatFwdTrackCandidate& track, double tol ) {
  if( UNLIKELY( msgLevel(MSG::DEBUG) ) ) 
      debug() << "Adding stereo coordinates, tol = " << tol << endmsg;

  PatFwdHits temp; // no need actually for PatFwdHits -- a container with objects which have 'projection', 'planeCode','selected' (and pointer to hit) suffices

  for (unsigned int sta=0; sta<nSta; sta++) {
    for (unsigned int lay=1; lay<nLay-1; lay++) {
      for (unsigned int region=0; region<nReg; region++) {

        const Tf::EnvelopeBase* regionB = m_tHitManager->region(sta,lay,region);
        double dz = regionB->z() - m_fwdTool->zReference();
        double yRegion = track.y( dz );

        double tolY = m_yCompatibleTol;
        if (yRegion+tolY < regionB->ymin() ||
            yRegion-tolY > regionB->ymax() ) continue;

        double xPred   = track.x( dz );

        //== Correct for stereo
        double xHitMin = xPred - fabs( yRegion * regionB->sinT() ) - 40. - tol;

        //== get position and slope at z=0 from track at zReference (0 for y/ySlope functions)
        double ty = track.ySlope( 0. );
        double y0 = track.y( 0. - m_fwdTool->zReference() );  // Extrapolate from back...

        //== Project in Y, in fact in X but oriented, such that a displacement in Y is a
        //== displacement also in this projectio.
        bool  flipSign = ( regionB->sinT()  < 0 );
        double minProj = tol;
        if ( region< nOTReg ) minProj += 1.5;

        for ( auto* hit : m_tHitManager->hitsWithMinX(xHitMin, sta, lay, region) ) {
    	  if (hit->hit()->ignore() || !hit->hit()->isYCompatible( yRegion, tolY ) ) continue;

          updateHitForTrack( hit, y0, ty );

          hit->setIgnored( false );
          hit->setSelected( driftOK(*hit) );
          if (!hit->isSelected()) continue;

          double xRef = ( hit->x() - xPred ) ;
          hit->setProjection( flipSign ? -xRef : xRef );

          if (  minProj <  xRef ) break;
          if ( -minProj <= xRef ) temp.push_back( hit );
        }
      }
    }
  }

  //== Sort by projection
  std::sort( std::begin(temp), std::end( temp), Tf::increasingByProjection<PatForwardHit>() );

  int minYPlanes = 4;

  if( UNLIKELY( msgLevel(MSG::DEBUG) ) ) 
    debug() << "List size = " << temp.size() << endmsg;
  if ( minYPlanes > (int)temp.size() ) return false;


  if( UNLIKELY( msgLevel(MSG::DEBUG) ) ) {
    for ( const PatFwdHit *hit : temp ) {
      debug() << format( " Selected:  Z %10.2f Xp %10.2f X%10.2f  St%2d lay%2d typ%2d Prev%2d Next%2d",
                         hit->z(),
                         hit->projection(),
                         hit->x(),
                         hit->hit()->station(),
                         hit->hit()->layer(),
                         hit->hit()->region(),
                         hit->hasPrevious(),
                         hit->hasNext() ) << endmsg;
    }
  }

  //== Select a coherent group
  auto bestList = std::make_pair(temp.end(),temp.end());;
  int inbDifferent = 0;
  double size = 1000.;

  auto itP = std::begin(temp);
  const auto last = std::end(temp);
  while (  itP + (minYPlanes - 1) <  last ) {

    MaxSpread predicate{ allowedStereoSpread(*itP) } ; 

    /// collapse these two calls into one...
    auto itE = itP + (minYPlanes -1);
    std::tie( itP, itE ) = find_range( itP, itE, last, predicate );
    if ( itP == itE ) {
        if( UNLIKELY( msgLevel(MSG::VERBOSE) ) ) verbose() << "   not enough planes in spread" << endmsg;
        continue;
    }
    std::tie( itP, itE) = make_single_zone( itP, itE, last, minYPlanes, predicate);
    if ( itP == itE ) {
        if( UNLIKELY( msgLevel(MSG::VERBOSE) ) ) verbose() << "   Not enough x planes : " << endmsg;
        continue;
    }
    ///

    if( UNLIKELY( msgLevel(MSG::VERBOSE) ) ) {
      verbose() << format( "Found Y group from %9.2f to %9.2f with %2d entries and %2d planes, spread %9.2f",
                           (*itP)->projection(), (*(itE-1))->projection(), itE-itP, nbDifferent(itP,itE), predicate.spread)
                << endmsg;
    }
    //== We have the first list. The best one ????
    auto cnt = nbDifferent( itP, itE ); // can we re-use the result computed in make_single_zone???
    if ( cnt >= inbDifferent ) {
      auto delta = (*std::prev(itE))->projection() - (*itP)->projection();
      if ( cnt > inbDifferent || delta < size ) {
        inbDifferent =  cnt;
        size = delta;
        bestList = { itP, itE } ;
        //break; /// Keep first one !
      }
    }
    itP = itE;
  }

  if ( (int)std::distance(bestList.first,bestList.second) < minYPlanes ) return false;

  if( UNLIKELY( msgLevel(MSG::DEBUG) ) ) 
    debug() << "...Selected " << std::distance(bestList.first,bestList.second) << " hits from "
            << (*bestList.first)->projection()
            << " to " << (*(bestList.second-1))->projection() << endmsg;

  track.addCoords( bestList.first, bestList.second );

  //== Sort by Z
  std::sort( std::begin(track), std::end(track), Tf::increasingByZ<PatForwardHit>() );
  return true;
}

//=========================================================================
//  Debug one vector of of Hits
//=========================================================================
void PatForwardTool::debugFwdHits ( const PatFwdTrackCandidate& track ) const {
  debugFwdHits( track, debug() );
}

void PatForwardTool::debugFwdHits ( const PatFwdTrackCandidate& track, MsgStream& msg ) const {
  for ( const auto *hit : track ) {
    msg << format( " Z %10.2f Xp %10.2f X%10.2f  St%2d lay%2d typ%2d Prev%2d Next%2d Drift %7.3f",
                   hit->z(),
                   hit->projection(),
                   hit->x(),
                   hit->hit()->station(),
                   hit->hit()->layer(),
                   hit->hit()->region(),
                   hit->hasPrevious(),
                   hit->hasNext(),
                   hit->driftDistance() );
    if ( track.fitted() ) msg << format( " chi2%7.2f", m_fwdTool->chi2Hit( track, hit) );
    /* for ( std::vector<int>::const_iterator itM = hit->MCKeys().begin();
       hit->MCKeys().end() != itM; ++itM ) {
       msg << " " << *itM;
       if ( (*itM) == m_MCKey ) msg << " <=*** ";
       }
    */
    msg << endmsg;
  }
}

//=========================================================================
//  Build the list of vector of X hit candidates.
//=========================================================================
void PatForwardTool::buildXCandidatesList ( PatFwdTrackCandidate& track ) {

  bool isDebug = msgLevel( MSG::DEBUG );
  m_candidates.clear();
  m_xHitsAtReference.clear();

  double xExtrap  = track.xStraight( m_fwdTool->zReference() );
  //== calculate if minPt or minMomentum sets the window size
  double minMom = m_minPt / track.sinTrack();
  //== calculate center of magnet from Velo track
  const double zMagnet =  m_fwdTool->zMagnet( track );
  const double dSlope =  m_magnetKickParams[0] / ( minMom - m_magnetKickParams[1] ) ;
  double maxRange = dSlope*( m_fwdTool->zReference() - zMagnet);

  double kick = 0.0;
  float dir =  1.0;
  double pt = track.track()->pt();
  double dSlope_kick = 0;
  //== based on momentum a wrong-charge sign window size is defined
  if (m_useMomentumEstimate && 0 != track.qOverP() && !m_withoutBField) {
    double q = track.qOverP() > 0 ? 1. : -1.;

    double magscalefactor = m_fwdTool->magscalefactor() ;
    dir = -q*magscalefactor;
    if(m_UseWrongSignWindow && pt>m_WrongSignPT){
      double minWrongSignedMom = m_WrongSignPT / track.sinTrack();
      dSlope_kick =m_magnetKickParams[0] / (minWrongSignedMom - m_magnetKickParams[1] ) ;
      kick = dSlope_kick*( m_fwdTool->zReference() - zMagnet);
    }
    if( UNLIKELY( isDebug ) ) 
      debug() << "   xExtrap = " << xExtrap
              << " q/p " << track.qOverP()
              << " predict " << xExtrap + kick << endmsg;
  }

  if( UNLIKELY( isDebug ) ) 
    debug() << "Search X coordinates, xMin " << xExtrap - maxRange
            << " xMax " << xExtrap + maxRange << endmsg;
  
  fillXList( track, kick, maxRange, zMagnet ,dir);
  
  if ( m_minXPlanes > (int)m_xHitsAtReference.size() ) return;

  int minXPlanes = m_minXPlanes;

  double lastEnd = -1.e10;

  auto itP = std::begin(m_xHitsAtReference);
  auto last = std::end(m_xHitsAtReference);
  while (  itP + minXPlanes < last ) {
    MaxSpread predicate{ allowedXSpread(*itP,xExtrap) } ;

    auto itE = itP + minXPlanes -1;
    std::tie(itP, itE) = find_range( itP, itE, last, predicate);
    if (itP == itE) continue;

    std::tie(itP, itE) = make_single_zone( itP, itE, last, minXPlanes, predicate);
    if (itP == itE) {
        if( UNLIKELY( msgLevel(MSG::VERBOSE) ) ) verbose() << "   Not enough x planes : " << endmsg;
        continue;
    }

    double x1 = (*itP)->projection();
    double x2 = (*(itE-1))->projection();
    if( UNLIKELY( msgLevel(MSG::VERBOSE) ) ) {
      verbose() << format( "Found X group from %9.2f to %9.2f with %2d entries and %2d planes, spread %9.2f",
                           x1, x2, itE-itP, nbDifferent(itP,itE), predicate.spread)
                << endmsg;
    }
    //== Protect against too dirty area.
    if ( itE < itP +  m_maxXCandidateSize ) { 
      //== Try to merge the lists, if the first new point is close to the last one...
      if ( x1 < lastEnd + predicate.spread ) {
        m_candidates.back().addCoords( itP,itE );
      } else {
        m_candidates.emplace_back( track.track(), itP,itE );
      }
      lastEnd = x2;
    }
    itP = itE;
  }
}

//=============================================================================
