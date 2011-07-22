// Include files

// from Gaudi
#include "GaudiKernel/ToolFactory.h"
#include "GaudiKernel/SystemOfUnits.h"

// from TrackEvent
#include "Event/StateParameters.h"

#include "TfKernel/RecoFuncs.h"
#include <boost/assign/list_of.hpp>

// local
#include "PatFwdTool.h"
#include "PatFwdPlaneCounter.h"
#include "PatFwdRegionCounter.h"
#include "PatFwdFitParabola.h"
#include "PatFwdFitLine.h"
//-----------------------------------------------------------------------------
// Implementation file for class : PatFwdTool
//
// 2005-04-01 : Olivier Callot
//
//-----------------------------------------------------------------------------


DECLARE_TOOL_FACTORY( PatFwdTool )

//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
PatFwdTool::PatFwdTool( const std::string& type,
                        const std::string& name,
                        const IInterface* parent )
  : GaudiTool ( type, name , parent )
{
  declareInterface<PatFwdTool>(this);

  declareProperty( "ZReference"      , m_zReference      = StateParameters::ZMidT); 
  declareProperty( "ZOutput"          ,m_zOutputs        = boost::assign::list_of(StateParameters::ZBegT)
		   (StateParameters::ZMidT)(StateParameters::ZEndT));
  declareProperty( "ZMagnetParams"   , m_zMagnetParams   = boost::assign::list_of
		   (0.0));
  declareProperty( "xParams"         , m_xParams         = boost::assign::list_of
		   (0.0));
  declareProperty( "yParams"         , m_yParams         = boost::assign::list_of
		   (0.0));
  declareProperty( "momentumParams"  , m_momentumParams  = boost::assign::list_of
		   (0.0));
  declareProperty( "xMagnetTol"      , m_xMagnetTol      = 3. );
  declareProperty( "xMagnetTolSlope" , m_xMagnetTolSlope = 40. );
  declareProperty( "withoutBField"   , m_withoutBField = false);
}
//=============================================================================
// Destructor
//=============================================================================
PatFwdTool::~PatFwdTool() {}

//=========================================================================
//  Initialisation, check parameters
//=========================================================================
StatusCode PatFwdTool::initialize ( ) {
  
  
  StatusCode sc = GaudiTool::initialize();
  if ( !sc ) return sc;

  m_magFieldSvc = svc<ILHCbMagnetSvc>( "MagneticFieldSvc", true );

  if ( 5 > m_zMagnetParams.size() || 6 > m_momentumParams.size() || 
       2 > m_xParams.size() || 2 > m_yParams.size()){
  
    m_zMagnetParams.clear();
    m_momentumParams.clear();
    m_xParams.clear();
    m_yParams.clear();

    
    if (m_magFieldSvc->useRealMap()){
      m_zMagnetParams   = boost::assign::list_of (5208.05) (318.502) (-1223.87) (9.80117e-06) (-304.272);
      m_xParams         = boost::assign::list_of (17.5815) (-5.94803);
      m_yParams         = boost::assign::list_of (-979.0) (-0.684947);
      m_momentumParams  = boost::assign::list_of (1.21174) (0.634127) (-0.242116) (0.412728) (2.82916) (-20.6599);
    } else {
      m_zMagnetParams   = boost::assign::list_of (5199.31) (334.725) (-1283.86) (9.59486e-06) (-413.281);
      m_xParams         = boost::assign::list_of (16.8238) (-6.35556);
      m_yParams         = boost::assign::list_of (-970.89) (-0.686028);
      m_momentumParams  = boost::assign::list_of (1.21909) (0.627841) (-0.235216) (0.433811) (2.92798) (-21.3909);
    }
  }

  return StatusCode::SUCCESS;
}


