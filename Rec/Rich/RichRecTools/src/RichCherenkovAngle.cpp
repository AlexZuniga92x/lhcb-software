// $Id: RichCherenkovAngle.cpp,v 1.6 2004-03-16 13:45:02 jonesc Exp $

// from Gaudi
#include "GaudiKernel/ToolFactory.h"
#include "GaudiKernel/ParticleProperty.h"
#include "GaudiKernel/IParticlePropertySvc.h"

// local
#include "RichCherenkovAngle.h"

// CLHEP
#include "CLHEP/Units/PhysicalConstants.h"

//-----------------------------------------------------------------------------
// Implementation file for class : RichCherenkovAngle
//
// 15/03/2002 : Chris Jones   Christopher.Rob.Jones@cern.ch
//-----------------------------------------------------------------------------

// Declaration of the Tool Factory
static const  ToolFactory<RichCherenkovAngle>          s_factory ;
const        IToolFactory& RichCherenkovAngleFactory = s_factory ;

// Standard constructor
RichCherenkovAngle::RichCherenkovAngle ( const std::string& type,
                                         const std::string& name,
                                         const IInterface* parent )
  : RichRecToolBase( type, name, parent ) {

  declareInterface<IRichCherenkovAngle>(this);

}

StatusCode RichCherenkovAngle::initialize() {

  debug() << "Initialize" << endreq;

  // Sets up various tools and services
  StatusCode sc = RichRecToolBase::initialize();
  if ( sc.isFailure() ) { return sc; }

  // Acquire instances of tools
  acquireTool( "RichExpectedTrackSignal", m_signal       );
  acquireTool( "RichRefractiveIndex",     m_refIndex     );
  acquireTool( "RichParticleProperties",  m_richPartProp );

  return StatusCode::SUCCESS;
}

StatusCode RichCherenkovAngle::finalize() {

  debug() << "Finalize" << endreq;

  // Execute base class method
  return RichRecToolBase::finalize();
}

double RichCherenkovAngle::avgCherenkovTheta( RichRecSegment * segment,
                                              const Rich::ParticleIDType id ) const {

  if ( !segment->averageCKTheta().dataIsValid(id) ) {
    double angle = 0;

    // total unscattered signal
    double unscat = m_signal->nSignalPhotons( segment, id );
    if ( unscat > 0 ) {

      // which radiator
      const Rich::RadiatorType rad = segment->trackSegment().radiator();

      // Beta for this segment
      const double beta = m_richPartProp->beta(segment, id);

      // loop over energy bins
      RichPhotonSpectra & sigSpectra = segment->signalPhotonSpectra();
      for ( unsigned int iEnBin = 0; iEnBin < sigSpectra.energyBins(); ++iEnBin ) {
        double temp = beta *
          m_refIndex->refractiveIndex( rad, sigSpectra.binEnergy(iEnBin) );
        angle += (sigSpectra.energyDist(id))[iEnBin] * ( temp>1 ? acos(1/temp) : 0 );
      }
      
      // normalise the angle
      angle /= unscat;

    }

    segment->setAverageCKTheta( id, angle );
  }

  return segment->averageCKTheta( id );
}

double RichCherenkovAngle::avgCherenkovTheta( RichRecSegment * segment ) const
{
  return avgCherenkovTheta( segment, segment->richRecTrack()->currentHypothesis() );
}
