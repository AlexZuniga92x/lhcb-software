
//-----------------------------------------------------------------------------
/** @file RichTabulatedSignalDetectionEff.cpp
 *
 *  Implementation file for tool : Rich::Rec::TabulatedSignalDetectionEff
 *
 *  @author Chris Jones   Christopher.Rob.Jones@cern.ch
 *  @date   15/03/2002
 */
//-----------------------------------------------------------------------------

// local
#include "RichTabulatedSignalDetectionEff.h"

// Gaudi
#include "GaudiKernel/ToolFactory.h"
#include "GaudiKernel/SystemOfUnits.h"

// All code is in general Rich reconstruction namespace
using namespace Rich::Rec;

//-----------------------------------------------------------------------------

DECLARE_TOOL_FACTORY( TabulatedSignalDetectionEff )

// Standard constructor
TabulatedSignalDetectionEff::
TabulatedSignalDetectionEff ( const std::string& type,
                              const std::string& name,
                              const IInterface* parent )
  : ToolBase         ( type, name, parent ),
    m_coneTrace      ( NULL ),
    m_ckAngle        ( NULL ),
    m_riches         ( Rich::NRiches ),
    m_qEffPedLoss    ( 0 ),
    m_traceModeRad   ( Rich::NRadiatorTypes ),
    m_nPoints        ( Rich::NRadiatorTypes, 50 ),
    m_last_segment   ( NULL ),
    m_last_ring      ( NULL ),
    m_last_hypo      ( Rich::Unknown )
{
  // interface
  declareInterface<ISignalDetectionEff>(this);
  // JOs
  declareProperty( "UseDetailedHPDsInRayTracing", m_useDetailedHPDsForRayT = false );
  declareProperty( "RadiatorRingPoints", m_nPoints );
}

StatusCode TabulatedSignalDetectionEff::initialize()
{
  // Sets up various tools and services
  const StatusCode sc = ToolBase::initialize();
  if ( sc.isFailure() ) { return sc; }

  // get tools
  acquireTool( "RichRayTraceCKCone", m_coneTrace );
  acquireTool( "RichCherenkovAngle", m_ckAngle   );

  // Rich1 and Rich2
  m_riches[Rich::Rich1] = getDet<DeRich1>( DeRichLocations::Rich1 );
  m_riches[Rich::Rich2] = getDet<DeRich2>( DeRichLocations::Rich2 );

  // PD panels
  m_pdPanels[Rich::Rich1][Rich::top]    = getDet<DeRichPDPanel>(pdPanelName(Rich::Rich1,Rich::top));
  m_pdPanels[Rich::Rich1][Rich::bottom] = getDet<DeRichPDPanel>(pdPanelName(Rich::Rich1,Rich::bottom));
  m_pdPanels[Rich::Rich2][Rich::left]   = getDet<DeRichPDPanel>(pdPanelName(Rich::Rich2,Rich::left));
  m_pdPanels[Rich::Rich2][Rich::right]  = getDet<DeRichPDPanel>(pdPanelName(Rich::Rich2,Rich::right));

  // the ray-tracing mode
  LHCb::RichTraceMode tmpMode ( LHCb::RichTraceMode::RespectHPDTubes,
                                ( m_useDetailedHPDsForRayT ?
                                  LHCb::RichTraceMode::FullHPDs :
                                  LHCb::RichTraceMode::SimpleHPDs ) );
  m_traceModeRad[Rich::Aerogel]  = tmpMode;
  m_traceModeRad[Rich::Aerogel].setAeroRefraction(true);
  m_traceModeRad[Rich::Rich1Gas] = tmpMode;
  m_traceModeRad[Rich::Rich2Gas] = tmpMode;

  // Quartz window eff
  const double qEff = m_riches[Rich::Rich1]->param<double>( "HPDQuartzWindowEff" );

  // Digitisation pedestal loss
  const double pLos = m_riches[Rich::Rich1]->param<double>( "HPDPedestalDigiEff" );

  // store cached value
  m_qEffPedLoss = qEff * pLos;

  // Informational Printout
  if ( msgLevel(MSG::DEBUG) )
  {
    debug() << "Aerogel  Track " << m_traceModeRad[Rich::Aerogel]  << endmsg;
    debug() << "Rich1Gas Track " << m_traceModeRad[Rich::Rich1Gas] << endmsg;
    debug() << "Rich2Gas Track " << m_traceModeRad[Rich::Rich2Gas] << endmsg;
    debug() << " PD quartz window efficiency  = " << qEff << endmsg
            << " Digitisation pedestal eff.   = " << pLos << endmsg;
  }

  // return  
  return sc;
}

const LHCb::RichRecRing *
TabulatedSignalDetectionEff::ckRing( LHCb::RichRecSegment * segment,
                                     const Rich::ParticleIDType hypo ) const
{
  // Make a CK ring for this segment and hypo. Used to work out which mirrors
  // and PDs we need to sample the reflectivities and Q.E. values from

  LHCb::RichRecRing * newRing = NULL;

  // protect against below threshold case
  if ( Rich::BelowThreshold == hypo ) return newRing;

  // Cherenkov theta for this segment/hypothesis combination
  // using emitted photon spectra (to avoid a circular information dependency)
  const double ckTheta = m_ckAngle->avgCherenkovTheta( segment, hypo, true );
  if ( msgLevel(MSG::DEBUG) )
    debug() << " -> Making new CK ring : theta = " << ckTheta << endmsg;
  if ( ckTheta > 0 )
  {
    // make a ring object
    newRing = new LHCb::RichRecRing( segment,
                                     (LHCb::RichRecRing::FloatType)(ckTheta),
                                     hypo );

    // set ring type info
    newRing->setType ( LHCb::RichRecRing::RayTracedCK );

    // ray tracing
    const Rich::RadiatorType rad = segment->trackSegment().radiator();
    const StatusCode sc = m_coneTrace->rayTrace( newRing, m_nPoints[rad], m_traceModeRad[rad] );
    if ( sc.isFailure() )
    {
      Warning( "Some problem occured during CK cone ray-tracing" ).ignore();
      delete newRing;
      newRing = NULL;
    }

  }

  // return the ring
  return newRing;
}

