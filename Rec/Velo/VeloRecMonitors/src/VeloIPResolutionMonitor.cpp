// $Id: VeloIPResolutionMonitor.cpp,v 1.17 2010-02-18 16:50:17 malexand Exp $
// Include files
#include "VeloIPResolutionMonitor.h"

// from Gaudi
#include "GaudiKernel/AlgFactory.h"
#include "GaudiKernel/SystemOfUnits.h"
#include "GaudiUtils/Aida2ROOT.h"
#include "GaudiKernel/PhysicalConstants.h"
#include "LHCbMath/MatrixTransforms.h"

#include <string>
#include <sstream>

#include <Event/State.h>
#include "Event/Track.h"
#include "Event/L0DUReport.h"
#include "Event/ODIN.h"

#include "Event/Node.h"
#include "Event/Measurement.h"
#include "Event/VeloPhiMeasurement.h"
#include "Event/VeloRMeasurement.h"

#include <VeloDet/DeVeloPhiType.h>
#include <VeloDet/DeVeloRType.h>

#include <TF1.h>

using namespace LHCb;
using namespace Gaudi::Utils;
using namespace Gaudi::Units;

//-----------------------------------------------------------------------------
// Implementation file for class : VeloIPResolutionMonitor
//
// 2009-05-15 : Michael Thomas Alexander
//-----------------------------------------------------------------------------

// Declaration of the Algorithm Factory
namespace Velo
{
  DECLARE_ALGORITHM_FACTORY( VeloIPResolutionMonitor );
}


//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
Velo::VeloIPResolutionMonitor::VeloIPResolutionMonitor( const std::string& name,
                                                        ISvcLocator* pSvcLocator)
  : GaudiTupleAlg ( name , pSvcLocator ),
    m_vertexer("TrackVertexer"),
    m_measurementProvider(0),
    m_trackFitter(0),
    m_tuple(0)
{
  // set whether to write the ntuple
  declareProperty("WriteTuple",m_writeTuple=false);

  // Set the binning options
  declareProperty("StatOverflows", m_statOverflows = false );
  declareProperty("UseLogScale",m_useLogScale = false );
  declareProperty("InversePTMin",m_InversePTMin = 0.0 );
  declareProperty("InversePTMax",m_InversePTMax = 4.0 );
  declareProperty("NBins",m_nBins = 20 );
  declareProperty("MinPVnTracks", m_minPVnTracks = 8 );

  // Set whether to save the underlying histograms used to make the plots of fit results
  declareProperty("SaveUnderlyingHistos",m_saveUnderlyingHistos=true);
  declareProperty("UnderlyingHistosLimitYIntercept1D", m_limitIntercept1D = 6.525937e-03F );
  declareProperty("UnderlyingHistosLimitGradient1D", m_limitGradient1D = 1.947187e-02F );
  declareProperty("UnderlyingHistosLimitYIntercept3D", m_limitIntercept3D = 8.524237e-03F );
  declareProperty("UnderlyingHistosLimitGradient3D", m_limitGradient3D = 2.690075e-02F );
  declareProperty("UnderlyingHistosLimitFactor", m_limitFactor = 10. );

  // Set whether to calculate residuals as a fn. of eta and phi compared to the 1/PT parametrisation
  // and which gradient & y intercept values to use
  declareProperty("CalcResiduals", m_calcResiduals = true );
  declareProperty("ResidualsYIntercept3D", m_res3DyIntercept = 8.680847e-03F );
  declareProperty("ResidualsLinear3D", m_res3Dgrad = 2.656325e-02F );
  declareProperty("ResidualsQuad3D", m_res3Dquad = 1.135474e-04F );

  // Set whether to check if each event has passed L0
  declareProperty("RequireL0", m_requireL0 = false );

  // Set whether to refit PVs without the track for which IP is being calculated
  declareProperty("RefitPVs", m_refitPVs = false );

  // Set fit option for output profiles
  declareProperty("FitOption", m_fitOption = "NoFit");
  
}
//=============================================================================
// Destructor
//=============================================================================
Velo::VeloIPResolutionMonitor::~VeloIPResolutionMonitor() {} 

