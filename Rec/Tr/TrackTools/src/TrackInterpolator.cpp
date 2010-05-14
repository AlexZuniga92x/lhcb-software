// $Id: TrackInterpolator.cpp,v 1.11 2009-10-30 14:39:39 cattanem Exp $
// Include files
// -------------
// from Gaudi
#include "GaudiKernel/ToolFactory.h"
#include "GaudiKernel/GenericVectorTypes.h"
#include "GaudiKernel/SymmetricMatrixTypes.h"

// From LHCbMath
#include "LHCbMath/MatrixManip.h"
#include "LHCbMath/MatrixInversion.h"

// from TrackEvent
#include "Event/TrackUnitsConverters.h"
#include "Event/FitNode.h"

// local
#include "TrackInterpolator.h"

// Disable warning on windows about using 'this' in constructors
#ifdef _WIN32
#pragma warning ( disable:4355 )
#endif

using namespace Gaudi;
using namespace Gaudi::Math;
using namespace LHCb;


//-----------------------------------------------------------------------------
// Implementation file for class : TrackInterpolator
//
// 2006-10-06 : Jeroen van Tilburg
//-----------------------------------------------------------------------------

DECLARE_TOOL_FACTORY( TrackInterpolator );

//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
TrackInterpolator::TrackInterpolator( const std::string& type,
                                      const std::string& name,
                                      const IInterface* parent )
  : GaudiTool ( type, name , parent ),
    m_extrapolator("TrackMasterExtrapolator",this)
{
  // interfaces
  declareInterface<ITrackInterpolator>(this);

  declareProperty( "Extrapolator", m_extrapolator );
};

//=============================================================================
// Destructor
//=============================================================================
TrackInterpolator::~TrackInterpolator() {};

//=============================================================================
// Initialization
//=============================================================================
StatusCode TrackInterpolator::initialize() {

  StatusCode sc = GaudiTool::initialize();
  if (sc.isFailure()) return sc;  // error already reported by base class

  // extrapolator
  sc = m_extrapolator.retrieve() ;
  
  return sc;
};

//=============================================================================
// Finalize
//=============================================================================
StatusCode TrackInterpolator::finalize() {
  m_extrapolator.release().ignore() ;
  return GaudiTool::finalize() ;
}

