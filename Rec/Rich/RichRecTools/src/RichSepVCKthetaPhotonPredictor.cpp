
//-----------------------------------------------------------------------------
/** @file RichSepVCKthetaPhotonPredictor.cpp
 *
 *  Implementation file for tool : Rich::Rec::SepVCKthetaPhotonPredictor
 *
 *  CVS Log :-
 *  $Id: RichSepVCKthetaPhotonPredictor.cpp,v 1.11 2007-08-09 16:38:31 jonrob Exp $
 *
 *  @author Chris Jones   Christopher.Rob.Jones@cern.ch
 *  @date   01/06/2005
 */
//-----------------------------------------------------------------------------

// local
#include "RichSepVCKthetaPhotonPredictor.h"

// from Gaudi
#include "GaudiKernel/ToolFactory.h"

// All code is in general Rich reconstruction namespace
using namespace Rich::Rec;

//-----------------------------------------------------------------------------

DECLARE_TOOL_FACTORY( SepVCKthetaPhotonPredictor );

// Standard constructor
SepVCKthetaPhotonPredictor::
SepVCKthetaPhotonPredictor( const std::string& type,
                            const std::string& name,
                            const IInterface* parent )
  : Rich::Rec::ToolBase( type, name, parent ),
    m_geomTool      ( NULL ),
    m_ckAngle       ( NULL ),
    m_richPartProp  ( NULL ),
    m_minROI        ( Rich::NRadiatorTypes, 0 ),
    m_maxROI        ( Rich::NRadiatorTypes, 0 ),
    m_ckThetaMax    ( Rich::NRadiatorTypes, 0 ),
    m_sepGMax       ( Rich::NRadiatorTypes, 0 ),
    m_tolF          ( Rich::NRadiatorTypes, 0 ),
    m_scale         ( Rich::NRadiatorTypes, 0 ),
    m_Nselected     ( Rich::NRadiatorTypes, 0 ),
    m_Nreject       ( Rich::NRadiatorTypes, 0 )
{

  // interface
  declareInterface<IPhotonPredictor>(this);

  // job options

  m_minROI[Rich::Aerogel]  = 110; // aerogel
  m_minROI[Rich::Rich1Gas] = 0;   // rich1Gas
  m_minROI[Rich::Rich2Gas] = 0;   // rich2Gas
  declareProperty( "MinTrackROI", m_minROI );

  m_maxROI[Rich::Aerogel]  = 390;   // aerogel
  m_maxROI[Rich::Rich1Gas] =  86;   // rich1Gas
  m_maxROI[Rich::Rich2Gas] = 165;   // rich2Gas
  declareProperty( "MaxTrackROI", m_maxROI );

  m_ckThetaMax[Rich::Aerogel]  = 0.24;
  m_ckThetaMax[Rich::Rich1Gas] = 0.0520;
  m_ckThetaMax[Rich::Rich2Gas] = 0.03;
  declareProperty( "CKthetaMax", m_ckThetaMax );

  m_sepGMax[Rich::Aerogel]   = 342;
  m_sepGMax[Rich::Rich1Gas]  = 75;
  m_sepGMax[Rich::Rich2Gas]  = 130;
  declareProperty( "SepGMax", m_sepGMax );

  m_tolF[Rich::Aerogel]   = 25;
  m_tolF[Rich::Rich1Gas]  = 15;
  m_tolF[Rich::Rich2Gas]  = 40;
  declareProperty( "TolerenceFactor", m_tolF );

}

StatusCode SepVCKthetaPhotonPredictor::initialize()
{
  // Initialise base class
  const StatusCode sc = RichRecToolBase::initialize();
  if ( sc.isFailure() ) { return sc; }

  // get tools
  acquireTool( "RichCherenkovAngle",        m_ckAngle  );
  acquireTool( "RichRecGeometry",           m_geomTool );
  acquireTool( "RichParticleProperties",  m_richPartProp );

  // loop over radiators
  for ( int rad = 0; rad < Rich::NRadiatorTypes; ++rad )
  {
    // cache some numbers
    m_minROI2.push_back( m_minROI[rad]*m_minROI[rad] );
    m_maxROI2.push_back( m_maxROI[rad]*m_maxROI[rad] );
    m_scale[rad] = (m_sepGMax[rad]/m_ckThetaMax[rad]);
    // printout for this rad
    std::string trad = Rich::text((Rich::RadiatorType)rad);
    trad.resize(8,' ');
    info() << trad << " : Sep. range     " 
           << boost::format("%5.1f") % m_minROI[rad] << " -> "
           << boost::format("%5.1f") % m_maxROI[rad] << " mm  : Tol. "
           << boost::format("%5.1f") % m_tolF[rad] << " mm" << endreq;
  }
  
  m_pidTypes = m_richPartProp->particleTypes();
  info() << "Particle types considered = " << m_pidTypes << endreq;

  return sc;
}

