// $Id: L0ConfExtrapolator.cpp,v 1.1 2008-03-20 14:15:47 albrecht Exp $
// Include files 

#include <cmath>

// from Gaudi
#include "GaudiKernel/DeclareFactoryEntries.h" 

#include "LineHypothesis.h"
#include "ParabolaHypothesis.h"
#include "FwdHypothesis.h"

//boost
#include <boost/assign/list_of.hpp>

// local
#include "L0ConfExtrapolator.h"

//-----------------------------------------------------------------------------
// Implementation file for class : L0ConfExtrapolator
//
// 2008-01-18 : Johannes Albrecht
//-----------------------------------------------------------------------------

// Declaration of the Tool Factory
DECLARE_TOOL_FACTORY( L0ConfExtrapolator );


//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
L0ConfExtrapolator::L0ConfExtrapolator( const std::string& type,
                                        const std::string& name,
                                        const IInterface* parent )
  : GaudiTool ( type, name , parent )
{
  declareInterface<IL0ConfExtrapolator>(this);

  //for getParabolaHypothesis
  declareProperty( "curvFactor" , m_curvFactor = 42. );
  
  // for getFwdHypothesis
  declareProperty( "zRef"       , m_zRef    = 8620. * Gaudi::Units::mm );
  declareProperty( "zMagnet"    , m_zMagnet = 5080. * Gaudi::Units::mm );
  declareProperty( "axParam"    , m_axParam = -153. );
  declareProperty( "cxParam"    , m_cxParam = 2.e-5 );
  declareProperty( "dxParam"    , m_dxParam = -3e-4 );
  declareProperty( "byParam"    , m_byParam = -.35  );
  declareProperty("fwdSigmaX2", m_fwdSigmaX2 = boost::assign::list_of(121.)(144.)(625.)(225.)(1225.) );
  declareProperty("fwdSigmaY2", m_fwdSigmaY2 = boost::assign::list_of(196.)(289.)(784.)(400.)(2025.) );
  
  //kick constants for ECal seeding
  declareProperty("ecalKick" , m_ecalKick = 1536443. );
  
  //kick constants for HCal (+ECal) seeding
  declareProperty("hCalEcalKick1" , m_HECalKick[0] = 11.5 );
  declareProperty("hCalEcalKick2" , m_HECalKick[1] = 1.585e6 );
  declareProperty("hCalKick1" , m_HCalKick[0] = 17. );
  declareProperty("hCalKick2" , m_HCalKick[1] = 1.712e6 );

}
//=============================================================================
// Destructor
//=============================================================================
L0ConfExtrapolator::~L0ConfExtrapolator() {} 

//=============================================================================
void L0ConfExtrapolator::muon2T( const LHCb::Track& muonTrack,
                                 LHCb::State& stateAtT ) const
{
  //muon track has x,y,tx,ty information 
  //perform linear extrapolation to T3
  const LHCb::State& aState = muonTrack.firstState();
  double dz = aState.z() - zEndT3;
   
  stateAtT.setState(aState.x() - dz * aState.tx(),
                    aState.y() - dz * aState.ty(),
                    zEndT3,
                    aState.tx(), aState.ty(), aState.qOverP());
  stateAtT.setCovariance( aState.covariance() );
}

// this private method does most of the dirty work for hcal2T and ecal2T
void L0ConfExtrapolator::calo2T( const LHCb::State& aState, double xkick,
                                 LHCb::State& statePosAtT, LHCb::State& stateNegAtT ) const
{
  double x = aState.x(), y = aState.y(), z = aState.z();
  // x and y central position are on line connecting calo with origin
  double xT3 = x * zEndT3 / z, yT3 = y * zEndT3 / z;
  // assume neutral particle slopes, correct for x-kick in T below
  double tx = x / z, ty = y / z;
  double dz = zEndT3 - z;

  statePosAtT.setState(xT3 - xkick, yT3, zEndT3, 
                       tx - xkick / dz, ty, aState.qOverP()); 
  stateNegAtT.setState(xT3 + xkick, yT3, zEndT3,
                       tx + xkick / dz, ty, aState.qOverP());
  statePosAtT.setCovariance( aState.covariance() );
  stateNegAtT.setCovariance( aState.covariance() );
}

void L0ConfExtrapolator::ecal2T( const LHCb::Track& ecalTrack, 
                                 LHCb::State& statePosAtT,
                                 LHCb::State& stateNegAtT ) const
{
  // kick in x depends on momentum
  double xkick = m_ecalKick * ecalTrack.firstState().qOverP();
  calo2T(ecalTrack.firstState(), xkick, statePosAtT, stateNegAtT);
}