//=============================================================================
// Interpolate between the nearest nodes
//=============================================================================
StatusCode TrackInterpolator::interpolate( const Track& track,
                                           double z,
                                           State& state )
{
  // Check if there are nodes on the track
  typedef LHCb::Track::ConstNodeRange NodeContainer ;
  NodeContainer nodes = track.nodes();
  if ( nodes.empty() ) return Error("No nodes on track found.");

  // If we are between the first and last node with a measurement, we
  // interpolate. If not, we extrapolate from the closest 'inside'
  // node. (That's more stable than interpolation.) In the end this
  // needs to work both for upstream and downstream fits. I am not
  // sure that it works for either now.

  // first find the pair of iterators such that z is between 'prevnode' and 'nextnode'
  NodeContainer::const_iterator nextnode = nodes.begin() ;
  if(  nodes.front()->z() < nodes.back()->z() ) {
    while( nextnode != nodes.end() && (*nextnode)->z() < z ) ++nextnode ;
  } else {
    while( nextnode != nodes.end() && (*nextnode)->z() > z ) ++nextnode ;
  }
  
  // determine where we are wrt to nodes with (active) measurements
  bool foundprecedingmeasurement(false) ; // is there measurement in nodes < nextnode?
  bool foundprocedingmeasurement(false) ; // is there a measurement in nodes >= nextnode?
  // any measurement nodes between begin and nextnode?
  for( NodeContainer::const_iterator inode = nodes.begin() ;
       inode != nextnode && !foundprecedingmeasurement; ++inode)
    foundprecedingmeasurement = (*inode)->type() == LHCb::Node::HitOnTrack ;
  for( NodeContainer::const_iterator inode = nextnode ;
       inode != nodes.end() && !foundprocedingmeasurement; ++inode)
    foundprocedingmeasurement = (*inode)->type() == LHCb::Node::HitOnTrack ;
  
  // we must find either of the two (there must be measurement nodes!)
  if( !foundprecedingmeasurement && !foundprocedingmeasurement)
    return Error("Did not find any measurement nodes on track!") ;

  // This is not necessarily a valid iterator, but that should be
  // caught by the logic later on.
  NodeContainer::const_iterator prevnode = nextnode ; --prevnode ;
  
  // interpolate only if we have measurements on both sides
  if( !foundprecedingmeasurement || !foundprocedingmeasurement) {
    const LHCb::Node* extrapolationnode = foundprocedingmeasurement ? *nextnode : *prevnode ;
    state = extrapolationnode->state() ;
    return m_extrapolator -> propagate( state, z ) ;
  } 
  
  // so, we interpolate. Get the nodes:
  const LHCb::FitNode* nodeNext   = dynamic_cast<const LHCb::FitNode*>(*nextnode) ;
  const LHCb::FitNode* nodePrev = dynamic_cast<const LHCb::FitNode*>(*prevnode) ;
  
  if( (z-nodeNext->z()) * (z-nodePrev->z()) > 0 ) {
    error() << "logic failure in locating nodes: " 
	    << z << ", " << nodePrev->z() << "," << nodeNext->z() << endreq ;
    return StatusCode::FAILURE ;
  }
  
  
  // bail out if we have actually reached our destination
  if( fabs(nodeNext->z() - z) < TrackParameters::propagationTolerance ) {
    state = nodeNext->state() ;
    return StatusCode::SUCCESS ;
  }
  if( fabs(nodePrev->z() - z) < TrackParameters::propagationTolerance ) {
    state = nodePrev->state() ;
    return StatusCode::SUCCESS ;
  }

  // Get the predicted states and filter if necessary
  State stateDown = nodePrev->predictedStateForward();
  if( nodePrev->type() == LHCb::Node::HitOnTrack ) {
    StatusCode sc = filter( *nodePrev, stateDown ) ;
    if( sc.isFailure() ) return sc ;
  }

  State stateUp   = nodeNext->predictedStateBackward();
  if( nodeNext->type() == LHCb::Node::HitOnTrack ) {
    StatusCode sc = filter( *nodeNext, stateUp ) ;
    if( sc.isFailure() ) return sc ;
  }
  
  // extrapolate the upstream and downstream states
  m_extrapolator -> propagate( stateDown, z );  
  m_extrapolator -> propagate( stateUp  , z );

  // Get the predicted downstream state and invert the covariance matrix
  const TrackVector& stateDownX = stateDown.stateVector();
  TrackSymMatrix invStateDownC = stateDown.covariance();
  if ( !Gaudi::Math::invertPosDefSymMatrix( invStateDownC ) )
    return Error( "inverting matrix in smoother" );

  // Get the predicted upstream state and invert the covariance matrix
  const TrackVector& stateUpX = stateUp.stateVector();
  TrackSymMatrix invStateUpC = stateUp.covariance();
  if ( !Gaudi::Math::invertPosDefSymMatrix( invStateUpC ) )
    return Error( "inverting matrix in smoother" );

  // Add the inverted matrices
  TrackSymMatrix& stateC = state.covariance();
  stateC = invStateDownC + invStateUpC;
  if ( !Gaudi::Math::invertPosDefSymMatrix( stateC ) )
    return Error( "inverting matrix in smoother" );
  
  // Get the state by calculating the weighted mean
  TrackVector& stateX = state.stateVector();
  stateX = stateC * ((invStateDownC * stateDownX) + (invStateUpC * stateUpX)) ;
  state.setZ( z );   

  if( msgLevel(MSG::DEBUG) ) 
    debug() << "filteredstate A: "
  	    << stateUpX << std::endl
  	    << "filteredstate B: "
  	    << stateDownX << std::endl
  	    << "smoothed state A: "
  	    << nodePrev->state()
  	    << "smoothed state B: "
  	    << nodeNext->state() 
  	    << "interpolated state: "
  	    << state << endreq ;

  return StatusCode::SUCCESS;
};


//=============================================================================
// Filter step (needed as the result of filter step is not stored in the Node)
//=============================================================================
StatusCode TrackInterpolator::filter(const FitNode& node, State& state) 
{
  const Measurement& meas = node.measurement();

  // check z position
  if ( std::abs(meas.z() - state.z()) > 1e-6) {
    if( msgLevel(MSG::DEBUG) ) debug() << "Warning, State at z=" << state.z() 
            << ", Measurement at z=" << meas.z() << endmsg;
    return Warning( "Z positions of State and Measurement are not equal", StatusCode::FAILURE, 1 );
  }

  // get the state vector and cov
  TrackVector&    X = state.stateVector();
  TrackSymMatrix& C = state.covariance();

  // Get the projected residual
  const TrackProjectionMatrix& H = node.projectionMatrix();
  double res = node.projectionTerm() - Vector1(H*X)(0) ;
  double errorMeas2 = node.errMeasure2();
  double errorRes2 = errorMeas2 + Similarity( H, C )(0,0) ;

  // calculate gain matrix K
  SMatrix<double,5,1> K = (C * Transpose(H)) / errorRes2;

  // update the state vector
  X += K.Col(0) * res ;

  // update the covariance matrix
  static const TrackSymMatrix unit = TrackSymMatrix( SMatrixIdentity());
  C = Symmetrize( Similarity( unit - ( K*H ), C ) 
                  +(errorMeas2*K)*Transpose(K) );

  return StatusCode::SUCCESS;
}