//=============================================================================
// Initialization
//=============================================================================
StatusCode Velo::VeloIPResolutionMonitor::initialize() {
  StatusCode sc = GaudiTupleAlg::initialize(); // must be executed first

  if ( sc.isFailure() ) return sc;  // error printed already by GaudiAlgorithm

  if ( msgLevel(MSG::DEBUG) ) debug() << "==> Initialize" << endmsg;

  if(m_writeTuple) m_tuple = nTuple( "IPResolutionsTuple" );

  // Set histo dir
  setHistoTopDir( "Velo/" );

  // Get vertexer if required
  if( m_refitPVs ) sc = m_vertexer.retrieve();

  // get the track extrapolator used in calculating the IPs
  m_trackExtrapolator = tool<ITrackExtrapolator>("TrackMasterExtrapolator","Extrapolator",this);
  m_measurementProvider = tool<IMeasurementProvider>("MeasurementProvider","MeasProvider", this );
  m_trackFitter = tool<ITrackFitter>("TrackMasterFitter","Fitter",this);

  // if useVariableBins=false (default) and no bin values have been defined in the options file, 
  // m_nBins equal sized bins between m_InversePTMin and m_InversePTMax are used
  double binWidth ;
  if( !m_useLogScale ) {
    binWidth = (m_InversePTMax - m_InversePTMin)/(1.*m_nBins);
    for( int i = 0; i<m_nBins+1; i++ ) m_bins.push_back( i*binWidth + m_InversePTMin );
  }
  else {
    if( m_InversePTMin <= 0 ) m_InversePTMin = 0.1;
    binWidth = (log10(m_InversePTMax) - log10(m_InversePTMin))/(1.*m_nBins);
    for( int i = 0; i<m_nBins+1; i++ ) m_bins.push_back( i*binWidth + log10(m_InversePTMin) );
  }  

  // book all histograms
  for( int i = 0; i<(int)m_bins.size()-1; i++ ){

    // make histogram titles and IDs from the bin values
    std::ostringstream tempID;
    tempID << m_bins[i] << "to" << m_bins[i+1];
    m_histoIDs.push_back( tempID.str() );
    
    std::ostringstream tempTitle;
    if( !m_useLogScale ) tempTitle << m_bins[i] << " < 1/p_{T} (GeV^{-1}) < " << m_bins[i+1] ;
    else tempTitle << m_bins[i] << " < log_{10}( 1/p_{T} (GeV^{-1}) )" << m_bins[i+1] ;
    m_histoTitles.push_back( tempTitle.str() );

    float limit1D;
    float limit3D;
    if ( !m_useLogScale ){
      limit1D = m_limitFactor * ( m_limitGradient1D * (float)m_bins[ i+1 ] + m_limitIntercept1D );
      limit3D = m_limitFactor * ( m_limitGradient3D * (float)m_bins[ i+1 ] + m_limitIntercept3D );
    }
    else{
      limit1D = m_limitFactor * ( m_limitGradient1D * pow( (float)10., (float)m_bins[ i+1 ] ) + m_limitIntercept1D );
      limit3D = m_limitFactor * ( m_limitGradient3D * pow( (float)10., (float)m_bins[ i+1 ] ) + m_limitIntercept3D );
    }

    // if underlying histograms are to be saved, the histograms for each bin are booked and pointers to the underlying 
    // ROOT histograms are stored in member vectors
    if( m_saveUnderlyingHistos ){
      
      m_IPres_X_histos.push_back( Aida2ROOT::aida2root( book( "IPres_X_"+m_histoIDs[i], 
                                                              "Resolution of IP_{X} for tracks with "+m_histoTitles[i], 
                                                              -limit1D*mm, limit1D*mm, 500 ) ) );
      m_IPres_Y_histos.push_back( Aida2ROOT::aida2root( book( "IPres_Y_"+m_histoIDs[i], 
                                                              "Resolution of IP_{Y} for tracks with "+m_histoTitles[i], 
                                                              -limit1D*mm, limit1D*mm, 500 ) ) );
      m_IPres_unsigned3D_histos.push_back( Aida2ROOT::aida2root( book( "IPres_unsigned3D_"+m_histoIDs[i], 
                                                                       "Resolution of unsigned IP_{3D} for tracks with "
                                                                       +m_histoTitles[i],
                                                                       0.0*mm, limit3D*mm, 500 ) ) );
      m_IPres_X_histos[i]->SetXTitle("IP Resolution (mm)");
      m_IPres_Y_histos[i]->SetXTitle("IP Resolution (mm)");
      m_IPres_unsigned3D_histos[i]->SetXTitle("IP Resolution (mm)");

    }
    // otherwise the underlying histograms are created as ROOT histograms, and pointers to them stored in member vectors
    else{
      std::string strID = "IPres_X_"+m_histoIDs[i];
      std::string strTitle = "Resolution of IP_{X} for tracks with "+m_histoTitles[i];
      m_IPres_X_histos.push_back( new TH1D( strID.c_str(), strTitle.c_str(), 500, -limit1D*mm, limit1D*mm ) );

      strID = "IPres_Y_"+m_histoIDs[i];
      strTitle = "Resolution of IP_{Y} for tracks with "+m_histoTitles[i];
      m_IPres_Y_histos.push_back( new TH1D( strID.c_str(), strTitle.c_str(), 500, -limit1D*mm, limit1D*mm ) );

      strID = "IPres_unsigned3D_"+m_histoIDs[i];
      strTitle = "Resolution of unsigned IP_{3D} for tracks with "+m_histoTitles[i];
      m_IPres_unsigned3D_histos.push_back( new TH1D( strID.c_str(), strTitle.c_str(), 500, 0.0*mm, limit3D*mm ) );
      
    }
    
  }  

  if( m_statOverflows ){
    for( int i = 0; i<(int)m_bins.size()-1; i++ ){
      m_IPres_X_histos[i]->StatOverflows();
      m_IPres_Y_histos[i]->StatOverflows();
      m_IPres_unsigned3D_histos[i]->StatOverflows();
    }
  }
  
  // book the histograms of fit results against 1/PT using the defined bins
  std::string XaxisTitle;
  if( !m_useLogScale ) XaxisTitle = "1/p_{T} (GeV^{-1})";
  else XaxisTitle = "log_{10}(1/p_{T}) (GeV^{-1})";

  std::string XprofileTitle, YprofileTitle, threeDprofileTitle;
  if( m_fitOption == "FitDouble" ){
    XprofileTitle = "Width of core of double Gaussian fit to IP_{X} resolution Vs 1/p_{T}";
    YprofileTitle = "Width of core of double Gaussian fit to IP_{Y} resolution Vs 1/p_{T}";
    threeDprofileTitle = "Mean of core of double '2D Gaussian' fit to IP_{3D} resolution Vs 1/p_{T}";
  } 
  else if( m_fitOption =="FitSingle" ){
    XprofileTitle = "Width of Gaussian fit to IP_{X} resolution Vs 1/p_{T}";
    YprofileTitle = "Width of Gaussian fit to IP_{Y} resolution Vs 1/p_{T}";
    threeDprofileTitle = "Mean of '2D Gaussian' fit to IP_{3D} resolution Vs 1/p_{T}";
  } 
  else{
    XprofileTitle = "Sample RMS of IP_{X} resolution Vs 1/p_{T}";
    YprofileTitle = "Sample RMS of IP_{Y} resolution Vs 1/p_{T}";
    threeDprofileTitle = "Sample Mean of IP_{3D} resolution Vs 1/p_{T}";
  }
  
  m_h_GaussWidthVsInversePT_X = Aida2ROOT::aida2root( book( "GaussWidth_IP_X_Vs_InversePT", 
                                                            XprofileTitle,
                                                            m_bins[0], m_bins[ (int)m_bins.size() - 1 ], 
                                                            (int)m_bins.size() -1 ));
  m_h_GaussWidthVsInversePT_X->SetXTitle(XaxisTitle.c_str());
  m_h_GaussWidthVsInversePT_X->SetYTitle("mm");

  m_h_GaussWidthVsInversePT_Y = Aida2ROOT::aida2root( book( "GaussWidth_IP_Y_Vs_InversePT", 
                                                            YprofileTitle,
                                                            m_bins[0], m_bins[ (int)m_bins.size() - 1 ], 
                                                            (int)m_bins.size() -1 ));
  m_h_GaussWidthVsInversePT_Y->SetXTitle(XaxisTitle.c_str());
  m_h_GaussWidthVsInversePT_Y->SetYTitle("mm");
  
  m_h_MeanVsInversePT_unsigned3D = 
    Aida2ROOT::aida2root( book( "Mean_unsigned3DIP_Vs_InversePT", 
                                threeDprofileTitle,
                                m_bins[0], m_bins[ (int)m_bins.size() - 1 ], 
                                (int)m_bins.size() - 1 ) );
  m_h_MeanVsInversePT_unsigned3D->SetXTitle(XaxisTitle.c_str());
  m_h_MeanVsInversePT_unsigned3D->SetYTitle("mm");
  
  // book additional histograms of track multiplicity and frequency of 1/PT
  m_h_TrackMultiplicity = Aida2ROOT::aida2root( book( "TrackMultiplicity", "PV Track Multiplicity", 0.0, 150.0, 75 ));
  m_h_TrackMultiplicity->SetXTitle("Number of tracks");

  m_h_InversePTFreq = Aida2ROOT::aida2root( book( "NTracks_Vs_InversePT", "Number of tracks found in each bin of 1/p_{T}", 
                                                  m_bins[0], m_bins[ (int)m_bins.size() - 1 ], (int)m_bins.size() -1 ));
  m_h_InversePTFreq->SetXTitle(XaxisTitle.c_str());
  m_h_InversePTFreq->SetYTitle("Number of tracks");

  // book plots of residuals wrt 1/PT parametrisation against eta and phi
  if( m_calcResiduals ){
    m_p_3DphiResiduals = Aida2ROOT::aida2root( bookProfile1D( "3DphiResiduals", 
                                                              "Residuals of 3D IP res. wrt. 1/p_{T} parametrisation Vs #phi",
                                                              -pi, pi ) );
    m_p_3DphiResiduals->SetXTitle("Track #phi");
    m_p_3DphiResiduals->SetYTitle("mm");
    
    m_p_3DetaResiduals = Aida2ROOT::aida2root( bookProfile1D( "3DetaResiduals", 
                                                              "Residuals of 3D IP res. wrt. 1/p_{T} parametrisation Vs #eta",
                                                              1.5, 5.5 ) );
    m_p_3DetaResiduals->SetXTitle("Track #eta");
    m_p_3DetaResiduals->SetYTitle("mm");

  }

  return sc;
}

