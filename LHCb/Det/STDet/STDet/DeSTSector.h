// $Id: DeSTSector.h,v 1.18 2007-07-23 09:34:18 wouter Exp $
#ifndef _DeSTSector_H_
#define _DeSTSector_H_

#include <string>
#include <vector>
#include <utility>

#include "Kernel/STChannelID.h"
#include "STDet/DeSTBaseElement.h"

#include "GaudiKernel/Plane3DTypes.h"
#include "LHCbMath/LineTypes.h"


/** @class DeSTSector DeSTSector.h "STDet/DeSTSector.h"
 *
 *  Class representing a TT Half module (7 sensors ganged together)
 *
 *  @author Matthew Needham Matthew.Needham@cern.ch
 *
 * <b> Additional information: </b>
 * \li <a href="http://doc.cern.ch//archive/electronic/cern/others/LHB/internal/lhcb-2006-034.pdf"><b>LHCb note on STDet</b></a>
   \li  <a href="http://ckm.physik.unizh.ch/software"><b>ST Software page</b></a><p>
 */


namespace LHCb{
  class Trajectory;
}


class DeSTSector : public DeSTBaseElement  {

public:

  /** Constructor */
  DeSTSector ( const std::string& name = "" ) ;

  /** Destructor */
  virtual ~DeSTSector(); 
 
  /** initialization method 
  * @return Status of initialisation
  */
  virtual StatusCode initialize();

  /** sector identfier 
  * @return id 
  */
  unsigned int id() const;

  /** set sector id */
  void setID(const unsigned int id);

  /** check whether contains
  *  @param  aChannel channel
  *  @return bool
  */
  bool contains(const LHCb::STChannelID aChannel) const;

  /** detector pitch
  * @return pitch
  */
  double pitch() const;

  /** number of strips
  * @return number of strips
  */
  unsigned int nStrip() const;

  /** 
  * check if valid strip number
  * 
  */
  bool isStrip(const unsigned int strip) const;

  /** convert a local u to a strip 
  * @param  u local u
  * @return strip 
  **/ 
  unsigned int localUToStrip(const double u) const;

  /** convert strip to local U
  * @param strip
  * @param offset
  * @return local u
  */
  double localU(const unsigned int strip, double offset = 0.) const;

  /** trajectory 
  * @return trajectory for the fit 
  */
  std::auto_ptr<LHCb::Trajectory> trajectory(const LHCb::STChannelID& aChan,
                                             const double offset) const;
  
  /** trajectory 
  * @return trajectory of the first strip
  */
  std::auto_ptr<LHCb::Trajectory> trajectoryFirstStrip() const;

  /** trajectory 
  * @return trajectory of the last strip
  */
  std::auto_ptr<LHCb::Trajectory> trajectoryLastStrip() const;

  /** create an xyzline for a strip*/
  Gaudi::XYZLineF lineTrajectory(unsigned int strip, float offset) const ;

  /** plane corresponding to the sector 
  * @return the plane 
  */
  Gaudi::Plane3D plane() const;  
 
  /** plane corresponding to the module entrance
  * @return the plane 
  */
  Gaudi::Plane3D entryPlane() const; 

  /** plane corresponding to the module exit
  * @return the plane 
  */
  Gaudi::Plane3D exitPlane() const; 

  /** localInActive
  * @param  point point in local frame
  * @param  tol   tolerance
  * @return bool in active region 
  */
  bool localInActive(const Gaudi::XYZPoint& point, 
                     Gaudi::XYZPoint tol = Gaudi::XYZPoint(0.,0.,0.)) const;
   
  /** localInBondGap
  * @param  v   v in local frame
  * @param  tol tolerance
  * @return bool in a bond gap - ie dead region
  */
  bool localInBondGap( const double v ,double tol = 0) const;
  
  /** localInBox
  * @param u    u in local frame
  * @param v    v in local frame
  * @param uTol
  * @param vTol
  * @return bool in active region of box 
  */
  bool localInBox(const double u, const double v, 
		  double uTol = 0, double vTol = 0) const;


  /** globalInActive
  * @param  point point in global frame
  * @param  tol   tolerance
  * @return bool in active region 
  */
  bool globalInActive(const Gaudi::XYZPoint& point, 
                     Gaudi::XYZPoint tol = Gaudi::XYZPoint(0.,0.,0.)) const;
   
  /** globalInBondGap
  * @param  gpoint point in global frame
  * @param  tol    tolerance
  * @return bool in a bond gap - ie dead region
  */
  bool globalInBondGap(const Gaudi::XYZPoint& gpoint ,double tol = 0) const;
  
