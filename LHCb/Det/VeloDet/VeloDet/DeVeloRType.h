// $Id: DeVeloRType.h,v 1.16 2006-04-28 14:22:50 mtobin Exp $
#ifndef VELODET_DEVELORTYPE_H 
#define VELODET_DEVELORTYPE_H 1

// Include files

// from Kernel
#include "Kernel/Point3DTypes.h"

/// from VeloDet
#include "VeloDet/DeVeloSensor.h"

/// From LHCb Kernel
#include "Kernel/VeloChannelID.h"

// get trajectory
#include "Kernel/CircleTraj.h"

// Unique class identifier
static const CLID& CLID_DeVeloRType = 1008102 ;


/** @class DeVeloRType DeVeloRType.h VeloDet/DeVeloRType.h
 *  
 *
 *  @author Mark Tobin
 *  @author Kurt Rinnert kurt.rinnert@cern.ch
 *  @date   2003-01-14
 */
class DeVeloRType : public DeVeloSensor {
public:
  /// Standard constructor
  DeVeloRType( const std::string& name = "" ); 

  virtual ~DeVeloRType( ); ///< Destructor

  /// object identifier (static method)
  static  const CLID& classID() { return CLID_DeVeloRType; }
  /// object identification
  virtual const CLID& clID()     const;

  /// Initialise the DeVeloSensor from the XML
  virtual StatusCode initialize();

  /// Calculate the nearest channel to a 3-d point.
  /// Also returns the fractional difference in the channel
  /// and the local pitch.
  virtual StatusCode pointToChannel(const Gaudi::XYZPoint& point,
                                    LHCb::VeloChannelID& channel,
                                    double& fraction,
                                    double& pitch) const;
  
  /// Get the nth nearest neighbour for a given channel
  virtual StatusCode neighbour(const LHCb::VeloChannelID& start, 
                               const int& nOffset, 
                               LHCb::VeloChannelID& channel) const;

  /// Return a trajectory (for track fit) from strip + offset
  virtual std::auto_ptr<LHCb::Trajectory> trajectory(const LHCb::VeloChannelID& id, const double offset) const;

  /// Residual of 3-d point to a VeloChannelID
  virtual StatusCode residual(const Gaudi::XYZPoint& point, 
                              const LHCb::VeloChannelID& channel,
                              double &residual,
                              double &chi2) const;
  
  /// Residual [see DeVelo for explanation]
  virtual StatusCode residual(const Gaudi::XYZPoint& point,
                      const LHCb::VeloChannelID& channel,
                      const double localOffset,
                      const double width,
                      double &residual,
                      double &chi2) const;
  
  /// The number of zones in the detector
  //  inline unsigned int numberOfZones(){return m_numberOfZones;}
 
  /// The zones number for a given strip
  virtual unsigned int zoneOfStrip(const unsigned int strip) const {
    return static_cast<unsigned int>(strip/512);
  }

  /// The number of strips in a zone
  virtual unsigned int stripsInZone(const unsigned int /*zone*/) const{
    return m_stripsInZone;
  }

  /// The minimum radius for a given zone of the sensor
  virtual double rMin(const unsigned int /*zone*/) const {
    return innerRadius();
  }
  
  /// The maximum radius for a given zone of the sensor
  virtual double rMax(const unsigned int /*zone*/) const {return outerRadius();}

  /// Determines if local 3-d point is inside sensor
  virtual StatusCode isInActiveArea(const Gaudi::XYZPoint& point) const;

  /// Determine if local point is in corner cut-offs
  virtual bool isCutOff(double x, double y) const;

  /// Zone for a given local phi
  unsigned int zoneOfPhi(double phi) const {
    unsigned int zone=0;
    if(m_phiMax[0] > phi){
      zone = 0;
    } else if(m_phiMin[1] <= phi && m_phiMax[1] > phi){
      zone = 1;
    } else if(m_phiMin[2] <= phi && m_phiMax[2] > phi) {
      zone = 2;
    } else if(m_phiMin[3] <= phi) {
      zone = 3;
    }
    return zone;
  }
  
  /// Minimum strip number for each zone
  inline unsigned int firstStrip(unsigned int zone) const {return zone*512;}

  /// Return the radius of the strip
  inline double rOfStrip(const unsigned int strip) const {
    return m_rStrips[strip];
  }

