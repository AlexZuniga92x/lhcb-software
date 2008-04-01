#include "MaterialLocatorBase.h"
#include "Event/CubicStateVectorInterpolationTraj.h"
#include "Event/TrackParameters.h"
#include "Event/StateParameters.h"
#include "GaudiKernel/SystemOfUnits.h"
#include "DetDesc/Material.h"

MaterialLocatorBase::MaterialLocatorBase(const std::string& type,
					 const std::string& name,
					 const IInterface* parent ) 
  : GaudiTool(type, name, parent),
    m_maxNumIntervals( 20 ),
    m_maxDeviation( 5*Gaudi::Units::cm),
    m_maxDeviationAtRefstates( 2*Gaudi::Units::mm),
    m_maxDeviationAtVeloRefstates( 0.5*Gaudi::Units::mm),
    m_scatteringtool("StateThickMSCorrectionTool"),
    m_dedxtool("StateSimpleBetheBlochEnergyCorrectionTool"),
    m_elecdedxtool("StateElectronEnergyCorrectionTool")
{
  declareInterface<IMaterialLocator>(this);
}

StatusCode MaterialLocatorBase::initialize()
{
  return 
    GaudiTool::initialize() &&
    m_scatteringtool.retrieve() && 
    m_dedxtool.retrieve() && 
    m_elecdedxtool.retrieve() ;
}

StatusCode MaterialLocatorBase::finalize()
{
  return 
    m_scatteringtool.release() && 
    m_dedxtool.release() && 
    m_elecdedxtool.release() && 
    GaudiTool::finalize() ;
}

size_t MaterialLocatorBase::intersect( const Gaudi::XYZPoint& p, const Gaudi::XYZVector& v, 
				       IMaterialLocator::Intersections& intersepts ) const 
  
{
  static ILVolume::Intersections origintersepts ;
  origintersepts.clear() ;
  size_t rc = intersect(p,v,origintersepts) ;
  double dz = v.z() ;
  double z1 = p.z() ;
  double tx = v.x()/v.z() ;
  double ty = v.y()/v.z() ;
  intersepts.resize(origintersepts.size()) ;
  int i(0) ;
  for( ILVolume::Intersections::const_iterator it = origintersepts.begin() ;
       it != origintersepts.end() ; ++it, ++i ) {
    intersepts[i].z1 = z1 + dz * it->first.first ;
    intersepts[i].z2 = z1 + dz * it->first.second ;
    intersepts[i].tx = tx ;
    intersepts[i].ty = ty ;
    intersepts[i].material  = it->second ;
  }
  return rc ;
}

inline double pointerror(const LHCb::StateVector& begin,
			 const LHCb::StateVector& end,
			 const LHCb::StateVector& mid )
{
  double len = end.z() - begin.z();
  double tx = (end.x() - begin.x())/len ;
  double ty = (end.y() - begin.y())/len ;
  double dz = mid.z() - begin.z() ;
  double dx = begin.x() + dz*tx - mid.x() ;
  double dy = begin.y() + dz*ty - mid.y() ;
  return sqrt(dx*dx+dy*dy) ;
}

inline double linearerror(const LHCb::StateVector& origin,
			  const LHCb::StateVector& destination)
{
  // calculate deviation from a parabola
  double dz = destination.z() - origin.z() ;
  double dx = origin.x() + origin.tx()*dz - destination.x() ;
  double dy = origin.y() + origin.ty()*dz - destination.y() ;
  return 0.25*sqrt(dx*dx+dy*dy) ;
}

inline std::list<LHCb::StateVector>::iterator next(const std::list<LHCb::StateVector>::iterator& iter)
{
  std::list<LHCb::StateVector>::iterator rc = iter ;
  return ++rc ;
}

