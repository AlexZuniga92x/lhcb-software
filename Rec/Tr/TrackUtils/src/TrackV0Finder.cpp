// Include files 


// from Gaudi
#include "GaudiAlg/GaudiAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"

/** @class TrackV0Finder TrackV0Finder.h
 *  
 *
 *  @author Wouter HULSBERGEN
 *  @date   2007-10-08
 */
class ITrajPoca ;
class IMagneticFieldSvc ;
class ITrackVertexer ;
class ITrackExtrapolator ;
class ITrackInterpolator ;

#include "Event/TwoProngVertex.h"
namespace LHCb {
  class ParticleProperty ;
}

class TrackV0Finder : public GaudiAlgorithm {
public: 
  /// Standard constructor
  TrackV0Finder( const std::string& name, ISvcLocator* pSvcLocator );

  virtual ~TrackV0Finder( ); ///< Destructor

  virtual StatusCode initialize();    ///< Algorithm initialization
  virtual StatusCode finalize();      ///< Algorithm finalization
  virtual StatusCode execute   ();    ///< Algorithm execution

protected:
  void constrainToVertex(const Gaudi::XYZPoint& pos,
			 const Gaudi::LorentzVector& p4,
			 const Gaudi::SymMatrix7x7& cov7x7,
			 const LHCb::RecVertex& pv,
			 double& chi2,
			 double& decaylength,
			 double& decaylengtherr) const ;
  
  bool hasV0Topology(LHCb::TwoProngVertex& vertex,
		     const LHCb::RecVertices& pvs,
		     double& decaylength) const ;
private:
  std::string m_trackInputListName; // Input Tracks container location
  std::string m_pvContainerName;
  std::string m_v0ContainerName;
  IMagneticFieldSvc* m_magfieldsvc ;
  ITrajPoca* m_pocatool ;
  ToolHandle<ITrackExtrapolator> m_extrapolator ;
  ToolHandle<ITrackInterpolator> m_interpolator ;
  ITrackVertexer* m_vertexer ;
  const LHCb::ParticleProperty* m_ksProperty ;
  const LHCb::ParticleProperty* m_lambdaProperty ;
  const LHCb::ParticleProperty* m_pionProperty ;
  const LHCb::ParticleProperty* m_protonProperty ;
  double m_maxTrackChi2PerDoF ;
  unsigned int m_maxNumCommonHits ;
  double m_zmin ;
  double m_zmax ;
  double m_minDeltaZ ;
  double m_distanceCutUpstream ;
  double m_distanceCutLong ;
  double m_maxChi2V0Vertex ;
  double m_maxChi2PVConstraint ;
  double m_minDecayLengthSignificance ;
  bool   m_correctBFlight ;
  double m_sigmaBFlightX ;
  double m_sigmaBFlightY ;
  double m_sigmaBFlightZ ;
  double m_ksmasscut ;
  double m_lambdamasscut ;
  double m_chisqProbCut ;
  double m_minCTauKs ;
  double m_minCTauLambda ;
  bool m_useExtrapolator ;
  bool m_excludePVTracks ;
  bool m_rejectUpstreamHits ;
  bool m_addExtraInfo ;
  bool m_addStateAtVertex ;
};

#include "GaudiKernel/AlgFactory.h" 
#include "GaudiKernel/Vector4DTypes.h"
#include "GaudiKernel/IMagneticFieldSvc.h"
#include "Kernel/IParticlePropertySvc.h"
#include "Kernel/ParticleProperty.h"
#include "Kernel/ParticleID.h"
#include "TrackKernel/TrackTraj.h"

#include "Kernel/ITrajPoca.h"
#include "TrackInterfaces/ITrackVertexer.h"
#include "TrackInterfaces/ITrackExtrapolator.h"
#include "TrackInterfaces/ITrackInterpolator.h"
#include <iterator>

#ifdef _WIN32
  // Disable warning C4355: 'this' : used in base member initializer list
  #pragma warning ( disable : 4355 )
#endif

