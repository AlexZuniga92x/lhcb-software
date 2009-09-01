#ifndef TRACKEXTRAPOLATORS_TRFIELDEXTRAPOLATORBASE_H 
#define TRACKEXTRAPOLATORS_TRFIELDEXTRAPOLATORBASE_H 


// Include files
// -------------
// from TrackExtrapolators
#include "TrackExtrapolator.h"

//#include "GaudiKernel/Vector3DTypes.h"
//#include "GaudiKernel/Point3DTypes.h"

class ILHCbMagnetSvc ;
#include "DetDesc/MagneticFieldGrid.h"
#include "Kernel/ILHCbMagnetSvc.h"

/** @class TrackFieldExtrapolatorBase TrackFieldExtrapolatorBase.h TrackExtrapolators/TrackFieldExtrapolatorBase.h
 *
 *  A TrackFieldExtrapolatorBase is a TrackExtrapolator with access to the magnetic field
 *
 *  @author Wouter Hulsbergen
 *  @date   16/07/2009
 */

class TrackFieldExtrapolatorBase: public TrackExtrapolator
{

public:
  typedef Gaudi::XYZVector FieldVector ;
  typedef Gaudi::Matrix3x3 FieldGradient ;
  /// constructor
  TrackFieldExtrapolatorBase( const std::string& type, 
			      const std::string& name, 
			      const IInterface* parent );

  /// destructor
  ~TrackFieldExtrapolatorBase();

  /// initialize (picks up the field service)
  StatusCode initialize() ;

  /// finalize
  StatusCode finalize() ;

  /// access to the field
  FieldVector fieldVector( const Gaudi::XYZPoint& position ) const {
    ++m_numFieldCalls ;
    return (m_fieldGrid->*m_fieldFunction)( position ) ;
    //return m_fieldSvc->fieldVector(position) ;
  }
  
  /// access to the field gradient
  FieldGradient fieldGradient( const Gaudi::XYZPoint& position ) const {
    return m_fieldGrid->fieldGradient( position ) ;
  }
  
private:
  bool m_useGridInterpolation ;                ///< Flag whether to interpolate on the grid or not
  std::string m_fieldSvcName;                  ///< Name of the field service
  const ILHCbMagnetSvc* m_fieldSvc ;           ///< Pointer to the magnetic field service
  const LHCb::MagneticFieldGrid* m_fieldGrid ; ///< Pointer to the magnetic field grid
  Gaudi::XYZVector (LHCb::MagneticFieldGrid::*m_fieldFunction)(const Gaudi::XYZPoint&)const ;
  mutable size_t m_numFieldCalls ;
};

#endif // TRACKEXTRAPOLATORS_TRLINEAREXTRAPOLATOR_H
