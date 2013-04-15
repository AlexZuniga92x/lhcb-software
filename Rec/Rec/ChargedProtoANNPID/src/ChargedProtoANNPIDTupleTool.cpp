
// from Gaudi
#include "GaudiKernel/ToolFactory.h"

// local
#include "ChargedProtoANNPIDTupleTool.h"

//-----------------------------------------------------------------------------
// Implementation file for class : ChargedProtoANNPIDTupleTool
//
// 2011-02-04 : Chris Jones
//-----------------------------------------------------------------------------

using namespace ANNGlobalPID;

// Declaration of the Tool Factory
DECLARE_TOOL_FACTORY( ChargedProtoANNPIDTupleTool )

//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
ChargedProtoANNPIDTupleTool::ChargedProtoANNPIDTupleTool( const std::string& type,
                                                          const std::string& name,
                                                          const IInterface* parent )
  : ChargedProtoANNPIDToolBase ( type, name, parent ),
    m_truth                    ( NULL )
{

  // interface
  declareInterface<IChargedProtoANNPIDTupleTool>(this);

  // Job options
  std::vector<std::string> tmp = boost::assign::list_of
                   // General event variables
                   ("NumProtoParticles")("NumCaloHypos")
                   ("NumLongTracks")("NumDownstreamTracks")("NumUpstreamTracks")
                   ("NumVeloTracks")("NumTTracks")("NumPVs")("NumSPDHits")
                   ("NumRich1Hits")("NumRich2Hits")("NumMuonTracks")
                   // Tracking
                   ("TrackP")("TrackPt")("TrackChi2PerDof")("TrackType")("TrackHistory")
                   ("TrackNumDof")("TrackLikelihood")("TrackGhostProbability")
                   ("TrackMatchChi2")("TrackFitMatchChi2")("TrackCloneDist")
                   ("TrackFitVeloChi2")("TrackFitVeloNDoF")("TrackFitTChi2")("TrackFitTNDoF")
                   // Combined DLLs
                   ("CombDLLe")("CombDLLmu")("CombDLLpi")("CombDLLk")("CombDLLp")
                   // RICH
                   ("RichUsedAero")("RichUsedR1Gas")("RichUsedR2Gas")
                   ("RichAboveElThres")("RichAboveMuThres")("RichAbovePiThres")
                   ("RichAboveKaThres")("RichAbovePrThres")
                   ("RichDLLe")("RichDLLmu")("RichDLLpi")("RichDLLk")("RichDLLp")("RichDLLbt")
                   // MUON
                   ("InAccMuon")
                   ("MuonMuLL")("MuonBkgLL")("MuonIsMuon")("MuonIsLooseMuon")("MuonNShared")
                   // ECAL
                   ("InAccEcal")("CaloChargedSpd")("CaloChargedPrs")("CaloChargedEcal")
                   ("CaloElectronMatch")("CaloTrMatch")("CaloEcalE")("CaloEcalChi2")
                   ("CaloClusChi2")("EcalPIDe")("EcalPIDmu")("CaloTrajectoryL")
                   // HCAL
                   ("InAccHcal")("CaloHcalE")("HcalPIDe")("HcalPIDmu")
                   // PRS
                   ("InAccPrs")("CaloPrsE")("PrsPIDe")
                   // SPD
                   ("InAccSpd")("CaloSpdE")
                   // BREM
                   ("InAccBrem")("CaloNeutralSpd")("CaloNeutralPrs")("CaloNeutralEcal")
                   ("CaloBremMatch")("CaloBremChi2")("BremPIDe")
                   // VELO
                   ("VeloCharge");
  declareProperty( "Variables", m_variables = tmp);

  // Turn off Tuple printing during finalize
  setProperty( "NTuplePrint", false );

}

//=============================================================================
// Destructor
//=============================================================================
ChargedProtoANNPIDTupleTool::~ChargedProtoANNPIDTupleTool() {}

//=============================================================================
// Initialization
//=============================================================================
StatusCode ChargedProtoANNPIDTupleTool::initialize()
{
  const StatusCode sc = ChargedProtoANNPIDToolBase::initialize();
  if ( sc.isFailure() ) return sc;

  // get tools
  m_truth = tool<Rich::Rec::MC::IMCTruthTool>( "Rich::Rec::MC::MCTruthTool",
                                               "MCTruth", this );


  // Get a vector of input accessor objects for the configured variables
  for ( StringInputs::const_iterator i = m_variables.begin(); 
        i != m_variables.end(); ++i )
  {
    m_inputs[*i] = getInput( *i );
  }
 
  return sc;
}

//=============================================================================
//  Finalize
//=============================================================================
StatusCode ChargedProtoANNPIDTupleTool::finalize()
{
  // Clean Up
  for ( Inputs::const_iterator iIn = m_inputs.begin();
        iIn != m_inputs.end(); ++iIn )
  { delete iIn->second; }
  m_inputs.clear();
  // return
  return ChargedProtoANNPIDToolBase::finalize();
}

//=============================================================================

StatusCode ChargedProtoANNPIDTupleTool::fill( const LHCb::ProtoParticle * proto,
                                              const LHCb::ParticleID pid ) const
{
  StatusCode sc = StatusCode::SUCCESS;

  // Check this is a charged track ProtoParticle
  const LHCb::Track * track = proto->track();
  if ( !track ) return sc;

  // make a tuple
  Tuple tuple = nTuple( "annInputs", "ProtoParticle PID Information for ANN Training" );

  // Loop over reconstruction variables
  for ( Inputs::const_iterator iIn = m_inputs.begin();
        iIn != m_inputs.end(); ++iIn )
  {
    // get the variable and fill ntuple
    sc = sc && tuple->column( iIn->first, (float) iIn->second->value(proto) );
  }

  // PID info
  sc = sc && tuple->column( "RecoPIDcode", pid.pid() );

  // MC variables
  const LHCb::MCParticle * mcPart = m_truth->mcParticle(track);
  sc = sc && tuple->column( "MCParticleType", mcPart ? mcPart->particleID().pid() : 0    );
  sc = sc && tuple->column( "MCParticleP",    mcPart ? mcPart->p()                : -999 );
  sc = sc && tuple->column( "MCParticlePt",   mcPart ? mcPart->pt()               : -999 );
  sc = sc && tuple->column( "MCVirtualMass",  mcPart ? mcPart->virtualMass()      : -999 );

  // MC history flags
  bool fromB(false), fromD(false);
  // Parent MC particle
  const LHCb::MCParticle * mcParent = ( mcPart ? mcPart->mother() : NULL );
  unsigned int iCount(0); // protect against infinite loops
  while ( mcParent && ++iCount < 99999 )
  {
    if ( mcParent->particleID().hasBottom() ) { fromB = true; }
    if ( mcParent->particleID().hasCharm()  ) { fromD = true; }
    mcParent = mcParent->mother();
  }
  // Save MC parent info
  sc = sc && tuple->column( "MCFromB", fromB );
  sc = sc && tuple->column( "MCFromD", fromD );

  // write the tuple for this ProtoParticle
  sc = sc && tuple->write();

  // return
  return sc;
}