void L0ConfExtrapolator::hcal2T( const LHCb::Track& hcalTrack,
                                 LHCb::State& statePosAtT,
                                 LHCb::State& stateNegAtT ) const
{
  // }
  double z = hcalTrack.firstState().z();
  if (msgLevel(MSG::DEBUG)) {
    debug() << "Seeding using ";
    if (13360.0 > z)
      debug() << "ECal and HCal" << endreq;
    else
      debug() << "HCal only" << endreq;
  }
  // kick in x depends on momentum
  // different constants for HCal-only and ECal/HCal seeding
  double xkick = hcalTrack.firstState().qOverP();
  if (13360. > z) xkick = m_HECalKick[0] + m_HECalKick[1] * xkick;
  else xkick = m_HCalKick[0] + m_HCalKick[1] * xkick;
  calo2T(hcalTrack.firstState(), xkick, statePosAtT, stateNegAtT);
}

ParabolaHypothesis L0ConfExtrapolator::getParabolaHypothesis(const LHCb::Track& seedTrack, 
                                                             double nSigma ) const
{
  return getParabolaHypothesis( seedTrack.firstState(), nSigma );
}


ParabolaHypothesis L0ConfExtrapolator::getParabolaHypothesis(const LHCb::State& aState, 
                                                             double nSigma ) const
{
  double z = aState.z();
  
  double ax = aState.qOverP() / m_curvFactor;
  double bx = aState.tx() - 2. * z * ax;
  double cx = aState.x() - z * (bx + ax * z);
  
  return ParabolaHypothesis(aState.ty(), aState.y() - aState.ty() * z,
                            ax, bx, cx, nSigma * std::sqrt(aState.errX2()),
                            nSigma * std::sqrt(aState.errY2()));
}

FwdHypothesis L0ConfExtrapolator::getFwdHypothesis( const LHCb::Track& veloTrack,
                                                    const LHCb::Track& caloTrack,
                                                    double nSigma ) const
{
  //Parametrization to get Fwd hypothesis from velo track + calo point
  //from Olivier Callot
  const LHCb::State& caloState = caloTrack.firstState();
  // end of Velo, to extrapolate.
  const LHCb::State& veloState = veloTrack.closestState( 1000. );
  
  // predict center of magnet x position from velo seed
  double xMag = veloState.x() + ( m_zMagnet - veloState.z() ) * veloState.tx();

  // zMagnet such that slope computed this way is the slope at zRef
  double bx = ( caloState.x() - xMag ) / ( caloState.z() - m_zMagnet );
  double dSlope = bx - veloState.tx();

  // parametric correction to simple straight line bending
  double ax = xMag + bx * ( m_zRef - m_zMagnet ) + m_axParam * dSlope;
  double cx = m_cxParam * dSlope;
  double dx = m_dxParam * cx;
  
  double by = (caloState.y() - veloState.y()) / ( caloState.z() - veloState.z() );
  by = by + m_byParam * dSlope * dSlope * veloState.ty();
  double ay = caloState.y() + ( m_zRef - caloState.z() ) * by;

  int reg = getCaloRegion(caloState.x(),caloState.y(),caloState.z());
  
  if ( reg < 0 || reg > 4 ) {
    error()<<"region not meaningful .. "<<endmsg;
    return FwdHypothesis(0,0,0,0,0,0,0,0,0);
  }
  
  return FwdHypothesis( ax, bx, cx,dx,
                        ay, by,
                        nSigma*std::sqrt(m_fwdSigmaX2[reg]), nSigma*std::sqrt(m_fwdSigmaY2[reg]),
                        m_zRef);
}

/*
 *   this helper function checks for the physical regions of ECal 
 *   and HCal respectively, depending on z
 */
int L0ConfExtrapolator::getCaloRegion(float stateX, 
                                      float stateY, 
                                      float stateZ ) const
{
  float x = std::abs(stateX);
  float y = std::abs(stateY);
  // two small tables to look up HCAL/ECAL region boundaries
  static const float xpos[] = { 650., 1940., 3880., 2101., 4202., -1. };
  static const float ypos[] = { 650., 1450., 2420., 1838., 3414., -1. };

  // depending on ECAL/HCAL+ECAL seeding, the starting offset into the
  // table is different
  int reg = (13360. > stateZ)?0:3;

  while (xpos[reg] > 0.0) {
    if (x < xpos[reg] && y < ypos[reg]) break;
    ++reg;
  }
 
  if (msgLevel(MSG::DEBUG)) {
    if (13360. > stateZ)
      debug() << "HCAL + ECAL Seeding" << endreq;
    else
      debug() << "HCAL Seeding" << endreq;
    if (-1. == xpos[reg])
      debug() << "CALO: not in calo!?!,  x,y = "
              << stateX << " , " << stateY << endreq;

    debug() << "HCAL+ ECAL section 0 : x,y = " << stateX << " , " << stateY
            << endreq
            << " .................................   region is: " << reg
            << endreq;
  }
  if (-1. == xpos[reg]) reg = -1;
  
  return reg;
}
