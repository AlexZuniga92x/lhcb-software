#include "Event/KalmanFitResult.h"
#include "Event/FitNode.h"
#include <boost/foreach.hpp>

namespace LHCb
{
  // default constructor. do nothing.
  KalmanFitResult::KalmanFitResult() 
    : m_nTrackParameters(5), m_chi2CacheValid(false),m_errorFlag(0x00),m_bidirectionalSmoother(true) {}
  
  // copy constructor
  KalmanFitResult::KalmanFitResult(const KalmanFitResult& rhs) 
    : TrackFitResult(rhs), m_seedCovariance(rhs.m_seedCovariance),m_nTrackParameters(rhs.m_nTrackParameters)
    , m_chi2CacheValid(rhs.m_chi2CacheValid),m_errorFlag(rhs.m_errorFlag)
    ,m_bidirectionalSmoother(rhs.m_bidirectionalSmoother)
  {
    establishNodeLinks() ;
  }
  
  // copy from TrackFitResult
  KalmanFitResult::KalmanFitResult(const TrackFitResult& rhs) 
    : TrackFitResult(rhs),m_nTrackParameters(5), m_chi2CacheValid(false)
    ,m_errorFlag(0x00),m_bidirectionalSmoother(true) {}
		    
  // destructor
  KalmanFitResult::~KalmanFitResult() {}

  // clone
  TrackFitResult* KalmanFitResult::clone() const
  {
    return new KalmanFitResult(*this) ;
  }

  // set the error flag out of direction, algorithm and error type identifiers 
  void KalmanFitResult::setErrorFlag(unsigned short direction, unsigned short algnum , unsigned short errnum) 
  {
    m_errorFlag = (((unsigned short) 1 ) << globalBits)
      +(((unsigned short)direction) << dirBits ) 
      +(((unsigned short)algnum) << algBits ) 
      +(((unsigned short)errnum) << typeBits);  
  }

  // check the global error status of the node
  bool KalmanFitResult::inError() const {
    if ( m_errorFlag == 0 ) return false;
    else return true;
  }

  // get the error description
  std::string KalmanFitResult::getError() const {
    unsigned short direction = ( m_errorFlag & dirMask ) >> dirBits ;
    unsigned short algnum = ( m_errorFlag & algMask ) >> algBits ;
    unsigned short errnum = ( m_errorFlag & typeMask ) ;
    std::ostringstream errMsg;
    std::ostringstream dir;
    // Set the direction
    switch (direction){
    case Forward:
      dir<<"forward "; break;
    case Backward:
      dir<<"backward "; break;
    default : dir<<""; break;
    }
    // Set the algorithm
    switch (algnum){
    case Predict:
      errMsg<<"Error in predict "<<dir.str()<<"function: ";
      if(errnum == Initialization) errMsg<<"seed covariance is not set!";
      else if(errnum == AlgError) errMsg<<"something goes wrong in the prediction";
      else errMsg<<"unknown error";
      break;
    case Filter:
      errMsg<<"Error in filter "<<dir.str()<<"function: ";
      if(errnum == Initialization) errMsg<<"projection matrix is not set!";
      else if(errnum == AlgError) errMsg<<"something goes wrong in the filtering";
      else errMsg<<"unknown error";
      break;
    case Smooth:
      errMsg<<"Error in smooth function: ";
      if( errnum == MatrixInversion )errMsg<<"error in matrix inversion";
      else if( errnum == AlgError ) errMsg<<"non positive diagonal element in coveriance matrix";
      else if( errnum == Other    )errMsg<<"problem with HCH.";
      else errMsg<<"unknown error";
      break;
    case ComputeResidual:
      errMsg<<"Error in compute residual: ";
      if( errnum == Other )errMsg<<" non positive variance.";
      else errMsg<<"unknown error";
      break;
    case WeightedAverage:
      errMsg<<"Error in weighted average: ";
      if( errnum == Other )errMsg<<" non positive variance.";
      else errMsg<<"unknown error";
      break;  
    default:
      errMsg<<"Unknown error...";
      break;
    }
    return errMsg.str();
  }

