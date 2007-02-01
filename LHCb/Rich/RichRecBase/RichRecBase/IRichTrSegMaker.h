
//-----------------------------------------------------------------------
/** @file IRichTrSegMaker.h
 *
 *  Header file for tool interface : Rich::Rec::ITrSegMaker
 *
 *  $Id: IRichTrSegMaker.h,v 1.2 2007-02-01 17:26:22 jonrob Exp $
 *
 *  @author Chris Jones         Christopher.Rob.Jones@cern.ch
 *  @author Antonis Papanestis  a.papanestis@rl.ac.uk
 *  @date   2003-10-28
 */
//-----------------------------------------------------------------------

#ifndef RICHKERNEL_IRICHTRSEGMAKER_H
#define RICHKERNEL_IRICHTRSEGMAKER_H 1

// from Gaudi
#include "GaudiKernel/IAlgTool.h"

// MathCore
#include "Kernel/Point3DTypes.h"
#include "Kernel/Vector3DTypes.h"

// Event model
class ContainedObject;

// forward decs
namespace LHCb
{
  class RichTrackSegment;
}

/// Static Interface Identification
static const InterfaceID IID_IRichTrSegMaker( "Rich::Rec::ITrSegMaker", 1, 0 );

//-----------------------------------------------------------------------------
/** @namespace Rich
 *
 *  General namespace for RICH software
 *
 *  @author Chris Jones  Christopher.Rob.Jones@cern.ch
 *  @date   08/07/2004
 */
//-----------------------------------------------------------------------------
namespace Rich
{

  /** @namespace Rich::Rec
   *
   *  General namespace for RICH reconstruction software
   *
   *  @author Chris Jones  Christopher.Rob.Jones@cern.ch
   *  @date   08/07/2004
   */
  namespace Rec
  {

    //-----------------------------------------------------------------------
    /** @class ITrSegMaker IRichTrSegMaker.h RichKernel/IRichTrSegMaker.h
     *
     *  Interface for tools to create RichTrackSegments from various tracking objects.
     *
     *  @author Chris Jones         Christopher.Rob.Jones@cern.ch
     *  @author Antonis Papanestis  a.papanestis@rl.ac.uk
     *  @date   2003-10-28
     */
    //-----------------------------------------------------------------------

    class ITrSegMaker : public virtual IAlgTool
    {

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
                                      std::vector<LHCb::RichTrackSegment*>& segments ) const = 0;

    };

  }
} // RICH

#endif // RICHKERNEL_IRICHTRSEGMAKER_H