//-----------------------------------------------------------------------------
// Implementation file for class : TrackV0Finder
//
// 2007-10-08 : Wouter HULSBERGEN
//-----------------------------------------------------------------------------

// Declaration of the Algorithm Factory
DECLARE_ALGORITHM_FACTORY( TrackV0Finder )

//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
TrackV0Finder::TrackV0Finder( const std::string& name,
                              ISvcLocator* pSvcLocator)
  : GaudiAlgorithm ( name , pSvcLocator ),
    m_trackInputListName(LHCb::TrackLocation::Default),
    m_extrapolator("TrackMasterExtrapolator",this),
    m_interpolator("TrackInterpolator",this),
    m_maxTrackChi2PerDoF(10),
    m_zmin(-100*Gaudi::Units::cm),
    m_zmax( 300*Gaudi::Units::cm),
    m_minDeltaZ(1*Gaudi::Units::cm),
    m_distanceCutUpstream(5*Gaudi::Units::mm),
    m_distanceCutLong(1*Gaudi::Units::mm),
    m_maxChi2V0Vertex(16),     // 1 dof
    m_maxChi2PVConstraint(20), // 2 dofs
    m_minDecayLengthSignificance(10),
    m_correctBFlight(false),
    m_sigmaBFlightX(0.5*Gaudi::Units::mm), // approx RMS of flight length of B in X
    m_sigmaBFlightY(0.5*Gaudi::Units::mm), // approx RMS of flight length of B in Y
    m_sigmaBFlightZ(10*Gaudi::Units::mm),  // approx RMS of flight length of B in Z
    m_ksmasscut(40*Gaudi::Units::MeV),
    m_lambdamasscut(20*Gaudi::Units::MeV),
    m_useExtrapolator(true),
    m_excludePVTracks(true),
    m_rejectUpstreamHits(true),
    m_addExtraInfo(false),
    m_addStateAtVertex(true)
{
  declareProperty( "TrackContainer", m_trackInputListName = LHCb::TrackLocation::Default  );
  declareProperty( "PVContainer", m_pvContainerName = LHCb::RecVertexLocation::Primary ) ;
  declareProperty( "V0Container", m_v0ContainerName = LHCb::RecVertexLocation::V0 ) ;
  declareProperty( "KsMassCut", m_ksmasscut) ;
  declareProperty( "LambdaMassCut", m_lambdamasscut) ;
  declareProperty( "UseExtrapolator", m_useExtrapolator) ;
  declareProperty( "ExcludePVTracks", m_excludePVTracks) ;
  declareProperty( "MaxNumCommonHits", m_maxNumCommonHits = 0 ) ;
  declareProperty( "MinDeltaZ", m_minDeltaZ) ;
  declareProperty( "MinDecayLengthSignificance",m_minDecayLengthSignificance) ;
  declareProperty( "MaxChi2V0Vertex", m_maxChi2V0Vertex) ;
  declareProperty( "MaxChi2PVConstraint", m_maxChi2PVConstraint ) ;
  declareProperty( "RejectUpstreamHits", m_rejectUpstreamHits ) ;
  declareProperty( "AddExtraInfo", m_addExtraInfo ) ;
  declareProperty( "Interpolator", m_interpolator ) ;
  declareProperty( "Extrapolator", m_extrapolator ) ;
  declareProperty( "MaxDistanceLong", m_distanceCutLong ) ;
  declareProperty( "MaxDistanceUpstream", m_distanceCutLong ) ;
  declareProperty( "AddStateAtVertex", m_addStateAtVertex ) ;
  declareProperty( "MinCTauKs",     m_minCTauKs     = -1) ; // ctau =27 mm
  declareProperty( "MinCTauLambda", m_minCTauLambda = -1) ; // ctau =79 mm
}

//=============================================================================
// Destructor
//=============================================================================
TrackV0Finder::~TrackV0Finder() {} 