StatusCode SepVCKthetaPhotonPredictor::finalize()
{

  if ( m_Nselected[Rich::Aerogel]  > 0 ||
       m_Nselected[Rich::Rich1Gas] > 0 ||
       m_Nselected[Rich::Rich2Gas] > 0 )
  {

    // statistical tool
    const PoissonEffFunctor occ("%10.2f +-%7.2f");

    // printout stats
    info() << "=================================================================" << endreq
           << "  Pixel/Segment combination selection summary :-" << endreq
           << "    Aerogel   : "
           << occ(m_Nselected[Rich::Aerogel],m_Nselected[Rich::Aerogel]+m_Nreject[Rich::Aerogel])
           << " % of possible candidates" << endreq
           << "    Rich1Gas  : "
           << occ(m_Nselected[Rich::Rich1Gas],m_Nselected[Rich::Rich1Gas]+m_Nreject[Rich::Rich1Gas])
           << " % of possible candidates" << endreq
           << "    Rich2Gas  : "
           << occ(m_Nselected[Rich::Rich2Gas],m_Nselected[Rich::Rich2Gas]+m_Nreject[Rich::Rich2Gas])
           << " % of possible candidates" << endreq
           << "=================================================================" << endreq;
  }

  // Execute base class method
  return RichRecToolBase::finalize();
}

// fast decision on whether a photon is possible
bool
SepVCKthetaPhotonPredictor::photonPossible( LHCb::RichRecSegment * segment,
                                            LHCb::RichRecPixel * pixel ) const
{

  // Default to not selected
  bool OK = false;

  // which radiator
  const Rich::RadiatorType rad = segment->trackSegment().radiator();

  // Are they in the same Rich detector ?
  if ( segment->trackSegment().rich() == pixel->detector() )
  {

    // segment / hit separation squared
    const double sep2 = m_geomTool->trackPixelHitSep2(segment,pixel);

    // Check overall boundaries
    if ( sep2 < m_maxROI2[rad] && sep2 > m_minROI2[rad] )
    {

      // Cache separation
      const double sep = sqrt(sep2);

      // Loop over mass hypos and check finer grained boundaries
      for ( Rich::Particles::const_iterator hypo = m_pidTypes.begin(); 
            hypo != m_pidTypes.end(); ++hypo )
      {

        // expected separation, scales linearly with expected CK angle
        const double expSep = m_ckAngle->avgCherenkovTheta(segment,*hypo) * m_scale[rad];

        // is this pixel/segment pair in the accepted range
        const double dsep = fabs(sep-expSep);
        if ( dsep < m_tolF[rad] )
        {
          OK = true;
          //if ( msgLevel(MSG::VERBOSE) )
          //{
          //  verbose() << "  -> " << *hypo << " fabs(sep-expSep)="
          //            << dsep << " PASSED tol=" << m_tolF[rad] << endreq;
          //}
          break;
        }
        //if ( msgLevel(MSG::VERBOSE) && !OK )
        //{
        //  verbose() << "  -> " << *hypo << " fabs(sep-expSep)="
        //            << dsep << " FAILED tol=" << m_tolF[rad] << " -> reject" << endreq;
        //}

      } // loop over hypos

    } // overall boundary check
    //else if ( msgLevel(MSG::VERBOSE) )
    //{
    //  verbose() << "  -> sep2=" << sep2
    //            << " FAILED overall boundary check " << m_minROI2[rad] << "->" << m_maxROI2[rad]
    //            << " -> reject" << endreq;
    //}

  } // same detector
  //else if ( msgLevel(MSG::VERBOSE) )
  //{
  //  verbose() << "  -> " << " FAILED RICH detector check -> reject" << endreq;
  //}

  if ( OK ) { ++m_Nselected[rad]; }
  else      { ++m_Nreject[rad];   }

  return OK;
}