double PatFwdTool::xAtReferencePlane ( PatFwdTrackCandidate& track, PatFwdHit* hit,
    bool store ) {

  double x;

  if (!m_withoutBField){
    double zMagnet    = ( m_zMagnetParams[0] +
			  m_zMagnetParams[2] * track.slX2() +
			  m_zMagnetParams[3] * hit->x() * hit->x() +
			  m_zMagnetParams[4] * track.slY2() );
    
    double xMagnet    = track.xStraight( zMagnet);
    double zHit       = hit->z();
    double xHit       = hit->x();
    
    const Tf::OTHit* othit = hit->hit()->othit();
    
    if ( othit ) {
      if ( hit->hasNext()     ) xHit += hit->driftDistance();
      if ( hit->hasPrevious() ) xHit -= hit->driftDistance();
    }
    
    double slopeAfter = ( xHit - xMagnet ) / ( zHit - zMagnet );
    double dSlope     = slopeAfter - track.slX();
    double dSl2       = dSlope * dSlope;
    zMagnet           = zMagnet + m_zMagnetParams[1] * dSl2;
    double dz         = 1.e-3 * ( zHit - m_zReference );
    double dyCoef     = dSl2 * track.slY();
    double dy         = dyCoef * ( m_yParams[0] + dz * m_yParams[1] );
    double dxCoef     = dz * dz * ( m_xParams[0] + m_xParams[1] * dz );
    xHit              = xHit + dy * hit->hit()->dxDy() - dxCoef * dSlope ;
    xMagnet           = track.xStraight( zMagnet );
    slopeAfter        = ( xHit - xMagnet ) / ( zHit - zMagnet );
    x                 = xMagnet + ( m_zReference - zMagnet ) * slopeAfter;
    
    if ( store ) {
      track.setParameters( x,
			   ( x - xMagnet ) / ( m_zReference - zMagnet ),
			   1.e-6 * m_xParams[0] * dSlope,
			   1.e-9 * m_xParams[1] * dSlope,
			   track.yStraight( m_zReference ) + dyCoef * m_yParams[0],
			   track.slY() + dyCoef * m_yParams[1] );
      m_zMagnet = zMagnet;
    }
  }
  else {
     
    double zMagnet    = 0.0;
    double xMagnet    = track.xStraight( zMagnet);
    double zHit       = hit->z();
    double xHit       = hit->x();
     
    const Tf::OTHit* othit = hit->hit()->othit();
    
    if ( othit ) {
      if ( hit->hasNext()     ) xHit += hit->driftDistance();
      if ( hit->hasPrevious() ) xHit -= hit->driftDistance();
    }
    
    double slopeAfter = ( xHit - xMagnet ) / ( zHit - zMagnet );
    
    x                 = xMagnet + ( m_zReference - zMagnet ) * slopeAfter;
    if ( store ) {
      track.setParameters( x, 
			   ( x - xMagnet ) / ( m_zReference - zMagnet ),
			   0, 
			   0, 
			   track.yStraight( m_zReference ),
			   track.slY());
      m_zMagnet = zMagnet;
    }
  }
    
  return x;
}


//=========================================================================
//  Fit the X projection of a PatFwdTrackCandidate. Iterative.
//=========================================================================