  /** globalInBox
  * @param  gpoint point in global frame
  * @param  tol    vTol
  * @return bool in active region of box 
  */
  bool globalInBox(const Gaudi::XYZPoint& gpoint, 
	 	   Gaudi::XYZPoint tol = Gaudi::XYZPoint(0.,0.,0.)) const;

  /** 
   * @return capacitance
  */ 
  double capacitance() const;

  /** strip length 
  * @return strip length
  */
  double stripLength() const;


  /** thickness 
  * @return thickness
  */
  double thickness() const;

  /** get the next channel left */
  LHCb::STChannelID nextLeft(const LHCb::STChannelID testChan) const;

  /** get the next channel right */
  LHCb::STChannelID nextRight(const LHCb::STChannelID testChan) const;

  /**
  * @return type
  */
  std::string type() const;

  /** @return stereo angle */
  double angle() const;

  /** @return sin of stereo angle */
  double sinAngle() const;

  /** @return cosine of stereo angle */
  double cosAngle() const;

  /** print to stream */
  std::ostream& printOut( std::ostream& os ) const;
     
  /** print to msgstream */
  MsgStream& printOut( MsgStream& os) const;

private:

  void clear();
  void determineSense();
  StatusCode cacheInfo();

  Gaudi::Plane3D m_plane;
  Gaudi::Plane3D m_entryPlane;
  Gaudi::Plane3D m_exitPlane;

  Gaudi::XYZVector m_direction;
  std::pair<double,double> m_range;

  unsigned int m_firstStrip;
  unsigned int m_id;
  double m_pitch;
  unsigned int m_nStrip;
  double m_capacitance;
  double m_stripLength;

  double m_thickness;
  double m_uMinLocal;
  double m_uMaxLocal;
  double m_vMinLocal; 
  double m_vMaxLocal;



  LHCb::Trajectory* m_midTraj;


  bool m_xInverted;
  bool m_yInverted;

  double m_deadWidth;
  std::vector<double> m_deadRegions;
  std::string m_type;

  Gaudi::XYZVectorF m_vectorLayer ;
  Gaudi::XYZPointF  m_positionLayer ;
  Gaudi::XYZVectorF m_vectorStrip ; 
  double m_angle ;
  double m_cosAngle ;
  double m_sinAngle ;

};

inline unsigned int DeSTSector::id() const{
  return m_id;
}

inline void DeSTSector::setID(unsigned int id) {
  m_id = id;
}

inline bool DeSTSector::contains(const LHCb::STChannelID aChannel) const{
  return (aChannel.uniqueSector() == elementID().uniqueSector());
}

inline double DeSTSector::pitch() const{
  return m_pitch;
}

inline unsigned int DeSTSector::nStrip() const{
  return m_nStrip;
}

inline bool DeSTSector::isStrip(const unsigned int strip) const {
  return (strip >= m_firstStrip && strip < m_firstStrip + m_nStrip);
}

inline double DeSTSector::capacitance() const{
  return m_capacitance;
}

inline double DeSTSector::stripLength() const {
  return m_stripLength;
}

inline double DeSTSector::thickness() const{
  return m_thickness;
}

inline std::string DeSTSector::type() const {
  return m_type;
}

inline Gaudi::Plane3D DeSTSector::plane() const {
  return m_plane;
}

inline Gaudi::Plane3D DeSTSector::entryPlane() const {
  return m_entryPlane;
}

inline Gaudi::Plane3D DeSTSector::exitPlane() const {
  return m_exitPlane;
}

inline double DeSTSector::angle() const {
  return m_angle;
}

inline double DeSTSector::sinAngle() const {
  return m_sinAngle;
}

inline double DeSTSector::cosAngle() const {
  return m_cosAngle;
}

inline Gaudi::XYZLineF DeSTSector::lineTrajectory(unsigned int strip,float offset ) const {
  float numstrips = offset + strip - m_firstStrip ;
  return Gaudi::XYZLineF( m_positionLayer + numstrips * m_vectorLayer, m_vectorStrip ) ;
}

/** ouput operator for class DeSTSector
 *  @see DeSTSector
 *  @see MsgStream
 *  @param os      reference to STL output stream
 *  @param aSector reference to DeSTSector object
 */
inline std::ostream& operator<<( std::ostream& os , const DeSTSector* aSector )
{ return aSector->printOut( os ); }


/** ouput operator for class DeSTSector
 *  @see DeSTSector
 *  @see MsgStream
 *  @param os      reference to MsgStream output stream
 *  @param aSector reference to DeSTSector object
 */
inline MsgStream& operator<<( MsgStream& os , const DeSTSector* aSector )
{ return aSector->printOut( os ); }


#endif // _DeSTSector_H