//=============================================================================
// Main execution
//=============================================================================
StatusCode Velo::VeloIPResolutionMonitor::execute() {

  if ( msgLevel(MSG::DEBUG) ) debug() << "==> Execute" << endmsg;

  counter( "Events Analysed" )++;

  // get L0DU report to check L0 decision, if required
  LHCb::L0DUReport* report = new L0DUReport();
  if( m_requireL0 && exist<LHCb::L0DUReport>(LHCb::L0DUReportLocation::Default) ){
    report = get<LHCb::L0DUReport>(LHCb::L0DUReportLocation::Default);
  }
  else if( m_requireL0 ) {
    if ( msgLevel(MSG::DEBUG) ) debug() << "Could not find L0DU report!" << endmsg;
    return StatusCode::FAILURE;
  }

  if( msgLevel(MSG::DEBUG)) info() << "L0 decision: " << report->decision() << endmsg;
  
  if( m_requireL0 && !report->decision() ) return StatusCode::SUCCESS;
      
  // Get PVs
  if( !exist<LHCb::RecVertices>(LHCb::RecVertexLocation::Primary) ){
    counter("No data at 'Rec/Vertex/Primary'")++;
    debug() << "No data at 'Rec/Vertex/Primary'!" << endmsg;
    return StatusCode::SUCCESS;
  }
  const LHCb::RecVertices* pvs = get<LHCb::RecVertices>(LHCb::RecVertexLocation::Primary);  

  counter("Events Selected")++;
    
  // Loop over PVs
  for ( LHCb::RecVertices::const_iterator ipv = pvs->begin() ;
        ipv != pvs->end() ; ++ipv ){
    if( !(*ipv)->isPrimary() ) continue;
        
    m_pv = *ipv;
    counter("PVs Analysed")++;
        
    // Get tracks from current PV & loop
    const SmartRefVector< LHCb::Track > & PVtracks = m_pv->tracks();

    // count number of tracks making this pv that are reconstructed in the velo 
    unsigned int nVeloTracks(0);
    for ( SmartRefVector< LHCb::Track >::const_iterator tr = PVtracks.begin(); 
          tr != PVtracks.end() ; tr++ ){
      if( (*tr)->type()==Track::Velo || (*tr)->type()==Track::Upstream || (*tr)->type()==Track::Long )
        nVeloTracks += 1;
    }
    
    // can't refit a PV with only one track!
    if( m_refitPVs && PVtracks.size() < 3 ) continue;
    // apply ntracks cut
    if( nVeloTracks < m_minPVnTracks && !m_writeTuple ) continue;
    counter("PVs Selected")++;
        
    m_h_TrackMultiplicity->Fill( PVtracks.size() );
        
    for ( SmartRefVector< LHCb::Track >::const_iterator tr = PVtracks.begin(); 
          tr != PVtracks.end() ; tr++ ){

      m_track = &(**tr);

      double inversePT;
      if( !m_useLogScale ) inversePT = 1./(m_track->pt()/GeV);
      else inversePT = log10( 1./(m_track->pt()/GeV) );

      // only select Long tracks for reliable PT measurement, unless tracks are to be written to tuple
      // Skip tracks outwith the bin range, unless tuple is to be written, then all tracks are taken
      if( (inversePT > *(m_bins.rbegin()) || *(m_bins.begin()) > inversePT || (*tr)->type() != Track::Long ) 
          && !m_writeTuple ) continue;

      counter("Tracks selected")++;
        
      // refit PV removing current track
      if( m_refitPVs ){
          
        std::vector< const LHCb::Track* > newTracks;
        for ( SmartRefVector< LHCb::Track >::const_iterator trackIt = PVtracks.begin(); 
              trackIt != PVtracks.end() ; trackIt++ ){
            
          if( *trackIt == *tr ) continue;
          newTracks.push_back( &(**trackIt) );
        }
          
        LHCb::RecVertex* newVertex  = m_vertexer->fit( newTracks );
        if( newVertex ){
          m_pv = newVertex;
        }
        else continue;
      }

      double ip3d, ip3dsigma, ipx, ipxsigma, ipy, ipysigma;
      calculateIPs( m_pv, m_track, ip3d, ip3dsigma, ipx, ipxsigma, ipy, ipysigma );

      // only make histos for Long tracks in given PT range
      if( inversePT < *(m_bins.rbegin()) && *(m_bins.begin()) < inversePT && m_track->type() == Track::Long )
      {

        m_h_InversePTFreq->Fill( inversePT );
          
        // fill histograms
        fillHistos( ip3d, ipx, ipy, inversePT );
          
        // fill histos of residuals of the 1/PT parametrisation as a fn. of eta and phi 
        if( m_calcResiduals ){
          if( m_useLogScale ) inversePT = pow( (double)10., inversePT );
          m_p_3DphiResiduals->Fill( m_track->phi(), 
                                    ip3d - m_res3DyIntercept - m_res3Dgrad * inversePT
                                    - m_res3Dquad * inversePT * inversePT );
          m_p_3DetaResiduals->Fill( m_track->pseudoRapidity(), 
                                    ip3d - m_res3DyIntercept - m_res3Dgrad * inversePT 
                                    - m_res3Dquad * inversePT * inversePT );
        }
      }

      if( m_writeTuple ){
        m_tuple->column( "TrackType", m_track->type() );
        m_tuple->column( "IPRes3D", ip3d );
        m_tuple->column( "IPRes3Dsigma", ip3dsigma );
        m_tuple->column( "IPRes_X", ipx );
        m_tuple->column( "IPRes_Xsigma", ipxsigma );
        m_tuple->column( "IPRes_Y", ipy );
        m_tuple->column( "IPRes_Ysigma", ipysigma );
        m_tuple->column( "InversePT", inversePT );
        m_tuple->column( "P", m_track->p() );
        m_tuple->column( "Eta", m_track->pseudoRapidity() );
        m_tuple->column( "Phi", m_track->phi() );
        m_tuple->column( "TrackTx", m_track->slopes().x() );
        m_tuple->column( "TrackTy", m_track->slopes().y() );
        m_tuple->column( "TrackCharge", m_track->charge() );
        m_tuple->column( "PVX", m_pv->position().x() );
        m_tuple->column( "PVY", m_pv->position().y() );
        m_tuple->column( "PVZ", m_pv->position().z() );
        m_tuple->column( "TrackChi2", m_track->chi2() );
        m_tuple->column( "TrackNDOF", m_track->nDoF() );
        m_tuple->column( "PVNTracks", m_pv->tracks().size() );
        m_tuple->column( "PVNVeloTracks", nVeloTracks );
        m_tuple->column( "PVChi2", m_pv->chi2() );
        m_tuple->column( "PVNDOF", m_pv->nDoF() );
        m_tuple->column( "PVXerr", sqrt( m_pv->covMatrix()(0,0) ) );
        m_tuple->column( "PVYerr", sqrt( m_pv->covMatrix()(1,1) ) );
        m_tuple->column( "PVZerr", sqrt( m_pv->covMatrix()(2,2) ) );
        if( m_refitPVs ){
          m_tuple->column( "PVXNoRefit", (*ipv)->position().x() );
          m_tuple->column( "PVYNoRefit", (*ipv)->position().y() );
          m_tuple->column( "PVZNoRefit", (*ipv)->position().z() );
          m_tuple->column( "PVXerrNoRefit", sqrt( (*ipv)->covMatrix()(0,0) ) );
          m_tuple->column( "PVYerrNoRefit", sqrt( (*ipv)->covMatrix()(1,1) ) );
          m_tuple->column( "PVZerrNoRefit", sqrt( (*ipv)->covMatrix()(2,2) ) );
        }
        std::vector<double> statesX;
        std::vector<double> statesY;
        std::vector<double> statesZ;
        std::vector<unsigned int> stationNos;
        std::vector<bool> isR;
        std::vector<unsigned int> sensorNos;
        for( LHCb::Track::ConstNodeRange::const_iterator inode = m_track->nodes().begin();
             inode != m_track->nodes().end(); ++inode){
            
          if( (*inode)->type() != LHCb::Node::HitOnTrack ) continue;
            
          if( (*inode)->measurement().lhcbID().isVelo() ){
              
            statesX.push_back( (*inode)->state().position().x() );
            statesY.push_back( (*inode)->state().position().y() );
            statesZ.push_back( (*inode)->state().position().z() );
            if( (*inode)->measurement().type() == LHCb::Measurement::VeloPhi ){
              stationNos.push_back( ((LHCb::VeloPhiMeasurement&)((*inode)->measurement())).sensor().station() );
              sensorNos.push_back( ((LHCb::VeloPhiMeasurement&)((*inode)->measurement())).sensor().sensorNumber() );
              isR.push_back( false );
            }
            else{
              stationNos.push_back( ((LHCb::VeloRMeasurement&)((*inode)->measurement())).sensor().station() );
              sensorNos.push_back( ((LHCb::VeloRMeasurement&)((*inode)->measurement())).sensor().sensorNumber() );
              isR.push_back( true );
            }
          }
        }
        m_tuple->farray( "VeloStates_X", statesX, "nMeasurements", 42 );
        m_tuple->farray( "VeloStates_Y", statesY, "nMeasurements", 42 );
        m_tuple->farray( "VeloStates_Z", statesZ, "nMeasurements", 42 );
        m_tuple->farray( "MeasStationNos", stationNos, "nMeasurements", 42 );
        m_tuple->farray( "MeasSensorNos", sensorNos, "nMeasurements", 42 );
        m_tuple->farray( "MeasIsR", isR, "nMeasurements", 42 );
        m_tuple->write();
      }

      if( m_refitPVs ) delete m_pv;
        
    } // close loop over tracks
      
  } // close loop over pvs

  return StatusCode::SUCCESS;
}

