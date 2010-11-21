
//-----------------------------------------------------------------------------
/** @file RichVeloTTTrackSelector.h
 *
 *  Header file for RICH reconstruction tool : Rich::Rec::VeloTTTrackSelector
 *
 *  @author Chris Jones   Christopher.Rob.Jones@cern.ch
 *  @date   12/08/2006
 */
//-----------------------------------------------------------------------------

#ifndef RICHRECTOOLS_RichVeloTTTrackSelector_H
#define RICHRECTOOLS_RichVeloTTTrackSelector_H 1

// base class
#include "RichBaseTrackSelector.h"

namespace Rich
{
  namespace Rec
  {

    //-----------------------------------------------------------------------------
    /** @class VeloTTTrackSelector RichVeloTTTrackSelector.h
     *
     *  Extension to the RichBaseTrackSelector for VeloTT tracks
     *
     *  @author Chris Jones   Christopher.Rob.Jones@cern.ch
     *  @date   2006-08-12
     */
    //-----------------------------------------------------------------------------

    class VeloTTTrackSelector : public BaseTrackSelector
    {

    public: // Gaudi methods

      /// Standard constructor
      VeloTTTrackSelector( const std::string& type,
                           const std::string& name,
                           const IInterface* parent );

      /// Destructor
      virtual ~VeloTTTrackSelector( );

    };

  }
} // RICH namespace

#endif // RICHRECTOOLS_RichVeloTTTrackSelector_H