  /// Return the radius of the strip+fraction
  inline double rOfStrip(unsigned int strip, double fraction) const {
    return m_rStrips[strip]+fraction*rPitch(strip);
  }

  /// Return the local pitch of the sensor for a given strip
  inline double rPitch(unsigned int strip) const {return m_rPitch[strip];}

  /// Return the local pitch of the sensor for a given strip +/- fraction
  inline double rPitch(unsigned int strip, double fraction) const {
    return exp(fraction)*m_rPitch[strip];
  }

  /// Return the local pitch at a given radius 
  inline double rPitch(double radius) const {  
    return m_innerPitch + m_pitchSlope*(radius - innerRadius());
  }

  /** The minimum phi for a zone assuming the radius is not known.  
      This is a constant value for zone 0 and should be used with caution **/
  inline double phiMinZone(unsigned int zone) const {return m_phiMin[zone];}

  /// Returns the minimum phi in a zone at given radius
  double phiMinZone(unsigned int zone, double radius) const;
  
  /// The maximum phi for a zone
  inline double phiMaxZone(unsigned int zone) const {return m_phiMax[zone];}
     
  /** The maximum phi for a zone assuming the radius is not known.  
      This is a constant value for zone 2 and should be used with caution **/
  double phiMaxZone(unsigned int zone, double radius) const;
   
  /// The minimum phi of a strip
  double phiMinStrip(unsigned int strip) const {
    return m_stripPhiLimits[strip].first;
  }
  
  /// The maximum phi of a strip
  double phiMaxStrip(unsigned int strip) const {
    return m_stripPhiLimits[strip].second; 
  }

  /// Return the strip limits for panoramix
  inline StatusCode stripLimits(unsigned int strip, double &radius,
                         double &phiMin, double &phiMax) const {
    radius = rOfStrip(strip);
    phiMin = phiMinStrip(strip);
    phiMax = phiMaxStrip(strip);
    return StatusCode::SUCCESS;
  }

private:
  /// Store the local radius for each strip in the sensor
  void calcStripLimits();

  /// Store the angular limits of phi zones
  void phiZoneLimits();

  /// Store the co-ordinates of the cut-offs
  void cornerLimits();

  /// Return x and y position for the intersect of the cut-off line and a given radius
  void intersectCutOff(const double radius, double& x, double& y) const;

  //  unsigned int m_numberOfZones;
  unsigned int m_stripsInZone;
  std::vector<double> m_rStrips;
  std::vector<double> m_rPitch;
  std::vector<double> m_phiMin;
  std::vector<double> m_phiMax;
  double m_cornerX1;
  double m_cornerY1;
  double m_cornerX2;
  double m_cornerY2;  
  double m_cornerXInt;//<X intercept for corner cut off
  double m_cornerYInt;//<Y intercept for corner cut off
  double m_gradCutOff;//<Gradient of line defining cut offs
  double m_intCutOff;//<x intercept for line which defines cut offs
  std::vector< std::pair<double,double> > m_stripPhiLimits;//Min/Max phi of strips
  std::pair<double,double> m_resolution;//Resolution from LHCB??????
  double m_innerPitch;
  double m_outerPitch;
  double m_innerR;//< Radius of first strip
  double m_outerR;//< Radius of outer strip
  double m_overlapInX;
  double m_pitchSlope;
  double m_halfAngle;
  double m_quarterAngle;
  double m_phiGap;

  /// Build up map of strip to routing line conversions
  void BuildRoutingLineMap();
  // Reverse middle group of four strips (0123->0213)
  unsigned int ScrambleStrip(unsigned int strip){
    if(1 == strip%4){
      strip++;
    } else if(2 == strip%4) {
      strip--;
    }
    return strip;
  }
  /// Select routing line area
  /// (0) Chips 15 to 13, (1) Chip 12, (2) Chip 11 and (3) Chips 10 to 8
  /// Pattern repeats for chips 7 to 0
  unsigned int RoutingLineArea(unsigned int routingLine);
  /// Return strip from routing line area
  unsigned int RoutLineToStrip(unsigned int routLine, unsigned int routArea);
  /// Number of strips in each readout area
  static const unsigned int m_nChan0=384;
  static const unsigned int m_nChan1=128;
  static const unsigned int m_nChan2=128;
  static const unsigned int m_nChan3=384;
};
#endif // VELODET_DEVELORTYPE_H
