// $Id: LSAdaptPV3DFitter.cpp,v 1.8 2010-01-27 09:52:17 rlambert Exp $
// Include files
// from Gaudi
#include "GaudiKernel/ToolFactory.h"
// from Event
#include "Event/Track.h"
#include "Event/State.h"
#include "Event/RecVertex.h"
// Local
#include "LSAdaptPV3DFitter.h"

DECLARE_TOOL_FACTORY(LSAdaptPV3DFitter);

//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
LSAdaptPV3DFitter::LSAdaptPV3DFitter(const std::string& type,
                                 const std::string& name,
                                 const IInterface* parent)
  : GaudiTool(type,name,parent),
    m_minTr(0),
    m_Iterations(0),
    m_maxIP2PV(0.),
    m_maxDeltaZ(0.),
    m_minTrackWeight(0.),
    m_TrackErrorScaleFactor(0.),
    m_x0MS(0.),
    m_scatCons(0.),
    m_myZero(1E-12),
    m_pvTracks(0)
{
  declareInterface<IPVFitter>(this);
  // Minimum number of tracks in vertex
  declareProperty("MinTracks", m_minTr = 5);
  // Number of iterations
  declareProperty("Iterations", m_Iterations = 20);
  // Maximum IP of a track to accept track
  declareProperty("maxIP2PV", m_maxIP2PV = 2.0 * Gaudi::Units::mm);
  // Fit convergence condition
  declareProperty("maxDeltaZ", m_maxDeltaZ = 0.001 * Gaudi::Units::mm);
  // Minimum Tukey's weight to accept a track
  declareProperty("minTrackWeight", m_minTrackWeight = 0.00001);
  declareProperty( "x0MS"            , m_x0MS          =  0.02          );
  declareProperty("TrackErrorScaleFactor", m_TrackErrorScaleFactor = 1.0 );
  declareProperty("CalculateMultipleScattering", m_CalculateMultipleScattering = true );
  declareProperty("AddMultipleScattering", m_AddMultipleScattering = true );
  declareProperty("trackMaxChi2", m_trackMaxChi2 = 9.);
  // Max chi2 tracks to be removed from next PV search
  declareProperty("trackMaxChi2Remove", m_trackMaxChi2Remove = 25.);
  // Min number of iterations
  declareProperty("minIter", m_minIter = 5);
  declareProperty("zVtxShift", m_zVtxShift = 0.0 );
}

//=========================================================================
// Initialize
//=========================================================================
StatusCode LSAdaptPV3DFitter::initialize()
{
  StatusCode sc = GaudiTool::initialize();
  if(!sc) return sc;

  double X0 = m_x0MS;
  m_scatCons = (13.6*sqrt(X0)*(1.+0.038*log(X0))) / ( 3.0 * Gaudi::Units::GeV );

  m_trackChi = std::sqrt(m_trackMaxChi2);

  return StatusCode::SUCCESS;
}

//=============================================================================
// Destructor
//=============================================================================
LSAdaptPV3DFitter::~LSAdaptPV3DFitter() {}

