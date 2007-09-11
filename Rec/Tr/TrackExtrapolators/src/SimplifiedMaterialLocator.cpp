#include "SimplifiedMaterialLocator.h"

#include <set>

#include "GaudiKernel/ToolFactory.h"
#include "DetDesc/Material.h"
#include "DetDesc/SolidTubs.h"
#include "DetDesc/SolidBox.h"
#include "DetDesc/DetDesc.h"
#include "DetDesc/IPVolume.h"
#include "DetDesc/LVolume.h"
#include "DetDesc/VolumeIntersectionIntervals.h"

namespace MaterialLocatorUtils {
  
  class PVolumeWrapper
  {
  public:
    PVolumeWrapper( const IPVolume& volume )
      : m_volume( volume )
    {
      // get the global position of the center 
      m_zcenter = volume.toMother( Gaudi::XYZPoint(0,0,0) ).z() ;
      // determine the size in z
      const SolidTubs* solidtubs = dynamic_cast<const SolidTubs*>(solid()) ;
      const SolidBox*  solidbox  = dynamic_cast<const SolidBox*>(solid()) ;
      if( solidbox ) {
	m_halfzlength = solidbox->zHalfLength() ;
      } else if( solidtubs ) {
	m_halfzlength = solidtubs->zHalfLength() ;
      } else {
	throw GaudiException(std::string("TrackingGeometry cannot deal with solid named: ") + solid()->name(),
			     "TrackingGeometry.h",StatusCode::FAILURE) ;
      }
      m_hasdaughters = lvolume()->noPVolumes()>0 ;
    }
    
    virtual ~PVolumeWrapper() {}
    
    const std::string& name() const { return m_volume.name() ; }
    inline unsigned int addintersections(const Gaudi::XYZPoint&  p,const Gaudi::XYZVector& v, ILVolume::Intersections& ) const ;
    const Material* material() const { return m_volume.lvolume()->material() ; }
    const ISolid* solid() const { return m_volume.lvolume()->solid() ; }
    const ILVolume* lvolume() const { return m_volume.lvolume() ; }

  private:
    const IPVolume& m_volume ;
    double m_zcenter ;
    double m_halfzlength ;
    bool m_hasdaughters ;
  } ;

  unsigned int PVolumeWrapper::addintersections(const Gaudi::XYZPoint&  p,const Gaudi::XYZVector& v,
						ILVolume::Intersections& intersections) const
  {
    int rc(0) ;
    // do a quick z-overlap test. the intersectionTicks calls
    // don't actually test for tick boundaries. here we use that
    // our ticks must be between 0 and 1.
    double z1 = p.z()-m_zcenter ;
    double z2 = z1 + v.z() ;
    if(z1>z2) std::swap(z1,z2) ;
    if(-m_halfzlength <= z2 && z1 <= m_halfzlength) {
      // now we are goign to use that we know that the toplevel transform is unity, except for a z-translation. so not:
      //   rc = m_volume.intersectLine(p,v,intersections,0,1,0) ;
      // but
      Gaudi::XYZPoint pprime = p ; 
      pprime.SetZ(p.z()-m_zcenter) ;
      if(m_hasdaughters) {
	static ILVolume::Intersections own ;
	own.clear() ;
	rc = m_volume.lvolume()->intersectLine(pprime,v,own,0,1,0) ;
	std::copy( own.begin(), own.end(),  std::back_inserter( intersections ) );
      } else {
	// half the volumes don't have daughters. if they don't, we do
	// this ourself because the intersectLine call above is just
	// too slow.
	static ISolid::Ticks ticks ;
	ticks.clear() ;
	solid()->intersectionTicks(pprime,v,ticks) ;
	if(ticks.size()%2==1) std::cout << "odd number of ticks!" << std::endl ;
	for(unsigned int itick=0; itick+1<ticks.size(); itick+=2) {
	  double firsttick  = ticks[itick] ;
	  double secondtick = ticks[itick+1] ;
	  if( firsttick <= 1 && secondtick >= 0 ) {
	    ILVolume::Intersection intersect ;
	    intersect.first.first = std::max(0.,firsttick) ;
	    intersect.first.second = std::min(1.,secondtick) ;
	    intersect.second = material() ;
	    intersections.push_back( intersect ) ;
	    ++rc ;
	  }
	}
      }
    }
    return rc ;
  }
}

DECLARE_TOOL_FACTORY( SimplifiedMaterialLocator );

SimplifiedMaterialLocator::SimplifiedMaterialLocator( const std::string& type,
								const std::string& name,
								const IInterface* parent )
  : MaterialLocatorBase(type, name, parent)
{ 
  declareInterface<IMaterialLocator>(this);
  declareProperty( "Geometry" , m_tgvolname = "/dd/TrackfitGeometry/Geometry/lvLHCb" );
}

StatusCode SimplifiedMaterialLocator::initialize()
{  
  info() << "SimplifiedMaterialLocator::initialize()" << endreq ;
  StatusCode sc = MaterialLocatorBase::initialize();
  if( sc.isSuccess() ) {
    
    DetDesc::Services* services = DetDesc::services();
    IDataProviderSvc*  detsvc  = services->detSvc();
    SmartDataPtr<const ILVolume> tgvol(detsvc,m_tgvolname) ;
    if( 0== tgvol ) {
      error() << "Did not find TrackfitGeometry volume " << m_tgvolname << endreq ;
      sc = StatusCode::FAILURE ;
    } else {
      info() << "Found TrackfitGeometry volume with " << tgvol->pvolumes().size() << " daughters." << endreq ;
      for( ILVolume::PVolumes::const_iterator it = tgvol->pvBegin() ;
	   it != tgvol->pvEnd() ; ++it) 
	m_volumes.push_back( new MaterialLocatorUtils::PVolumeWrapper(**it) ) ;
    }
  } 

  printProps( MSG::DEBUG );
  
  return sc;
}

StatusCode SimplifiedMaterialLocator::finalize()
{  
  info() << "SimplifiedMaterialLocator::finalize()" << endreq ;
  for( VolumeContainer::const_iterator it = m_volumes.begin() ; 
       it != m_volumes.end(); ++it) delete *it ;
  m_volumes.clear() ;
  return StatusCode::SUCCESS ;
}

inline bool compareFirstTick( const ILVolume::Intersection& lhs, const ILVolume::Intersection& rhs )
{
  return lhs.first.first<rhs.first.first ;
}

size_t SimplifiedMaterialLocator::intersect( const Gaudi::XYZPoint& start, const Gaudi::XYZVector& vect, 
					     ILVolume::Intersections& intersepts ) const 
{ 
  // for now, no navigation
  intersepts.clear();
  for( VolumeContainer::const_iterator it = m_volumes.begin() ; 
       it != m_volumes.end(); ++it) (*it)->addintersections(start,vect,intersepts) ;
  std::sort( intersepts.begin(), intersepts.end(), compareFirstTick) ;
  return intersepts.size() ;
}