bool PatFwdTool::fitXCandidate ( PatFwdTrackCandidate& track,
                                 double maxChi2, int minPlanes ) {
  bool isDebug = msgLevel( MSG::DEBUG );

  if ( minPlanes > track.setSelectedCoords() ) return false;
  PatFwdPlaneCounter maxPlanes( track.coordBegin(), track.coordEnd() );
  int bestPlanes = maxPlanes.nbDifferent();
  if ( minPlanes > maxPlanes.nbDifferent() ) return false;

  //=== Is there a region with 6 planes ?

  PatFwdHits::iterator itH;
  {
    int bestRegion = -1;
    double spread = 1000.;

    PatFwdRegionCounter regions( track.coordBegin(), track.coordEnd() );
    for( unsigned int maxRegion = 0; 6 > maxRegion; ++maxRegion ) {
      if ( regions.nbInRegion( maxRegion ) >= 6 ) {  // count by plane
        PatFwdPlaneCounter planes( track.coordBegin(), track.coordBegin() );
        double first = 1.e7;
        double last  = first;
        for ( itH =  track.coordBegin(); track.coordEnd() > itH; ++itH ) {
          unsigned int region = (*itH)->hit()->region();
	  if (region != Tf::RegionID::OT)
	    region+=2;
	  if ( region == maxRegion ) {
            if ( first > 1.e6 ) first = (*itH)->projection();
            last = (*itH)->projection();
            planes.addHit( *itH );
          }
        }
        if ( planes.nbDifferent() == 6 ) {
          double mySpread = last-first;
          if ( mySpread < spread ) {
            spread = mySpread;
            bestRegion = maxRegion;
          }
        }
      }
    }
    if ( 0 <= bestRegion ) {
      // remove other regions !
      if( UNLIKELY( msgLevel(MSG::DEBUG) ) ) 
        debug() << "========= Keep only hits of region " << bestRegion << endmsg;
      for ( itH =  track.coordBegin(); track.coordEnd() > itH; ++itH ) {
        unsigned int region = (*itH)->hit()->region();
        if (region != Tf::RegionID::OT)
          region+=2;
        if ( region != (unsigned int)(bestRegion) ) {
          (*itH)->setSelected( false );
        }
      }
    }
  }

  PatFwdHits::iterator itBeg, itEnd;
  itBeg = track.coordBegin();
  itEnd = itBeg + bestPlanes;

  //== get enough planes fired
  PatFwdPlaneCounter planeCount1( itBeg, itEnd );
  while ( itEnd != track.coordEnd() && bestPlanes > planeCount1.nbDifferent() ) {
    PatFwdHit* hit = *itEnd++;
    if ( hit->isSelected() ) planeCount1.addHit( hit );
  }
  if ( bestPlanes > planeCount1.nbDifferent() ) return false;
  double minDist = (*(itEnd-1))->projection() - (*itBeg)->projection();

  PatFwdHits::iterator itBest = itBeg;
  PatFwdHits::iterator itLast = itEnd;
  if ( isDebug ) {
    debug() << format( "        range minDist %7.2f from %8.3f to %8.3f bestPlanes %2d",
                       minDist, (*itBeg)->projection(), (*(itEnd-1))->projection(), bestPlanes )
            << endmsg;
  }

  //== Better range ? Remove first, try to complete, measure spread...
  while ( itEnd != track.coordEnd() &&  bestPlanes <= planeCount1.nbDifferent() ) {
    planeCount1.removeHit( *itBeg++ );
    while ( itEnd != track.coordEnd() && bestPlanes > planeCount1.nbDifferent() ) {
      PatFwdHit* hit = *itEnd++;
      if ( hit->isSelected() ) planeCount1.addHit( hit );
    }
    if ( bestPlanes <= planeCount1.nbDifferent() ) {
      if ( minDist > (*(itEnd-1))->projection() - (*itBeg)->projection() ) {
        minDist = (*(itEnd-1))->projection() - (*itBeg)->projection();
        itBest = itBeg;
        itLast = itEnd;
        if ( bestPlanes < planeCount1.nbDifferent() )  bestPlanes = planeCount1.nbDifferent();
        if ( isDebug ) {
          debug() << format( " better range minDist %7.2f from %8.3f to %8.3f bestPlanes %2d",
                             minDist, (*itBeg)->projection(), (*(itEnd-1))->projection(), bestPlanes )
                  << endmsg;
        }
      }
    }
  }

  //== OK, itBest is the start...

  itBeg = itBest;
  itEnd = itLast;

  //== Add hits before/after
  PatFwdRegionCounter regions( itBeg, itEnd );
  double tolSide = .2;
  if ( 2 > regions.maxRegion() ) tolSide = 2.0;

  double minProj = (*itBeg)->projection() - tolSide;
  while ( itBeg != track.coordBegin() ) {
    PatFwdHit* hit = *(--itBeg); 
    if ( minProj > hit->projection() ) {
      ++itBeg;
      break;
    }
  }
  double maxProj = (*(itEnd-1))->projection() + tolSide;
  while ( itEnd != track.coordEnd() ) {
    PatFwdHit* hit = *itEnd;
    if ( maxProj < hit->projection() )  break;
    ++itEnd;
  }

  PatFwdPlaneCounter planeCount( itBeg, itEnd );
  if ( isDebug ) debug() << "... X fit, planeCount " << planeCount.nbDifferent()
                         << " size " << itEnd - itBeg << endmsg;


  // initial value;
  int minHit = (itEnd - itBeg) / 2;
  xAtReferencePlane( track, track.coords()[minHit], true );

  updateHitsForTrack( track, itBeg, itEnd );

  setRlDefault( track, itBeg, itEnd );

  bool first = true;
  double highestChi2 = 1.e10;

  while ( maxChi2 < highestChi2 && minPlanes <= planeCount.nbDifferent() ) {

    if (!fitXProjection( track, itBeg, itEnd, true )) {
      if( UNLIKELY( msgLevel(MSG::DEBUG) ) ) 
        debug() << "Abandon: Matrix not positive definite." << endmsg;
      return false;
    }

    highestChi2 = 0;
    PatFwdHit* worst = *itBeg;

    for ( itH = itBeg; itEnd != itH ; ++itH ) {
      PatFwdHit* hit = *itH;
      if ( !hit->isSelected() ) continue;
      double chi2 = chi2Hit( track, hit );
      if ( highestChi2 < chi2 ) {
        highestChi2 = chi2;
        worst = hit;
      }
    }

    if ( maxChi2 < highestChi2 ) {
      planeCount.removeHit( worst );
      worst->setSelected( false );
      if( UNLIKELY( msgLevel(MSG::VERBOSE) ) ) 
        verbose() << "--- Remove worst and retry, plane count = " << planeCount.nbDifferent() << endmsg;
    }



    if ( first && highestChi2 <  20 * maxChi2 ) {  // Add possibly removed hits
      first = false;
      bool hasNewHits = false;
      double minChi2 = maxChi2;
      if ( highestChi2 > maxChi2 ) {
        minChi2 = highestChi2 - 0.0001;  // down't find again the worst...
      }
      if( UNLIKELY( msgLevel(MSG::DEBUG) ) ) 
        debug() << "Collect all hits with chi2 below " << minChi2 << endmsg;
      PatFwdHits::iterator oldItBeg = itBeg;
      for ( itH = track.coordBegin(); track.coordEnd() != itH ; ++itH ) {
        PatFwdHit* hit = *itH;
        if ( hit->isIgnored() ) {
          hit->setSelected( false );
          continue;
        }

        bool close = minChi2 > chi2Hit( track, hit );
        if ( close ) {
          bool isNew = ( itH < oldItBeg || itH+1 > itEnd || !hit->isSelected() );
          if ( itH   < itBeg ) itBeg = itH;
          if ( itH+1 > itEnd ) itEnd = itH+1;
          if ( isNew ) hasNewHits = true;
        }
        hit->setSelected( close );
      }
      if ( hasNewHits ) {
        PatFwdPlaneCounter temp( itBeg, itEnd );
        planeCount = temp;  // update counter...
        if( UNLIKELY( msgLevel(MSG::DEBUG) ) ) 
          debug() << "   new plane count " << planeCount.nbDifferent() << endmsg;
        //setRlDefault( track, itBeg, itEnd );
        highestChi2 = 2*maxChi2; // force an iteration...
      } else {
      if( UNLIKELY( msgLevel(MSG::DEBUG) ) ) 
        debug() << "   no new hit added to the track." << endmsg;
      }
    }
  }

  return minPlanes <= planeCount.nbDifferent();
}




