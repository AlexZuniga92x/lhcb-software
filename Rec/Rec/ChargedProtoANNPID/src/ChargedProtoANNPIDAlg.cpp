
//-------------------------------------------------------------------------------
/** @file ChargedProtoANNPIDAlg.cpp
 *
 *  Implementation file for ANN Combined PID algorithm ChargedProtoANNPIDAlg
 *
 *  @author Chris Jones       Christopher.Rob.Jones@cern.ch
 *  @date   10/09/2010
 */
//-------------------------------------------------------------------------------

// local
#include "ChargedProtoANNPIDAlg.h"

//-----------------------------------------------------------------------------

using namespace ANNGlobalPID;

// Declaration of the Algorithm Factory
DECLARE_ALGORITHM_FACTORY( ChargedProtoANNPIDAlg );

//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
ChargedProtoANNPIDAlg::ChargedProtoANNPIDAlg( const std::string& name,
                                              ISvcLocator* pSvcLocator )
  : ChargedProtoANNPIDBase ( name , pSvcLocator ),
    m_trSel                ( NULL               ),
    m_netHelper            ( NULL               )
{
  // JOs
  declareProperty( "Configuration",     m_configFile );
  declareProperty( "TrackSelectorType", m_trSelType = "TrackSelector" );
  declareProperty( "NetworkVersion",    m_netVersion = "MC2010Tune" );
  // turn off histos and ntuples
  setProperty( "HistoProduce",   false );
  setProperty( "NTupleProduce",  false );
  setProperty( "EvtColsProduce", false );
}

//=============================================================================
// Destructor
//=============================================================================
ChargedProtoANNPIDAlg::~ChargedProtoANNPIDAlg() { }

//=============================================================================
// Initialization
//=============================================================================
StatusCode ChargedProtoANNPIDAlg::initialize()
{
  const StatusCode sc = ChargedProtoANNPIDBase::initialize();
  if ( sc.isFailure() ) return sc;

#ifdef __GNUC__

  // ParamFile root
  const std::string paramEnv = "CHARGEDPROTOANNPIDPARAM";
  if ( !getenv(paramEnv.c_str()) ) return Error( paramEnv+" not set" );
  const std::string paramRoot = ( std::string(getenv(paramEnv.c_str())) + 
                                  "/data/" + m_netVersion + "/" );

  // configuration file path
  if ( m_configFile.empty() ) return Error( "No configuration file specified" );
  const std::string configFile = paramRoot+m_configFile;

  // Open the config file
  std::ifstream config( configFile.c_str() );
  if ( config.is_open() )
  {
    // Read the particle type
    std::string particleType;
    config >> particleType;

    // Read the track Type
    std::string trackType;
    config >> trackType;

    // Track selection cuts
    double minP(0),minPt(0),maxChiSq(0),minLikelihood(0),maxGhostProb(0);
    config >> minP;
    config >> minPt;
    config >> maxChiSq;
    config >> minLikelihood;
    config >> maxGhostProb;

    // Track Pre-Selection
    config >> m_trackPreSel;
    
    // Proto variable to fill
    if      ( "electron" == particleType ) { m_protoInfo = LHCb::ProtoParticle::ProbNNe; }
    else if ( "muon"     == particleType ) { m_protoInfo = LHCb::ProtoParticle::ProbNNmu; }
    else if ( "pion"     == particleType ) { m_protoInfo = LHCb::ProtoParticle::ProbNNpi; }
    else if ( "kaon"     == particleType ) { m_protoInfo = LHCb::ProtoParticle::ProbNNk; }
    else if ( "proton"   == particleType ) { m_protoInfo = LHCb::ProtoParticle::ProbNNp; }
    else    { return Error( "Unknown particle type " + particleType ); }

    // Read the network type
    std::string annType;
    config >> annType;

    // Read network parameters file name
    std::string paramFileName;
    config >> paramFileName;
    paramFileName = paramRoot+paramFileName;
    std::ifstream ftest(paramFileName.c_str());
    if ( !ftest.is_open() ) return Error( "Network parameters file '" + 
                                          paramFileName + "' cannot be opened" );
    ftest.close();

    // Read the list of inputs
    std::string input;
    StringInputs inputs;
    while ( config >> input ) 
    { 
      if ( !input.empty() ) 
      {
        if ( input.find("#") == std::string::npos )
        {
          inputs.push_back(input); 
        }
      }
    }
    
    // Load the network
    if ( "NeuroBayes" == annType )
    {
      m_netHelper = new NeuroBayesANN(paramFileName,variableIDs(inputs),this);
    }
    else
    {
      return Error( "Unknown ANN type '"+annType+"'" );
    }

    // Set options for owned TrackSelector
    const std::string trSelName = "TrackSelector";
    DoubleProperty       pProp    ( "MinPCut",    minP  );
    DoubleProperty      ptProp    ( "MinPtCut",   minPt );
    DoubleProperty      chiProp   ( "MaxChi2Cut", maxChiSq );
    DoubleProperty      likProp   ( "MinLikelihoodCut", minLikelihood );
    DoubleProperty      ghostProp ( "MaxGhostProbCut", maxGhostProb );
    StringArrayProperty tkProp    ( "TrackTypes", boost::assign::list_of(trackType) );
    IJobOptionsSvc * joSvc = svc<IJobOptionsSvc>("JobOptionsSvc");
    joSvc->addPropertyToCatalogue( name()+"."+trSelName, pProp     );
    joSvc->addPropertyToCatalogue( name()+"."+trSelName, ptProp    );
    joSvc->addPropertyToCatalogue( name()+"."+trSelName, chiProp   );
    joSvc->addPropertyToCatalogue( name()+"."+trSelName, likProp   );
    joSvc->addPropertyToCatalogue( name()+"."+trSelName, ghostProp );
    joSvc->addPropertyToCatalogue( name()+"."+trSelName, tkProp    );
    release(joSvc);

    // get an instance of the track selector
    m_trSel = tool<ITrackSelector>( m_trSelType, trSelName, this );

    // print a summary of the configuration
    if ( msgLevel(MSG::DEBUG) )
      debug() << "Particle type    = " << particleType << endmsg
              << "Track Selection  = " << trackType << " " << m_trackPreSel << endmsg
              << "Network type     = " << annType << endmsg
              << "ParamFile        = " << paramFileName << endmsg
              << "ANN inputs (" << inputs.size() << ")  = " << inputs 
              << endmsg;

  }
  else
  {
    return Error( "Failed to open configuration file '"+paramRoot+m_configFile+"'" );
  }

  // Close the config file
  config.close();

#endif

  // return
  return sc;
}

