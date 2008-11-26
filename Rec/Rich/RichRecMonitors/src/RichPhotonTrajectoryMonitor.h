
//---------------------------------------------------------------------------
/** @file RichPhotonTrajectoryMonitor.h
 *
 *  Header file for algorithm class : Rich::Rec::MC::PhotonTrajectoryMonitor
 *
 *  CVS Log :-
 *  $Id: RichPhotonTrajectoryMonitor.h,v 1.1 2008-11-26 13:42:38 jonrob Exp $
 *
 *  @author Chris Jones       Christopher.Rob.Jones@cern.ch
 *  @date   05/04/2002
 */
//---------------------------------------------------------------------------

#ifndef RICHRECMONITOR_PhotonTrajectoryMonitor_H
#define RICHRECMONITOR_PhotonTrajectoryMonitor_H 1

// base class
#include "RichRecBase/RichRecTupleAlgBase.h"

// from Gaudi
#include "GaudiKernel/AlgFactory.h"

// Event
#include "Event/MCRichOpticalPhoton.h"

// Interfaces
#include "RichRecBase/IRichTrackSelector.h"
#include "MCInterfaces/IRichRecMCTruthTool.h"

// temporary histogramming numbers
#include "RichRecBase/RichDetParams.h"

namespace Rich
{
  namespace Rec
  {
    namespace MC
    {

      //---------------------------------------------------------------------------
      /** @class PhotonTrajectoryMonitor RichPhotonTrajectoryMonitor.h
       *
       *  Monitor the reconstructed photon trajectories
       *
       *  @author Chris Jones   Christopher.Rob.Jones@cern.ch
       *  @date   05/04/2002
       */
      //---------------------------------------------------------------------------

      class PhotonTrajectoryMonitor : public RichRecTupleAlgBase
      {

      public:

        /// Standard constructor
        PhotonTrajectoryMonitor( const std::string& name,
                                 ISvcLocator* pSvcLocator );

        virtual ~PhotonTrajectoryMonitor( ); ///< Destructor

        virtual StatusCode initialize();    // Algorithm initialization
        virtual StatusCode execute   ();    // Algorithm execution

      private: // data

        const Rich::Rec::MC::IMCTruthTool * m_richRecMCTruth; ///< Pointer to RichRecMCTruthTool interface

        /// Track selector
        const ITrackSelector * m_trSelector;

      };

    }
  }
}

#endif // RICHRECMONITOR_PhotonTrajectoryMonitor_H