//=========================================================================
//  Fit only Y with the stereo hits.
//=========================================================================

bool PatFwdTool::fitStereoCandidate ( PatFwdTrackCandidate& track,
                                      double maxChi2, int minPlanes ) const {

  bool isDebug = msgLevel( MSG::DEBUG );
  PatFwdHits::iterator itH;

  //== get enough planes fired
  PatFwdPlaneCounter planeCount( track.coordBegin(), track.coordEnd() );
  if ( minPlanes > planeCount.nbDifferent() ) return false;

  if ( isDebug ) debug() << "+++ Stereo fit, planeCount " << planeCount.nbDifferent()
                         << " size " << track.coordEnd() - track.coordBegin() << endmsg;

  updateHitsForTrack( track, track.coordBegin(), track.coordEnd() );
  setRlDefault( track, track.coordBegin(), track.coordEnd() );

  double highestChi2 = 10*maxChi2;
  bool   ignoreX = true;

  while ( highestChi2 > maxChi2 ) {
    //== Improve X parameterisation
    if (!fitXProjection( track, track.coordBegin(), track.coordEnd(), false )) {
      if( UNLIKELY( msgLevel(MSG::DEBUG) ) ) 
        debug() << "Abandon: Matrix not positive definite." << endmsg;
	    return false;
    }

    for ( unsigned int kk = 0; 10 > kk; ++kk ) {
      PatFwdFitLine line;
      for ( itH = track.coordBegin(); track.coordEnd() != itH ; ++itH ) {
        PatFwdHit* hit = *itH;
        if ( !hit->isSelected() ) continue;
        if ( hit->hit()->layer() == 0 ||
             hit->hit()->layer() == 3) continue;
        double dz    = hit->z() - m_zReference;

        double dist  = - distanceForFit( track, hit ) / hit->hit()->dxDy();
        double w     = hit->hit()->weight();
        line.addPoint( dz, dist, w );
      }
      if (!line.solve()) {
        if( UNLIKELY( msgLevel(MSG::DEBUG) ) ) 
          debug() << "Abandon: Matrix not positive definite." << endmsg;
        return false;
      }
      double day = line.ax();
      double dby = line.bx();
      if( UNLIKELY( msgLevel(MSG::VERBOSE) ) ) 
        verbose() << "    day " << day << " dby " << dby << endmsg;

      track.updateParameters( 0., 0., 0., 0., day, dby );
      updateHitsForTrack( track, track.coordBegin(), track.coordEnd() );
      if ( fabs( day ) < 0.05 && fabs( dby ) < 0.00005 ) break;
    }

    highestChi2 = 0;
    PatFwdHit* worst = *track.coordBegin();

    for ( itH = track.coordBegin(); track.coordEnd() != itH ; ++itH ) {
      PatFwdHit* hit = *itH;
      if ( !hit->isSelected() ) continue;
      if ( ignoreX &&
           (hit->hit()->layer() == 0 ||
            hit->hit()->layer() == 3 )) continue;
      double chi2 = chi2Hit( track, hit );
      if ( highestChi2 < chi2 ) {
        highestChi2 = chi2;
        worst = hit;
      }
    }

    if ( highestChi2 > maxChi2 ) {
      planeCount.removeHit( worst );
      worst->setSelected( false );
      if( UNLIKELY( msgLevel(MSG::DEBUG) ) ) 
        debug() << " Remove hit and try again " << endmsg;
      //== Remove in one go all hits with bad contribution...
      if ( 1000. < highestChi2 ) {
        for ( itH = track.coordBegin(); track.coordEnd() != itH ; ++itH ) {
          PatFwdHit* hit = *itH;
          if ( !hit->isSelected() ) continue;
          if ( ignoreX &&
               (hit->hit()->layer() == 0 ||
                hit->hit()->layer() == 4)) continue;
          double chi2 = chi2Hit( track, hit );
          if ( 1000. < chi2 ) {
            planeCount.removeHit( hit );
            hit->setSelected( false );
          }
        }
      }

      if ( minPlanes > planeCount.nbDifferent() ) {
        if( UNLIKELY( msgLevel(MSG::DEBUG) ) ) 
          debug() << " Abandon: Only " << planeCount.nbDifferent() << " planes, min " << minPlanes
                  << " highestChi2 " << highestChi2 << endmsg;
        return false;
      }
    }
    //== If almost there, force one iteration with X fitting, at least...
    if ( highestChi2 < 2 * maxChi2 ) {
      if ( ignoreX ) {
        ignoreX = false;
        highestChi2 = 2.* maxChi2;
      }
    }
  }

  if( UNLIKELY( msgLevel(MSG::DEBUG) ) ) 
    debug() << ".. OK with " << planeCount.nbDifferent() << " planes, min " << minPlanes
            << " highestChi2 " << highestChi2 << endmsg;
  if ( minPlanes > planeCount.nbDifferent() ) return false;
  return true;
}


