// $Id: PatForwardTool.cpp,v 1.3 2007-11-14 09:06:53 smenzeme Exp $
// Include files

// from Gaudi
#include "GaudiKernel/ToolFactory.h"
#include "GaudiKernel/IRegistry.h"
#include "GaudiKernel/SystemOfUnits.h"

#include "TfKernel/HitExtension.h"
#include "TfKernel/RecoFuncs.h"

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


DECLARE_TOOL_FACTORY( PatForwardTool );


//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
PatForwardTool::PatForwardTool( const std::string& type,
                                const std::string& name,
                                const IInterface* parent )
  : GaudiTool ( type, name , parent )
{
  declareInterface<IPatForwardTool>(this);
  declareInterface<ITracksFromTrack>(this);

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
  declareProperty( "MaxChi2"               , m_maxChi2               =  20       );
  declareProperty( "MaxChi2Track"          , m_maxChi2Track          =  20       );
  declareProperty( "MinHits"               , m_minHits               =  14       );
  declareProperty( "MinOTHits"             , m_minOTHits             =  16       );
  declareProperty( "CenterOTYSize"         , m_centerOTYSize         =  100 * Gaudi::Units::mm );
  declareProperty( "MaxDeltaY"             , m_maxDeltaY             =  30.      );
  declareProperty( "MaxDeltaYSlope"        , m_maxDeltaYSlope        =  300.      );

  declareProperty( "RangePerMeV"           , m_rangePerMeV           =  5250. * Gaudi::Units::GeV );
  declareProperty( "MinRange"              , m_minRange              =   300. * Gaudi::Units::mm  );
  declareProperty( "RangeErrorFraction"    , m_rangeErrorFraction    =  0.60        );

  declareProperty("StateErrorX2",  m_stateErrorX2  =   4.0);
  declareProperty("StateErrorY2",  m_stateErrorY2  = 400.);
  declareProperty("StateErrorTX2", m_stateErrorTX2 =  6.e-5);
  declareProperty("StateErrorTY2", m_stateErrorTY2 =  1.e-4);
  declareProperty("StateErrorP",   m_stateErrorP   =  0.15);

  declareProperty("AddTTClusterName", m_addTtToolName = "PatAddTTCoord" );

  declareProperty( "WithoutBField"         , m_withoutBField         = false);

}
//=============================================================================
// Destructor
//=============================================================================
PatForwardTool::~PatForwardTool() {};

//=========================================================================
//  Initialization, get the tools
//=========================================================================
StatusCode PatForwardTool::initialize ( ) {
  StatusCode sc = GaudiTool::initialize(); // must be executed first
  if ( sc.isFailure() ) return sc;  // error printed already by GaudiAlgorithm

  debug() << "==> Initialize" << endmsg;


  m_tHitManager  = tool<Tf::TStationHitManager<PatForwardHit> >("PatTStationHitManager");
  m_fwdTool      = tool<PatFwdTool>( "PatFwdTool");

  if ( "" != m_addTtToolName ) {
    m_addTTClusterTool = tool<IAddTTClusterTool>( m_addTtToolName );
  } else {
    m_addTTClusterTool = NULL;
  }

  return StatusCode::SUCCESS;
}

//=========================================================================
//  Main execution: Extend a track.
//=========================================================================
void PatForwardTool::forwardTrack( const LHCb::Track* tr, LHCb::Tracks* output ) {

  std::vector<LHCb::Track*> outvec;
  tracksFromTrack(*tr,outvec).ignore();

  for (unsigned int i=0; i<outvec.size(); i++)
    output->insert(outvec[i]);

}

