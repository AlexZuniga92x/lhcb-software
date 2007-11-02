// $Id: VeloClusterPosition.h,v 1.9 2007-11-02 13:55:48 szumlat Exp $
#ifndef VELOCLUSTERPOSITION_H 
#define VELOCLUSTERPOSITION_H 1

// Include files
// from Gaudi
#include "GaudiAlg/GaudiTool.h"
#include "Kernel/IVeloClusterPosition.h"            // Interface
#include "Kernel/VeloChannelID.h"
#include "Kernel/SiPositionInfo.h"
#include "Event/StateVector.h"
#include "GaudiMath/GaudiMath.h"

/** @class VeloClusterPosition VeloClusterPosition.h
 *  
 *
 *  @author Tomasz Szumlak
 *  @date   2005-09-30
 */

class DeVelo;
class DeVeloSensor;

class VeloClusterPosition: public GaudiTool, virtual public IVeloClusterPosition{
public:

  // throughout the code LA stands for Linear Approximation
  // typedefs for object returned by tool

  enum conv{
    RAD_TO_DEG=180
  };
  //-- type of the error parametrization
  enum paraType{
    PITCH_PARA=1,
    ANGLE_PARA
  };
  
  typedef IVeloClusterPosition::toolInfo toolInfo;
  typedef IVeloClusterPosition::Direction Direction;
  typedef std::pair<double, double> Pair;
  // structure to keep resolution parametrisations
  typedef std::pair<double, Pair> ResPair;
  typedef std::vector<ResPair> ResVector;

  virtual StatusCode initialize();    ///< Tool initialization
  virtual StatusCode finalize();      ///< Tool finalization
  /// Standard constructor
  VeloClusterPosition( const std::string& type, 
                       const std::string& name,
                       const IInterface* parent);
  virtual ~VeloClusterPosition( ); ///< Destructor
  //-- public interface ----------------------------------------------
  virtual toolInfo position(const LHCb::VeloCluster* aCluster) const;
  virtual toolInfo position(const LHCb::VeloCluster* aCluster,
                            const Gaudi::XYZPoint& aPoint,
                            const Direction& aDirection);
  virtual toolInfo position(const LHCb::VeloCluster* aCluster,
                            const LHCb::StateVector& aState);
  //------------------------------------------------------------------
  //-- returns value of the projected angle
  double projectedAngle() const;
  
protected:

  toolInfo weightedLAPos(const LHCb::VeloCluster* cluster) const;
  //-- if state is not available use simpler version of the 
  //-- error estimation
  double meanResolution(const double& pitch) const;
  //-- this method can be used when full state is available
  double errorEstimate(const double projAngle, const double pitch) const;
  GaudiMath::Interpolation::Type splineType() const;
  //-- calculates the value of the projected angle
  Pair projectedAngle(const DeVeloSensor* sensor);
  virtual double fracPosLA(const LHCb::VeloCluster* aCluster) const;
  double angleOfTrack(const Direction& localSlopes,
                      Gaudi::XYZVector& parallel2Track) const;
  Direction localTrackDirection(const Gaudi::XYZVector& globalTrackDir,
                                const DeVeloSensor* sensor) const;
  
private:

  DeVelo* m_veloDet;                       /// detector element
  std::vector<double> m_defaultResolution; /// resolution para one angle
  std::vector<double> m_p0Values;          /// vector of constants
  std::vector<double> m_p1Values;          /// vector of slopes
  std::vector<double> m_projAngles;        /// proj angle range
  std::vector<double> m_errAnglePara;      /// angle projection para
  mutable double m_projectedAngle;         /// value of projected angle
  bool m_calculateExactProjAngle;          /// method of calculation flag
  unsigned int m_errorParaType;     /// angle bins==1, pitch bins==2
  GaudiMath::SimpleSpline* m_p0;    /// spline for constants
  GaudiMath::SimpleSpline* m_p1;    /// spline for slopes
  std::string m_splineType;         /// type of spline - string
  double m_minAngle;                /// first angle - lower limit
  double m_maxAngle;                /// last angle - higher limit
  Gaudi::XYZVector m_trackDir;      /// a track direction in global ref. frame
  Gaudi::XYZPoint m_gloPoint;       /// point on sensor global ref. frame
  LHCb::VeloChannelID m_centreChan; /// lite cluster centre channel
  double m_fracPos;                 /// fractional position
    
};
#endif // VELOCLUSTERPOS_H
