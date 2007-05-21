#ifndef TRACKEXTRAPOLATORS_DETAILEDMATERIALLOCATOR_H 
#define TRACKEXTRAPOLATORS_DETAILEDMATERIALLOCATOR_H

// Include files
// -------------
// from Gaudi
// #include "GaudiAlg/GaudiTool.h"
#include "GaudiKernel/ServiceHandle.h"

#include "DetDesc/ITransportSvc.h"

// from TrackInterfaces
#include "MaterialLocatorBase.h"

/** @class DetailedMaterialLocator DetailedMaterialLocator.h \
 *         "DetailedMaterialLocator.h"
 *
 * Implementation of a IMaterialLocator that uses the TransportSvc for
 * finding materials on a trajectory.
 *
 *  @author Wouter Hulsbergen
 *  @date   21/05/2007
 */

class DetailedMaterialLocator : public MaterialLocatorBase
{
public: 
  /// Constructor
  DetailedMaterialLocator( const std::string& type, 
			   const std::string& name, 
			   const IInterface* parent ) ;
  /// intialize
  virtual StatusCode initialize();
  virtual StatusCode finalize() { return StatusCode::SUCCESS ; }
  
  /// Intersect a line with volumes in the geometry
  virtual size_t intersect( const Gaudi::XYZPoint& p, const Gaudi::XYZVector& v, 
			    ILVolume::Intersections& intersepts ) const ;
  
private:
  double m_minRadThickness ;                     ///< minimum radiation thickness
  ServiceHandle<ITransportSvc> m_transportSvc ;  ///< handle to transport service
};

#endif // TRACKEXTRAPOLATORS_DETAILEDMATERIALLOCATOR_H 