//=============================================================================
// Main execution
//=============================================================================
StatusCode ChargedProtoANNPIDAlg::execute()
{

#ifdef __GNUC__

  // Load the charged ProtoParticles
  LHCb::ProtoParticles * protos = get<LHCb::ProtoParticles>( m_protoPath );

  // Loop over ProtoParticles
  for ( LHCb::ProtoParticles::iterator iP = protos->begin();
        iP != protos->end(); ++iP )
  {
    LHCb::ProtoParticle * proto = *iP;

    // Select ProtoParticles
    if ( !proto->track() ) { return Error( "Charged ProtoParticle has NULL Track pointer" ); }
    if ( !m_trSel->accept(*(proto->track())) ) continue;

    // Track Pre-selection
    if      ( "TrackPreSelIsLooseMuon" == m_trackPreSel && 
              getInput(proto,"MuonIsLooseMuon") < 0.5   ) { continue; }
    else if ( "TrackPreSelIsMuon"      == m_trackPreSel && 
              getInput(proto,"MuonIsMuon")      < 0.5   ) { continue; }
    
    // get the ANN output for this proto
    const double nnOut = m_netHelper->getOutput( proto );
    if ( msgLevel(MSG::VERBOSE) )
      verbose() << " -> ANN value = " << nnOut << endmsg;

    // add to protoparticle
    if ( proto->hasInfo(m_protoInfo) )
    {
      std::ostringstream mess;
      mess << "ProtoParticle already has '" << m_protoInfo << "' info -> Replacing.";
      Warning( mess.str() ).ignore();
    }
    proto->addInfo( m_protoInfo, nnOut );
  }

#endif

  return StatusCode::SUCCESS;
}

//=============================================================================
// Get ANN output for NeuroBayes network helper
//=============================================================================
#ifdef __GNUC__
double
ChargedProtoANNPIDAlg::NeuroBayesANN::getOutput( const LHCb::ProtoParticle * proto ) const
{
  // Fill the array of network inputs
  unsigned int input = 0;
  for ( ChargedProtoANNPIDBase::IntInputs::const_iterator iIn = m_inputs.begin();
        iIn != m_inputs.end(); ++iIn, ++input )
  {
    m_inArray[input] = static_cast<float>(m_parent->getInput(proto,*iIn));
  }

  // get the NN output
  const double nnOut = m_expert->nb_expert(m_inArray);

  // return final output, rescaled to the range 0 to 1
  return ( 1.0 + nnOut ) * 0.5;
}
#endif

//=============================================================================
//  Finalize
//=============================================================================
StatusCode ChargedProtoANNPIDAlg::finalize()
{
  // Clean Up
  delete m_netHelper;
  m_netHelper = NULL;
  // return
  return ChargedProtoANNPIDBase::finalize();
}

//=============================================================================
