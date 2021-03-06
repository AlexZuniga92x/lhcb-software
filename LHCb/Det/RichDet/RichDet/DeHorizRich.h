
/** @file DeHorizRich.h
 *
 *  Header file for detector description class : DeHorizRich
 *
 *  @author Antonis Papanestis antonis.papanestis@stfc.ac.uk
 *  @date   2012-10-18
 */

#ifndef RICHDET_DEHORIZRICH_H
#define RICHDET_DEHORIZRICH_H 1

// local
#include "RichDet/DeRich.h"

// External declarations
extern const CLID CLID_DEHorizRich;


/** @class DeHorizRich RichDet/DeHorizRich.h
 *
 * This is the DeRich detector element for HorizRich
 *
 * @author Antonis Papanestis a.papanestis@rl.ac.uk
 */

class DeHorizRich: public DeRich
{

public:
  /**
   * Constructor for this class
   */
  DeHorizRich(const std::string & name = "");

  /**
   * Default destructor
   */
  virtual ~DeHorizRich();

  /**
   * Retrieves reference to class identifier
   * @return the class identifier for this class
   */
  const CLID& clID() const { return classID(); }

  /**
   * Retrieves reference to class identifier
   * @return the class identifier for this class
   */
  static const CLID& classID();

  /**
   * Specific HorizRich initialisation
   * @return Status of initialisation
   */
  virtual StatusCode initialize();

  /**
   * Returns the nominal centre of curvature of the spherical mirror for this Rich
   *
   * @param side Which side: left or right
   * @return The nominal centre of curvature
   */
  virtual const Gaudi::XYZPoint& nominalCentreOfCurvature(const Rich::Side side) const;

  /**
   * Returns the nominal normal vector of the flat mirror plane for this Rich
   *
   * @param side Which side: left or right
   * @return The nominal normal vector
   */
  virtual const Gaudi::XYZVector& nominalNormal(const Rich::Side side) const;

  /**
   * Returns the nominal flat mirror plane for this Rich
   *
   * @param side Which side: left or right
   * @return The nominal flat mirror plane
   */
  virtual const Gaudi::Plane3D& nominalPlane(const Rich::Side side) const;

  /**
   * Check on which side of this Rich lies this point
   *
   * @param point A point in the global coordinate system
   * @return The side for this point
   */
  virtual Rich::Side side( const Gaudi::XYZPoint& point ) const;

private:

  /// Access the name for a given panel
  virtual const std::string panelName( const Rich::Side panel ) const;

private:

  Gaudi::Plane3D m_nominalPlaneLeft;   ///< The left nominal flat mirror plane
  Gaudi::Plane3D m_nominalPlaneRight;  ///< The right nominal flat mirror plane

  /// The nominal centre of curvature of the spherical mirror (negative side)
  Gaudi::XYZPoint  m_nominalCentreOfCurvatureRight;

  /// The nominal centre of curvature of the spherical mirror (positive side)
  Gaudi::XYZPoint  m_nominalCentreOfCurvatureLeft;

  /// The nominal normal vector of the flat mirror plane (negative side)
  Gaudi::XYZVector m_nominalNormalRight;

  /// The nominal normal vector of the flat mirror plane (positive side)
  Gaudi::XYZVector m_nominalNormalLeft;

};

#endif    // RICHDET_DEHORIZRICH_H
