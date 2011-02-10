
//---------------------------------------------------------------------------
/** @file RichCherenkovAngleMonitor.cpp
 *
 *  Implementation file for algorithm class : RichCherenkovAngleMonitor
 *
 *  @author Chris Jones       Christopher.Rob.Jones@cern.ch
 *  @date   05/04/2002
 */
//---------------------------------------------------------------------------

// local
#include "RichCherenkovAngleMonitor.h"

// From Gaudi
#include "GaudiKernel/SystemOfUnits.h"

// namespace
using namespace Rich::Rec::MC;

//---------------------------------------------------------------------------

DECLARE_ALGORITHM_FACTORY( CherenkovAngleMonitor )

// Standard constructor, initializes variables
CherenkovAngleMonitor::CherenkovAngleMonitor( const std::string& name,
                                              ISvcLocator* pSvcLocator )
  : Rich::Rec::TupleAlgBase ( name, pSvcLocator ),
    m_richPartProp      ( NULL ),
    m_richRecMCTruth    ( NULL ),
    m_ckAngle           ( NULL ),
    m_trSelector        ( NULL )
{
  // min beta
  declareProperty( "MinBeta",     m_minBeta   = 0.999 );
  // number of bins
  setProperty( "NBins1DHistos", 100 );
  setProperty( "NBins2DHistos", 100 );
  // # CK phi regions
  declareProperty( "NumPhiRegions", m_nPhiRegions = 4 );
}

// Destructor
CherenkovAngleMonitor::~CherenkovAngleMonitor() {}

//  Initialize
StatusCode CherenkovAngleMonitor::initialize()
{
  // Sets up various tools and services
  const StatusCode sc = Rich::Rec::TupleAlgBase::initialize();
  if ( sc.isFailure() ) { return sc; }

  // Acquire instances of tools
  acquireTool( "RichParticleProperties", m_richPartProp );
  acquireTool( "RichRecMCTruthTool",   m_richRecMCTruth );
  acquireTool( "RichCherenkovAngle",      m_ckAngle     );
  acquireTool( "TrackSelector",      m_trSelector, this );

  return sc;
}