  // return (chisq,dof) out of the differnet contribution
  void KalmanFitResult::computeChiSquares() const 
  {
    // This routine calculates the chisquare contributions from
    // different segments of the track. It uses the 'delta-chisquare'
    // contributions from the bi-directional kalman fit. Summing these
    // leads to a real chisquare only if the contributions are
    // uncorrelated. For a Velo-TT-T track you can then calculate:
    //
    // - the chisuare of the T segment and the T-TT segment by using the
    // 'upstream' contributions 
    //
    // - the chisquare of the Velo segment and the Velo-TT segment by
    // using the 'downstream' contributions
    //
    // Note that you cannot calculate the contribution of the TT segment
    // seperately (unless there are no T or no Velo hits). Also, if
    // there are Muon hits, you cannot calculate the T station part, so
    // for now this only works for tracks without muon hits.
    
    // first reset everything
    m_chi2Muon = m_chi2MuonT = m_chi2Velo =  m_chi2VeloTT = m_chi2VeloTTT = m_chi2 = ChiSquare() ;

    if( !nodes().empty() ) {

      // Compute the chisquare integrals for forward and backward
      double chisqMuon[2] = {0,0} ;
      double chisqT[2]    = {0,0} ;
      double chisqTT[2]   = {0,0} ;
      double chisqVelo[2] = {0,0} ;
      int    nhitsMuon(0), nhitsT(0), nhitsTT(0), nhitsVelo(0) ; 
      BOOST_FOREACH( const LHCb::Node* node, nodes() ) {
	if( node->type() == LHCb::Node::HitOnTrack ) {
	  const LHCb::FitNode* fitnode = static_cast<const LHCb::FitNode*>(node) ;
	  
	  switch( node->measurement().type() ) {
	  case Measurement::VeloR:
	  case Measurement::VeloPhi:
	  case Measurement::VeloLiteR:
	  case Measurement::VeloLitePhi:
	    chisqVelo[0] += fitnode->deltaChi2Forward() ;
	    chisqVelo[1] += fitnode->deltaChi2Backward() ;
	    ++nhitsVelo ;
	    break;
	  case Measurement::TT:
	  case Measurement::TTLite:
	    chisqTT[0] += fitnode->deltaChi2Forward() ;
	    chisqTT[1] += fitnode->deltaChi2Backward() ;
	    ++nhitsTT ;
	    break;
	  case Measurement::OT:
	  case Measurement::IT:
	  case Measurement::ITLite:
	    chisqT[0] += fitnode->deltaChi2Forward() ;
	    chisqT[1] += fitnode->deltaChi2Backward() ;
	    ++nhitsT ;
	    break;
	  case Measurement::Muon:
	    chisqMuon[0] += fitnode->deltaChi2Forward() ;
	    chisqMuon[1] += fitnode->deltaChi2Backward() ;
	    ++nhitsMuon ;
	    break;
	  case Measurement::Unknown:
	  case Measurement::Calo:
	  case Measurement::VeloPixLite:
	    break ;
	  }
	}
      }
      
      bool upstream =  nodes().front()->z() > nodes().back()->z() ;
      int nPar = nTrackParameters() ;
      
      if( nhitsMuon > 0 )
	m_chi2Muon  = ChiSquare(upstream ? chisqMuon[0] : chisqMuon[1],nhitsMuon - nPar) ;

      if( nhitsT + nhitsMuon > 0 ) 
	m_chi2MuonT  = ChiSquare(upstream ? (chisqT[0]+ chisqMuon[0]) : (chisqT[1] +chisqMuon[0]),nhitsT + nhitsMuon - nPar) ;

      if( nhitsVelo > 0 ) 
	m_chi2Velo   =  ChiSquare(upstream ? chisqVelo[1] : chisqVelo[0],nhitsVelo - nPar) ;

      if( nhitsVelo + nhitsTT >0 )
	m_chi2VeloTT = ChiSquare(upstream ? chisqVelo[1] + chisqTT[1] : chisqVelo[0] + chisqTT[0],nhitsVelo + nhitsTT - nPar ) ;
      
      if( nhitsVelo + nhitsTT + nhitsT >0 )
	m_chi2VeloTTT = ChiSquare(upstream ? chisqVelo[1] + chisqTT[1] + chisqT[1] : chisqVelo[0] + chisqTT[0] + chisqT[0],
				  nhitsVelo + nhitsTT + nhitsT - nPar )  ;
      
      if( nhitsMuon + nhitsT + nhitsVelo + nhitsTT > 0 )
	m_chi2 = ChiSquare( std::max( chisqMuon[0] + chisqT[0]+chisqTT[0]+chisqVelo[0], chisqMuon[1]+chisqT[1]+chisqTT[1]+chisqVelo[1] ),
			    nhitsMuon+ nhitsT + nhitsVelo + nhitsTT - nPar) ;
    }
    
    m_chi2CacheValid = true ;
  } 

  // return (chisq,dof) for the forward direction fit
  ChiSquare KalmanFitResult::computeChiSquareForwardFit() const 
  {
    const LHCb::FitNode* lastnode(0) ;
    double chisq(0) ; int ndof(0) ;
    BOOST_FOREACH( LHCb::Node* node, nodes()) {
      if( node->type()==LHCb::Node::HitOnTrack ) {
	const LHCb::FitNode* fitnode = static_cast<const FitNode*>(node) ;
	chisq   += fitnode->deltaChi2Forward() ;
	lastnode = fitnode ;
	++ndof ;
      }
    }
    // Count the number of active track parameters. For now, just look at the momentum.
    if(lastnode) {
      const double threshold = 0.1 ;
      size_t npar = (lastnode->filteredState(LHCb::FitNode::Forward).covariance()(4,4) 
		     / m_seedCovariance(4,4) < threshold ? 5 : 4) ;
      //setNTrackParameters( npar ) ;
      ndof -= npar ;
    }
    return ChiSquare( chisq, ndof ) ;
  }
  
  /// setup the link to previous/next fitnode
  void KalmanFitResult::establishNodeLinks() 
  {
    LHCb::FitNode* prev(0) ;
    for( NodeContainer::const_iterator it = nodes().begin() ;
	 it != nodes().end(); ++it ) {
      LHCb::FitNode* fitnode = static_cast<FitNode*>(*it) ;
      fitnode->setPreviousNode( prev ) ;
      fitnode->setParent(this) ;
      prev = fitnode ;
    }
  }
    
}