size_t MaterialLocatorBase::intersect( const LHCb::ZTrajectory& traj,
				       IMaterialLocator::Intersections& intersepts ) const 
{
  intersepts.clear() ;
  if( std::abs(traj.endRange() - traj.beginRange())  > TrackParameters::propagationTolerance) {
    // The material locators can only use straight lines, so we
    // approximate the trajectory by straight lines. The less
    // intervals the better. We determine the number of intervals from
    // the maximum allowed deviation. Because 'queuering' the
    // trajectory for a state is potentially expensive (e.g. for the
    // tracktraj), we also limit the number of calls to the trajectory
    // as much as possible. There are two simple ways of calculating
    // the error: Either we can queuery the trajkectory halfway
    // ('midpointerror'), or we can estimate the halfway deviation
    // from extrapolating to the end ('linearerror'). The latter is
    // cheaper and more conservative; the former is more optimal
    // if tracks aren't very quadratic.

    // The first two ndoes are just the endpoints. We sort the in z to
    // make things easier.
    std::list<LHCb::StateVector> nodes ;
    const size_t maxnumnodes = m_maxNumIntervals+1 ;
    nodes.push_back(traj.stateVector(std::min(traj.beginRange(),traj.endRange()))) ;
    nodes.push_back(traj.stateVector(std::max(traj.beginRange(),traj.endRange()))) ;
    std::list<LHCb::StateVector>::iterator inode(nodes.begin()), nextnode ;
    
    // reference states for this trajectory. may be empty.
    if( m_maxDeviationAtRefstates > 0) {
      const std::vector<LHCb::StateVector> refstates = traj.refStateVectors() ;
      // First insert nodes at the reference positions, if required
      if( !refstates.empty()) {
	while(  (nextnode = next(inode)) != nodes.end() && nodes.size() < maxnumnodes ) {
	  std::vector<LHCb::StateVector>::const_iterator worstref = refstates.end() ;
	  double reldeviation(0) ;
	  for(std::vector<LHCb::StateVector>::const_iterator iref = refstates.begin() ;
	      iref != refstates.end(); ++iref) 
	    if( inode->z() < iref->z() && iref->z() < nextnode->z() ) {
	      double thisdeviation    = pointerror( *inode, *nextnode, *iref ) ;
	      double thisreldeviation =  thisdeviation / 
		( iref->z() < StateParameters::ZEndVelo ? 
		  m_maxDeviationAtVeloRefstates : m_maxDeviationAtRefstates ) ;
	      if( thisreldeviation > reldeviation ) {
		reldeviation = thisreldeviation ;
		worstref = iref ;
	      }
	    }
	  if( reldeviation > 1 && worstref!=refstates.end()) {
	    nodes.insert(nextnode,*worstref) ;
	  }
	  else 
	    ++inode ;
	}
      }
    }
    
    // now the usual procedure
    inode = nodes.begin() ;
    double worstdeviation(0) ;
    std::list<LHCb::StateVector>::iterator worstnode(inode)  ;
    while(  (nextnode = next(inode)) != nodes.end() && nodes.size() < maxnumnodes ) {
      // make sure we are fine at the midpoint
      LHCb::StateVector midpoint = traj.stateVector( 0.5*(inode->z()+nextnode->z())) ;
      double deviation = pointerror( *inode, *nextnode, midpoint  ) ;
      if( deviation > m_maxDeviation ) {
	nodes.insert(nextnode,midpoint ) ;
      } else {
	if( deviation > worstdeviation ) {
	  worstdeviation = deviation ;
	  worstnode = inode ;
	}
	++inode ;
      }
    }
    
    // issue a warning if we didn't make it
    if( nodes.size()==maxnumnodes )
      warning() << "Trajectory approximation did not reach desired accuracy. " << endreq ;
    
    // debug output
    if(msgLevel( MSG::VERBOSE ) ||
       (msgLevel( MSG::DEBUG) && nodes.size()==maxnumnodes ) ) {
      debug() << "Trajectory approximation: numnodes=" << nodes.size() 
	      << ", deviation=" << worstdeviation 
	      << " at z= " << 0.5*(worstnode->z()+next(worstnode)->z())
	      << endreq ;
      for( inode = nodes.begin(); (nextnode = next(inode)) != nodes.end(); ++inode) {
	LHCb::StateVector midpoint = traj.stateVector( 0.5*(inode->z()+nextnode->z() ) ) ;
	debug() << "interval: "
		<< "(" << inode->z() << ", " << nextnode->z() << ")"
		<< " ---> midpoint deviation: " 
		<< pointerror(*inode,*nextnode,midpoint) << endreq ;
      }
    }
    
    // Now create intersections for each of the intervals.
    static IMaterialLocator::Intersections tmpintersepts ;
    Gaudi::XYZPoint p1 = nodes.front().position() ;
    for( inode = nodes.begin(); (nextnode = next(inode)) != nodes.end(); ++inode) {
      Gaudi::XYZPoint p2 = nextnode->position() ;
      MaterialLocatorBase::intersect(p1,p2-p1,tmpintersepts) ;
      intersepts.insert(intersepts.end(),tmpintersepts.begin(),tmpintersepts.end()) ;
      p1 = p2 ;
    }
  }
  
  return intersepts.size() ;
} ;

