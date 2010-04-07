
//----------------------------------------------------------------------------------------
/** @file RichFunctionalCKResForRecoTracks.cpp
 *
 *  Implementation file for tool : Rich::Rec::FunctionalCKResForRecoTracks
 *
 *  @author Chris Jones   Christopher.Rob.Jones@cern.ch
 *  @date   17/10/2004
 */
//----------------------------------------------------------------------------------------

// from Gaudi
#include "GaudiKernel/ToolFactory.h"
#include "GaudiKernel/SystemOfUnits.h"

// from TrackEvent
#include "Event/Node.h"

// local
#include "RichFunctionalCKResForRecoTracks.h"

// All code is in general Rich reconstruction namespace
using namespace Rich::Rec;

//----------------------------------------------------------------------------------------

DECLARE_TOOL_FACTORY( FunctionalCKResForRecoTracks );

// Standard constructor
FunctionalCKResForRecoTracks::
FunctionalCKResForRecoTracks ( const std::string& type,
                               const std::string& name,
                               const IInterface* parent )
  : HistoToolBase   ( type, name, parent  ),
    m_ckAngle       ( NULL                          ),
    m_refIndex      ( NULL                          ),
    m_trExt         ( NULL                          ),
    m_Ext           ( "TrackRungeKuttaExtrapolator" ),
    m_transSvc      ( NULL                          ),
    m_scatt         ( 13.6e-03                      ) // should be used with p in GeV
{
  using namespace boost::assign;

  // define interface
  declareInterface<ICherenkovResolution>(this);

  // job options

  declareProperty( "TrackExtrapolator", m_Ext );

  // Allows for additional adhoc contributions as needed
  m_asmpt[Rich::Aerogel]  = std::vector<double>( Rich::Rec::Track::NTrTypes, 0.0 );
  declareProperty( "AerogelAsymptopicErr", m_asmpt[Rich::Aerogel] );
  m_asmpt[Rich::Rich1Gas] = std::vector<double>( Rich::Rec::Track::NTrTypes, 0.0 );
  declareProperty( "Rich1GasAsymptopicErr", m_asmpt[Rich::Rich1Gas] );
  m_asmpt[Rich::Rich2Gas] = std::vector<double>( Rich::Rec::Track::NTrTypes, 0.0 );
  declareProperty( "Rich2GasAsymptopicErr", m_asmpt[Rich::Rich2Gas] );

  declareProperty( "HPDErrors",           m_hpdErr = list_of(0.0005)(0.0006)(0.0002) );

  declareProperty( "MaxCKThetaRes",       m_maxRes = list_of(0.01)(0.01)(0.01) );

  declareProperty( "UseLastMeasPoint", m_useLastMP = list_of(false)(false)(false) );

  declareProperty( "ScaleFactor",          m_scale = list_of(1.0)(1.0)(1.0) );

  // default to having histograms disabled
  setProduceHistos ( false             );
  // the directory for histograms
  setHistoDir      ( "CKFunctionalRes" );

}

StatusCode FunctionalCKResForRecoTracks::initialize()
{
  // Sets up various tools and services
  const StatusCode sc = HistoToolBase::initialize();
  if ( sc.isFailure() ) { return sc; }

  // Acquire instances of tools
  acquireTool( "RichCherenkovAngle",      m_ckAngle      );
  acquireTool( "RichTrackEffectiveRefIndex", m_refIndex  );
  acquireTool( "RichParticleProperties",  m_richPartProp );

  m_pidTypes = m_richPartProp->particleTypes();
  info() << "Particle types considered = " << m_pidTypes << endmsg;

  debug() << "Aerogel  Asymptopic Errors : " << m_asmpt[Rich::Aerogel]  << endmsg
          << "Rich1Gas Asymptopic Errors : " << m_asmpt[Rich::Rich1Gas] << endmsg
          << "Rich2Gas Asymptopic Errors : " << m_asmpt[Rich::Rich2Gas] << endmsg;

  return sc;
}

