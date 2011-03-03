
// Local
#include "RichHPDImageSummary.h"

//-----------------------------------------------------------------------------
// Implementation file for class : RichHPDImageSummary
//
// 2010-03-16 : Thomas BLAKE
//-----------------------------------------------------------------------------

using namespace Rich::HPDImage;

// Declaration of the Algorithm Factory
DECLARE_ALGORITHM_FACTORY( Summary )

//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
  Summary::Summary( const std::string& name,
                    ISvcLocator* pSvcLocator )
    : HistoAlgBase ( name , pSvcLocator ) ,
      m_nEvt       ( 0    )
{
  setProperty( "StatPrint", false );
  declareProperty( "DisplaySmartIDWarnings" , m_displayWarnings = false );
  declareProperty( "BoundaryCutThreshold" , m_params.cutFraction = 0.1 );
  declareProperty( "MinHPDOccupancy", m_minOccupancy = 1000 );
  declareProperty( "MinBoundaryPixels" , m_params.minBoundary = 3 );
  declareProperty( "CompareToCondDB" , m_compareCondDB = true );
  declareProperty( "MaxAllowedMovement" , m_maxMovement = 0.3 );
  declareProperty( "Keep2DHistograms", m_keep2Dhistos = false );
}

//=============================================================================
// Destructor
//=============================================================================
Summary::~Summary() {}

//=============================================================================
// Initialization
//=============================================================================

StatusCode Summary::initialize()
{
  const StatusCode sc = HistoAlgBase::initialize();
  if ( sc.isFailure() ) return sc;

  m_RichSys = getDet<DeRichSystem>( DeRichLocations::RichSystem );

  acquireTool( "RichSmartIDDecoder", m_SmartIDDecoder, 0, true );

  const LHCb::RichSmartID::Vector & activeHPDs = m_RichSys->activeHPDRichSmartIDs();

  for (  LHCb::RichSmartID::Vector::const_iterator iHPD = activeHPDs.begin();
         iHPD != activeHPDs.end(); ++iHPD )
  {
    const Rich::DAQ::HPDCopyNumber hpdID = m_RichSys->copyNumber( *iHPD );

    std::ostringstream name;
    name << "Rich_HPD_" << hpdID.data() << "_Image";

    if ( msgLevel(MSG::DEBUG) ) debug() << " Booking histogram " << name.str() << endmsg ;

    m_histo.insert( std::make_pair(hpdID.data(),this->create2D(name.str()) ) );
  }

  return sc;
}

//=============================================================================
// Main execution
//=============================================================================

StatusCode Summary::execute()
{
  ++m_nEvt;

  LHCb::ODIN* odin = get<LHCb::ODIN*>( LHCb::ODINLocation::Default );
  if ( odin ) {
    counter("RICH_EventTime") += odin->gpsTime() ;
  } else {
    if ( m_displayWarnings ) Warning(" Unable to retrieve ODIN ").ignore();
  }

  // Standard loop over Rich Smart IDs
  const Rich::DAQ::L1Map& mapUKL1 = m_SmartIDDecoder->allRichSmartIDs();

  for ( Rich::DAQ::L1Map::const_iterator iUKL1 = mapUKL1.begin();
        iUKL1 != mapUKL1.end(); ++iUKL1 )
  {
    const Rich::DAQ::IngressMap& mapIngress = iUKL1->second;

    for ( Rich::DAQ::IngressMap::const_iterator iIngress = mapIngress.begin();
          iIngress != mapIngress.end(); ++iIngress )
    {

      const Rich::DAQ::HPDMap& mapHPD = (iIngress->second).hpdData() ;

      for ( Rich::DAQ::HPDMap::const_iterator iHPD = mapHPD.begin();
            iHPD != mapHPD.end(); ++iHPD )
      {
        const LHCb::RichSmartID &smartID = (iHPD->second).hpdID();

        // Skip inhibited HPDs
        if ( (iHPD->second).header().inhibit() ) continue;

        if ( !smartID.isValid() )
        {
          if ( m_displayWarnings )
          {
            Warning(" Invalid Rich Smart ID ").ignore();
          }
          continue;
        }

        const LHCb::RichSmartID::Vector& hitIDs = (iHPD->second).smartIDs() ;

        const Rich::DAQ::HPDCopyNumber hpdID = m_RichSys->copyNumber( smartID );
        TH2D* hist = m_histo[ hpdID.data() ];

        if ( NULL == hist )
        {
          if ( m_displayWarnings )
          {
            Warning(" Can not retrieve boundary FCN, invalid hardware ID ").ignore();
          }
          continue;
        }

        for ( LHCb::RichSmartID::Vector::const_iterator iHit = hitIDs.begin();
              iHit != hitIDs.end(); ++iHit )
        {
          hist->Fill( iHit->pixelCol(), iHit->pixelRow() ) ;
        }

      }
    }
  }

  return StatusCode::SUCCESS;
}

//=============================================================================
//  Finalize
//=============================================================================

StatusCode Summary::finalize()
{

  if ( msgLevel(MSG::DEBUG) )
  {
    debug() << "==> Finalize" << endmsg;
    debug() << "    Algorithm has seen " << m_nEvt << " events" << endmsg;
  }

  for ( m_iter = m_histo.begin() ; m_iter != m_histo.end() ; ++m_iter )
  {
    summaryINFO( m_iter->first, m_iter->second );
  }

  if ( !m_keep2Dhistos )
  {
    for ( m_iter = m_histo.begin() ; m_iter != m_histo.end() ; ++m_iter )
    {
      delete m_iter->second;
    }
  }

  m_histo.clear() ;

  if ( msgLevel(MSG::DEBUG) ) debug() << " Freed map of HPD objects " << endmsg;

  return HistoAlgBase::finalize();  // must be called after all other actions
}