StatusCode PatForwardTool::tracksFromTrack( const LHCb::Track& seed,
                                            std::vector<LHCb::Track*>& output ){


  LHCb::Track* tr = (LHCb::Track*) &seed;

  bool isDebug = msgLevel( MSG::DEBUG );

  if ( tr->checkFlag( LHCb::Track::Invalid  ) ) return StatusCode::SUCCESS;
  if ( tr->checkFlag( LHCb::Track::Backward ) ) return StatusCode::SUCCESS;

  PatFwdTrackCandidate track( tr );

  if ( isDebug ) {
    debug() << format( "**** Velo track %3d  x%8.2f  y%8.2f  tx%9.5f  ty%9.5f q/p = %8.6f",
                       tr->key(), track.xStraight( m_zAfterVelo ),
                       track.yStraight( m_zAfterVelo ),
                       track.slX(), track.slY(), 1000. * track.qOverP() ) << endreq;
  }



  //== Build the initial list of X candidates


  buildXCandidatesList( track );

  int minPlanes = m_minPlanes;  //== Initial value, can be updated later...

  std::vector<PatFwdTrackCandidate> xCandidates;
  std::vector<PatFwdTrackCandidate>::iterator itL;

  int minOTX = int( 1.5 * m_minXPlanes );

  for( itL = m_candidates.begin(); m_candidates.end() != itL; ++itL ) {

    //== Fit the candidate, and store them

    while ( m_fwdTool->fitXCandidate( *itL, m_maxChi2, m_minXPlanes ) ) {

      // Copy the whole stuff, keep only used ones and store it.
      PatFwdTrackCandidate temp = *itL;
      temp.cleanCoords();
      m_fwdTool->chi2PerDoF( temp );  // Compute and store chi2/dof
      if ( isDebug ) {
        debug() << "Chi2/nDof = " << temp.chi2PerDoF() << " nDoF " << temp.nDoF()
                << " dist at center " << m_fwdTool->distAtMagnetCenter( temp )
                << endreq;
        debugFwdHits( temp );
      }


      //== Check the chi2 with magnet center constraint.

      if ( m_maxChi2Track > temp.chi2PerDoF()  &&
           (!m_withoutBField || fabs(temp.slX()-temp.xSlope(0))<0.005)){
        //== Count how many weighted hits
        PatFwdRegionCounter regions( temp.coordBegin(), temp.coordEnd() );
        int nbHit = regions.nbOT() + 2*regions.nbIT();
        bool inCenter = m_centerOTYSize > fabs( temp.y( 0. ) );

        if ( minOTX <= nbHit || inCenter ) {
          xCandidates.push_back( temp );
          debug() << "+++ Store candidate " << xCandidates.size()-1 << endreq;
        } else {
          debug() << " --- not enough hits " << nbHit << endreq;
        }
      }

      //== tag these coordinates in the original, so that we don't find the same track again
      for ( PatFwdHits::iterator itH = (*itL).coordBegin(); (*itL).coordEnd() != itH; ++itH ) {
        if ( (*itH)->isSelected() ) {
          (*itH)->setIgnored( true );
        }
      }
    }
  }

  if ( isDebug ) {
    debug() << "************ List of X candidates , N = " << xCandidates.size() << endreq;
    for ( itL = xCandidates.begin(); xCandidates.end() != itL; ++itL ) {
      debug() << "Candidate " << itL - xCandidates.begin()
              << " Chi2/nDof = " << (*itL).chi2PerDoF() << endreq;
      debugFwdHits( *itL );
    }
    if ( xCandidates.size() > 0 ) debug() << "---- Now get the stereo hits on these ---" << endreq;
  }

  //== Now try to get space track from these X track.


  std::vector<PatFwdTrackCandidate> goodCandidates;
  int maxPlanes = 0;


  for ( itL = xCandidates.begin(); xCandidates.end() != itL; ++itL ) {
    debug() << "--- Candidate " << itL - xCandidates.begin()
            << "  X cord size " << (*itL).coordEnd() - (*itL).coordBegin()
            << endreq;

    PatFwdHits::iterator itH;
    for ( itH = (*itL).coordBegin(); (*itL).coordEnd() != itH ; ++itH ) {
      (*itH)->setIgnored( false);  // restore normal flag.
    }
    PatFwdTrackCandidate& temp = *itL;
    temp.setSelectedCoords( );

    double qOverP = 1000. * m_fwdTool->qOverP( temp );  // in 1/GeV

    if (m_withoutBField)
      if (m_minMomentum !=0)
        qOverP = 1/m_minMomentum;
      else
        qOverP = 1;

    double tol = m_maxSpreadY + m_maxSpreadSlopeY * qOverP *  qOverP;

    debug() << "Adding stereo coordinates, tol = " << tol << endreq;

    if ( !fillStereoList( temp, tol ) ) continue; // Get the stereo coordinates

    if ( isDebug )  debugFwdHits( temp );
    temp.setSelectedCoords( );

    //== Fit and reject if not good enough

    if ( !m_fwdTool->fitStereoCandidate( temp, m_maxChi2, minPlanes ) ) continue;

    temp.cleanCoords();
    m_fwdTool->chi2PerDoF( temp );  //== compute and store Chi2PerDoF
    if ( isDebug ) {
      debug() << "  ... fit OK  chi2 " << temp.chi2PerDoF() << " nDoF " << temp.nDoF()
              << " dist at center " << m_fwdTool->distAtMagnetCenter( temp )
              << endreq;
      debugFwdHits( temp );
    }

    //== Remove stereo coordinates incompatible in Y

    double yTol = m_yCompatibleTolFinal;
    if ( !m_fwdTool->removeYIncompatible( temp, yTol, minPlanes ) ) continue;
    temp.cleanCoords();
    debug() << "  ... Y is compatible" << endreq;

    double quality = 0.;

    // Enough stereo planes
    PatFwdPlaneCounter<PatForwardHit> fullCount( temp.coordBegin(), temp.coordEnd() );
    int nbY = fullCount.nbStereo();
    if ( 4 > nbY ) {
      debug() << "Not enough Y planes : " << nbY << endreq;
      continue;
    }

    if ( m_maxDeltaY + m_maxDeltaYSlope * qOverP *qOverP < fabs(  m_fwdTool->changeInY( temp ) ))  {
      debug() << "  --- Too big change in Y : " <<  m_fwdTool->changeInY( temp ) << endreq;
      continue;
    }

    quality  = 5. * fabs(  m_fwdTool->changeInY( temp ) ) / ( m_maxDeltaY + qOverP * qOverP * m_maxDeltaYSlope );
    quality += temp.chi2PerDoF() / 10.;
    quality += 10 * fabs(qOverP);  // low momentum are worse

    temp.setQuality( quality );

    //== Verify if enough OT measurements, counting IT for 2/plane
    //== Ignore the y central region, OT inefficient there.

    PatFwdRegionCounter regions( temp.coordBegin(), temp.coordEnd() );
    int nbHit = regions.nbOT() + 2*regions.nbIT();
    temp.setNbIT( regions.nbIT() );
    temp.setNbOT( regions.nbOT() );

    bool inCenter = m_centerOTYSize > fabs( temp.y( 0. ) );
    if ( !inCenter ) {
      if ( m_minHits > nbHit ){
        debug() << "  --- Not enough hits : " << nbHit << endreq;
        continue;
      }
      if ( temp.nbIT() == 0 && temp.nbOT() < m_minOTHits ) {
        debug() << " Too few OT for OT only track : " << temp.nbOT() << endreq;
        continue;
      }
    }

    int nbPlanes = fullCount.nbDifferent();
    if ( maxPlanes < nbPlanes ) maxPlanes = nbPlanes;

    goodCandidates.push_back( temp );

    //== Update requirement according to already found good solutions...
    PatFwdPlaneCounter<PatForwardHit> planeCounter( temp.coordBegin(), temp.coordEnd() );
    if ( minPlanes+1 < planeCounter.nbDifferent() ) minPlanes =  planeCounter.nbDifferent()-1;
  }

  //================================================================================
  //  Now some filtering of tracks, in case of multiple candidates.
  //================================================================================

  if ( 1 < goodCandidates.size()  ) {

    // remove track with sensibly lower number of planes
    int minPlanes = maxPlanes - 1;
    double bestQuality = 1000.;
    int maxOT = 0;

    debug() << "Require enough planes : " << minPlanes << endreq;
    std::vector<PatFwdTrackCandidate> tempCandidates( goodCandidates );
    goodCandidates.clear();
    for ( itL = tempCandidates.begin(); tempCandidates.end() != itL; ++itL ) {
      PatFwdPlaneCounter<PatForwardHit> tmp( (*itL).coordBegin(), (*itL).coordEnd() );
      if ( tmp.nbDifferent() >= minPlanes ) {
        goodCandidates.push_back( *itL );
        if ( (*itL).quality() < bestQuality ) bestQuality = (*itL).quality();
      } else {
        debug() << "Ignore candidate " << itL-tempCandidates.begin()
                << " : not enough planes = " << tmp.nbDifferent() << endreq;
      }
    }
    // remove worst quality
    bestQuality += 1.0;
    tempCandidates = goodCandidates;
    goodCandidates.clear();
    for ( itL = tempCandidates.begin(); tempCandidates.end() != itL; ++itL ) {
      if ( (*itL).quality() < bestQuality ) {
        goodCandidates.push_back( *itL );
        if ( 2*(*itL).nbIT() + (*itL).nbOT() > maxOT ) maxOT =  2*(*itL).nbIT()+(*itL).nbOT();
      } else {
        debug() << "Ignore candidate " << itL-tempCandidates.begin()
                << " : quality too low = " << (*itL).quality() << endreq;
      }
    }
    // remove if sensibly less OT
    if ( 24 < maxOT ) maxOT = 24;
    maxOT -= 3;
    tempCandidates = goodCandidates;
    goodCandidates.clear();
    for ( itL = tempCandidates.begin(); tempCandidates.end() != itL; ++itL ) {
      if ( 2*(*itL).nbIT() + (*itL).nbOT() > maxOT ) {
        goodCandidates.push_back( *itL );
      } else {
        debug() << "Ignore candidate " << itL-tempCandidates.begin()
                << " : not enough OT = " << (*itL).nbOT() << " mini " << maxOT << endreq;
      }
    }
  }

  debug() << "Storing " << goodCandidates.size() << " good tracks " << endreq;
  //=== Store tracks...
  for ( itL = goodCandidates.begin(); goodCandidates.end() != itL; ++itL ) {
    LHCb::Track* fwTra = tr->clone();
    fwTra->clearAncestors();
    fwTra->addToAncestors( tr );  // Set the Velo track as only ancestor of the Forward track
    output.push_back( fwTra );
    fwTra->setType( LHCb::Track::Long );
    fwTra->setHistory( LHCb::Track::PatForward );

    //== Add a new state in the T stations ...
    LHCb::State temp;
    double dz = m_fwdTool->zOutput() - m_fwdTool->zReference();
    double qOverP = m_fwdTool->qOverP( *itL );
    // set q/p in all of the existing states
    const std::vector< LHCb::State * > states = fwTra->states();
    std::vector< LHCb::State * >::const_iterator iState;
    for ( iState = states.begin() ; iState != states.end() ; ++iState ){
      (*iState)->setQOverP(qOverP);
      (*iState)->setErrQOverP2(qOverP*qOverP*0.012*0.012);
    }
    temp.setLocation( LHCb::State::AtT );
    temp.setState( (*itL).x( dz ), (*itL).y( dz ), m_fwdTool->zOutput(),
                   (*itL).xSlope( dz ), (*itL).ySlope( dz ), qOverP );

    //== overestimated covariance matrix, as input to the Kalman fit
    Gaudi::TrackSymMatrix cov;
    cov(0,0) = m_stateErrorX2;
    cov(1,1) = m_stateErrorY2;
    cov(2,2) = m_stateErrorTX2;
    cov(3,3) = m_stateErrorTY2;
    double errQOverP = m_stateErrorP*qOverP;
    cov(4,4) = errQOverP * errQOverP;

    temp.setCovariance( cov );

    debug() << " added State " << temp.stateVector()
            << " cov \n" << temp.covariance() << endreq;

    fwTra->addToStates( temp );
    //== New information, on the track fit.
    fwTra->setChi2PerDoF( (*itL).chi2PerDoF() );
    fwTra->setNDoF(       (*itL).nDoF() );
    fwTra->addInfo(LHCb::Track::PatQuality, (*itL).quality());

    //== Add reference to the used clusters/, T stations
    for ( PatFwdHits::iterator itH = (*itL).coordBegin(); (*itL).coordEnd() != itH; itH++ ) {
      PatFwdHit* myHit = (*itH);
      if ( !myHit->isSelected() ) continue;
      fwTra->addToLhcbIDs( myHit->hit()->lhcbID() );
      myHit->hit()->setStatus(Tf::HitBase::UsedByPatForward);
      myHit->setIsUsed(true);
    }
    fwTra -> setPatRecStatus( LHCb::Track::PatRecIDs );


    if ( NULL != m_addTTClusterTool ) {
      StatusCode sc = m_addTTClusterTool->addTTClusters( *fwTra );
      if (sc.isFailure())
        debug()<<" Failure in adding TT clusters to track"<<endreq;
    }
  }
  debug() << "Finished track" << endreq;
  return StatusCode::SUCCESS;
}
//=========================================================================
//  Fill the vector of hit pointer, sorted by projection.
//=========================================================================