double
FunctionalCKResForRecoTracks::ckThetaResolution( LHCb::RichRecSegment * segment,
                                                 const Rich::ParticleIDType id ) const
{
  using namespace Gaudi::Units;

  // Protect against the non-physical below threshold hypothesis
  if ( Rich::BelowThreshold == id ) return 0;

  if ( !segment->ckThetaResolution().dataIsValid(id) )
  {
    double res2 = 0;

    // track segment shortcut
    const LHCb::RichTrackSegment & tkSeg = segment->trackSegment();

    // momentum for this segment, in GeV units
    const double ptot = std::sqrt(tkSeg.bestMomentum().Mag2()) / GeV;
    if ( ptot > 0 )
    {

      // Reference to track ID object
      const RichTrackID & tkID = segment->richRecTrack()->trackID();

      // track type
      const Rich::Rec::Track::Type tkType = tkID.trackType();

      // radiator
      const Rich::RadiatorType rad = tkSeg.radiator();

      //-------------------------------------------------------------------------------
      // asymtopic error
      //-------------------------------------------------------------------------------
      const double asymptotErr = gsl_pow_2( (m_asmpt[rad])[tkType] );
      res2 += asymptotErr;
      //-------------------------------------------------------------------------------

      //-------------------------------------------------------------------------------
      // RICH contributions (pixel, PSF errors etc...)
      //-------------------------------------------------------------------------------
      const double hpdErr = gsl_pow_2( m_hpdErr[rad] );
      res2 += hpdErr;
      //-------------------------------------------------------------------------------

      //-------------------------------------------------------------------------------
      // multiple scattering
      //-------------------------------------------------------------------------------
      double scattErr(0), effectiveLength(0);
      if ( m_useLastMP[rad] )
      {
        Gaudi::XYZPoint startPoint;
        const bool ok = findLastMeasuredPoint( segment, startPoint );
        effectiveLength = 
          transSvc()->distanceInRadUnits( (ok ? startPoint : tkSeg.entryPoint()), 
                                          tkSeg.exitPoint() );
      }
      else
      {
        effectiveLength = transSvc()->distanceInRadUnits( tkSeg.entryPoint(),
                                                          tkSeg.exitPoint() );
      }
      if ( effectiveLength > 0 )
      {
        const double multScattCoeff = ( m_scatt * std::sqrt(effectiveLength) * 
                                        (1+0.038*std::log(effectiveLength)) );
        scattErr                    = 2.0 * gsl_pow_2(multScattCoeff/ptot);
      }
      res2 += scattErr;
      //-------------------------------------------------------------------------------

      //-------------------------------------------------------------------------------
      // track curvature in the radiator volume
      //-------------------------------------------------------------------------------
      const double curvErr =
        ( Rich::Aerogel == rad ? 0 :
          gsl_pow_2( Rich::Geom::AngleBetween( tkSeg.entryMomentum(), 
                                               tkSeg.exitMomentum() ) / 4.0 ) );
      res2 += curvErr;
      //-------------------------------------------------------------------------------

      //-------------------------------------------------------------------------------
      // tracking direction errors
      //-------------------------------------------------------------------------------
      const double dirErr = tkSeg.entryErrors().errTX2() + tkSeg.entryErrors().errTY2();
      res2 += dirErr;
      //-------------------------------------------------------------------------------

      // Fill all mass hypos in one go, to save cpu (transport service slow)
      for ( Rich::Particles::const_iterator hypo = m_pidTypes.begin();
            hypo != m_pidTypes.end(); ++hypo )
      {
        double hypo_res2 = res2;

        // Expected Cherenkov theta angle
        const double ckExp = m_ckAngle->avgCherenkovTheta( segment, *hypo );
        if ( ckExp > 1e-6 )
        {
          // cache tan(cktheta)
          const double tanCkExp = std::tan(ckExp);

          //-------------------------------------------------------------------------------
          // chromatic error
          //-------------------------------------------------------------------------------
          const double index      = m_refIndex->refractiveIndex(segment);
          const double chromFact  = ( index>0 ? 
                                      m_refIndex->refractiveIndexRMS(segment)/index : 0 );
          const double chromatErr = gsl_pow_2( chromFact / tanCkExp );
          hypo_res2 += chromatErr;
          //-------------------------------------------------------------------------------

          //-------------------------------------------------------------------------------
          // momentum error
          //-------------------------------------------------------------------------------
          const double mass2      = m_richPartProp->massSq(*hypo)/(GeV*GeV);
          const double massFactor = mass2 / ( mass2 + ptot*ptot );
          const double momErr     = ( tkSeg.entryErrors().errP2()/(GeV*GeV) *
                                      gsl_pow_2( massFactor / ptot / tanCkExp ) );
          hypo_res2 += momErr;
          //-------------------------------------------------------------------------------

          //-------------------------------------------------------------------------------
          // Global Scale factor
          //-------------------------------------------------------------------------------
          hypo_res2 *= gsl_pow_2( m_scale[rad] );
          //-------------------------------------------------------------------------------

          // Histos
          if ( produceHistos() )
          {
            // Histo stuff
            const Rich::HistoID hid;
            MAX_CKTHETA_RAD;
            MIN_CKTHETA_RAD;
            const std::string tkT = Rich::text(tkType);
            // Versus CK theta
            profile1D( ckExp, std::sqrt(asymptotErr), hid(rad,*hypo,tkT+"/asymErrVc"),
                       "Asymptotic CK theta error V CK theta",
                       minCkTheta[rad], maxCkTheta[rad], nBins1D() );
            profile1D( ckExp, std::sqrt(hpdErr), hid(rad,*hypo,tkT+"/hpdErrVc"),
                       "HPD CK theta error V CK theta",
                       minCkTheta[rad], maxCkTheta[rad], nBins1D() );
            profile1D( ckExp, std::sqrt(chromatErr), hid(rad,*hypo,tkT+"/chroErrVc"),
                       "Chromatic CK theta error V CK theta",
                       minCkTheta[rad], maxCkTheta[rad], nBins1D() );
            profile1D( ckExp, std::sqrt(scattErr), hid(rad,*hypo,tkT+"/scatErrVc"),
                       "Scattering CK theta error V CK theta",
                       minCkTheta[rad], maxCkTheta[rad], nBins1D() );
            profile1D( ckExp, std::sqrt(curvErr), hid(rad,*hypo,tkT+"/curvErrVc"),
                       "Curvature CK theta error V CK theta",
                       minCkTheta[rad], maxCkTheta[rad], nBins1D() );
            profile1D( ckExp, std::sqrt(dirErr), hid(rad,*hypo,tkT+"/dirErrVc"),
                       "Track direction CK theta error V CK theta",
                       minCkTheta[rad], maxCkTheta[rad], nBins1D() );
            profile1D( ckExp, std::sqrt(momErr), hid(rad,*hypo,tkT+"/momErrVc"),
                       "Track momentum CK theta error V CK theta",
                       minCkTheta[rad], maxCkTheta[rad], nBins1D() );
            profile1D( ckExp, std::sqrt(hypo_res2), hid(rad,*hypo,tkT+"/overallErrVc"),
                       "Overall CK theta error V CK theta",
                       minCkTheta[rad], maxCkTheta[rad], nBins1D() );
            // Versus momentum
            profile1D( ptot, std::sqrt(asymptotErr), hid(rad,*hypo,tkT+"/asymErrVp"),
                       "Asymptotic CK theta error V momentum",
                       0, 100, nBins1D() );
            profile1D( ptot, std::sqrt(hpdErr), hid(rad,*hypo,tkT+"/hpdErrVp"),
                       "HPD CK theta error V momentum",
                       0, 100, nBins1D() );
            profile1D( ptot, std::sqrt(chromatErr), hid(rad,*hypo,tkT+"/chroErrVp"),
                       "Chromatic CK theta error V momentum",
                       0, 100, nBins1D() );
            profile1D( ptot, std::sqrt(scattErr), hid(rad,*hypo,tkT+"/scatErrVp"),
                       "Scattering CK theta error V momentum",
                       0, 100, nBins1D() );
            profile1D( ptot, std::sqrt(curvErr), hid(rad,*hypo,tkT+"/curvErrVp"),
                       "Curvature CK theta error V momentum",
                       0, 100, nBins1D() );
            profile1D( ptot, std::sqrt(dirErr), hid(rad,*hypo,tkT+"/dirErrVp"),
                       "Track direction CK theta error V momentum",
                       0, 100, nBins1D() );
            profile1D( ptot, std::sqrt(momErr), hid(rad,*hypo,tkT+"/momErrVp"),
                       "Track momentum CK theta error V momentum",
                       0, 100, nBins1D() );
            profile1D( ptot, std::sqrt(hypo_res2), hid(rad,*hypo,tkT+"/overallErrVp"),
                       "Overall CK theta error V momentum",
                       0, 100, nBins1D() );
          } // do histos

          if ( msgLevel(MSG::DEBUG) )
          {
            debug() << "Track " << segment->richRecTrack()->key() << " " << rad << " " << *hypo
                    << " : ptot " << ptot << " ckExp " << ckExp << endmsg;
            debug() << "  Rad length " << effectiveLength << endmsg;
            debug() << "  Asmy " << asymptotErr << " chro " << chromatErr << " scatt "
                    << scattErr << " curv " << curvErr << " dir " << dirErr
                    << " mom " << momErr << " : Overall " << std::sqrt(hypo_res2) << endmsg;
          }

        } // ckexp > 0

        // the final resolution
        float ckRes = (float) std::sqrt(hypo_res2);

        // apply absolute max limit
        if ( ckRes > m_maxRes[rad] ) ckRes = m_maxRes[rad];

        // Save final resolution value
        segment->setCKThetaResolution( *hypo, ckRes );
        if ( msgLevel(MSG::VERBOSE) )
        {
          verbose() << "Segment " << segment->key() << " : "
                    << *hypo << " ckRes " << ckRes << endmsg;
        }

      } // loop over mass hypos

    } // ptot>0

  } // res not already calculated

  return segment->ckThetaResolution( id );
}