//=============================================================================
//  Finalize
//=============================================================================
StatusCode Velo::VeloIPResolutionMonitor::finalize() {

  StatusCode sc( StatusCode::SUCCESS );

  if ( msgLevel(MSG::DEBUG) ) debug() << "==> Finalize" << endmsg;

  if( m_fitOption == "FitDouble" ) {
    fitDbl2DGausAndPlotMean( m_IPres_unsigned3D_histos, m_h_MeanVsInversePT_unsigned3D );
    fitDblGaussAndPlotWidth( m_IPres_X_histos, m_h_GaussWidthVsInversePT_X );    
    fitDblGaussAndPlotWidth( m_IPres_Y_histos, m_h_GaussWidthVsInversePT_Y );
  }
  else if( m_fitOption == "FitSingle" ) {
    fit2DGausAndPlotMean( m_IPres_unsigned3D_histos, m_h_MeanVsInversePT_unsigned3D );
    fitGaussAndPlotWidth( m_IPres_X_histos, m_h_GaussWidthVsInversePT_X );    
    fitGaussAndPlotWidth( m_IPres_Y_histos, m_h_GaussWidthVsInversePT_Y );
  }
  else{
    plotMean( m_IPres_unsigned3D_histos, m_h_MeanVsInversePT_unsigned3D );
    plotRMS( m_IPres_X_histos, m_h_GaussWidthVsInversePT_X );    
    plotRMS( m_IPres_Y_histos, m_h_GaussWidthVsInversePT_Y );
  }
    
  if( !m_saveUnderlyingHistos ){
    for( int i=0; i<(int)m_bins.size()-1; i++ ){
      delete m_IPres_X_histos[i];
      delete m_IPres_Y_histos[i];
      delete m_IPres_unsigned3D_histos[i];
    }
  }

  if( m_refitPVs ) sc = m_vertexer.release();
  
  return sc && GaudiTupleAlg::finalize();  // must be called after all other actions
}

