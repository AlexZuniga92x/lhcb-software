
//-----------------------------------------------------------------------------
/** @file RichCKthetaBandsPhotonPredictor.h
 *
 *  Header file for tool : Rich::Rec::CKthetaBandsPhotonPredictor
 *
 *  CVS Log :-
 *  $Id: RichCKthetaBandsPhotonPredictor.h,v 1.2 2007-08-10 18:15:27 jonrob Exp $
 *
 *  @author Chris Jones   Christopher.Rob.Jones@cern.ch
 *  @date   26/07/2007
 */
//-----------------------------------------------------------------------------

#ifndef RICHRECTOOLS_RichCKthetaBandsPhotonPredictor_H
#define RICHRECTOOLS_RichCKthetaBandsPhotonPredictor_H 1

// base class
#include "RichRecBase/RichRecToolBase.h"

// Event
#include "Event/RichRecPixel.h"
#include "Event/RichRecSegment.h"

// Interfaces
#include "RichRecBase/IRichPhotonPredictor.h"
#include "RichRecBase/IRichRecGeomTool.h"
#include "RichRecBase/IRichCherenkovAngle.h"
#include "RichRecBase/IRichCherenkovResolution.h"
#include "RichKernel/IRichParticleProperties.h"

// RichKernel
#include "RichKernel/RichPoissonEffFunctor.h"

// GSL
#include "gsl/gsl_math.h"

// boost
#include "boost/format.hpp"

namespace Rich
{
  namespace Rec
  {

    //-----------------------------------------------------------------------------
    /** @class CKthetaBandsPhotonPredictor RichCKthetaBandsPhotonPredictor.h
     *
     *  Tool which performs the association between RichRecTracks and
     *  RichRecPixels to form RichRecPhotons.
     *
     *  This particular implementation defines bans around the expected CK theta
     *  values for the allows mass hyothesis types, that are defined as n sigma times
     *  the expected CK resolution. 
     *  The hit is required to be in range for at least one possible meass hypothesis.
     *
     *  @author Chris Jones   Christopher.Rob.Jones@cern.ch
     *  @date   26/07/2007
     */
    //-----------------------------------------------------------------------------

    class CKthetaBandsPhotonPredictor : public RichRecToolBase,
                                        virtual public IPhotonPredictor
    {

    public: // Methods for Gaudi Framework

      /// Standard constructor
      CKthetaBandsPhotonPredictor( const std::string& type,
                                   const std::string& name,
                                   const IInterface* parent );

      /// Destructor
      virtual ~CKthetaBandsPhotonPredictor(){}

      // Initialize method
      StatusCode initialize();

    public: // methods (and doxygen comments) inherited from public interface

      // Is it possible to make a photon candidate using this segment and pixel.
      bool photonPossible( LHCb::RichRecSegment * segment,
                           LHCb::RichRecPixel * pixel ) const;

    private: // private data

      /// Geometry tool
      const IGeomTool * m_geomTool;

      /// Pointer to Cherenkov angle tool
      const ICherenkovAngle * m_ckAngle;

      /// Pointer to Cherenkov angle resolution tool
      const ICherenkovResolution * m_ckRes;

      /// Pointer to RichParticleProperties interface
      const IParticleProperties * m_richPartProp;

      std::vector<double> m_minROI;  ///< Min hit radius of interest around track centres
      std::vector<double> m_maxROI;  ///< Max hit radius of interest around track centres
      std::vector<double> m_maxROI2; ///< Square of m_maxROI
      std::vector<double> m_minROI2; ///< Square of m_minROI
      std::vector<double> m_ckThetaMax; ///< Scaling parameter - Max CK theta point
      std::vector<double> m_sepGMax;    ///< Scaling parameter - Max separation point
      std::vector<double> m_nSigma;     ///< N sigma for acceptance bands
      std::vector<double> m_scale;      ///< Internal cached parameter for speed

      /// Particle ID types to consider in the likelihood minimisation
      Rich::Particles m_pidTypes;

    };

  }
}

#endif // RICHRECTOOLS_RichCKthetaBandsPhotonPredictor_H