void PatForwardTool::fillXList ( PatFwdTrackCandidate& track, double xMin, double xMax ) {

  PatFwdHits::const_iterator itFwdH;

  for (unsigned int sta = 0; sta < m_nSta; sta ++){
    for (unsigned int lay = 0; lay< m_nLay; lay++){
      if (lay == 1 || lay == 2) continue;  // X layers
      for (unsigned int region = 0; region <m_nReg; region ++){
        const Tf::EnvelopeBase* regionB = m_tHitManager->region(sta,lay,region);

        double yCompat = m_yCompatibleTol + 50 * fabs(track.slY());
        double yRegion = track.yStraight( regionB->z() );
        double xHitMin = xMin * regionB->z() / m_fwdTool->zReference();
        xHitMin        = xHitMin - fabs( yRegion * regionB->sinT() ) - 20.;
        double ty = track.slY();
        double y0 = track.yStraight( 0. );

        if ( yRegion+yCompat < regionB->ymin() ||
             yRegion-yCompat > regionB->ymax() ) {
          continue;
        }

        Tf::TStationHitManager<PatForwardHit>::HitRange range =
          m_tHitManager->hitsWithMinX(xHitMin, sta, lay, region);

        for ( PatFwdHits::const_iterator itH = range.begin();
              range.end() != itH; ++itH ) {

          PatFwdHit* hit = *itH;
          updateHitForTrack( hit, y0, ty);

          if ( !hit->hit()->isYCompatible( (float)yRegion, (float)yCompat ) )
            continue;


          hit->setSelected( true );
          hit->setIgnored( false );

          if (hit->hit()->region()<m_nOTReg)
            if ( m_maxOTDrift < hit->driftDistance() ||
                 m_minOTDrift > hit->driftDistance() ) {
              hit->setSelected( false );
              continue;
            }

          double xRef = m_fwdTool->xAtReferencePlane( track, hit );
          hit->setProjection( xRef );

          if ( xMin > xRef ) continue;
          if ( xMax < xRef ) break;

          m_xHitsAtReference.push_back( hit );

        }
      }
    }
  }


  std::sort( m_xHitsAtReference.begin(),
             m_xHitsAtReference.end(),
             Tf::increasingByProjection<PatForwardHit>() );

}