//====================================================================
// Calculate ip_3D, ip_X and ip_y & their associated chi2
//====================================================================

StatusCode Velo::VeloIPResolutionMonitor::calculateIPs( const LHCb::RecVertex* pv, const LHCb::Track* track, 
                                                        double& ip3d, double& ip3dsigma, double& ipx, double& ipxsigma,
                                                        double& ipy, double& ipysigma ){
  
  StatusCode isSuccess;

  // extrapolate the current track to its point of closest approach to the PV 
  // to get the 3D IP
  LHCb::State POCAtoPVstate;
  isSuccess = m_trackExtrapolator->propagate( *track, pv->position(), POCAtoPVstate );
  isSuccess = distance( pv, POCAtoPVstate, ip3d, ip3dsigma, 0 );
  
  // extrapolate the current track to the same Z position as the PV to get X & Y IP
  LHCb::State stateAtPVZ = track->firstState();
  isSuccess = m_trackExtrapolator->propagate( stateAtPVZ, pv->position().z() );

  isSuccess = distance( pv, stateAtPVZ, ipx, ipxsigma, 1 );

  isSuccess = distance( pv, stateAtPVZ, ipy, ipysigma, 2 );

  return isSuccess;

}

//====================================================================
// calculate the distance & distance error between a vertex & a track state
// error is calculated assuming the PV position & state position are uncorrelated
//====================================================================