//=============================================================================
// Least square adaptive fitting method
//=============================================================================
StatusCode LSAdaptPV3DFitter::fitVertex(const Gaudi::XYZPoint seedPoint,
                                        std::vector<const LHCb::Track*>& rTracks,
                                        LHCb::RecVertex& vtx, 
                                        std::vector<const LHCb::Track*>& tracks2remove)
{
  if(msgLevel(MSG::DEBUG)) debug() << "================Test==================" << endmsg;

  tracks2remove.clear();
  
  Gaudi::XYZPoint xyzvtx = seedPoint;
  // prepare tracks
  m_pvTracks.clear();

  std::vector<const LHCb::Track*>::iterator itr;
  for(itr = rTracks.begin(); itr != rTracks.end(); itr++) {
    const LHCb::Track* track = *itr;
    if ( !(track->hasVelo()) ) continue;
    addTrackForPV(track, m_pvTracks, seedPoint);
  }

  if((int) m_pvTracks.size() < m_minTr) {
    if(msgLevel(MSG::DEBUG)) {
      verbose() << "Too few tracks to fit PV" << endmsg;
    }
    return StatusCode::FAILURE;
  }

  Gaudi::SymMatrix3x3 hess;
  ROOT::Math::SVector<double,3> d0vec;

  bool converged = false;
  int nbIter = 0;
  while( (nbIter < m_minIter) || (!converged && nbIter < m_Iterations) )
  {
    ++nbIter;

    for(int i = 0; i < 3; i++) {
      d0vec[i] = 0.0;
      for(int j = 0; j < 3; j++) {
        hess(i,j) = 0.0;
      }
    }

    int ntrin=0;
    for(PVTracks::iterator pvTrack = m_pvTracks.begin(); pvTrack != m_pvTracks.end(); pvTrack++) {
      double unitVectStd[3];
      unitVectStd[0] = pvTrack->unitVect.x();
      unitVectStd[1] = pvTrack->unitVect.y();
      unitVectStd[2] = pvTrack->unitVect.z();
      Gaudi::XYZVector ipVector =
        impactParameterVector(xyzvtx, pvTrack->stateG.position(), pvTrack->unitVect);
      pvTrack->vd0 = ipVector;
      pvTrack->d0 = ipVector.Mag2();
      if(pvTrack->err2d0<m_myZero) 
      {
        if(msgLevel(MSG::DEBUG)) debug() << "fitVertex: pvTrack error is too small for computation" << endmsg;
        pvTrack->chi2=pvTrack->d0/m_myZero;
      }
      else pvTrack->chi2 = pvTrack->d0/ pvTrack->err2d0;

      pvTrack->weight = getTukeyWeight(pvTrack->chi2, nbIter);

      if (  pvTrack->weight > m_minTrackWeight ) ntrin++;

      double invs = 1.0;
      if(pvTrack->err2d0<m_myZero) 
      {
        invs = (2.0 / m_myZero) * pvTrack->weight;
      }
      else invs = (2.0 / pvTrack->err2d0) * pvTrack->weight;

      double vd0Std[3];
      vd0Std[0] = ipVector.x();
      vd0Std[1] = ipVector.y();
      vd0Std[2] = ipVector.z();
      for(int k = 0; k < 3; k++) {
        double vd0 = 0.;
        for (int l = 0; l < 3; l++) {
          double delta_kl = (k == l) ? 1.0 : 0.0;
          double val = delta_kl - unitVectStd[k] * unitVectStd[l];
          vd0 += vd0Std[l] * val * invs;
          if(l <= k) {
            hess(k,l) += val * invs;
          }
        }
        d0vec[k] += vd0;
      }

    }

    // check nr of tracks that entered the fit
    if(ntrin < 2) {
      return StatusCode::FAILURE;
    }
    
    int fail;
    hess.Inverse(fail);
    if (0 != fail) {
      warning() << "Error inverting hessian matrix" << endmsg;
      return StatusCode::FAILURE;
    } else {
      hess.InvertFast();
    }
    ROOT::Math::SVector<double,3> delta;
    delta = hess * d0vec;

    if(msgLevel(MSG::DEBUG)) {
      debug() <<  " delta " <<  delta[0] << " " <<  delta[1] << " " <<  delta[2]  << endmsg;
    }

    xyzvtx.SetX(xyzvtx.X()+delta[0]);
    xyzvtx.SetY(xyzvtx.Y()+delta[1]);
    xyzvtx.SetZ(xyzvtx.Z()+delta[2]);

    if(fabs(delta[2]) < m_maxDeltaZ) {
      converged = true;
    }
  } // end iteration loop
  if(!converged) return StatusCode::FAILURE;
  // Check number of tracks and calculate chi2
  int outTracks = 0;
  int nDoF = -3;
  double chi2 = 0.0;
  for(PVTracks::iterator iFitTr = m_pvTracks.begin(); iFitTr != m_pvTracks.end(); iFitTr++) {
    if( iFitTr->weight > m_minTrackWeight) {
      outTracks++;
      chi2 += iFitTr->chi2 * iFitTr->weight ;
      nDoF += 2;
    }
  }

  if(outTracks < m_minTr) {
    if(msgLevel(MSG::DEBUG)) {
      debug() << "Too few tracks after PV fit" << endmsg;
    }
    return StatusCode::FAILURE;
  }

  // accepted PV
  vtx.setChi2(chi2);
  vtx.setNDoF(nDoF);

  xyzvtx.SetZ(xyzvtx.z() + m_zVtxShift);
  vtx.setPosition(xyzvtx);
  hess = hess * 2.0;
  vtx.setCovMatrix(hess);
  // Set tracks
  vtx.clearTracks();
  for(PVTracks::iterator iFitTr = m_pvTracks.begin(); iFitTr != m_pvTracks.end(); iFitTr++) {
    if( iFitTr->weight > m_minTrackWeight) {
      vtx.addToTracks( iFitTr->refTrack, (float) iFitTr->weight );
    }
  }
  vtx.setTechnique(LHCb::RecVertex::Primary);

  // fill tracks to remove from next PV search
  for(PVTracks::iterator iFitTr = m_pvTracks.begin(); iFitTr != m_pvTracks.end(); iFitTr++) {
    if( iFitTr->chi2 < m_trackMaxChi2Remove) {
      tracks2remove.push_back( iFitTr->refTrack);
    }
  }

  if(msgLevel(MSG::DEBUG)) {
    debug() << "Vertex" << endmsg;
    debug() << "===================" << endmsg;
    debug() << format( "chi2/ndof %7.2f  %7.2f / %5d xyz %7.3f %7.3f %7.3f  err xyz: %7.4f %7.4f %7.4f", 
               vtx.chi2()/vtx.nDoF(), vtx.chi2(), vtx.nDoF(),
               vtx.position().X(), vtx.position().Y(), vtx.position().Z(),
	       sqrt(vtx.covMatrix()(0,0)), sqrt(vtx.covMatrix()(1,1)), sqrt(vtx.covMatrix()(2,2)) ) 
            << endmsg << endmsg;
    debug() << "Tracks in this vertex" << endmsg;
    debug() << "---------------------" << endmsg;
    for(PVTracks::iterator iFitTr = m_pvTracks.begin(); iFitTr != m_pvTracks.end(); iFitTr++) {
      int invx = 0;
      if( iFitTr->weight > m_minTrackWeight) invx = 1;
        debug() << format( "chi2 d0 w zc %7.2f %7.2f %7.2f %7.2f %3d",
                   iFitTr->chi2, iFitTr->d0, iFitTr->weight, iFitTr->refTrack->firstState().z(),invx) << endmsg;

    }
  }
  return StatusCode::SUCCESS;
}

