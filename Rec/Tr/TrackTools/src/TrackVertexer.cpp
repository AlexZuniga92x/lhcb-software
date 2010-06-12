 // $Id: TrackVertexer.cpp,v 1.6 2009-07-09 09:47:52 wouter Exp $
// Include files 

// from Gaudi
#include "GaudiKernel/ToolFactory.h" 

// local
#include "TrackVertexer.h"

// from Event
#include "Event/TwoProngVertex.h"
#include "Event/RecVertex.h"
#include "TrackKernel/TrackStateVertex.h"

//-----------------------------------------------------------------------------
// Implementation file for class : TrackVertexer
//
// 2007-11-07 : Wouter HULSBERGEN
//-----------------------------------------------------------------------------

// Declaration of the Tool Factory
DECLARE_TOOL_FACTORY( TrackVertexer );

//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
TrackVertexer::TrackVertexer( const std::string& type,
                              const std::string& name,
                              const IInterface* parent )
  : GaudiTool ( type, name , parent ),
    m_maxNumIter(10),
    m_maxDChisq(0.01),
    m_computeMomCov(true),
    m_discardFailedFits(false)
{
  declareInterface<ITrackVertexer>(this);
  declareProperty("MaxNumberOfIterations",m_maxNumIter) ;
  declareProperty("MaxDeltaChisqForConvergence",m_maxDChisq) ;
  declareProperty("ComputeMomentumCovariance",m_computeMomCov) ;
  declareProperty("DiscardFailedFits",m_discardFailedFits) ;
}
//=============================================================================
// Destructor
//=============================================================================
TrackVertexer::~TrackVertexer() {} 

LHCb::TwoProngVertex*
TrackVertexer::fit(const LHCb::State& stateA, const LHCb::State& stateB) const
{
  LHCb::TwoProngVertex* rc(0) ;
  StateContainer states(2) ;
  states.front() = &stateA ;
  states.back() = &stateB ;
  LHCb::TrackStateVertex vertex( states, m_maxDChisq, m_maxNumIter ) ;
  if( vertex.fitStatus() == LHCb::TrackStateVertex::FitSuccess || !m_discardFailedFits) {
    // create a two prong vertex
    rc = new LHCb::TwoProngVertex(vertex.position()) ;
    rc->setChi2(vertex.chi2()) ;
    rc->setCovMatrix( vertex.covMatrix() ) ;
    rc->setNDoF(vertex.nDoF()) ;
    rc->momA() = vertex.mom(0) ;
    rc->momB() = vertex.mom(1) ;
    if( m_computeMomCov ) {
      rc->momcovA()    = vertex.momCovMatrix(0) ;
      rc->momposcovA() = vertex.momPosCovMatrix(0) ;
      rc->momcovB()    = vertex.momCovMatrix(1) ;
      rc->momposcovB() = vertex.momPosCovMatrix(1) ;
      rc->mommomcov()  = vertex.momMomCovMatrix(0,1) ;
    }
  }
  return rc ;
}

LHCb::RecVertex*
TrackVertexer::fit(const StateContainer& tracks) const
{
  LHCb::RecVertex* recvertex(0) ;
  
  if( tracks.size()>=2 ) {
    LHCb::TrackStateVertex vertex( tracks, m_maxDChisq, m_maxNumIter ) ;
    if( vertex.fitStatus() == LHCb::TrackStateVertex::FitSuccess || !m_discardFailedFits) {
      recvertex = new LHCb::RecVertex(vertex.position()) ;
      recvertex->setChi2AndDoF( vertex.chi2(), vertex.nDoF() ) ;
      recvertex->setCovMatrix( vertex.covMatrix() ) ;
    }
  }
  return recvertex ;
}

LHCb::RecVertex*
TrackVertexer::fit(const TrackContainer& tracks) const
{
  // TODO: THIS IS NOT EXTRAPOLATING YET!!
  // Warning("TrackVertexer::fit(const TrackContainer& tracks) is not yet extrapolating!",1) ;

  // get the states. this needs to becomes smarter.
  StateContainer states ;
  states.reserve( tracks.size() ) ;
  for( TrackContainer::const_iterator itrack = tracks.begin() ;
       itrack != tracks.end(); ++itrack ) 
    states.push_back( &((*itrack)->firstState()) ) ; 
  
  // fit the states
  LHCb::RecVertex* recvertex = fit( states ) ;
  
  // add the tracks
  if( recvertex ) 
    for( TrackContainer::const_iterator itrack = tracks.begin() ;
	 itrack != tracks.end(); ++itrack ) 
      recvertex->addToTracks( *itrack ) ;
  
  return recvertex ;
}

namespace {
  inline Gaudi::Vector3 transform( const Gaudi::XYZVector& vec)
  {
    return Gaudi::Vector3(vec.X(),vec.Y(),vec.Z()) ;
  }
}

bool
TrackVertexer::computeDecayLength(const LHCb::TwoProngVertex& vertex,
				  const LHCb::RecVertex& pv,
				  double& chi2,
				  double& decaylength,
				  double& decaylengtherr) const
{
  // This calculation is basically a 1-iteration beamspot fit. The
  // constraint is 
  //
  //    r = x - lambda p/|p| - xbs
  // 
  // where x and p are the position of the decay vertex of the
  // candidate and its momentum, lambda is the decaylength and xbs
  // the position of the beamspot. The covariance in the constraint
  // is
  //
  //    V = Vbs + Vxx - a * Vxp - a Vxp^T + a^2 * Vpp
  //
  // where a=lambda/|p|^2. It needs an initial estimate for the
  // flightlength, for which we simply take the projection of deltaX
  // on the direction. We now minimize  the chisquare contribution 
  // 
  //     chi^2 = r^T V^{-1} r
  //
  // for lambda.


  const Gaudi::XYZPoint pos = vertex.position() ;
  // I really only need p3, not p4
  Gaudi::SymMatrix6x6 cov6  = vertex.covMatrix6x6() ;
  Gaudi::XYZVector p3 = vertex.p3() ;
  Gaudi::Vector3 dx = transform(pos - pv.position()) ;
  double p3mag = p3.R() ; 
  Gaudi::Vector3 dir = transform(p3.Unit()) ;
  Gaudi::SymMatrix3x3 W = pv.covMatrix() ;
    
   //double a = (ROOT::Math::Transpose(dir)*dx)/p3mag  ;
  double a = ROOT::Math::Dot(dir,dx)/p3mag  ;
  for(size_t row=0; row<3; ++row)
    for(size_t col=0; col<=row; ++col)
      W(row,col) += 
	cov6(row,col)
	+ a*a * cov6(row+3,col+3)
	- a * (cov6(row+3,col) + cov6(col+3,row)) ;
  
  int OK = W.InvertChol() ;
  if( !OK ) info() << "inversion error in fitDecayLength" << endmsg ;
  
  double halfdChi2dLam2 = ROOT::Math::Similarity(W,dir) ;
  decaylength    = ROOT::Math::Dot(dir,W*dx)/halfdChi2dLam2 ;
  decaylengtherr = std::sqrt(1/halfdChi2dLam2) ;
  
  Gaudi::Vector3 res = dx - decaylength * dir ;
  
  chi2 = ROOT::Math::Similarity(W,res) ;

  return OK ;
}