StatusCode Velo::VeloIPResolutionMonitor::distance( const LHCb::RecVertex* pv, LHCb::State& state, 
                                                    double& dist, double& sigma, int type=0 )
{
  const Gaudi::XYZVector delta ( pv->position() - state.position() ) ;
  
  Gaudi::SymMatrix3x3 covpv ( pv->covMatrix() ) ;
  const Gaudi::SymMatrix3x3 covpos( state.errPosition() );
  
  if( type==0 ){
    dist = sqrt( delta.mag2() );
    sigma = sqrt( covpv(0,0) + covpos(0,0) + covpv(1,1) + covpos(1,1) + covpv(2,2) + covpos(2,2) );
  }
  else if( type==1 ){ 
    dist = delta.x();
    sigma = sqrt( covpv(0,0) + covpos(0,0) );
  }
  else if( type==2 ){ 
    dist = delta.y();
    sigma = sqrt( covpv(1,1) + covpos(1,1) ); 
  }
  else{
    dist = -999;
    sigma = -999;
  }

  /*if ( !cov.Invert() ){
    chi2 = -999;
    return Error ( "Error in matrix inversion" , 905 ) ;
  }
  chi2 = Gaudi::Math::Similarity ( delta , cov ) ;*/
  

  return StatusCode::SUCCESS ;
}

//====================================================================
// fill histograms for a given set of track IPs and PT
//====================================================================
StatusCode Velo::VeloIPResolutionMonitor::fillHistos( double& ip3d, double& ipx, double& ipy, double& inversePT )
{
  // select the bin of 1/PT to which the track belongs
  for( int i = 0; i<(int)m_bins.size()-1; i++ ){
    if( m_bins[i] < inversePT && inversePT < m_bins[i+1] ){
      plotInBin( ip3d, ipx, ipy, i );
    }
  }
  
  return StatusCode::SUCCESS;
}

//====================================================================
// fill histograms in a given bin of 1/PT for the given IPs
//====================================================================
StatusCode Velo::VeloIPResolutionMonitor::plotInBin( double& ip3d, double& ipx, double& ipy, int& binNo )
{

  m_IPres_X_histos[binNo]->Fill( ipx/mm );
  m_IPres_Y_histos[binNo]->Fill( ipy/mm );
  m_IPres_unsigned3D_histos[binNo]->Fill( ip3d/mm );
  
  return StatusCode::SUCCESS;
}

//=========================================================================
//  Take the RMS of each of a set of input histos & plot it in an output histo
//=========================================================================
StatusCode Velo::VeloIPResolutionMonitor::plotRMS ( std::vector< TH1D* > sourceHistos, TH1D* outHisto ) {

  debug() << "Making RMS histo" << endmsg;
  
  if( (int)sourceHistos.size() != outHisto->GetNbinsX() ) return StatusCode::SUCCESS;  
  
  for( int i=0; i<(int)sourceHistos.size(); i++ ){
    
    if( sourceHistos[i]->GetEntries() == 0 ) continue;

    outHisto->SetBinContent( i+1, sourceHistos[i]->GetRMS() );
    outHisto->SetBinError( i+1, sourceHistos[i]->GetRMSError()  );
    
  }
  
  return StatusCode::SUCCESS;
  
}

