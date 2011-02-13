
//-----------------------------------------------------------------------------
/** @file RichPhotonRecoTestAlg.h
 *
 *  Header file for algorithm class : Rich::Rec::PhotonRecoTestAlg
 *
 *  @author Chris Jones       Christopher.Rob.Jones@cern.ch
 *  @date   21/02/2006
 */
//-----------------------------------------------------------------------------

#ifndef RICHRECMONITOR_RICHPIXELMONITOR_H
#define RICHRECMONITOR_RICHPIXELMONITOR_H 1

// base class
#include "RichRecBase/RichRecHistoAlgBase.h"

// Interfaces
#include "RichRecBase/IRichPhotonReconstruction.h"

// RichDet
#include "RichDet/DeRichRadiator.h"

// RichKernel
#include "RichKernel/BoostArray.h"

namespace Rich
{
  namespace Rec
  {

    //-----------------------------------------------------------------------------
    /** @class PhotonRecoTestAlg RichPhotonRecoTestAlg.h
     *
     *  Debug algorithm for photon reconstruction.
     *
     *  Performs some "fixed" reconstructions in each radiator, to allow easy
     *  comparisions between different software versions etc.
     *
     *  @author Chris Jones   Christopher.Rob.Jones@cern.ch
     *  @date   21/02/2006
     */
    //-----------------------------------------------------------------------------

    class PhotonRecoTestAlg : public Rich::Rec::HistoAlgBase
    {

    public:

      /// Standard constructor
      PhotonRecoTestAlg( const std::string& name,
                         ISvcLocator* pSvcLocator );

      virtual ~PhotonRecoTestAlg( ); ///< Destructor

      virtual StatusCode initialize();    // Algorithm initialization
      virtual StatusCode execute   ();    // Algorithm execution

    private: // data

      /// Pointer to photon reconstruction tool
      const IPhotonReconstruction * m_photonReco;

      /// Photon reconstruction tool runtime name
      std::string m_photonRecoName;

      /// typedef of array of DeRichRadiators
      typedef boost::array<const DeRichRadiator *, Rich::NRadiatorTypes> Radiators;
      /// Array of radiators
      Radiators m_radiators;

    };

  }
}

#endif // RICHRECMONITOR_RICHPIXELMONITOR_H