size_t MaterialLocatorBase::intersect( const LHCb::StateVector& origin, 
				       const LHCb::StateVector& target,
				       IMaterialLocator::Intersections& intersepts ) const 
{
  intersepts.clear() ;
  if( std::abs(origin.z() - target.z()) > TrackParameters::propagationTolerance) {
    // create a trajectory for interpolation
    LHCb::CubicStateVectorInterpolationTraj traj(origin,target) ;
    intersect(traj,intersepts) ;
  }
  return intersepts.size() ;
} ;


void
MaterialLocatorBase::computeMaterialCorrection(Gaudi::TrackSymMatrix& noise,
                                               Gaudi::TrackVector& delta,
                                               const IMaterialLocator::Intersections& intersepts,
                                               double zorigin,
                                               double ztarget,
                                               double momentum,
                                               LHCb::ParticleID pid) const
{
  double qop = 1/momentum ;
  bool upstream = zorigin > ztarget ;
  // reset whatever comes in
  delta = Gaudi::TrackVector() ;
  noise = Gaudi::TrackSymMatrix() ;
  // loop over all intersections and do the work. note how we
  // explicitely keep the momentum constant. note that the way we
  // write this down, we rely on the fact that it is totally
  // irrelevant how the intersepts are sorted (because the propagation
  // is assumed to be linear.) 


  // the only thing that is tricky is dealing with the fact that z1
  // and z2 need not be in increasing value, nor intersept.z1 and
  // intersept.z2. that makes calculating the overlap ('thickness') a
  // bit difficult. that's why we just reorder them.
  double zmin(zorigin),zmax(ztarget) ;
  if(upstream) std::swap(zmin,zmax) ;
  const IStateCorrectionTool* dedxtool = 11 == pid.abspid() ? &(*m_elecdedxtool) : &(*m_dedxtool) ;
  
  //static Gaudi::TrackMatrix F = Gaudi::TrackMatrix( ROOT::Math::SMatrixIdentity() );
  for( Intersections::const_iterator it = intersepts.begin() ; it != intersepts.end(); ++it) {
    double z1 = std::max( zmin, std::min(it->z1,it->z2)) ;
    double z2 = std::min( zmax, std::max(it->z1,it->z2)) ;
    double thickness = z2 - z1 ; // negative thickness means no overlap
    if( thickness > TrackParameters::propagationTolerance ) {
      double thickness = z2-z1 ;
      
      // create a state. probably it is faster not to create it. but then we need to reset the noise every time.
      LHCb::State state ;
      state.setQOverP( qop ) ;
      state.setTx( it->tx ) ;
      state.setTy( it->ty ) ;
      
      // now add the wall
      m_scatteringtool->correctState( state, it->material, thickness, upstream, pid );
      dedxtool->correctState( state, it->material, thickness, upstream, pid );
      
      // extract the energy loss
      delta(4) += state.qOverP() - qop ;

      // propagate the noise to the target. linear propagation, only
      // non-zero contributions
      double dz = upstream ? ztarget - z1 : ztarget - z2 ; 
      state.covariance()(0,0) += 2*dz*state.covariance()(2,0) + dz*dz*state.covariance()(2,2) ;
      state.covariance()(2,0) += dz*state.covariance()(2,2) ;
      state.covariance()(1,1) += 2*dz*state.covariance()(3,1) + dz*dz*state.covariance()(3,3) ;
      state.covariance()(3,1) += dz*state.covariance()(3,3) ;
      noise +=  state.covariance() ;
    }
  }
}