// Main execution
StatusCode CherenkovAngleMonitor::execute()
{

  // Check event status
  if ( !richStatus()->eventOK() ) return StatusCode::SUCCESS;

  // Histogramming
  const Rich::HistoID hid;
  //            Radiator          Aerogel  Rich1Gas  Rich2Gas
  const double ckRange[]      = { 0.015,   0.01,     0.005   };
  MAX_CKTHETA_RAD;
  MIN_CKTHETA_RAD;
  const double xLimit[]       = { 400,     200,      1400    };
  const double yLimit[]       = { 400,     200,      1400    };

  // min and max P for histos
  const double maxP = m_trSelector->maxPCut() * Gaudi::Units::GeV;
  const double minP = m_trSelector->minPCut() * Gaudi::Units::GeV;

  // Iterate over segments
  for ( LHCb::RichRecSegments::const_iterator iSeg = richSegments()->begin();
        iSeg != richSegments()->end(); ++iSeg )
  {
    LHCb::RichRecSegment * segment = *iSeg;

    // apply track selection
    if ( !m_trSelector->trackSelected(segment->richRecTrack()) ) continue;

    // segment momentum
    const double pTot = sqrt(segment->trackSegment().bestMomentum().Mag2());

    // beta for pion
    const double pionbeta = m_richPartProp->beta( pTot, Rich::Pion );
    if ( pionbeta < m_minBeta ) continue; // skip non-saturated tracks

    // MC type
    const Rich::ParticleIDType mcType = m_richRecMCTruth->mcParticleType( segment );

    // Radiator info
    const Rich::RadiatorType rad = segment->trackSegment().radiator();

    // Expected Cherenkov theta angle for true particle type
    // if MC type is unknown, assume pion (maybe type should be job option ???)
    const double thetaExpTrue = ( mcType == Rich::Unknown ?
                                  m_ckAngle->avgCherenkovTheta( segment, Rich::Pion ) :
                                  m_ckAngle->avgCherenkovTheta( segment, mcType ) );

    // Get photons for this segment
    const LHCb::RichRecSegment::Photons & photons = photonCreator()->reconstructPhotons( segment );
    verbose() << " Found " << photons.size() << " photon candidates" << endmsg;

    // loop over photons
    for ( LHCb::RichRecSegment::Photons::const_iterator iPhot = photons.begin();
          iPhot != photons.end();
          ++iPhot )
    {
      LHCb::RichRecPhoton * photon = *iPhot;

      // Hit point associated to photon
      const Gaudi::XYZPoint & hitPnt = photon->richRecPixel()->globalPosition();

      // Cherenkov angles
      const double thetaRec = photon->geomPhoton().CherenkovTheta();
      const double phiRec   = photon->geomPhoton().CherenkovPhi();
      const double delTheta = thetaRec-thetaExpTrue;

      // phi 'region'
      const PhiDesc & phiR = phiRegionDesc( phiRec );

      richHisto1D( hid(rad,"ckTheta"), "Reconstructed Cherenkov theta",
                   minCkTheta[rad], maxCkTheta[rad], nBins1D() )->fill(thetaRec);
      richHisto1D( hid(rad,"ckThetaExp"), "Expected Cherenkov theta",
                   minCkTheta[rad], maxCkTheta[rad], nBins1D() )->fill(thetaExpTrue);
      richHisto1D( hid(rad,"ckPhi"), "Reconstructed Cherenkov phi", 0, 2*M_PI, nBins1D() )->fill(phiRec);
      richHisto1D( hid(rad,"ckDiffAll"), "Rec-Exp CK theta all",
                   -ckRange[rad], ckRange[rad], nBins1D() )->fill(delTheta);
      richHisto1D( hid(rad,"ckDiffAll"+phiR.first), "Rec-Exp CK theta all : "+phiR.second,
                   -ckRange[rad], ckRange[rad], nBins1D() )->fill(delTheta);
      richHisto2D( hid(rad,"ckDiffAllVPhi"), "Rec-Exp CK theta all V CK Phi",
                   0, 2*M_PI, nBins2D(), -ckRange[rad], ckRange[rad], nBins2D() )->fill(phiRec,delTheta);

      // theta versus phi plots
      richHisto2D( hid(rad,"thetaVphi"), "CK theta V phi",
                   0, 2*M_PI, nBins2D(), minCkTheta[rad], maxCkTheta[rad],
                   nBins2D() )->fill(phiRec,thetaRec);
      if ( hitPnt.y() < 0 && hitPnt.x() < 0 )
      {
        richHisto2D( hid(rad,"thetaVphiR1"), "CK theta V phi : y<0 x<0",
                     0, 2*M_PI, nBins2D(), minCkTheta[rad], maxCkTheta[rad],
                     nBins2D() )->fill(phiRec,thetaRec);
      }
      else if ( hitPnt.y() > 0 && hitPnt.x() < 0 )
      {
        richHisto2D( hid(rad,"thetaVphiR2"), "CK theta V phi : y>0 x<0",
                     0, 2*M_PI, nBins2D(), minCkTheta[rad], maxCkTheta[rad],
                     nBins2D() )->fill(phiRec,thetaRec);
      }
      else if ( hitPnt.y() < 0 && hitPnt.x() > 0 )
      {
        richHisto2D( hid(rad,"thetaVphiR3"), "CK theta V phi : y<0 x>0",
                     0, 2*M_PI, nBins2D(), minCkTheta[rad], maxCkTheta[rad],
                     nBins2D() )->fill(phiRec,thetaRec);
      }
      else if ( hitPnt.y() > 0 && hitPnt.x() > 0 )
      {
        richHisto2D( hid(rad,"thetaVphiR4"), "CK theta V phi : y>0 x>0",
                     0, 2*M_PI, nBins2D(), minCkTheta[rad], maxCkTheta[rad],
                     nBins2D() )->fill(phiRec,thetaRec);
      }

      richProfile1D( HID("avgCKThetaVGloX",rad),
                     "Average CK theta V Global X in radiator : Isolated Tracks",
                     -xLimit[rad], xLimit[rad], 100 )
        ->fill( segment->trackSegment().middlePoint().x(), thetaRec );

      richProfile1D( HID("avgCKThetaVGloY",rad),
                     "Average CK theta V Global Y in radiator : Isolated Tracks",
                     -yLimit[rad], yLimit[rad], 100 )
        ->fill( segment->trackSegment().middlePoint().y(), thetaRec );

      if ( mcType != Rich::Unknown )
      {
        // true CK photon ?
        const LHCb::MCParticle * photonParent = m_richRecMCTruth->trueCherenkovPhoton(photon);
        if ( photonParent )
        {

          // CK angles for true photons
          richHisto1D( hid(rad,"ckThetaTrue"), "Reconstructed Cherenkov theta : true",
                       minCkTheta[rad], maxCkTheta[rad], nBins1D() )->fill(thetaRec);
          richHisto1D( hid(rad,mcType,"ckThetaTrue"), "Reconstructed Cherenkov theta : true",
                       minCkTheta[rad], maxCkTheta[rad], nBins1D() )->fill(thetaRec);
          richHisto1D( hid(rad,"ckThetaExpTrue"), "Expected Cherenkov theta : true",
                       minCkTheta[rad], maxCkTheta[rad], nBins1D() )->fill(thetaExpTrue);
          richHisto1D( hid(rad,mcType,"ckPhiTrue"), "Cherenkov phi : true", 0, 2*M_PI, nBins1D() )->fill(phiRec);
          richHisto1D( hid(rad,"ckDiffTrue"), "Rec-Exp CK theta all : true",
                       -ckRange[rad],ckRange[rad], nBins1D() )->fill(delTheta);
          richHisto1D( hid(rad,"ckDiffTrue"+phiR.first), "Rec-Exp CK theta all : true : "+phiR.second,
                       -ckRange[rad],ckRange[rad], nBins1D() )->fill(delTheta);
          richHisto2D( hid(rad,"ckDiffTrueVPhi"), "Rec-Exp CK theta all V CK Phi : true",
                       0, 2*M_PI, nBins2D(), -ckRange[rad], ckRange[rad], nBins2D() )->fill(phiRec, delTheta);

          richProfile1D( hid(rad,"ckDiffTrueVP"), "Rec-Exp CK theta Versus Ptot all : true",
                         minP, maxP, 50 )->fill(pTot,delTheta);
          richProfile1D( hid(rad,"ckDiffTrueVCKtheta"), "Rec-Exp CK theta Versus CK theta all : true",
                         minCkTheta[rad], maxCkTheta[rad], 50 )->fill(thetaRec,delTheta);

          // theta versus phi plots
          richHisto2D( hid(rad,"thetaVphiTrue"), "CK theta V phi : true CK photons",
                       0, 2*M_PI, nBins2D(), minCkTheta[rad], maxCkTheta[rad], nBins2D() )
            ->fill(phiRec, thetaRec);
          if ( hitPnt.y() < 0 && hitPnt.x() < 0 )
          {
            richHisto2D( hid(rad,"thetaVphiR1True"), "CK theta V phi : y<0 x<0 : true CK photons",
                         0, 2*M_PI, nBins2D(), minCkTheta[rad], maxCkTheta[rad], nBins2D() )
              ->fill(phiRec, thetaRec);
          }
          else if ( hitPnt.y() > 0 && hitPnt.x() < 0 )
          {
            richHisto2D( hid(rad,"thetaVphiR2True"), "CK theta V phi : y>0 x<0 : true CK photons",
                         0, 2*M_PI, nBins2D(), minCkTheta[rad], maxCkTheta[rad], nBins2D() )
              ->fill(phiRec, thetaRec);
          }
          else if ( hitPnt.y() < 0 && hitPnt.x() > 0 )
          {
            richHisto2D( hid(rad,"thetaVphiR3True"), "CK theta V phi : y<0 x>0 : true CK photons",
                         0, 2*M_PI, nBins2D(), minCkTheta[rad], maxCkTheta[rad], nBins2D() )
              ->fill(phiRec, thetaRec);
          }
          else if ( hitPnt.y() > 0 && hitPnt.x() > 0 )
          {
            richHisto2D( hid(rad,"thetaVphiR4True"), "CK theta V phi : y>0 x>0 : true CK photons",
                         0, 2*M_PI, nBins2D(), minCkTheta[rad], maxCkTheta[rad], nBins2D() )
              ->fill(phiRec, thetaRec);
          }

          // Associated MCRichOpticalPhoton
          const LHCb::MCRichOpticalPhoton * mcPhot = m_richRecMCTruth->trueOpticalPhoton(photon);
          double thetaMC       = -999;
          double phiMC         = -999;
          double delThetaMC    = -999;
          double delThetaExpMC = -999;
          if ( mcPhot )
          {
            thetaMC       = mcPhot->cherenkovTheta();
            phiMC         = mcPhot->cherenkovPhi();
            delThetaMC    = thetaRec-mcPhot->cherenkovTheta();
            delThetaExpMC = thetaExpTrue-mcPhot->cherenkovTheta();

            richHisto1D( hid(rad,"ckThetaMC"), "MC Cherenkov theta",
                         minCkTheta[rad], maxCkTheta[rad], nBins1D() )->fill(thetaMC);
            richHisto1D( hid(rad,"mcckDiffTrue"), "Rec-MC CK theta true",
                         -ckRange[rad],ckRange[rad], nBins1D() )->fill(delThetaMC);
            richHisto1D( hid(rad,"mcckDiffTrue"+phiR.first), "Rec-MC CK theta true : "+phiR.second,
                         -ckRange[rad],ckRange[rad], nBins1D() )->fill(delThetaMC);
            richHisto1D( hid(rad,"mcExpDiff"), "MC-Exp CK theta true",
                         -ckRange[rad],ckRange[rad], nBins1D() )->fill(delThetaExpMC);
            richHisto1D( hid(rad,"mcExpDiff"+phiR.first), "MC-Exp CK theta true : "+phiR.second,
                         -ckRange[rad],ckRange[rad], nBins1D() )->fill(delThetaExpMC);
            richHisto2D( hid(rad,"mcckDiffTrueVPhi"), "Rec-MC CK theta true V true CK Phi",
                         0, 2*M_PI, nBins2D(), -ckRange[rad], ckRange[rad], nBins2D() )->fill(phiMC, delThetaMC);

          } // mc photon

          // make a tuple

          Tuple tuple = nTuple( hid(rad,"ckResTuple"), "CKTuple" ) ;
          StatusCode sc = StatusCode::SUCCESS;
          sc = sc && tuple->column( "RecoPtot", pTot );
          sc = sc && tuple->column( "RecoCKtheta" , thetaRec );
          sc = sc && tuple->column( "RecoCKphi" ,   phiRec );
          sc = sc && tuple->column( "McCKtheta" , thetaMC );
          sc = sc && tuple->column( "McCKphi" ,  phiMC );
          sc = sc && tuple->column( "ExpCKtheta", thetaExpTrue );
          sc = sc && tuple->write();
          if ( sc.isFailure() ) return sc;

        }
        else // fake photon
        {

          // CK angles for fake photons
          richHisto1D( hid(rad,mcType,"ckThetaFake"), "Cherenkov theta : fake",
                       minCkTheta[rad], maxCkTheta[rad], nBins1D() )->fill(thetaRec);
          richHisto1D( hid(rad,"ckThetaExpFake"), "Expected Cherenkov theta : fake",
                       minCkTheta[rad], maxCkTheta[rad], nBins1D() )->fill(thetaExpTrue);
          richHisto1D( hid(rad,mcType,"ckPhiFake"), "Cherenkov phi : fake", 0, 2*M_PI, nBins1D() )->fill(phiRec);
          richHisto1D( hid(rad,"ckDiffFake"), "Rec-Exp CK theta all : fake",
                       -ckRange[rad],ckRange[rad], nBins1D() )->fill(delTheta);
          richHisto1D( hid(rad,"ckDiffFake"+phiR.first), "Rec-Exp CK theta all : fake : "+phiR.second,
                       -ckRange[rad],ckRange[rad], nBins1D() )->fill(delTheta);
          richProfile1D( hid(rad,"ckDiffFakeVP"), "Rec-Exp CK theta Versus Ptot all : fake",
                         minP, maxP, 50 )->fill(pTot,delTheta);

        }

      } // mc type known

    } // loop over segment photons

  } // loop over segments

  return StatusCode::SUCCESS;
}

const CherenkovAngleMonitor::PhiDesc &
CherenkovAngleMonitor::phiRegionDesc( const double phi ) const
{
  const int region = phiRegion(phi);
  Rich::Map<int,PhiDesc>::const_iterator iDesc = m_rDesc.find(region);
  if ( m_rDesc.end() == iDesc )
  {
    const double phiInc = 2.0 / m_nPhiRegions;
    const double phimin = (phiInc*region     - (phiInc/2.0));
    const double phimax = (phiInc*(region+1) - (phiInc/2.0));
    std::ostringstream desc;
    desc << "phi = " << boost::format("%6.2f PI -> %6.2f PI") % phimin % phimax;
    m_rDesc[region] = PhiDesc( boost::lexical_cast<std::string>(region), desc.str() );
    return m_rDesc[region];
  }
  return iDesc->second;
}
