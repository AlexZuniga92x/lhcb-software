// $Id: RichBinnedCherenkovResolution.cpp,v 1.7 2004-02-02 14:26:57 jonesc Exp $

// from Gaudi
#include "GaudiKernel/ToolFactory.h"
#include "GaudiKernel/ParticleProperty.h"
#include "GaudiKernel/IParticlePropertySvc.h"

// local
#include "RichBinnedCherenkovResolution.h"

// CLHEP
#include "CLHEP/Units/PhysicalConstants.h"

//-----------------------------------------------------------------------------
// Implementation file for class : RichBinnedCherenkovResolution
//
// 15/03/2002 : Chris Jones   Christopher.Rob.Jones@cern.ch
//-----------------------------------------------------------------------------

// Declaration of the Tool Factory
static const  ToolFactory<RichBinnedCherenkovResolution>          s_factory ;
const        IToolFactory& RichBinnedCherenkovResolutionFactory = s_factory ;

// Standard constructor
RichBinnedCherenkovResolution::RichBinnedCherenkovResolution ( const std::string& type,
                                                               const std::string& name,
                                                               const IInterface* parent )
  : RichRecToolBase( type, name, parent ) {

  declareInterface<IRichCherenkovResolution>(this);

  // Define job option parameters

  (m_binEdges[Rich::Aerogel]).push_back( 0.2 );
  (m_binEdges[Rich::Aerogel]).push_back( 0.25 );
  declareProperty( "NAerogelResBins", m_binEdges[Rich::Aerogel] );
  (m_theerr[Rich::Aerogel][Rich::Track::Forward]).push_back( 0.00320 );
  (m_theerr[Rich::Aerogel][Rich::Track::Forward]).push_back( 0.00242 );
  (m_theerr[Rich::Aerogel][Rich::Track::Forward]).push_back( 0.00221 );
  declareProperty( "AerogelForwardRes", m_theerr[Rich::Aerogel][Rich::Track::Forward] );
  (m_theerr[Rich::Aerogel][Rich::Track::Match]).push_back( 0.00323 );
  (m_theerr[Rich::Aerogel][Rich::Track::Match]).push_back( 0.00262 );
  (m_theerr[Rich::Aerogel][Rich::Track::Match]).push_back( 0.00221 );
  declareProperty( "AerogelMatchRes", m_theerr[Rich::Aerogel][Rich::Track::Match] );
  (m_theerr[Rich::Aerogel][Rich::Track::UpStream]).push_back( 0.00300 );
  (m_theerr[Rich::Aerogel][Rich::Track::UpStream]).push_back( 0.00272 );
  (m_theerr[Rich::Aerogel][Rich::Track::UpStream]).push_back( 0.00230 );
  declareProperty( "AerogelUpStreamRes", m_theerr[Rich::Aerogel][Rich::Track::UpStream] );
  (m_theerr[Rich::Aerogel][Rich::Track::Seed]).push_back( 999 );
  (m_theerr[Rich::Aerogel][Rich::Track::Seed]).push_back( 999 );
  (m_theerr[Rich::Aerogel][Rich::Track::Seed]).push_back( 999 );
  declareProperty( "AerogelSeedRes", m_theerr[Rich::Aerogel][Rich::Track::Seed] );
  (m_theerr[Rich::Aerogel][Rich::Track::VeloTT]).push_back( 0.01 );
  (m_theerr[Rich::Aerogel][Rich::Track::VeloTT]).push_back( 0.00478 );
  (m_theerr[Rich::Aerogel][Rich::Track::VeloTT]).push_back( 0.00273 );
  declareProperty( "AerogelVTTRes", m_theerr[Rich::Aerogel][Rich::Track::VeloTT] );
  (m_theerr[Rich::Aerogel][Rich::Track::Velo]).push_back( 999 );
  (m_theerr[Rich::Aerogel][Rich::Track::Velo]).push_back( 999 );
  (m_theerr[Rich::Aerogel][Rich::Track::Velo]).push_back( 999 );

  (m_binEdges[Rich::C4F10]).push_back( 0.03 );
  (m_binEdges[Rich::C4F10]).push_back( 0.05 );
  declareProperty( "NC4F10ResBins", m_binEdges[Rich::C4F10] );
  (m_theerr[Rich::C4F10][Rich::Track::Forward]).push_back( 0.00357 );
  (m_theerr[Rich::C4F10][Rich::Track::Forward]).push_back( 0.00226 );
  (m_theerr[Rich::C4F10][Rich::Track::Forward]).push_back( 0.00150 );
  declareProperty( "C4F10ForwardRes", m_theerr[Rich::C4F10][Rich::Track::Forward] );
  (m_theerr[Rich::C4F10][Rich::Track::Match]).push_back( 0.00385 );
  (m_theerr[Rich::C4F10][Rich::Track::Match]).push_back( 0.00231 );
  (m_theerr[Rich::C4F10][Rich::Track::Match]).push_back( 0.00180 );
  declareProperty( "C4F10MatchRes", m_theerr[Rich::C4F10][Rich::Track::Match] );
  (m_theerr[Rich::C4F10][Rich::Track::UpStream]).push_back( 0.00393 );
  (m_theerr[Rich::C4F10][Rich::Track::UpStream]).push_back( 0.00242 );
  (m_theerr[Rich::C4F10][Rich::Track::UpStream]).push_back( 0.00242 );
  declareProperty( "C4F10UpStreamRes", m_theerr[Rich::C4F10][Rich::Track::UpStream] );
  (m_theerr[Rich::C4F10][Rich::Track::Seed]).push_back( 999 );
  (m_theerr[Rich::C4F10][Rich::Track::Seed]).push_back( 999 );
  (m_theerr[Rich::C4F10][Rich::Track::Seed]).push_back( 999 );
  declareProperty( "C4F10SeedRes", m_theerr[Rich::C4F10][Rich::Track::Seed] );
  (m_theerr[Rich::C4F10][Rich::Track::VeloTT]).push_back( 0.00806 );
  (m_theerr[Rich::C4F10][Rich::Track::VeloTT]).push_back( 0.00307 );
  (m_theerr[Rich::C4F10][Rich::Track::VeloTT]).push_back( 0.00211 );
  declareProperty( "C4F10VTTRes", m_theerr[Rich::C4F10][Rich::Track::VeloTT] );
  (m_theerr[Rich::C4F10][Rich::Track::Velo]).push_back( 999 );
  (m_theerr[Rich::C4F10][Rich::Track::Velo]).push_back( 999 );
  (m_theerr[Rich::C4F10][Rich::Track::Velo]).push_back( 999 );

  (m_binEdges[Rich::CF4]).push_back( 0.03 );
  (m_binEdges[Rich::CF4]).push_back( 0.05 );
  declareProperty( "NCF4ResBins", m_binEdges[Rich::CF4] );
  (m_theerr[Rich::CF4][Rich::Track::Forward]).push_back( 0.00119 );
  (m_theerr[Rich::CF4][Rich::Track::Forward]).push_back( 0.00122 );
  (m_theerr[Rich::CF4][Rich::Track::Forward]).push_back( 0.00083 );
  declareProperty( "CF4ForwardRes", m_theerr[Rich::CF4][Rich::Track::Forward] );
  (m_theerr[Rich::CF4][Rich::Track::Match]).push_back( 0.00119 );
  (m_theerr[Rich::CF4][Rich::Track::Match]).push_back( 0.00132 );
  (m_theerr[Rich::CF4][Rich::Track::Match]).push_back( 0.00091 );
  declareProperty( "CF4MatchRes", m_theerr[Rich::CF4][Rich::Track::Match] );
  (m_theerr[Rich::CF4][Rich::Track::UpStream]).push_back( 0.00133 );
  (m_theerr[Rich::CF4][Rich::Track::UpStream]).push_back( 0.00131 );
  (m_theerr[Rich::CF4][Rich::Track::UpStream]).push_back( 0.00109 );
  declareProperty( "CF4UpStreamRes", m_theerr[Rich::CF4][Rich::Track::UpStream] );
  (m_theerr[Rich::CF4][Rich::Track::Seed]).push_back( 0.00138 );
  (m_theerr[Rich::CF4][Rich::Track::Seed]).push_back( 0.00117 );
  (m_theerr[Rich::CF4][Rich::Track::Seed]).push_back( 0.00099 );
  declareProperty( "CF4SeedRes", m_theerr[Rich::CF4][Rich::Track::Seed] );
  (m_theerr[Rich::CF4][Rich::Track::VeloTT]).push_back( 999 );
  (m_theerr[Rich::CF4][Rich::Track::VeloTT]).push_back( 999 );
  (m_theerr[Rich::CF4][Rich::Track::VeloTT]).push_back( 999 );
  declareProperty( "CF4VTTRes", m_theerr[Rich::CF4][Rich::Track::VeloTT] );
  (m_theerr[Rich::CF4][Rich::Track::Velo]).push_back( 999 );
  (m_theerr[Rich::CF4][Rich::Track::Velo]).push_back( 999 );
  (m_theerr[Rich::CF4][Rich::Track::Velo]).push_back( 999 );

}