double
TabulatedSignalDetectionEff::photonDetEfficiency( LHCb::RichRecSegment * segment,
                                                  const Rich::ParticleIDType hypo,
                                                  const double energy ) const
{
  // protect against below threshold case
  if ( Rich::BelowThreshold == hypo ) return 0;

  if ( msgLevel(MSG::DEBUG) )
    debug() << "Computing detection efficiency for " << segment << " " << hypo
            << " photon energy=" << energy << endmsg;

  // Get a (local) ring for this segment/hypo
  if ( segment != m_last_segment || m_last_hypo != hypo )
  {
    delete m_last_ring;
    m_last_segment = segment;
    m_last_hypo    = hypo;
    m_last_ring    = ckRing( segment, hypo );
  }
  if ( !m_last_ring ) 
  { 
    if ( msgLevel(MSG::DEBUG) ) debug() << " -> No Ring" << endmsg; 
    return 0;
  }

  typedef Rich::Map<const LHCb::RichSmartID,unsigned int> PDCount;
  typedef Rich::Map<const DeRichSphMirror *,unsigned int> MirrorCount;

  PDCount pdCount;
  MirrorCount primMirrCount, secMirrCount;
  unsigned int totalInPD(0);
  for ( LHCb::RichRecPointOnRing::Vector::const_iterator iP = m_last_ring->ringPoints().begin();
        iP != m_last_ring->ringPoints().end(); ++iP )
  {
    if ( (*iP).acceptance() == LHCb::RichRecPointOnRing::InHPDTube )
    {
      // Count accepted points
      ++totalInPD;
      // Count PDs hit by this ring
      ++pdCount [ (*iP).smartID() ];
      // Count primary mirrors
      if ( (*iP).primaryMirror()   ) { ++primMirrCount [ (*iP).primaryMirror()   ]; }
      // Count secondary mirrors
      if ( (*iP).secondaryMirror() ) { ++secMirrCount  [ (*iP).secondaryMirror() ]; }
    }
  }

  if ( msgLevel(MSG::DEBUG) )
    debug() << " -> Found " << totalInPD << " usable ring points out of "
            << m_last_ring->ringPoints().size() << endmsg;
  if ( 0 == totalInPD ) { return 0; }

  // Get weighted average PD Q.E.
  double pdQEEff(0);
  if ( !pdCount.empty() )
  {
    totalInPD = 0;
    for ( PDCount::const_iterator iPD = pdCount.begin();
          iPD != pdCount.end(); ++iPD )
    {
      // Count HPDs
      totalInPD += iPD->second;
      // get pointer to PD
      const DeRichPD * pd = dePD( iPD->first );
      // add up Q.E. eff
      pdQEEff += (double)(iPD->second) * (*(pd->pdQuantumEff()))[energy*Gaudi::Units::eV]/100 ;
    }
    // normalise the result
    pdQEEff /= (double)(totalInPD);
  }
  else
  {
    pdQEEff = 1;
    Warning( "No PDs found -> Assuming Av. PD Q.E. of 1", StatusCode::SUCCESS );
  }

  // Weighted primary mirror reflectivity
  double primMirrRefl(0);
  if ( !primMirrCount.empty() )
  {
    totalInPD = 0;
    for ( MirrorCount::const_iterator iPM = primMirrCount.begin();
          iPM != primMirrCount.end(); ++iPM )
    {
      // count mirrors
      totalInPD += iPM->second;
      // add up mirror refl.
      primMirrRefl +=
        (double)(iPM->second) * (*(iPM->first->reflectivity()))[energy*Gaudi::Units::eV];
    }
    // normalise the result
    primMirrRefl /= (double)(totalInPD);
  }
  else
  {
    primMirrRefl = 1;
    Warning( "No primary mirrors found -> Assuming Av. reflectivity of 1", StatusCode::SUCCESS ).ignore();
  }

  // Weighted secondary mirror reflectivity
  double secMirrRefl(0);
  if ( !secMirrCount.empty() )
  {
    totalInPD = 0;
    for ( MirrorCount::const_iterator iSM = secMirrCount.begin();
          iSM != secMirrCount.end(); ++iSM )
    {
      // count mirrors
      totalInPD += iSM->second;
      // add up mirror refl.
      secMirrRefl +=
        (double)(iSM->second) * (*(iSM->first->reflectivity()))[energy*Gaudi::Units::eV];
    }
    // normalise the result
    secMirrRefl /= (double)(totalInPD);
  }
  else
  {
    secMirrRefl = 1;
    Warning( "No secondary mirrors found -> Assuming Av. reflectivity of 1", StatusCode::SUCCESS );
  }

  if ( msgLevel(MSG::DEBUG) )
    debug() << " -> Av. PD Q.E. = " << pdQEEff << " Prim. Mirr Refl. = " << primMirrRefl
            << " Sec. Mirr. Refl. " << secMirrRefl << endmsg;

  // return overall efficiency
  return m_qEffPedLoss * pdQEEff * primMirrRefl * secMirrRefl;
}

//================================================================================
// The PD panel location
//================================================================================
const std::string &
TabulatedSignalDetectionEff::pdPanelName( const Rich::DetectorType rich,
                                          const Rich::Side         panel ) const
{
  return (m_riches[rich]->paramVect<std::string>("HPDPanelDetElemLocations"))[panel];
}