//=========================================================================
//  Fit gaussian to a set of histograms, and plot the fitted gaus width in an output histo
//=========================================================================
StatusCode Velo::VeloIPResolutionMonitor::fitGaussAndPlotWidth ( std::vector< TH1D* > sourceHistos, TH1D* outHisto ) {

  debug() << "Making Gauss fit histo" << endmsg;
  
  if( (int)sourceHistos.size() != outHisto->GetNbinsX() ) return StatusCode::SUCCESS;
  
  TF1* Gauss = new TF1("Gauss","gaus(0)");
  int fitResult;
  
  for( int i=0; i<(int)sourceHistos.size(); i++ ){
    
    if( sourceHistos[i]->GetEntries() == 0 ) continue;
    
    Gauss->SetParameters( sourceHistos[i]->GetMaximum(), sourceHistos[i]->GetMean(), sourceHistos[i]->GetRMS() );
    fitResult = sourceHistos[i]->Fit( Gauss, "QN" );
    if( fitResult!=0 ){
      if( msgLevel(MSG::DEBUG) ) debug() << "Fit failed for histo '" << sourceHistos[i]->GetTitle() << "'" << endmsg;} 
    else{
      outHisto->SetBinContent( i+1, fabs( Gauss->GetParameter(2) ) );
      outHisto->SetBinError( i+1, Gauss->GetParError(2) );
      if( Gauss->GetNDF()!= 0 && m_saveUnderlyingHistos ) 
        plot( Gauss->GetChisquare()/Gauss->GetNDF(), "FitChi2PerNDOF",
              "Chi^{2}/NDOF of fits performed in making final histograms", 0., 10. );
    }    
  }
  
  return StatusCode::SUCCESS;
  
}

//=========================================================================
//  Fit double gaussian to a set of histograms, and plot the fitted core gaus width in an output histo
//=========================================================================
StatusCode Velo::VeloIPResolutionMonitor::fitDblGaussAndPlotWidth ( std::vector< TH1D* > sourceHistos, TH1D* outHisto ) {

  debug() << "Making double Gauss fit histo" << endmsg;
  
  if( (int)sourceHistos.size() != outHisto->GetNbinsX() ) return StatusCode::SUCCESS;
  
  TF1* Gauss = new TF1("Gauss","gaus(0)+gaus(3)");
  int fitResult;
  
  for( int i=0; i<(int)sourceHistos.size(); i++ ){
    
    if( sourceHistos[i]->GetEntries() == 0 ) continue;
    
    Gauss->SetParameters( sourceHistos[i]->GetMaximum(), sourceHistos[i]->GetMean(), sourceHistos[i]->GetRMS(),
                          sourceHistos[i]->GetMaximum()/10., sourceHistos[i]->GetMean(), sourceHistos[i]->GetRMS()*10. );
    fitResult = sourceHistos[i]->Fit( Gauss, "QN" );
    if( fitResult!=0 ){
      if( msgLevel(MSG::DEBUG) ) debug() << "Fit failed for histo '" << sourceHistos[i]->GetTitle() << "'" << endmsg;} 
    else{
      if( Gauss->GetParameter(0) > Gauss->GetParameter(3) ){
        outHisto->SetBinContent( i+1, fabs( Gauss->GetParameter(2) ) );
        outHisto->SetBinError( i+1, Gauss->GetParError(2) );
      }
      else{
        outHisto->SetBinContent( i+1, fabs( Gauss->GetParameter(5) ) );
        outHisto->SetBinError( i+1, Gauss->GetParError(5) );
      }
      if( Gauss->GetNDF()!= 0 && m_saveUnderlyingHistos ) 
        plot( Gauss->GetChisquare()/Gauss->GetNDF(), "FitChi2PerNDOF",
              "Chi^{2}/NDOF of fits performed in making final histograms", 0., 10. );
    }    
  }
  
  return StatusCode::SUCCESS;
  
}

//=========================================================================
//  Fit landau to a set of histograms, and plot the fitted MPV in an output histo  
//=========================================================================
StatusCode Velo::VeloIPResolutionMonitor::fitLandauAndPlotMPV ( std::vector< TH1D* > sourceHistos, TH1D* outHisto ) {

  debug() << "Making Landau fit histo" << endmsg;
  
  if( (int)sourceHistos.size() != outHisto->GetNbinsX() ) return StatusCode::SUCCESS;
  
  TF1* Landau = new TF1("Landau","landau(0)");
  int fitResult;
  
  for( int i=0; i<(int)sourceHistos.size(); i++ ){

    if( sourceHistos[i]->GetEntries() == 0 ) continue;

    Landau->SetParameters( sourceHistos[i]->GetMaximum(), sourceHistos[i]->GetMean(), sourceHistos[i]->GetRMS() );
    fitResult = sourceHistos[i]->Fit( Landau, "QN" );
    if( fitResult!=0 ){
      if( msgLevel(MSG::DEBUG) ) debug() << "Fit failed for histo '" << sourceHistos[i]->GetTitle() << "'" << endmsg;} 
    else{
      outHisto->SetBinContent( i+1, fabs( Landau->GetParameter(1) ) );
      outHisto->SetBinError( i+1, Landau->GetParError(1) );
      if( Landau->GetNDF()!=0 && m_saveUnderlyingHistos ) 
        plot( Landau->GetChisquare()/Landau->GetNDF(), "FitChi2PerNDOF", 
              "Chi^{2}/NDOF of fits performed in making final histograms", 0., 10. );

    }    
  }
  
  return StatusCode::SUCCESS;
  
}