StatusCode RichBinnedCherenkovResolution::initialize() {

  MsgStream msg( msgSvc(), name() );
  msg << MSG::DEBUG << "Initialize" << endreq;

  // Sets up various tools and services
  if ( !RichRecToolBase::initialize() ) return StatusCode::FAILURE;

  // Acquire instances of tools
  acquireTool( "RichCherenkovAngle", m_ckAngle );

  // Informational Printout
  msg << MSG::DEBUG
      << " Using binned track resolutions" << endreq;
  for ( int iR = 0; iR < Rich::NRadiatorTypes; ++iR ) {
    msg << " " << (Rich::RadiatorType)iR << " Resolution bins = " << m_binEdges[iR] << endreq;
    for ( int iT = 0; iT < Rich::Track::NTrTypes; ++iT ) {
      msg << " " << (Rich::RadiatorType)iR << " " << (Rich::Track::Type)iT
          << " Cherenkov Resolution = " << m_theerr[iR][iT] << endreq;
    }
  }

  return StatusCode::SUCCESS;
}

StatusCode RichBinnedCherenkovResolution::finalize() {

  MsgStream msg( msgSvc(), name() );
  msg << MSG::DEBUG << "Finalize" << endreq;

  // release tools
  releaseTool( m_ckAngle );

  // Execute base class method
  return RichRecToolBase::finalize();
}

double
RichBinnedCherenkovResolution::ckThetaResolution( RichRecSegment * segment,
                                                  const Rich::ParticleIDType id ) const {

  // Expected Cherenkov theta angle
  const double thetaExp = m_ckAngle->avgCherenkovTheta( segment, id );
  if ( thetaExp < 0.000001 ) return 0;

  const Rich::RadiatorType rad = segment->trackSegment().radiator();
  const Rich::Track::Type type = segment->richRecTrack()->trackID().trackType();
  double res = 0;
  if ( thetaExp > 0. &&  thetaExp < (m_binEdges[rad])[0] ) {
    res = (m_theerr[rad][type])[0];
  } else if ( thetaExp > (m_binEdges[rad])[0] &&
              thetaExp < (m_binEdges[rad])[1] ) {
    res = (m_theerr[rad][type])[1];
  } else if ( thetaExp > (m_binEdges[rad])[1] ) {
    res = (m_theerr[rad][type])[2];
  }

  return res;
}