//=============================================================================
// Initialization
//=============================================================================
StatusCode TrackV0Finder::initialize() {
  StatusCode sc = GaudiAlgorithm::initialize(); // must be executed first
  if ( sc.isFailure() ) return sc;  // error printed already by GaudiAlgorithm
  
  m_magfieldsvc = svc<IMagneticFieldSvc>( "MagneticFieldSvc", true );
  m_pocatool = tool<ITrajPoca>( "TrajPoca" );
  m_vertexer = tool<ITrackVertexer>( "TrackVertexer" );
  if(m_useExtrapolator) {
    sc = m_extrapolator.retrieve() ;
    if( sc.isFailure() ) {
      error() << "Cannot retrieve extrapolator" << endmsg ;
      return sc; 
    }
    sc = m_interpolator.retrieve() ;
    if( sc.isFailure() ) {
       error() << "Cannot retrieve interpolator" << endmsg ;
      return sc; 
    }
  }

  LHCb::IParticlePropertySvc* propertysvc = 
    svc<LHCb::IParticlePropertySvc>("LHCb::ParticlePropertySvc",true) ;
  m_ksProperty     = propertysvc->find( "KS0" ) ;
  m_lambdaProperty = propertysvc->find( "Lambda0" ) ;
  m_pionProperty   = propertysvc->find( "pi+" ) ;
  m_protonProperty = propertysvc->find( "p+" ) ;
  if( m_ksProperty==0 ||  m_lambdaProperty==0 || m_pionProperty==0 || m_protonProperty==0) {
    error() << "Did not find all properties." << endmsg ;
    sc = StatusCode::FAILURE ;
  }
  
  return sc;
}

StatusCode TrackV0Finder::finalize() {
  if( m_useExtrapolator ) {
    m_extrapolator.release().ignore() ;
    m_interpolator.release().ignore() ;
  }
  return GaudiAlgorithm::finalize() ;
}

inline bool inAnyVertex( const LHCb::Track& track,
                         const LHCb::RecVertices& vertexcontainer)
{
  bool found(false) ;
  for( LHCb::RecVertices::const_iterator ivertex = vertexcontainer.begin() ;
       ivertex != vertexcontainer.end() && !found; ++ivertex) 
    for( SmartRefVector<LHCb::Track>::const_iterator itrack = (*ivertex)->tracks().begin() ;
         itrack != (*ivertex)->tracks().end() && !found; ++itrack) 
      found = &track == &(**itrack) ;
  return found ;
}

