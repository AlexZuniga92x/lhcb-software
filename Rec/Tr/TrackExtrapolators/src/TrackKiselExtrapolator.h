#ifndef TRACKKISELEXTRAPOLATOR_H
#define TRACKKISELEXTRAPOLATOR_H 1

// Include files
#include "TrackExtrapolator.h"
#include <vector>

// Forward declarations
class IMagneticFieldSvc;

/** @class TrackKiselExtrapolator TrackKiselExtrapolator.h
 *
 *  A TrackKiselExtrapolator is a ITrackExtrapolator which does a 'Kisel'
 *  extrapolation of a State.
 *  This code was orignally developed by I Kisel for CBM
 *  It doesn't take into account Multiple Scattering.
 *
 *  @author M Needham
 *  @date   02-06-2006
 */

class TrackKiselExtrapolator: public TrackExtrapolator
{

public:
  /// Constructor
  TrackKiselExtrapolator( const std::string& type, 
                          const std::string& name, 
                          const IInterface* parent );

  /// destructor
  virtual ~TrackKiselExtrapolator();

  /// initialize
  virtual StatusCode initialize();

  /// Propagate a state vector from zOld to zNew
  /// Transport matrix is calulated when transMat pointer is not NULL
  virtual StatusCode propagate( Gaudi::TrackVector& stateVec,
                                double zOld,
                                double zNew,
                                Gaudi::TrackMatrix* transMat,
                                LHCb::ParticleID pid = LHCb::ParticleID(211) );

  /// Propagate a state to a given z-position
  virtual StatusCode propagate( LHCb::State& pState, 
                                double z,
                                LHCb::ParticleID partId = LHCb::ParticleID(211) );

private:

  
 void getCoefficients( const double x, const double y,
                       double Xi  [3][3]      , double Yi  [3][3],
                       double Xii [3][3][3]   , double Yii [3][3][3], 
                       double Xiii[3][3][3][3], double Yiii[3][3][3][3]);

  void integrateField(const Gaudi::XYZPoint& p0, 
                      const Gaudi::XYZPoint& p1, 
                      const Gaudi::XYZPoint& p2,
                          double si  [3]      , double Si  [3],
                          double sii [3][3]   , double Sii [3][3], 
			  double siii[3][3][3], double Siii[3][3][3]);

  void extrapolateAnalytic( const double T_in [], 
                            double T_out[], 
                            double J[],const double z_in, 
                            double  z_out);

  int m_order;

  Gaudi::XYZPoint           m_point;  ///< to compute the field
  std::vector<Gaudi::XYZVector>  m_B;      ///< returned field

  std::string m_fieldSvc; /// field service name
  IMagneticFieldSvc* m_pIMF;   ///< Pointer to the magnetic field service

};

#endif // TRACKKISELEXTRAPOLATOR_H
