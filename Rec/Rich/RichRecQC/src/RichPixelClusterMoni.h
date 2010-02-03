
//-----------------------------------------------------------------------------
/** @file RichPixelClusterMoni.h
 *
 *  Header file for algorithm class : Rich::Rec::PixelClusterMoni
 *
 *  CVS Log :-
 *  $Id: RichPixelClusterMoni.h,v 1.2 2010-02-03 08:17:54 jonrob Exp $
 *
 *  @author Chris Jones       Christopher.Rob.Jones@cern.ch
 *  @date   31/02/2010
 */
//-----------------------------------------------------------------------------

#ifndef RICHRECMONITOR_RichPixelClusterMoni_H
#define RICHRECMONITOR_RichPixelClusterMoni_H 1

// STD
#include <sstream>

// from Gaudi
#include "GaudiKernel/AlgFactory.h"

// base class
#include "RichRecBase/RichRecHistoAlgBase.h"

// RichKernel
#include "RichKernel/RichPoissonEffFunctor.h"
#include "RichKernel/RichStatDivFunctor.h"

// temporary histogramming numbers
#include "RichRecBase/RichDetParams.h"

// Event
#include "Event/MCRichDigitSummary.h"

// RichDet
#include "RichDet/DeRichSystem.h"

// Interfaces
#include "MCInterfaces/IRichMCTruthTool.h"
#include "MCInterfaces/IRichRecMCTruthTool.h"
#include "RichKernel/IRichRawBufferToSmartIDsTool.h"

namespace Rich
{
  namespace Rec
  {

    //-----------------------------------------------------------------------------
    /** @class PixelClusterMoni RichPixelClusterMoni.h
     *
     *  Monitor class for the clustering information in the RICH
     *
     *  @author Chris Jones   Christopher.Rob.Jones@cern.ch
     *  @date   31/02/2010
     */
    //-----------------------------------------------------------------------------

    class PixelClusterMoni : public Rich::Rec::HistoAlgBase
    {

    public:

      /// Standard constructor
      PixelClusterMoni( const std::string& name,
                        ISvcLocator* pSvcLocator );

      virtual ~PixelClusterMoni( ); ///< Destructor

      virtual StatusCode initialize();    // Algorithm initialization
      virtual StatusCode execute   ();    // Algorithm execution

    protected:

      /// Pre-Book all (non-MC) histograms
      virtual StatusCode prebookHistograms();

    private: // methods


    private: // data


    };

  }
}

#endif // RICHRECMONITOR_RichPixelClusterMoni_H