//=========================================================================
//  Parabolic fit to projection.
//=========================================================================

bool PatFwdTool::fitXProjection ( PatFwdTrackCandidate& track,
                                  PatFwdHits::iterator itBeg,
                                  PatFwdHits::iterator itEnd,
                                  bool onlyXPlanes  ) const {

  double errCenter = m_xMagnetTol + track.dSlope() * track.dSlope() * m_xMagnetTolSlope;
  for ( unsigned int kk = 0 ; 10 > kk ; ++kk ) {
    //= Fit the straight line, forcing the magnet centre. Use only position and slope.
    double dz   = m_zMagnet - m_zReference;
    double dist = distAtMagnetCenter( track );
    double w    = 1./errCenter;

    PatFwdFitParabola  parabola( dz, dist, w);
    PatFwdFitLine      line;
    if (m_withoutBField){
      line.addPoint(dz,dist,w);
    }

    for (PatFwdHits::iterator itH = itBeg; itEnd != itH ; ++itH ) {
      PatFwdHit* hit = *itH;
      if ( !hit->isSelected() ) continue;
      if ( onlyXPlanes && !(hit->hit()->layer()==0 || hit->hit()->layer()==3) ) continue;
      double dist2 = distanceForFit( track, hit );
      dz    = hit->z() - m_zReference ;
      w     = hit->hit()->weight();
      if (!m_withoutBField)
        parabola.addPoint( dz, dist2, w );
      else {
        line.addPoint(dz, dist2, w);
      }
    }

    double dax, dbx, dcx;
    
    if (!m_withoutBField){
      if (!parabola.solve()) return false;
      dax = parabola.ax();
      dbx = parabola.bx();
      dcx = parabola.cx();   
    } else {
      if (!line.solve()) return false;
      dax = line.ax();
      dbx = line.bx();
      dcx = 0.0;    
    }
    

    track.updateParameters( dax, dbx, dcx );

    if( UNLIKELY( msgLevel(MSG::VERBOSE) ) ) {
      verbose() << format( " dax %10.4f dbx%10.4f dcx %10.4f distCent %10.2f",
                           dax, dbx*1.e3, dcx*1.e6, distAtMagnetCenter( track ) ) << endmsg;
    }

    if ( fabs( dax ) < 5.e-3 &&
         fabs( dbx ) < 5.e-6 &&
         fabs( dcx ) < 5.e-9    ) break;  // wait until stable, due to OT.
  }
  return true;
}