//=========================================================================
//  Fit double 2D Gaussian to a set of histograms, and plot the fitted mean of the core in an output histo  
//=========================================================================
StatusCode Velo::VeloIPResolutionMonitor::fit2DGausAndPlotMean ( std::vector< TH1D* > sourceHistos, TH1D* outHisto ) {

  debug() << "Making 2D Gaussian fit histo" << endmsg;
  
  if( (int)sourceHistos.size() != outHisto->GetNbinsX() ) return StatusCode::SUCCESS;
  
  TF1* Gaus2D = new TF1("Gaus2D","[0]*x*exp( (-0.5*x^2) / ([1]^2) )");
  int fitResult;
  
  for( int i=0; i<(int)sourceHistos.size(); i++ ){

    if( sourceHistos[i]->GetEntries() == 0 ) continue;

    Gaus2D->SetParameters( sourceHistos[i]->GetMaximum(), sqrt( 2./pi )*sourceHistos[i]->GetMean() );
    
    fitResult = sourceHistos[i]->Fit( Gaus2D, "QN" );
    if( fitResult!=0 ){
      if( msgLevel(MSG::DEBUG) ) debug() << "Fit failed for histo '" << sourceHistos[i]->GetTitle() << "'" << endmsg;} 
    else{
      outHisto->SetBinContent( i+1, sqrt( pi/2. )*fabs( Gaus2D->GetParameter(1) ) );
      outHisto->SetBinError( i+1, sqrt( pi/2. )*Gaus2D->GetParError(1) );
      if( Gaus2D->GetNDF()!=0 && m_saveUnderlyingHistos ) 
        plot( Gaus2D->GetChisquare()/Gaus2D->GetNDF(), "FitChi2PerNDOF", 
              "Chi^{2}/NDOF of fits performed in making final histograms", 0., 10. );
    }    
  }
  
  return StatusCode::SUCCESS;
  
}

//=========================================================================
//  Fit double 2D Gaussian to a set of histograms, and plot the fitted mean of the core in an output histo  
//=========================================================================
StatusCode Velo::VeloIPResolutionMonitor::fitDbl2DGausAndPlotMean ( std::vector< TH1D* > sourceHistos, TH1D* outHisto ) {

  debug() << "Making double 2D Gaussian fit histo" << endmsg;
  
  if( (int)sourceHistos.size() != outHisto->GetNbinsX() ) return StatusCode::SUCCESS;
  
  TF1* Gaus2D = new TF1("Gaus2D","[0]*x*exp( (-0.5*x^2) / ([1]^2) ) + [2]*x*exp( (-0.5*x^2) / ([3]^2) )");
  int fitResult;
  
  for( int i=0; i<(int)sourceHistos.size(); i++ ){

    if( sourceHistos[i]->GetEntries() == 0 ) continue;

    Gaus2D->SetParameters( sourceHistos[i]->GetMaximum(), sqrt( 2./pi )*sourceHistos[i]->GetMean(), 
                           sourceHistos[i]->GetMaximum()/100., sqrt( 2./pi )*sourceHistos[i]->GetMean()/10. );
    fitResult = sourceHistos[i]->Fit( Gaus2D, "QN" );
    if( fitResult!=0 ){
      if( msgLevel(MSG::DEBUG) ) debug() << "Fit failed for histo '" << sourceHistos[i]->GetTitle() << "'" << endmsg;} 
    else{
      if( Gaus2D->GetParameter(0) > Gaus2D->GetParameter(2) ){
        outHisto->SetBinContent( i+1, sqrt( pi/2. )*fabs( Gaus2D->GetParameter(1) ) );
        outHisto->SetBinError( i+1, sqrt( pi/2. )*Gaus2D->GetParError(1) );
      }
      else{
        outHisto->SetBinContent( i+1, sqrt( pi/2. )*fabs( Gaus2D->GetParameter(3) ) );
        outHisto->SetBinError( i+1, sqrt( pi/2. )*Gaus2D->GetParError(3) );
      }
      if( Gaus2D->GetNDF()!=0 && m_saveUnderlyingHistos ) 
        plot( Gaus2D->GetChisquare()/Gaus2D->GetNDF(), "FitChi2PerNDOF", 
              "Chi^{2}/NDOF of fits performed in making final histograms", 0., 10. );

    }    
  }
  
  return StatusCode::SUCCESS;
  
}

//=========================================================================
//  plot the mean of a set of input histos in an output histo  
//=========================================================================
StatusCode Velo::VeloIPResolutionMonitor::plotMean ( std::vector< TH1D* > sourceHistos, TH1D* outHisto ) {

  debug() << "Making mean histo" << endmsg;
  
  if( (int)sourceHistos.size() != outHisto->GetNbinsX() ) return StatusCode::SUCCESS;
  
  for( int i=0; i<(int)sourceHistos.size(); i++ ){
    
    if( sourceHistos[i]->GetEntries() == 0 ) continue;
    
    outHisto->SetBinContent( i+1, sourceHistos[i]->GetMean() );
    outHisto->SetBinError( i+1, sourceHistos[i]->GetMeanError() );
  }
  
  return StatusCode::SUCCESS;
  
}
