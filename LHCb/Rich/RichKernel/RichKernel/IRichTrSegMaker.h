
/** @file IRichTrSegMaker.h
 *
 *  Header file for tool interface : IRichTrSegMaker
 *
 *  $Id: IRichTrSegMaker.h,v 1.6 2005-01-25 14:11:46 cattanem Exp $
 *
 *  @author Chris Jones         Christopher.Rob.Jones@cern.ch
 *  @author Antonis Papanestis  a.papanestis@rl.ac.uk
 *  @date   2003-10-28
 */

#ifndef RICHKERNEL_IRICHTRSEGMAKER_H
#define RICHKERNEL_IRICHTRSEGMAKER_H 1

// from Gaudi
#include "GaudiKernel/IAlgTool.h"

// CLHEP (explicit include needed for CLHEP 1.9.*)
#include "CLHEP/Geometry/Point3D.h"
#include "CLHEP/Geometry/Vector3D.h"

// Event model
class ContainedObject;

// Rich Kernel
class RichTrackSegment;


/// Static Interface Identification
static const InterfaceID IID_IRichTrSegMaker( "IRichTrSegMaker", 1, 0 );

/** @class IRichTrSegMaker IRichTrSegMaker.h RichKernel/IRichTrSegMaker.h
 *
 *  Interface for tools to create RichTrackSegments from various tracking objects.
 *
 *  @author Chris Jones         Christopher.Rob.Jones@cern.ch
 *  @author Antonis Papanestis  a.papanestis@rl.ac.uk
 *  @date   2003-10-28
 */

class IRichTrSegMaker : public virtual IAlgTool {

public:

  /** static interface identification
   *  @return unique interface identifier
   */
  static const InterfaceID& interfaceID() { return IID_IRichTrSegMaker; }

  /** Create RichTrackSegments for a given tracking object. Using tracking information
   *  to find intersection points, and directions at those points, with the three
   *  radiator media.
   *
   *  @param track    Pointer to tracking object to create RichTrackSegments from
   *  @param segments Returned vector of RichTrackSegment objects
   *
   *  @return Number of RichTrackSegment objects created
   *  @retval 0   Unable to create any segments - Track did not traverse any radiator
   *  @retval 1-3 Track traversed at least one radiator
   */
  virtual int constructSegments ( const ContainedObject * track,
                                  std::vector<RichTrackSegment>& segments ) const = 0;
  
};

#endif // RICHKERNEL_IRICHTRSEGMAKER_H