//=========================================================================
//  Compute the chi2 per DoF of the track
//=========================================================================

double PatFwdTool::chi2PerDoF ( PatFwdTrackCandidate& track ) const {
  double totChi2 = 0.;
  int nDof = 0;
  bool hasStereo = false;

  //== Error component due to the contraint of the magnet centre
  double dist      = distAtMagnetCenter( track );
  double errCenter = m_xMagnetTol + track.dSlope() * track.dSlope() * m_xMagnetTolSlope;
  totChi2 = dist * dist / errCenter;
  if( UNLIKELY( msgLevel(MSG::DEBUG) ) ) 
    debug() << "   chi2 magnet center " << totChi2 << " dist " << dist << " err " << errCenter << endmsg;

  PatFwdHits::iterator itH;
  for ( itH = track.coordBegin(); track.coordEnd() != itH ; ++itH ) {
    PatFwdHit* hit = *itH;
    if ( !hit->isSelected() ) continue;
    double chi2 = chi2Hit( track, hit );
    totChi2 += chi2;
    nDof    += 1;
    if ( !(hit->hit()->layer()==0 || hit->hit()->layer()==3) ) hasStereo = true;
  }
  nDof -= 2;  // Fitted parabola in X, constraint to magnet centre
  if ( hasStereo ) nDof -= 1;  // Linear fit, magnet centre constraint

  totChi2 /= nDof;

  track.setChi2PerDoF( totChi2 );
  track.setNDoF( nDof );

  return totChi2;
}