//=============================================================================

TH2D* Summary::create2D( const std::string& name )
{
  using namespace Gaudi::Utils;
  TH2D * hist(NULL);
  if ( m_keep2Dhistos )
  {
    hist = Aida2ROOT::aida2root(book2D(name,name,-0.5,31.5,32, -0.5,31.5,32));
  }
  else
  {
    hist = new TH2D(name.c_str(),name.c_str(),32,-0.5,31.5,32,-0.5,31.5 ) ;
  }
  return hist;
}

//=============================================================================

double Summary::distanceToCondDBValue( const unsigned int ID,
                                       const double x0,
                                       const double y0 ) const
{
  const LHCb::RichSmartID smartID = m_RichSys->richSmartID( Rich::DAQ::HPDCopyNumber(ID) );

  std::ostringstream sensorpath;
  sensorpath << m_RichSys->getDeHPDLocation(smartID);
  sensorpath << "/SiSensor:" << ID;

  DetectorElement * dd = getDet<DetectorElement>( sensorpath.str() );

  Gaudi::XYZPoint zero;
  Gaudi::XYZPoint offsetCondDB = (dd->geometry()->ownMatrix())*zero;

  const double condDBx = -offsetCondDB.x();
  const double condDBy = -offsetCondDB.y();

  return std::sqrt( std::pow(x0-condDBx,2) +
                    std::pow(y0-condDBy,2) ) ;
}

//=============================================================================

void Summary::summaryINFO( const unsigned int ID,
                           const TH2D* hist ) const
{
  if ( !hist ) return;

  const unsigned int nPix = (unsigned int) (hist->Integral());
  if ( nPix < m_minOccupancy ) return ;

  // Do the fit
  const Rich::HPDImage::HPDFit fitter;
  const HPDFit::Result result = fitter.fit( *hist, m_params );

  // if fit failed, don't fill.
  if ( ! result.OK() )
  {
    std::ostringstream mess;
    mess << "Fit for HPD " << ID << " FAILED";
    Warning( mess.str() ).ignore();
    return;
  }

  const unsigned int nHPDs = 484;

  const double x0     = result.x();
  const double y0     = result.y();
  const double xErr0  = result.xErr();
  const double yErr0  = result.yErr();
  const double Rad    = result.radInMM();
  const double RadErr = result.radErrInMM();

  const double OneOverXErrSq = ( xErr0>0.0  ? 1.0/(xErr0*xErr0)   : 0.0 );
  const double OneOverYErrSq = ( yErr0>0.0  ? 1.0/(yErr0*yErr0)   : 0.0 );
  const double OneOverRErrSq = ( RadErr>0.0 ? 1.0/(RadErr*RadErr) : 0.0 );

  const double ds = distanceToCondDBValue( ID, x0, y0 );

  plot1D( ds, "dPosCondDB", "Distance between image centre and CondDB value",0.0,3.0,30);
  plot1D( ID, "dPosCondDBvsCopyNr", "Distance versus HPD",-0.5,nHPDs-0.5,nHPDs,ds);

  // Update these to allow the weighted mean of the fit results to be correctly computed
  // when ROOT first are merged.  Need to compute
  //                weighted mean     = Sum( x_i / error_i^2 ) / Sum( 1 / error_i^2 )
  //                (error of mean)^2 = 1 /  Sum( 1 / error_i^2 )
  plot1D( ID, "dPosXvsCopyNr",    "x-displacement versus HPD",      -0.5,nHPDs-0.5,nHPDs,x0*OneOverXErrSq);
  plot1D( ID, "dPosXvsCopyNrErr", "x-displacement error versus HPD",-0.5,nHPDs-0.5,nHPDs,OneOverXErrSq);
  plot1D( ID, "dPosYvsCopyNr",    "y-displacement versus HPD",      -0.5,nHPDs-0.5,nHPDs,y0*OneOverYErrSq);
  plot1D( ID, "dPosYvsCopyNrErr", "y-displacement error versus HPD",-0.5,nHPDs-0.5,nHPDs,OneOverYErrSq);

  plot1D( ID, "RadiusvsCopyNr", "Fitted image radius vs HPD",-0.5,nHPDs-0.5,nHPDs,Rad*OneOverRErrSq);
  plot1D( ID, "RadiusErrvsCopyNr", "Fitted image radius error vs HPD",-0.5,nHPDs-0.5,nHPDs,OneOverRErrSq);

  plot1D( ID, "entriesvsCopyNr", "# entries for HPD Copy Nr",-0.5,nHPDs-0.5,nHPDs,nPix);

  if ( m_compareCondDB && ( ds < m_maxMovement ) )
  {
    if ( msgLevel(MSG::DEBUG) ) debug() << " Exisiting CondDB value ok for " << ID <<  endmsg;
  }
  else
  {
    std::ostringstream nameHPD;
    nameHPD << "RICH_HPD_" << ID;

    if ( msgLevel(MSG::DEBUG) ) debug() << "Adding counter " << nameHPD.str() << endmsg ;

    const double x0ErrSq  = std::pow(xErr0,2);
    const double y0ErrSq  = std::pow(yErr0,2);
    const double RadErrSq = std::pow(RadErr,2);

    counter( nameHPD.str() + "_XOffset" ) = StatEntity( nPix, nPix*x0,  nPix*x0ErrSq,  0. , 0. );
    counter( nameHPD.str() + "_YOffset" ) = StatEntity( nPix, nPix*y0,  nPix*y0ErrSq,  0. , 0. );
    counter( nameHPD.str() + "_Radius" )  = StatEntity( nPix, nPix*Rad, nPix*RadErrSq, 0. , 0. );
  }
}

//=============================================================================