//=========================================================================
//  Fill the vector of hit pointer, sorted by projection.
//=========================================================================
bool PatForwardTool::fillStereoList ( PatFwdTrackCandidate& track, double tol ) {

  PatFwdHits temp;
  PatFwdHits::const_iterator itH;

  for (unsigned int sta=0; sta<m_nSta; sta++) {
    for (unsigned int lay=1; lay<m_nLay-1; lay++) {
      for (unsigned int region=0; region<m_nReg; region++) {

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
        double y0 = track.y( 0. )  -  m_fwdTool->zReference()  * ty;  // Extrapolate from back...

        //== Project in Y, in fact in X but oriented, such that a displacement in Y is a
        //== displacement also in this projectio.
        double sign = -1.;
        if ( regionB->sinT() >0 ) sign = +1;

        double minProj = tol;
        if ( region< m_nOTReg ) minProj += 1.5;

        Tf::TStationHitManager<PatForwardHit>::HitRange range = m_tHitManager->hitsWithMinX(xHitMin, sta, lay, region);

        for ( PatFwdHits::const_iterator itH = range.begin();
              range.end() != itH; ++itH ) {



          PatFwdHit* hit = *itH;

          if ( !hit->hit()->isYCompatible( (float)yRegion, (float)tolY ) ) continue;

          updateHitForTrack( hit, y0, ty );

          hit->setSelected( true );
          hit->setIgnored( false );
          if (hit->hit()->region()<m_nOTReg)
            if ( m_maxOTDrift < hit->driftDistance() ||
                 m_minOTDrift > hit->driftDistance()  ) {
              hit->setSelected( false );
              continue;
            }
          double xRef = ( hit->x() - xPred ) * sign;
          hit->setProjection( xRef );

          if ( -minProj > xRef * sign ) continue;
          if (  minProj < xRef * sign ) break;

          temp.push_back( hit );
        }
      }
    }
  }

  //== Sort by projection

  std::sort( temp.begin(), temp.end(), Tf::increasingByProjection<PatForwardHit>() );

  int minYPlanes = 4;
  double maxSpread = 3.;

  debug() << "List size = " << temp.size() << endreq;
  if ( minYPlanes > (int)temp.size() ) return false;


  if ( msgLevel( MSG::DEBUG ) ) {
    for ( itH = temp.begin(); temp.end() != itH; ++itH ) {
      PatFwdHit* hit = *itH;
      debug() << format( " Selected:  Z %10.2f Xp %10.2f X%10.2f  St%2d lay%2d typ%2d Prev%2d Next%2d",
                         hit->z(),
                         hit->projection(),
                         hit->x(),
                         hit->hit()->station(),
                         hit->hit()->layer(),
                         hit->hit()->region(),
                         hit->hasPrevious(),
                         hit->hasNext() ) << endreq;
    }
  }

  //== Select a coherent group
  PatFwdHits bestList;
  int nbDifferent = 0;
  double size = 1000.;

  PatFwdHits::iterator itP, itB, itE, itF;
  for ( itP = temp.begin();  temp.end() - minYPlanes >= itP; ++itP ) {
    itE = itP + minYPlanes -1;
    double spread = maxSpread;
    if ( (*itP)->hit()->region() < m_nOTReg ) spread += 1.5;  // OT drift ambiguities...

    if ( msgLevel( MSG::VERBOSE) ){
      verbose() << format( "  first %8.2f +minXPlanes -> %8.2f (diff: %8.2f) Spread %6.2f ",
                           (*itP)->projection(), (*itE)->projection(),
                           (*itE)->projection() - (*itP)->projection(), spread );
    }

    //== If not enough hits in the maximum spread, skip
    if (  spread < (*itE)->projection() - (*itP)->projection() ) {
      while( spread < (*itE)->projection() - (*itP)->projection() ) itP++;
      --itP; // as there will be a ++ in the loop !
      verbose() << "   not enough planes in spread" << endreq;
      continue;
    }


    //== Add all hits inside the maximum spread. If not enough planes, restart
    while ( itE != temp.end() &&
            spread > (*itE)->projection() - (*itP)->projection() ) itE++;
    PatFwdPlaneCounter<PatForwardHit> planeCount( itP, itE );
    //== Enough different planes
    if ( minYPlanes > planeCount.nbDifferent() ) {
      debug() << "   Not enough y planes : " << planeCount.nbDifferent() << endreq;
      continue;
    }
    verbose() << endreq;

    //== Try to make a single zone, by removing the first and adding other as
    //   long as the spread and minXPlanes conditions are met.
    itB = itP;
    itF = itE;
    while ( itP <= itE - minYPlanes && itF < temp.end() ) {
      planeCount.removeHit( *itP );
      ++itP;
      verbose() << " try to extend from itP : " << (*itP)->projection()
                << " itF " << (*itF)->projection() << endreq;
      while ( itF < temp.end() &&
              spread > (*itF)->projection() - (*itP)->projection() ) {
        planeCount.addHit( *itF++ );
      }
      if ( minYPlanes <= planeCount.nbDifferent() ) itE = itF;
    }

    double x1 = (*itB)->projection();
    double x2 = (*(itE-1))->projection();
    if ( msgLevel( MSG::VERBOSE )  ) {
      PatFwdPlaneCounter<PatForwardHit> pc( itB, itE );
      verbose() << format( "Found Y group from %9.2f to %9.2f with %2d entries and %2d planes, spread %9.2f",
                           x1, x2, itE-itB, pc.nbDifferent(), spread)
                << endreq;
    }
    //== We have the first list. The best one ????
    PatFwdPlaneCounter<PatForwardHit> cnt( itB, itE );
    if ( cnt.nbDifferent() >= nbDifferent ) {
      if ( cnt.nbDifferent() > nbDifferent || x2-x1 < size ) {
        nbDifferent =  cnt.nbDifferent();
        size = x2-x1;
        bestList.clear();
        for ( itP = itB; itE != itP; ++itP ) {
          bestList.push_back( *itP );
        }
        //break; /// Keep first one !
      }
    }
    itP = --itE;
  }

  if ( minYPlanes > (int)bestList.size() ) return false;
  debug() << "...Selected " << bestList.size() << " hits from " << (*bestList.begin())->projection()
          << " to " << (*bestList.rbegin())->projection() << endreq;



  for ( itP = bestList.begin(); bestList.end() != itP; ++itP ) {
    track.addCoord( *itP );
  }



  //== Sort by Z

  std::sort( track.coordBegin(), track.coordEnd(), 
	     Tf::increasingByZ<PatForwardHit>() );

  return true;
}