//=============================================================================
// Main execution
//=============================================================================
StatusCode TrackV0Finder::execute() 
{
  // Create the output container 
  typedef KeyedContainer<LHCb::TwoProngVertex, Containers::HashMap> TwoProngVertices;
  TwoProngVertices* v0container = new TwoProngVertices() ;
  put(v0container, m_v0ContainerName) ;
  
  // Get the primary vertices. Locate the one that's most downstream.
  const LHCb::RecVertices* pvcontainer = get<LHCb::RecVertices>( m_pvContainerName ) ;
  if( pvcontainer->empty() ) 
    return Warning("No primary verticesfound. Skip V0 search",StatusCode::SUCCESS,0) ;

  double zprimary = 9999 ;
  for( LHCb::RecVertices::const_iterator ipv = pvcontainer->begin() ; 
       ipv != pvcontainer->end(); ++ipv ) 
    zprimary = std::min(zprimary, (*ipv)->position().z()) ;
  
  // Get the Tracks
  const LHCb::Tracks* tracks = get<LHCb::Tracks>( m_trackInputListName );

  // Sort them by charge, make some cuts
  typedef std::vector<const LHCb::Track*> TrackContainer ;
  TrackContainer postracks,negtracks ;
  for( LHCb::Tracks::const_iterator it = tracks->begin() ;
       it != tracks->end(); ++it) {
    const LHCb::Track* track = *it ;
    // require tracks with T and (TT or Velo)
    if( (track->type() == LHCb::Track::Long ||
	 track->type() == LHCb::Track::Downstream ) &&
	track->chi2PerDoF() < m_maxTrackChi2PerDoF &&
        // remove tracks from PVs, if required
        (pvcontainer==0 || !m_excludePVTracks || !inAnyVertex(*track,*pvcontainer) ) ) {
      if( (*it)->firstState().qOverP()>0 ) postracks.push_back(*it) ;
      else                                 negtracks.push_back(*it) ;
    }
  }
  
  // turn them into trajectories
  typedef std::vector<LHCb::TrackTraj> TrackTrajContainer ;
  TrackTrajContainer postrajs,negtrajs ;
  for( TrackContainer::const_iterator ipos = postracks.begin() ;
       ipos != postracks.end(); ++ipos) 
    postrajs.push_back(LHCb::TrackTraj(**ipos,m_magfieldsvc)) ;
  for( TrackContainer::const_iterator ineg = negtracks.begin() ;
       ineg != negtracks.end(); ++ineg) 
    negtrajs.push_back(LHCb::TrackTraj(**ineg,m_magfieldsvc)) ;
  
  counter("numcombinations") += postracks.size()*negtracks.size() ;
  counter("numpostracks") += postracks.size() ;
  counter("numnegtracks") += negtracks.size() ;
  const double pimass = m_pionProperty->mass() ;
  const double pmass  = m_protonProperty->mass() ;
  const double ksmass  = m_ksProperty->mass() ;
  const double lambdamass = m_lambdaProperty->mass() ;
  
  for( TrackContainer::iterator ipos = postracks.begin() ;
       ipos != postracks.end(); ++ipos) 
    for( TrackContainer::iterator ineg = negtracks.begin() ;
         ineg != negtracks.end(); ++ineg)
      if( (*ipos)->nCommonLhcbIDs(**ineg) <= m_maxNumCommonHits ) {

	const LHCb::TrackTraj& postraj = postrajs[std::distance(postracks.begin(),ipos)] ;
	const LHCb::TrackTraj& negtraj = negtrajs[std::distance(negtracks.begin(),ineg)] ;
	
	// seed the z position with the intersection in the yz-plane
	double mupos = postraj.beginRange() + 10*Gaudi::Units::mm ;
	double muneg = negtraj.beginRange() + 10*Gaudi::Units::mm ;
	double ypos  = postraj.position(mupos).y() ;
	double yneg  = negtraj.position(muneg).y() ;
	double typos = postraj.direction(mupos).y() ;
	double tyneg = negtraj.direction(muneg).y() ;
	double z_yz = - (ypos - yneg + muneg*tyneg - mupos*typos) / ( typos - tyneg ) ;
	mupos = muneg = z_yz ;
	
	// Calls pocatool
	Gaudi::XYZVector deltaX; 
	StatusCode sc = m_pocatool->minimize( postraj,mupos,negtraj,muneg,deltaX,0.001*Gaudi::Units::mm );
	if( sc.isSuccess() ) {
	  double distance = deltaX.R() ;
	  double z = 0.5*(mupos+muneg) ;
	  // make the cut on the distance and the z-position
	  bool isVeloCombi = (*ipos)->hasVelo() && (*ineg)->hasVelo() ;
	  if( (distance < m_distanceCutLong ||
	       (distance < m_distanceCutUpstream && !isVeloCombi)) &&
	      m_zmin < z && z< m_zmax && zprimary + m_minDeltaZ < z) {
	    
	    // now make an invariant mass cut
	    Gaudi::XYZVector mompos = postraj.momentum(mupos) ;
	    Gaudi::XYZVector momneg = negtraj.momentum(muneg) ;
	    Gaudi::LorentzVector p4pos(mompos.X(),mompos.Y(),mompos.Z(),
				       std::sqrt(mompos.Mag2()+pimass*pimass)) ;
	    Gaudi::LorentzVector p4neg(momneg.X(),momneg.y(),momneg.Z(),
				       std::sqrt(momneg.Mag2()+pimass*pimass)) ;
	    Gaudi::LorentzVector p4pipi = p4pos+p4neg ;
	    double pipimass = p4pipi.M() ;
	    p4pos.SetE( std::sqrt(mompos.Mag2()+pmass*pmass)) ;
	    Gaudi::LorentzVector p4ppi = p4pos+p4neg ;
	    double ppimass = p4ppi.M() ;
	    p4pos.SetE( std::sqrt(mompos.Mag2()+pimass*pimass)) ;
	    p4neg.SetE( std::sqrt(momneg.Mag2()+pmass*pmass)) ;
	    Gaudi::LorentzVector p4pip = p4pos+p4neg ;
	    double pipmass = p4pip.M() ;
	    double mom     = p4pipi.P() ;
	    bool iskscandidate = std::abs(pipimass - ksmass) < m_ksmasscut ;
	    bool islambdacandidate = std::abs(ppimass - lambdamass) < m_lambdamasscut ;
	    bool isantilambdacandidate = std::abs(pipmass - lambdamass) < m_lambdamasscut ;
	    
	    if( iskscandidate || islambdacandidate || isantilambdacandidate) {
	      // Determine the states passed to the vertexer. At this
	      // stage combinatorics is small enough that we can
	      // eventually use a real extrapolator.
	      LHCb::State posstate,negstate ;
	      if( m_useExtrapolator ) {
		// If we can, we use interpolators, because those can do
		// this correctly even if we are between nodes. (The
		// extrapolators are guaranteed to be wrong half the
		// time!)
		if( (*ipos)->nodes().empty() ) 
		  sc = m_extrapolator->propagate( **ipos, z, posstate ) ;
		else {
		  sc = m_interpolator->interpolate( **ipos, z, posstate ) ;
		  if( sc.isSuccess() ) posstate.setLocation(LHCb::State::V0Vertex) ;
		}
		if(!sc.isSuccess() ) {
		  Warning("Extrapolation failed. Rely on trajectory interpolation.",StatusCode::SUCCESS,0).ignore() ;
		  posstate = postraj.state(z) ;
		} 
		if( (*ineg)->nodes().empty() ) 
		  sc = m_extrapolator->propagate( **ineg, z, negstate ) ;
		else {
		  sc = m_interpolator->interpolate( **ineg, z, negstate ) ;
		  if( sc.isSuccess() ) negstate.setLocation(LHCb::State::V0Vertex) ;
		}
		if(!sc.isSuccess() ) {
		  Warning("Extrapolation failed. Rely on trajectory interpolation.",StatusCode::SUCCESS,0).ignore() ;
		  negstate = negtraj.state(z) ;
		}
	      } else {
		posstate = postraj.state(z) ;
		negstate = negtraj.state(z) ;
	      }
	      
	      // finally, create the vertex and cut on the significance
	      LHCb::TwoProngVertex* vertex = m_vertexer->fit(posstate,negstate ) ;
	      double decaylength ;
	      if( vertex == 0 ) {
		Warning("TrackVertexer Failure", StatusCode::SUCCESS, 0 ).ignore() ;
	      } else if( vertex->chi2() < m_maxChi2V0Vertex 
			 && hasV0Topology( *vertex, *pvcontainer,decaylength ) ) {

		// cut on ctau
		if( m_minCTauKs>0 || m_minCTauLambda >0 ) {
		  iskscandidate = iskscandidate && decaylength * pipimass / mom > m_minCTauKs ;
		  islambdacandidate = islambdacandidate && decaylength * ppimass / mom > m_minCTauLambda ;
		  isantilambdacandidate = isantilambdacandidate && decaylength * pipmass / mom > m_minCTauLambda ;
		}

		// one last check: test that there are no hits upstream of the vertex on either track
		bool hasUpstreamHits = false ;
		if( m_rejectUpstreamHits ) {
		  const LHCb::State* mstatepos = (*ipos)->stateAt(LHCb::State::FirstMeasurement ) ;
		  const LHCb::State* mstateneg = (*ineg)->stateAt(LHCb::State::FirstMeasurement ) ;
		  hasUpstreamHits =
		    (mstatepos && mstatepos->z() < vertex->position().z() ) ||
		    (mstateneg && mstateneg->z() < vertex->position().z() ) ;
		}
		
		if( !hasUpstreamHits && (iskscandidate || islambdacandidate || isantilambdacandidate )) {
		  
		  if( m_addExtraInfo ) {
		    vertex->addInfo(100,distance) ;
		    vertex->addInfo(101,z-zprimary) ;
		    vertex->addInfo(102,pipimass) ;
		    vertex->addInfo(103,ppimass) ;
		    vertex->addInfo(104,pipmass) ;
		  }
		  vertex->addToTracks(*ipos) ;
		  vertex->addToTracks(*ineg) ;
		  if(iskscandidate) {
		    LHCb::ParticleID pid = LHCb::ParticleID(m_ksProperty->pdgID());
		    vertex->addPID( pid ) ;
		  }
		  if(islambdacandidate) {
		    LHCb::ParticleID pid = LHCb::ParticleID(m_lambdaProperty->pdgID());
		    vertex->addPID( pid ) ;
		  }
		  if(isantilambdacandidate) {
		    LHCb::ParticleID pid = LHCb::ParticleID(m_lambdaProperty->antiParticle()->pdgID());
		    vertex->addPID( pid ) ;
		  }
		  v0container->add( vertex ) ;
		  // finally add the states to the tracks, provided the extrapolation was done properly
		  // (the 'Vertex' location was only set for properly interpolated tracks.
		  if( m_addStateAtVertex ) {
		    if( posstate.location()==LHCb::State::V0Vertex ) 
		      (const_cast<LHCb::Track*>(*ipos))->addToStates( posstate ) ;
		    if( negstate.location()==LHCb::State::V0Vertex ) 
		      (const_cast<LHCb::Track*>(*ineg))->addToStates( negstate ) ;
		  }
		}
	      } else {
		delete vertex ;
	      }
	    }
	  }
	} else {
	  Warning("TrajPoca Failure",StatusCode::SUCCESS,0).ignore() ;
	}
      }
  
  counter("numselected") += v0container->size() ;
  return StatusCode::SUCCESS;
}

