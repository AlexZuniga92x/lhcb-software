#ifndef TRACKTOOLS_TRAJECTORYPROVIDER_H 
#define TRACKTOOLS_TRAJECTORYPROVIDER_H 1

// Include files
// -------------
// from Gaudi
#include "GaudiAlg/GaudiTool.h"

// from GaudiKernel
#include "GaudiKernel/IMagneticFieldSvc.h"

// from LHCbKernel
#include "Kernel/IVeloClusterPosition.h"
#include "Kernel/ISTClusterPosition.h"
#include "Kernel/LHCbID.h"
#include "Kernel/Trajectory.h"

// from TrackEvent
#include "Event/Measurement.h"
#include "Event/State.h"

// from TrackInterfaces
#include "TrackInterfaces/ITrajectoryProvider.h"

// from XxxDet
#include "VeloDet/DeVelo.h"
#include "OTDet/DeOTDetector.h"
#include "STDet/DeSTDetector.h"

static const InterfaceID IID_TrajectoryProvider ( "TrajectoryProvider", 1, 0 );

/** @class TrajectoryProvider TrajectoryProvider.h
 *  
 *
 *  @author Eduardo Rodrigues
 *  @date   2006-02-17
 */
class TrajectoryProvider : public GaudiTool,
                           virtual public ITrajectoryProvider {
public:
  /// Standard constructor
  TrajectoryProvider( const std::string& type, 
                      const std::string& name,
                      const IInterface* parent);
  
  virtual ~TrajectoryProvider( ); ///< Destructor
  
  StatusCode initialize();
  
  /// Return a "Measurement Trajectory" from a Measurement
  virtual LHCb::Trajectory* trajectory( LHCb::Measurement& meas );
  
  /** Return a "Measurement Trajectory" from an LHCbID
   *  Note: the meaning of the offset input depends on the sub-detector type
   *  @return Pointer to the Trajectory created
   *  @param  id:     input LHCbID
   *  @param  offset: input offset
   */
  virtual LHCb::Trajectory* trajectory( const LHCb::LHCbID& id,
                                        double offset = 0 );
  
  /// Return a "State Trajectory" from a State
  virtual LHCb::Trajectory* trajectory( const LHCb::State& state );
  
  /// Return a "State Trajectory" from a State vector and a z-position
  virtual LHCb::Trajectory* trajectory( const Gaudi::TrackVector& stateVector,
                                        double z );
  
protected:
  // Interfaces
  IVeloClusterPosition* m_veloPositionTool;   ///< Velo cluster position tool
  ISTClusterPosition*   m_stPositionTool;     ///< ST cluster position tool
  std::string           m_stPositionToolName; ///< ST cluster position tool name
  std::string           m_veloPositionToolName; ///< Velo cluster position tool name

  // Geometry information
  DeVelo*       m_veloDet;
  DeSTDetector* m_ttDet;
  DeSTDetector* m_itDet;
  DeOTDetector* m_otDet;

  std::map<unsigned int,DetectorElement*> m_dets;
  
  std::string   m_veloDetPath;        ///< Name of the Velo XML geom path
  std::string   m_ttDetPath;          ///< Name of the TT XML geom path
  std::string   m_itDetPath;          ///< Name of the IT XML geom path
  std::string   m_otDetPath;          ///< Name of the OT XML geom path
  
  // Magnetic Field Service
  IMagneticFieldSvc* m_magsvc;
  std::string        m_magsvcname;    ///< Name of the Magnetic Field Service
  
};
#endif // TRACKTOOLS_TRAJECTORYPROVIDER_H
