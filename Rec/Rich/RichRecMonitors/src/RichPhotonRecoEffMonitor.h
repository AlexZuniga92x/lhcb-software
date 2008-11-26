
//---------------------------------------------------------------------------
/** @file RichPhotonRecoEffMonitor.h
 *
 *  Header file for algorithm class : Rich::Rec::MC::PhotonRecoEffMonitor
 *
 *  CVS Log :-
 *  $Id: RichPhotonRecoEffMonitor.h,v 1.6 2008-11-26 13:42:38 jonrob Exp $
 *
 *  @author Chris Jones       Christopher.Rob.Jones@cern.ch
 *  @date   05/04/2002
 */
//---------------------------------------------------------------------------

#ifndef RICHRECMONITOR_RICHTRACKRESOLUTIONMONI_H
#define RICHRECMONITOR_RICHTRACKRESOLUTIONMONI_H 1

// base class
#include "RichRecBase/RichRecHistoAlgBase.h"

// from Gaudi
#include "GaudiKernel/AlgFactory.h"

// Event
#include "Event/MCRichOpticalPhoton.h"

// Interfaces
#include "RichRecBase/IRichTrackSelector.h"
#include "MCInterfaces/IRichRecMCTruthTool.h"
#include "RichRecBase/IRichCherenkovAngle.h"
#include "RichRecBase/IRichRecGeomTool.h"

// temporary histogramming numbers
#include "RichRecBase/RichDetParams.h"

namespace Rich
{
  namespace Rec
  {
    namespace MC
    {

      //---------------------------------------------------------------------------
      /** @class PhotonRecoEffMonitor RichPhotonRecoEffMonitor.h
       *
       *  Monitor the photon reconstruction efficiency
       *
       *  @author Chris Jones   Christopher.Rob.Jones@cern.ch
       *  @date   05/04/2002
       */
      //---------------------------------------------------------------------------

      class PhotonRecoEffMonitor : public RichRecHistoAlgBase
      {

      public:

        /// Standard constructor
        PhotonRecoEffMonitor( const std::string& name,
                              ISvcLocator* pSvcLocator );

        virtual ~PhotonRecoEffMonitor( ); ///< Destructor

        virtual StatusCode initialize();    // Algorithm initialization
        virtual StatusCode execute   ();    // Algorithm execution
        virtual StatusCode finalize  ();    // Algorithm finalization

      private: // data

        const Rich::Rec::MC::IMCTruthTool* m_richRecMCTruth;    ///< Pointer to RichRecMCTruthTool interface
        const ICherenkovAngle * m_ckAngle;          ///< Rich Cherenkov angle calculator tool
        const IGeomTool * m_geomTool;            ///< Simple geometrical questions
        const IPhotonCreator * m_forcedPhotCreator; ///< Forced photon creator
        const ITrackSelector * m_trSelector;  ///< Track selector

      };

    }
  }
}

#endif // RICHRECMONITOR_RICHTRACKRESOLUTIONMONI_H