//=============================================================================
// Check how well this V0 candidate fits to the PV
//=============================================================================

inline Gaudi::Vector3 transform( const Gaudi::XYZVector& vec)
{
  return Gaudi::Vector3(vec.X(),vec.Y(),vec.Z()) ;
}

void
TrackV0Finder::constrainToVertex(const Gaudi::XYZPoint& pos,
				 const Gaudi::LorentzVector& p4,
				 const Gaudi::SymMatrix7x7& cov7,
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
  
  Gaudi::Vector3 dx = transform(pos - pv.position()) ;
  Gaudi::XYZVector p3 = p4.Vect() ;
  double p3mag = p3.R() ; 
  Gaudi::Vector3 dir = transform(p3.Unit()) ;
  
  Gaudi::SymMatrix3x3 W = pv.covMatrix() ; // we'll repace this with a constant error that contains B motion
  
  if(m_correctBFlight) {
    // For determining whether this candidate is compatible with the
    // PV, we want to take into account the the Ks may come from a
    // B. So, we need to add something to the PV vertex. However, we
    // want to count the B flight length in the decay length, so we
    // don't want to add the B flight length to the PV error in z. The
    // trick is to remove the contribution of B-flight along the V0
    // direction.

    static Gaudi::SymMatrix3x3 covBFlight ;
    covBFlight(0,0) = m_sigmaBFlightX*m_sigmaBFlightX  ;
    covBFlight(1,1) = m_sigmaBFlightY*m_sigmaBFlightY  ;
    covBFlight(2,2) = m_sigmaBFlightZ*m_sigmaBFlightZ  ;

    // now project out the part perpendicular to the direction. 
    //   W +=  (1-P) * covB * (1-P)
    // where P is the projection matrix
    //   P_ij = dir_i * dir_j
    // 
    // I am sure that there is something left to optimize here ...
    Gaudi::SymMatrix3x3 UnitMinusP ;
    UnitMinusP(0,0) = UnitMinusP(1,1) = UnitMinusP(2,2) = 1 ;
    for( size_t irow = 0; irow<3; ++irow )
      for( size_t jrow = 0; jrow<=irow; ++jrow )
     	UnitMinusP(irow,jrow) -= dir(irow) * dir(jrow) ;
    
    // here we could use that W is diagonal. that saves a lot of time
    //W +=  ROOT::Math::Similarity(covBFlight,UnitMinusP) ;
    for( size_t irow = 0; irow<3; ++irow ) 
      for( size_t jrow = 0; jrow<=irow; ++jrow )
	for( size_t krow = 0; krow<3; ++krow )
	  W(irow,jrow) += UnitMinusP(irow,krow) * covBFlight(krow,krow) *  UnitMinusP(krow,jrow) ;
    
  }

  //double a = (ROOT::Math::Transpose(dir)*dx)/p3mag  ;
  double a = ROOT::Math::Dot(dir,dx)/p3mag  ;
  for(size_t row=0; row<3; ++row)
    for(size_t col=0; col<=row; ++col)
      W(row,col) += 
	cov7(row,col)
	+ a*a * cov7(row+3,col+3)
	- a * (cov7(row+3,col) + cov7(col+3,row)) ;
  
  int OK = W.Invert() ;
  if( !OK ) error() << "inversion error in constrainToVertex" << endmsg ;
  
  double halfdChi2dLam2 = ROOT::Math::Similarity(W,dir) ;
  decaylength    = ROOT::Math::Dot(dir,W*dx)/halfdChi2dLam2 ;
  decaylengtherr = std::sqrt(1/halfdChi2dLam2) ;
  
  Gaudi::Vector3 res = dx - decaylength * dir ;
  
  chi2 = ROOT::Math::Similarity(W,res) ;
}