bool
FunctionalCKResForRecoTracks::findLastMeasuredPoint( LHCb::RichRecSegment * segment,
                                                     Gaudi::XYZPoint & point ) const
{
  // pointer to underlying track
  const LHCb::Track * tr =
    dynamic_cast<const LHCb::Track*>(segment->richRecTrack()->parentTrack());
  if ( !tr ) Exception( "Null Track pointer" );

  // track segment shortcut
  const LHCb::RichTrackSegment & tkSeg = segment->trackSegment();

  // get z position of last measurement before start of track segment
  // a better search could perhaps be used here ?
  const LHCb::Node * lastMeas = NULL;
  const LHCb::Track::ConstNodeRange nodes = tr->nodes() ;
  for ( LHCb::Track::ConstNodeRange::const_iterator iM = nodes.begin();
        iM != nodes.end(); ++iM )
    if( (*iM)->type() == LHCb::Node::HitOnTrack ) {
      if      ( (*iM)->z() < tkSeg.entryPoint().z() ) { lastMeas = *iM; }
      else if ( (*iM)->z() > tkSeg.entryPoint().z() ) { break;          }
    }
  if ( !lastMeas ) return false;

  // get the track position at this z
  trackExtrap()->position( *tr, lastMeas->z(), point );

  return true;
}
