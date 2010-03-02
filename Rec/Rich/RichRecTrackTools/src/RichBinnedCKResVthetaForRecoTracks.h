
//-----------------------------------------------------------------------------
/** @file RichBinnedCKResVthetaForRecoTracks.h
 *
 *  Header file for tool : Rich::Rec::BinnedCKResVthetaForRecoTracks
 *
 *  CVS Log :-
 *  $Id: RichBinnedCKResVthetaForRecoTracks.h,v 1.2 2009-01-20 15:49:30 cattanem Exp $
 *
 *  @author Chris Jones   Christopher.Rob.Jones@cern.ch
 *  @date   15/03/2002
 */
//-----------------------------------------------------------------------------

#ifndef RICHRECTOOLS_RICHBINNEDCKRESVTHETAFORTRSTOREDTRACKS_H
#define RICHRECTOOLS_RICHBINNEDCKRESVTHETAFORTRSTOREDTRACKS_H 1

// base class
#include "RichRecBase/RichRecToolBase.h"

// from Gaudi
#include "GaudiKernel/ToolFactory.h"

// Event model
#include "Event/RichRecSegment.h"

// interfaces
#include "RichRecBase/IRichCherenkovResolution.h"
#include "RichRecBase/IRichCherenkovAngle.h"
#include "RichKernel/IRichParticleProperties.h"

// Kernel
#include "RichKernel/RichMap.h"

namespace Rich
{
  namespace Rec
  {

    //-----------------------------------------------------------------------------
    /** @class BinnedCKResVthetaForRecoTracks RichBinnedCKResVthetaForRecoTracks.h
     *
     *  Tool to calculate the Cherenkov angle resolution. This version is
     *  for reconstructed Tracks and implements an approach that uses simple bins in the
     *  cherenkov angle theta space and also varies according to the track algorithm type.
     *
     *  @author Chris Jones   Christopher.Rob.Jones@cern.ch
     *  @date   15/03/2002
     */
    //-----------------------------------------------------------------------------

    class BinnedCKResVthetaForRecoTracks : public Rich::Rec::ToolBase,
                                           virtual public ICherenkovResolution
    {

    public: // Methods for Gaudi Framework

      /// Standard constructor
      BinnedCKResVthetaForRecoTracks( const std::string& type,
                                      const std::string& name,
                                      const IInterface* parent );

      /// Destructor
      virtual ~BinnedCKResVthetaForRecoTracks() {};

      // Initialize method
      StatusCode initialize();

      // Finalize method
      StatusCode finalize();

    public: // methods (and doxygen comments) inherited from public interface

      // Photon resolution
      double ckThetaResolution( LHCb::RichRecSegment * segment,
                                const Rich::ParticleIDType id = Rich::Pion ) const;

    private: // methods

      // Photon resolution
      bool ckThetaResolution_Imp( LHCb::RichRecSegment * segment,
                                  const Rich::ParticleIDType id,
                                  double & res ) const;

    private: // Private data

      /// Pointer to RichCherenkovAngle interface
      const ICherenkovAngle * m_ckAngle;

      /// Pointer to RichParticleProperties interface
      const IParticleProperties * m_richPartProp;

      typedef std::vector<double> BinData;
      typedef std::map< Rich::Rec::Track::Type, BinData > BinDataMap;
      /// The averged resolutions in each bin, for each track type
      mutable BinDataMap m_theerr[Rich::NRadiatorTypes];

      typedef std::vector<std::pair<double,double> > BinEdges;
      /// The boundaries for the resolution bins
      BinEdges m_binEdges[Rich::NRadiatorTypes];

      /// make sure all hypo resolutions are the same
      bool m_normalise;

      /// Particle ID types to consider in the photon creation checks
      Rich::Particles m_pidTypes;

    };

  }
}

#endif // RICHRECTOOLS_RICHBINNEDCKRESVTHETAFORTRSTOREDTRACKS_H
