
/** @file RichRecTools_load.cpp
 *
 *  Declaration of objects in the component library RichRecTools
 *
 *  CVS Log :-
 *  $Id: RichRecTools_load.cpp,v 1.19 2004-10-30 19:38:44 jonrob Exp $
 *  $Log: not supported by cvs2svn $
 *  Revision 1.18  2004/07/27 20:15:32  jonrob
 *  Add doxygen file documentation and CVS information
 *
 *
 *  @author Chris Jones   Christopher.Rob.Jones@cern.ch
 *  @date   15/03/2002
 */

#include "GaudiKernel/DeclareFactoryEntries.h"

DECLARE_FACTORY_ENTRIES( RichRecTools ) 
{

  // Rich reconstruction working object creators
  DECLARE_TOOL( RichDelegatedTrackCreatorFromTrStoredTracks );
  DECLARE_TOOL( RichTrackCreatorFromTrStoredTracks );
  DECLARE_TOOL( RichSegmentCreator );
  DECLARE_TOOL( RichPixelCreatorFromRichDigits );
  DECLARE_TOOL( RichPixelCreatorFromRawBuffer );
  DECLARE_TOOL( RichPhotonCreator );
  DECLARE_TOOL( RichPhotonPredictor );
  DECLARE_TOOL( RichPhotonPredictorUsingRings );
  DECLARE_TOOL( RichStatusCreator ); 
  DECLARE_TOOL( RichMassHypothesisRingCreator );

  // Detector geometry
  DECLARE_TOOL( RichRecGeomTool );

  // Geometrical efficiency calculators
  DECLARE_TOOL( RichGeomEffPhotonTracing );
  DECLARE_TOOL( RichGeomEffFixedValue );

  // Signal calculators
  DECLARE_TOOL( RichExpectedTrackSignal );
  DECLARE_TOOL( RichPhotonSignal );
  DECLARE_TOOL( RichTabulatedSignalDetectionEff );

  // Physical properties
  DECLARE_TOOL( RichSellmeirFunc );
  DECLARE_TOOL( RichParticleProperties );
  DECLARE_TOOL( RichFunctionalRayleighScatter );
  DECLARE_TOOL( RichTabulatedRayleighScatter );
  DECLARE_TOOL( RichTabulatedGasQuartzWindowAbs );

  // Cherenkov angle tools
  DECLARE_TOOL( RichCherenkovAngle );
  DECLARE_TOOL( RichBinnedCKResVthetaForTrStoredTracks );
  DECLARE_TOOL( RichInterpCKResVthetaForTrStoredTracks );
  DECLARE_TOOL( RichFunctionalCKResVpForTrStoredTracks );
  DECLARE_TOOL( RichInterpCKResVpForTrStoredTracks );

  // ray tracing
  DECLARE_TOOL( RichRayTraceCherenkovCone );

};