//=============================================================================
// Add track for PV
//=============================================================================
void LSAdaptPV3DFitter::addTrackForPV(const LHCb::Track* pvtr,
                                      PVTracks& pvTracks, const Gaudi::XYZPoint& seed)
{
  // Add new PVTrack
  PVTrack pvtrack;
  pvtrack.isUsed = false;
  pvtrack.stateG = pvtr->firstState();
  pvtrack.unitVect = pvtrack.stateG.slopes().Unit();
  pvtrack.d0 = ( (impactParameterVector(seed,  pvtrack.stateG.position(),  pvtrack.unitVect)).Mag2() );
  if(pvtrack.d0 > m_maxIP2PV*m_maxIP2PV) return;
  pvtrack.err2d0 = err2d0(pvtr, seed);
  pvtrack.chi2 = 0;
  //does it make sense to add such an error... why is chi2 0 for this case? it should be a very big number
  if(pvtrack.err2d0 > m_myZero) pvtrack.chi2 = (pvtrack.d0)/pvtrack.err2d0;
  else
  {
    if(msgLevel(MSG::DEBUG)) debug() << "addTrackForPV: pvTrack error is too small for computation" << endmsg;
    pvtrack.chi2=pvtrack.d0/m_myZero;
  }
  // Keep reference to the original track
  pvtrack.refTrack = pvtr;
  pvTracks.push_back(pvtrack);
  return;
}

//=============================================================================
//  err2d0
//=============================================================================
double LSAdaptPV3DFitter::err2d0(const LHCb::Track* track, const Gaudi::XYZPoint& seed) {

  // fast parametrization of track parameters

  //  double x     = track->firstState().x();
  //  double y     = track->firstState().y();
  double z     = track->firstState().z();
  double tx    = track->firstState().tx();
  double ty    = track->firstState().ty();

  double ex2 = track->firstState().errX2();
  double ey2 = track->firstState().errY2();

  double tr2   = tx*tx+ty*ty;
  double fcos2 = 1./(1.+tr2);
  double fsin2 = tr2/(1.+tr2);
  double err2 = (ex2+ey2)*fcos2;

  bool backward = track->checkFlag( LHCb::Track::Backward);

  double z_seed = seed.z();
  double dz_pv = z - z_seed;
  if ( backward ) dz_pv *= -1.;
  double et2 = 0.;
  if (dz_pv > 0. ) {
    double etx2 = track->firstState().errTx2(); 
    double ety2 = track->firstState().errTy2();
    et2 = dz_pv*dz_pv*(etx2+ety2);
    err2 += et2; 
  }

  // add multiple scattering if track errors do not contain it (Hlt case)
  double corr2 = 0.;
  double l_scat2 = 0.;
  if( m_AddMultipleScattering ) {
    if ( m_CalculateMultipleScattering) {
        // mutliple scattering from RF foil at r = 10 mm
        double r_ms = 10.;
        l_scat2 = r_ms*r_ms/fsin2;
        corr2 = m_scatCons*m_scatCons*l_scat2;
        err2 += corr2;
    }
    else {
      err2 += 0.05*0.05;
    }
  }

  return (err2);
}


Gaudi::XYZVector LSAdaptPV3DFitter::impactParameterVector(const Gaudi::XYZPoint& vertex,
                                        const Gaudi::XYZPoint& point,
                                        const Gaudi::XYZVector& direction) {
  Gaudi::XYZVector udir = direction.unit();
  Gaudi::XYZVector distance = point - vertex;
  return udir.Cross(distance.Cross(udir));
}

//=============================================================================
// Get Tukey's weight
//=============================================================================
double LSAdaptPV3DFitter::getTukeyWeight(double trchi2, int iter)
{
  if (iter<1 ) return 1.;

  double ctrv = m_trackChi * (m_minIter -  iter);
  if (ctrv < m_trackChi) ctrv = m_trackChi;
  double cT2 = trchi2 / (ctrv*ctrv*m_TrackErrorScaleFactor*m_TrackErrorScaleFactor);
  double weight = 0.;
  if(cT2 < 1.) {
    weight = 1. - cT2;
    weight = weight * weight;
  }
  return weight;
}
