
/** @file DeRich.h
 *
 *  Header file for detector description class : DeRich
 *
 *  CVS Log :-
 *  $Id: DeRich.h,v 1.7 2004-09-01 15:20:19 papanest Exp $
 *  $Log: not supported by cvs2svn $
 *  Revision 1.6  2004/07/27 08:55:22  jonrob
 *  Add doxygen file documentation and CVS information
 *
 *
 *  @author Antonis Papanestis a.papanestis@rl.ac.uk
 *  @date   2004-06-18
 */

#ifndef RICHDET_DERICH_H
#define RICHDET_DERICH_H 1

// DetDesc
#include "DetDesc/DetectorElement.h"
#include "DetDesc/TabulatedProperty.h"

// CLHEP
#include "CLHEP/Geometry/Point3D.h"
#include "CLHEP/Geometry/Vector3D.h"
#include "CLHEP/Geometry/Plane3D.h"

// LHCbKernel
#include "Kernel/RichSide.h"

#include "RichDet/RichMirrorSegPosition.h"

/** @namespace DeRichLocation
 *
 *  Namespace for the xml location of the detector elements Rich1 & Rich2
 *
 *  @author Antonis Papanestis a.papanestis@rl.ac.uk
 *  @date   2004-06-18
 */
namespace DeRichLocation {

  /// Rich1 location in transient detector store
  static const std::string& Rich1 = "/dd/Structure/LHCb/Rich1";

  /// Rich2 location in transient detector store
  static const std::string& Rich2 = "/dd/Structure/LHCb/Rich2";

}

/** @class DeRich DeRich.h
 *
 * Class for generic info about the Rich(1/2) system.
 * DeRich is an abstract class contaning the common parts of the DeRich1
 * and DeRich2 classes.
 *
 * @author Antonis Papanestis a.papanestis@rl.ac.uk
 */
class DeRich: public DetectorElement {

public:
  /**
   * Constructor for this class
   */
  DeRich();

  /**
   * Default destructor
   */
  virtual ~DeRich();

  /**
   * This is where most of the geometry is read and variables initialised
   * @return Status of initialisation
   * @retval StatusCode::FAILURE Initialisation failed, program should
   * terminate
   */
  virtual StatusCode initialize();

  /**
   * Returns the nominal centre of curvature of the spherical mirror for 
   * this Rich
   *
   * @param side Which side: top, bottom (Rich1), left, right (Rich2)
   * @return The nominal centre of curvature
   */
  virtual const HepPoint3D& nominalCentreOfCurvature(Rich::Side side) const =0;

  /**
   * Returns the nominal normal vector of the flat mirror plane for this Rich
   *
   * @param side Which side: top, bottom (Rich1), left, right (Rich2)
   * @return The nominal normal vector
   */
  virtual const HepNormal3D& nominalNormal(Rich::Side side) const = 0;

  /**
   * Returns the nominal flat mirror plane for this Rich
   *
   * @param side Which side: top, bottom (Rich1), left, right (Rich2)
   * @return The nominal flat mirror plane
   */
  virtual const HepPlane3D& nominalPlane(Rich::Side side) const = 0;

  /**
   * Check on which side of this Rich lies this point
   *
   * @param point A point in the global coordinate system
   * @return The side for this point
   */
  virtual Rich::Side side( const HepPoint3D& point ) const = 0;


  /**
   * Returns the nominal spherical mirror radius for this Rich
   *
   * @return The nominal spherical mirror radius
   */
  inline virtual double sphMirrorRadius() const {
    return m_sphMirrorRadius;
  }

  /**
   * Returns a pointer to the tabulated property that holds the refractive
   * index for the gas window for this Rich
   *
   * @return Pointer to gas window refIndex
   */
  inline virtual const TabulatedProperty* gasWinRefIndex() {
    return m_gasWinRefIndex;
  }

  /**
   * Returns a pointer to the tabulated property that holds the absorption
   * length of the gas window for this Rich
   *
   * @return Pointer gas window absorption length
   */
  inline virtual const TabulatedProperty* gasWinAbsLength() {
    return m_gasWinAbsLength;
  }

  /**
   * Returns a pointer to the tabulated property that holds the nominal quantum
   * efficiency of an HPD. 
   *
   * @return Pointer to quantum efficiency (can be null)
   */
  inline virtual const TabulatedProperty* nominalHPDQuantumEff() {
    return m_HPDQuantumEff;
  }

  /**
   * Method to find the row/column of a spherical mirror segment.  It can 
   * be used to test if the mirror segment is at the edge or not
   * @return Position (row/column) for this spherical mirror segment
   */
  virtual RichMirrorSegPosition sphMirrorSegPos( int mirrorNumber );

  /**
   * Method to find the row/column of a flat mirror segment. It can be used to 
   * test if the mirror segment is at the edge or not
   *
   * @return Position (row/column) for this flat mirror segment
   */
  virtual RichMirrorSegPosition flatMirrorSegPos( int mirrorNumber );

  /**
   * Method to test if the user parameter vector is in the xml description
   *
   * @return The outcome of the test
   * @retval true  The vector exists as a user parameter in the xml
   * @retval false The vector is not in the xml description
   */
  bool hasParamVector( std::string vectorName );

  /**
   * Method to test if the user parameter is in the xml description
   *
   * @return The outcome of the test
   * @retval true  The parameter exists as a user parameter in the xml
   * @retval false The parameter is not in the xml description
   */
  bool hasParam( std::string paramName );

protected:

  /// vector of strings
  typedef std::vector<std::string> strings;

  double m_sphMirrorRadius; ///< The nominal radius of the spherical mirror

  /// The nominal centre of curvature of the spherical mirror (positive side)
  HepPoint3D  m_nominalCentreOfCurvature;

  /// The nominal normal vector of the flat mirror plane (positive side)
  HepNormal3D m_nominalNormal;

  typedef std::vector<std::string> strings;
  strings m_vectorNames;  ///< the names of the user parameter vectors
  strings m_paramNames;   ///< the names of the user parameters

  int m_sphMirrorSegRows;  ///< number of spherical mirror rows
  int m_sphMirrorSegCols;  ///< number of spherical mirror columns
  int m_flatMirrorSegRows; ///< number of flat mirror rows
  int m_flatMirrorSegCols; ///< number of flat mirror columns

  /// flag to test if the xml supports mirror position info  
  bool m_positionInfo;

  /// refractive index of the quartz gas window
  const TabulatedProperty* m_gasWinRefIndex; 
  /// absorption length of the quartz gas window
  const TabulatedProperty* m_gasWinAbsLength; 
  /// HPD quantum efficiency
  const TabulatedProperty* m_HPDQuantumEff;

};

#endif    // RICHDET_DERICH_H