bool
TrackV0Finder::hasV0Topology(LHCb::TwoProngVertex& vertex,
			     const LHCb::RecVertices& pvs,
			     double& decaylength) const
{
  // returns true if V0 candidate accepted. we veto two types of background:
  // * V0 candidates that do not point to any PV ( chi2 > m_maxChi2PVConstraint)
  // * V0 candidates that point to a PV but with too small decay length

  bool accept = true ;

  if(pvs.size()>0) {
    bool isFromPV = false ;
    double chi2(999),decaylengtherr(1) ;
    decaylength = 0 ;

    // first find the best fitting primary vertex
    const Gaudi::XYZPoint& pos = vertex.position() ;
    Gaudi::SymMatrix7x7 cov7x7  = vertex.covMatrix7x7(0,0) ;
    Gaudi::LorentzVector p4     = vertex.momentum(0,0) ;
    for( LHCb::RecVertices::const_iterator ipv = pvs.begin() ;
	 ipv != pvs.end(); ++ipv ) {
      double tmpchi2, tmpdecaylength, tmpdecaylengtherr ;
      constrainToVertex(pos,p4,cov7x7,**ipv,tmpchi2, tmpdecaylength, tmpdecaylengtherr) ;
      // is this the best fitting PV?
      if( ipv == pvs.begin() || tmpchi2 < chi2 ) {
	chi2 = tmpchi2 ;
	decaylength = tmpdecaylength ;
	decaylengtherr = tmpdecaylengtherr ;
      }
      // veto this candidate if it is compatible with this PV
      isFromPV = isFromPV ||
	( tmpchi2 < m_maxChi2PVConstraint &&
	  std::abs(tmpdecaylength / tmpdecaylengtherr) < m_minDecayLengthSignificance ) ;
    }

    if(m_addExtraInfo) {
      vertex.addInfo( 1000, chi2 ) ;
      vertex.addInfo( 1001, decaylength/decaylengtherr ) ;
    }
    
    accept = !isFromPV 
      && chi2 < m_maxChi2PVConstraint 
      && decaylength/decaylengtherr > m_minDecayLengthSignificance ;
  }
  
  return accept ;
}