//=========================================================================
//  Debug one vector of of Hits
//=========================================================================
void PatForwardTool::debugFwdHits ( PatFwdTrackCandidate& track ) {
  debugFwdHits( track, debug() );
}

void PatForwardTool::debugFwdHits ( PatFwdTrackCandidate& track, MsgStream& msg ) {
  PatFwdHits::iterator itP;
  for ( itP = track.coordBegin(); track.coordEnd() != itP; ++itP ) {
    msg << format( " Z %10.2f Xp %10.2f X%10.2f  St%2d lay%2d typ%2d Prev%2d Next%2d Drift %7.3f",
                   (*itP)->z(),
                   (*itP)->projection(),
                   (*itP)->x(),
                   (*itP)->hit()->station(),
                   (*itP)->hit()->layer(),
                   (*itP)->hit()->region(),
                   (*itP)->hasPrevious(),
                   (*itP)->hasNext(),
                   (*itP)->driftDistance() );
    if ( track.fitted() ) msg << format( " chi2%7.2f", m_fwdTool->chi2Hit( track, (*itP)) );
    /* for ( std::vector<int>::const_iterator itM = (*itP)->MCKeys().begin();
       (*itP)->MCKeys().end() != itM; ++itM ) {
       msg << " " << *itM;
       if ( (*itM) == m_MCKey ) msg << " <=*** ";
       }
    */
    msg << endreq;
  }
}




