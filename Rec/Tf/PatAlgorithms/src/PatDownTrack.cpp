// $Id: PatDownTrack.cpp,v 1.3 2009-04-07 15:30:40 ocallot Exp $
// Include files 

// local
#include "PatDownTrack.h"

//-----------------------------------------------------------------------------
// Implementation file for class : PatDownTrack, from Pat/PatKShort package
//
// 2007-10-18 : Olivier Callot
//-----------------------------------------------------------------------------

//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
PatDownTrack::PatDownTrack( LHCb::Track* tr, 
                            double zTT,
                            std::vector<double> magnetParams,
                            std::vector<double> momentumParams,
                            std::vector<double> yParams,
                            double errZMag,
                            double magnetScale) {
  m_zTT    = zTT;
  m_magPar = magnetParams;
  m_momPar = momentumParams;
  m_magnetScale = magnetScale;
  
  m_track = tr;

  int nbIT = 0;
  for ( std::vector<LHCb::LHCbID>::const_iterator itId = tr->lhcbIDs().begin();
        tr->lhcbIDs().end() != itId; ++itId ) {
    if ( (*itId).isIT() ) nbIT++;
  }

  m_state = &tr->closestState( 10000. );
  
  double zMagnet = 
    m_magPar[0] + 
    m_magPar[1] * m_state->ty() * m_state->ty() +
    m_magPar[2] * m_state->tx() * m_state->tx();

  double dz      = zMagnet - m_state->z();
  double xMagnet = m_state->x() + dz * m_state->tx();
  m_slopeX       = xMagnet / zMagnet;
  
  double dSlope2 = (m_slopeX - m_state->tx())*(m_slopeX - m_state->tx());

  double by = m_state->y() / ( m_state->z() + 
                               ( yParams[0] * fabs(m_state->ty()) * zMagnet + yParams[1] )* dSlope2  );
  if ( 4 < nbIT ) by = m_state->ty(); 
  m_slopeY = by * ( 1. + yParams[0] * fabs(by) * dSlope2 );
 
  double yMagnet = m_state->y() + dz * by - yParams[1] * by * dSlope2;
  m_magnet = Gaudi::XYZPoint( xMagnet, yMagnet, zMagnet );

  m_errXMag = errZMag * fabs( m_slopeX - m_state->tx() ) + 2.;
  m_errYMag = 15.;
  if ( 4 < nbIT ) m_errYMag = 4.;

  //=== Save for reference
  m_magnetSave = m_magnet;
  m_slopeXSave = m_slopeX;
  m_displX     = 0.;
  m_displY     = 0.;

  //=== Initialize all other data members
  m_moment     = 0.;
  m_chisq      = 0.;

  m_curvature  = 1.7e-5 * ( m_state->tx() - m_slopeX );
}
//=============================================================================