//=========================================================================
//  Returns the q/p of the track
//=========================================================================
double PatFwdTool::qOverP ( const PatFwdTrackCandidate& track ) const {
  double qop(1.0/Gaudi::Units::GeV) ;
  double magscalefactor = m_magFieldSvc->signedRelativeCurrent() ;
  if( std::abs(magscalefactor) > 1e-6 ) {
    double bx = track.bx();
    double bx2 = bx * bx;
    double coef = ( m_momentumParams[0] +
		    m_momentumParams[1] * bx2 +
		    m_momentumParams[2] * bx2 * bx2 +
		    m_momentumParams[3] * bx * track.slX() +
		    m_momentumParams[4] * track.slY2() +
		    m_momentumParams[5] * track.slY2() * track.slY2() );
    double proj = sqrt( ( 1. + track.slX2() + track.slY2() ) / ( 1. + track.slX2() ) );
    qop = track.dSlope() / ( coef * Gaudi::Units::GeV * proj * magscalefactor*(-1)) ;
  }
  return qop ;
}

//=========================================================================
//  Set the RL flag for OT candidates, if obvious.
//=========================================================================

void PatFwdTool::setRlDefault( PatFwdTrackCandidate& track,
                               PatFwdHits::iterator itBeg,
                               PatFwdHits::iterator itEnd  ) const {
  PatFwdHits::iterator itH;
  PatFwdHits temp;
  for ( int planeCode = 0 ;12 > planeCode  ; ++planeCode ) {
    temp.clear();
    for ( itH = itBeg; itEnd != itH; ++itH ) {
      PatFwdHit* hit = (*itH);
      if ( planeCode != hit->planeCode() ) continue;
      hit->setRlAmb( 0 );   // default
      //if ( !hit->isSelected() ) continue;
      if ( hit->hit()->type() != Tf::RegionID::OT ) continue;    // IT ->no ambiguity!
      temp.push_back( hit );
    }
    if ( 2 > temp.size() ) continue;   // no RL solved if at most one hit

    std::sort( temp.begin(), temp.end(), Tf::increasingByX<PatForwardHit>() ); 

    if( UNLIKELY( msgLevel(MSG::DEBUG) ) ) 
      debug() << "-- Hit of plane " << planeCode << endmsg;

    double prevDistM = 10.;
    double prevDistP = 10.;
    PatFwdHit* prevHit = *(temp.begin());

    for ( itH = temp.begin(); temp.end() != itH; ++itH ) {
      PatFwdHit* hit = (*itH);
      hit->setRlAmb( -1 );
      double distM = distanceHitToTrack( track, hit );
      hit->setRlAmb( +1 );
      double distP = distanceHitToTrack( track, hit );
      hit->setRlAmb( 0 );

      double minDist = 0.3;
      int    vP = 0;
      int    vC = 0;

      if ( fabs( distM - prevDistP ) < minDist ) {
        minDist =  fabs( distM - prevDistP );  vP = +1;  vC = -1;
      }
      if ( fabs( distP - prevDistM ) < minDist ) {
        minDist =  fabs( distP - prevDistM );  vP = -1;  vC = +1;
      }
      prevHit->setRlAmb( vP );
      hit->setRlAmb( vC );

      prevHit   = hit;
      prevDistP = distP;
      prevDistM = distM;

      if( UNLIKELY( msgLevel(MSG::DEBUG) ) ) {
        debug() << format( "  z%10.2f x%10.2f region%2d P%2d N%2d distM%7.3f distP%7.3f minDist%7.3f vC%3d vP%3d",
                           hit->z(), hit->x(), hit->hit()->region(),
                           hit->hasPrevious(), hit->hasNext(), distM, distP, minDist, vC, vP ) << endmsg;
      }
    }
  }
}
//=============================================================================


void PatFwdTool::updateHitsForTrack ( const PatFwdTrackCandidate& track,
                                      PatFwdHits::iterator itBeg,
                                      PatFwdHits::iterator itEnd ) const {
  for ( PatFwdHits::iterator it = itBeg; itEnd != it; ++it ) {
    double sly = track.ySlope( (*it)->z() - m_zReference );
    double y0  = track.y( (*it)->z() - m_zReference ) - (*it)->z() * sly;
    updateHitForTrack( (*it), y0, sly);
  }
}