//=========================================================================
//  Build the list of vector of X hit candidates.
//=========================================================================
void PatForwardTool::buildXCandidatesList ( PatFwdTrackCandidate& track ) {

  m_candidates.clear();
  m_xHitsAtReference.clear();

  double xExtrap  = track.xStraight( m_fwdTool->zReference() );
  double minMom = m_minPt / track.sinTrack();
  if ( m_minMomentum > minMom ) minMom = m_minMomentum;
  double maxRange = m_rangePerMeV / minMom;

  if ( 0 != track.qOverP() && !m_withoutBField) {
    debug() << "   xExtrap = " << xExtrap
            << " q/p " << track.qOverP()
            << " predict " << xExtrap + (m_rangePerMeV * track.qOverP()) << endreq;
    xExtrap += m_rangePerMeV * track.qOverP();
    maxRange = m_minRange + m_rangeErrorFraction * m_rangePerMeV * fabs( track.qOverP() );
  }

  double minProj  = xExtrap - maxRange;
  double maxProj  = xExtrap + maxRange;

  debug() << "Search X coordinates, xMin " << minProj
          << " xMax " << maxProj << endreq;


  fillXList( track, minProj, maxProj );

  if ( m_minXPlanes > m_xHitsAtReference.end() -  m_xHitsAtReference.begin() ) return;

  PatFwdHits::iterator itP, itB, itE, itF;
  int minXPlanes = m_minXPlanes;

  double lastEnd = -1.e10;

  for ( itP = m_xHitsAtReference.begin();  m_xHitsAtReference.end() - minXPlanes > itP; ++itP ) {
    itE = itP + minXPlanes -1;
    double spreadSl = ( (*itP)->projection() - xExtrap ) * m_maxSpreadSlopeX;
    double spread = m_maxSpreadX + fabs( spreadSl );


    if ( (*itP)->hit()->region() < m_nOTReg ) spread += 1.5;  // OT drift ambiguities...

    //== If not enough hits in the maximum spread, skip
    if (  spread < (*itE)->projection() - (*itP)->projection() ) {
      while( spread < (*itE)->projection() - (*itP)->projection() ) itP++;
      --itP; // as there will be a ++ in the loop !
      continue;
    }

    if ( msgLevel( MSG::VERBOSE ) ){
      verbose() << format( "  first %8.2f +minXPlanes -> %8.2f (diff: %8.2f) Spread %6.2f ",
                           (*itP)->projection(), (*itE)->projection(),
                           (*itE)->projection() - (*itP)->projection(), spread )<<endmsg;
    }

    //== Add all hits inside the maximum spread. If not enough planes, restart
    while ( itE != m_xHitsAtReference.end() &&
            spread > (*itE)->projection() - (*itP)->projection() ) itE++;
    PatFwdPlaneCounter<PatForwardHit> planeCount( itP, itE );
    //== Enough different planes
    if ( minXPlanes > planeCount.nbDifferent() ) {
      verbose() << "   Not enough x planes : " << planeCount.nbDifferent() << endreq;
      continue;
    }
    verbose() << endreq;

    //== Try to make a single zone, by removing the first and adding other as
    //   long as the spread and minXPlanes conditions are met.
    itB = itP;
    itF = itE;
    while ( itP <= itE - minXPlanes && itF < m_xHitsAtReference.end() ) {
      planeCount.removeHit( *itP );
      ++itP;
      while ( itF < m_xHitsAtReference.end() &&
              spread > (*itF)->projection() - (*itP)->projection() ) {
        planeCount.addHit( *itF++ );
      }
      if ( minXPlanes <= planeCount.nbDifferent() ) itE = itF;
    }

    double x1 = (*itB)->projection();
    double x2 = (*(itE-1))->projection();
    if ( msgLevel( MSG::VERBOSE )  ) {
      PatFwdPlaneCounter<PatForwardHit> pc( itB, itE );
      verbose() << format( "Found X group from %9.2f to %9.2f with %2d entries and %2d planes, spread %9.2f",
                           x1, x2, itE-itB, pc.nbDifferent(), spread)
                << endreq;
    }
    //== Try to merge the lists, if the first new point is close to the last one...
    PatFwdHits temp( itB, itE );       // Create a copy of the list
    if ( spread > x1 - lastEnd ) {
      (*m_candidates.rbegin()).addCoords( temp );
    } else {
      PatFwdTrackCandidate aCandidate( track.track(), temp );
      m_candidates.push_back( aCandidate );
    }
    lastEnd = x2;
    itP = --itE;
  }

}

//=============================================================================
