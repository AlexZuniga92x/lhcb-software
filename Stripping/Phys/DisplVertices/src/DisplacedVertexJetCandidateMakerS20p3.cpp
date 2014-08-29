// include files
#include <sstream>
#include <boost/foreach.hpp>

#include "Kernel/PFParticle.h"
#include "Kernel/JetEnums.h"

#include "Kernel/IJetMaker.h"

#include "LoKi/ParticleCuts.h"
#include "LoKi/ParticleContextCuts.h"
#include "LoKi/TrackCuts.h"
using namespace LoKi::Cuts;
#include "LoKi/IHybridFactory.h"
#include "KalmanFilter/FastVertex.h"

// local
#include "DisplacedVertexJetCandidateMakerS20p3.h"

//-----------------------------------------------------------------------------
// Implementation file for class : DisplacedVertexJetCandidateMaker, Stripping20p3 version
//
// 2012-06-26 : Victor Coco
// 2013-12-18 : Pieter David
//-----------------------------------------------------------------------------

// Declaration of the Algorithm Factory
DECLARE_ALGORITHM_FACTORY( DisplacedVertexJetCandidateMakerS20p3 )

namespace {
  typedef LoKi::Constant<const LHCb::Particle*,double> _PDOUBLE;
  typedef LoKi::Constant<const LHCb::Particle*,bool>   _PBOOL;
}

//==============================================================================
// Constructor
//==============================================================================
DisplacedVertexJetCandidateMakerS20p3::DisplacedVertexJetCandidateMakerS20p3
  ( const std::string& name, ISvcLocator* pSvcLocator)
  : DaVinciAlgorithm(name, pSvcLocator)
  , PFTYPE         (_PDOUBLE(-1.0))
  , ISGOODINPUT    (_PBOOL(false))
  , ISPERVINPUT    (_PBOOL(false))
  , ISALLVINPUT    (_PBOOL(false))
  , HASPOINTINGINFO(_PBOOL(false))
  // TODO: below can probably be removed, or moved into a dedicated tool
  , BestPVIPChi2   (_PDOUBLE(-1.0))
  , MinPVIP        (_PDOUBLE(-1.0))
  , m_dist         (0)
  , m_jetMaker     (0)
  , m_JECtool      (0)
  , m_jetIDInfoTool(0)
  , JETIDCUT       (_PBOOL(false))
  , m_combiner     (0)
{
  declareProperty("Factory"             , m_factory = "LoKi::Hybrid::Tool/HybridFactory:PUBLIC"
                 , "The Type/Name for C++/Python Hybrid Factory"
                 )->declareUpdateHandler( &DisplacedVertexJetCandidateMakerS20p3::updateHandlerCuts, this );
  declareProperty("Preambulo"           , m_preambulo
                 , "The preambulo to be used for decoding the Bender/Python cuts"
                 )->declareUpdateHandler( &DisplacedVertexJetCandidateMakerS20p3::updateHandlerCuts, this );

  declareProperty("PFInputLocation"
                 , m_pfInputLocation = "Phys/PFParticles/Particles"
                 , "Location of the Particle Flow input particles");
  declareProperty("PFInputCut"          , m_pfInputCode = "PALL"
                 , "Cut to apply on the PF inputs before making jets"
                 )->declareUpdateHandler( &DisplacedVertexJetCandidateMakerS20p3::updateHandlerCuts, this );

  // Partitioning
  declareProperty("MaxIP2DV"            , m_maxIP2DV = 3.
                 , "Maximum IP to displaced vertex");
  declareProperty("MinIP2PV"            , m_minIp2PV = 0.
                 , "Minimum IP to primary vertex");
  declareProperty("MinIPChi22PV"        , m_minIpChi22PV = 20.
                 , "Minimum IPChi2 to primary vertex");
  declareProperty("MinDOCABL"           , m_minDOCABL = -1.
                 , "Minimum DOCA to the beamline");
  declareProperty("MaxIPChi22DVDown"    , m_maxIpChi22DVDown = 30.
                 , "Maximum IPChi2 to displaced vertex, for downstream part");

  // Jet making and selection
  declareProperty("JetMaker"            , m_jetMakerName = "LoKi::FastJetMaker"
                 , "Jet-maker tool handle (IJetMaker interface)") ;
  declareProperty("JetIDTool"           , m_jetIDInfoToolName = "AddJetIDInfoS20p3"
                 , "IExtraInfoTool to use for filling the JetID variables");
  declareProperty("JetIDCut"            , m_jetIDCode = "PALL"
                 , "JetID cut"
                 )->declareUpdateHandler( &DisplacedVertexJetCandidateMakerS20p3::updateHandlerCuts, this );
  // Candidate
  declareProperty("MinNumberOfJets"     , m_minJet = 1, "Minimum number of jets per candidates");
  declareProperty("CandidatePID"        , m_candPIDName = "~chi_10"
                 , "ParticleID for the candidates");
}
//==============================================================================
// Destructor
//==============================================================================
DisplacedVertexJetCandidateMakerS20p3::~DisplacedVertexJetCandidateMakerS20p3()
{

}

//==============================================================================
// Algorithm initialization
//==============================================================================
StatusCode DisplacedVertexJetCandidateMakerS20p3::initialize()
{
  if (msgLevel(MSG::DEBUG)) { debug() << "==> Initialize" << endmsg; }

  StatusCode sc = DaVinciAlgorithm::initialize();
  if ( sc.isFailure() ) { return sc; }

  svc<IService>("LoKiSvc"); // ensure the proper initialization of LoKi service

  // hardcoded functors
  // TODO these can go into a header file (which can also be used for the dictionaries -> Hybrid usage)
  PFTYPE = INFO(LHCb::PFParticle::Type, -1.);
  LoKi::PhysTypes::Cut PFISCHARGED    = ( PFTYPE > LHCb::PFParticle::Charged ) && ( PFTYPE < LHCb::PFParticle::Neutral );
  LoKi::PhysTypes::Cut PFISBADCHARGED = (PFTYPE == LHCb::PFParticle::ChargedInfMomentum) || (PFTYPE == LHCb::PFParticle::Charged0Momentum);
  LoKi::PhysTypes::Cut PFISNEUTRAL    = ( PFTYPE > LHCb::PFParticle::Neutral ) && ( PFTYPE < LHCb::PFParticle::Composite );
  LoKi::PhysTypes::Cut PFISBADNEUTRAL = (PFTYPE == LHCb::PFParticle::BadPhotonMatchingT) || (PFTYPE == LHCb::PFParticle::BadPhoton) || (PFTYPE == LHCb::PFParticle::IsolatedPhoton);
  LoKi::PhysTypes::Cut PFISCOMPOSITE  = ( PFTYPE > LHCb::PFParticle::Composite ) && ( PFTYPE < LHCb::PFParticle::BadParticle );
  ISPERVINPUT = PFISCHARGED || PFISBADCHARGED || PFISCOMPOSITE ;
  ISALLVINPUT = PFISNEUTRAL || PFISBADNEUTRAL;
  HASPOINTINGINFO = INTREE( (HASTRACK && TRCUT(TrHASVELO)) || PFISCOMPOSITE );
  // user-specifed functors
  sc = updateCuts();
  if (sc.isFailure()) { return Error("Error from updateCuts", sc); }

  m_dist = distanceCalculator("LoKi");
  m_jetMaker = tool<IJetMaker>( m_jetMakerName, this );
  if ( m_jetMaker == 0 ) { return Error("Could not retrieve IJetMaker", sc); }
  m_JECtool = particleReFitter("JEC");
  if ( m_JECtool == 0 ) { warning() << "Could not retrieve IParticleReFitter for JEC" << endmsg; }
  m_jetIDInfoTool = tool<IExtraInfoTool>( m_jetIDInfoToolName, this );
  if ( m_jetIDInfoTool == 0 ) { return Error("Could not retrieve IExtraInfoTool for JetID", sc); }
  m_combiner = particleCombiner("MomentumCombiner");

  BestPVIPChi2 = BPVIPCHI2("")  ;
  MinPVIP      = MINIPWITHDV("")  ;
  m_beamspot = new LoKi::BeamSpot(10.);
  if ( ! m_beamspot ) {
    return Error("Could not get beamspot ", StatusCode::FAILURE);
  }

  m_candProp = ppSvc()->find(m_candPIDName);
  if ( NULL == m_candProp ) {
    return Error("Could not find PID "+m_candPIDName, StatusCode::FAILURE);
  }

  return StatusCode::SUCCESS;
}

//==============================================================================
// Algorithm finalization
//==============================================================================
StatusCode DisplacedVertexJetCandidateMakerS20p3::finalize()
{
  if (msgLevel(MSG::DEBUG)) { debug() << "==> Finalize" << endmsg; }

  // reset functors
  PFTYPE          = _PDOUBLE(-1.0);
  ISGOODINPUT     = _PBOOL(false);
  ISPERVINPUT     = _PBOOL(false);
  ISALLVINPUT     = _PBOOL(false);
  HASPOINTINGINFO = _PBOOL(false);
  BestPVIPChi2    = _PDOUBLE(-1.0);
  MinPVIP         = _PDOUBLE(-1.0);
  JETIDCUT        = _PBOOL(false);
  if ( m_beamspot ) {
    delete m_beamspot;
  }

  return DaVinciAlgorithm::finalize();
}

// Concatenate preambulo lines
std::string DisplacedVertexJetCandidateMakerS20p3::preambulo() const
{
  std::ostringstream result;
  for ( std::vector<std::string>::const_iterator iline = m_preambulo.begin(), iline_end = m_preambulo.end();
        iline != iline_end; ++iline )
  {
    if ( m_preambulo.begin() != iline ) { result << "\n"; }
    result << (*iline);
  }
  return result.str();
}

//==============================================================================
// Update handler: re-decode cuts
//==============================================================================
void DisplacedVertexJetCandidateMakerS20p3::updateHandlerCuts(Property& /*p*/)
{
  if ( Gaudi::StateMachine::INITIALIZED <= FSMState() ) {
      StatusCode sc = updateCuts();
      if (sc.isFailure()) { Error("Error from updateCuts", sc); }
  } // otherwise, it will get called from initialize()
}
StatusCode DisplacedVertexJetCandidateMakerS20p3::updateCuts()
{
  LoKi::IHybridFactory* factory = tool<LoKi::IHybridFactory>( m_factory, this );
  std::string preAmbulo = preambulo();
  StatusCode sc = factory->get( m_pfInputCode, ISGOODINPUT, preAmbulo );
  if (sc.isFailure()) { return Error("Error from LoKi/Bender 'hybrid' factory for Code='"+m_pfInputCode+"'", sc); }
  sc = factory->get( m_jetIDCode, JETIDCUT, preAmbulo );
  if (sc.isFailure()) { return Error("Error from LoKi/Bender 'hybrid' factory for Code='"+m_jetIDCode+"'", sc); }
  if (msgLevel(MSG::DEBUG)) { debug() << "Updated cuts: input selection \"" << ISGOODINPUT << "\"; JetID \"" << JETIDCUT << "\"" << endmsg; }
  return sc;
}

//==============================================================================
// Execute method
//==============================================================================
StatusCode DisplacedVertexJetCandidateMakerS20p3::execute()
{
  if (msgLevel(MSG::DEBUG)) { debug() << "==> Execute" << endmsg; }

  setFilterPassed(false);
  if ( ! exist<LHCb::Particle::Range>(m_pfInputLocation) ) {
    return Error("No valid input particles at "+m_pfInputLocation);
  }
  LHCb::Particle::Range allPFInputs = get<LHCb::Particle::Range>(m_pfInputLocation);

  double t_ip, t_chi2;
  LoKi::FastVertex::Line beamline( Gaudi::XYZPoint( m_beamspot->x(), m_beamspot->y(), 0. )
                                 , Gaudi::XYZVector( 0., 0., 1.) );
  StatusCode sc;
  BOOST_FOREACH( const LHCb::Particle* dvCand, particles() ) {
    const LHCb::Vertex* vertex = dvCand->endVertex();
    if ( vertex == 0 ) {
      Warning("Input candidate without endVertex -> skipping");
    } else {
      // Prepare inputs
      IJetMaker::Input jetInputs;
      BOOST_FOREACH( const LHCb::Particle* pfPart, allPFInputs ) {
        if ( ISGOODINPUT(pfPart) ) {
          if ( ISALLVINPUT(pfPart) ) {
            jetInputs.push_back(pfPart);
          } else if ( ISPERVINPUT(pfPart) ) {
            if ( HASTRACK(pfPart) && ISDOWN(pfPart) ) {
              sc = m_dist->distance( pfPart, vertex, t_ip, t_chi2 );
              if ( sc.isSuccess() && ( t_chi2 <= m_maxIpChi22DVDown ) ) {
                jetInputs.push_back(pfPart);
              }
            } else {
              // tracks with velo segment and (all) composites

              if ( HASTRACK(pfPart) ) {
                LoKi::FastVertex::distance( pfPart->proto()->track(), beamline, t_ip );
                if ( t_ip < m_minDOCABL )
                  continue;
              }

              // NOTE this can be done cleaner
              if ( BestPVIPChi2(pfPart) < m_minIpChi22PV )
                continue;

              double iptopv = MinPVIP(pfPart) ;
              if ( iptopv < m_minIp2PV )
                continue;

              sc = m_dist->distance( pfPart, vertex, t_ip, t_chi2 );
              if ( sc.isFailure() || ( t_ip > m_maxIP2DV ) )
                continue;
              // partition with respect to PV
              if ( t_ip > iptopv )
                continue;

              jetInputs.push_back(pfPart);
            }
          } else {
            Warning("PF input of unknown type");
          }
        }
      } // END gather inputs NOTE can move this to a separate method for readability

      // Now make the jets
      IJetMaker::Jets jets;
      sc = m_jetMaker->makeJets( jetInputs, jets );
      if (sc.isFailure()) { return Error ( "Error from jet maker" , sc ) ; }
      if (msgLevel(MSG::DEBUG)) { debug() << "input size " << jetInputs.size() << " jet size " << jets.size() << endmsg; }

      std::vector<LHCb::Particle*> goodJets; // NOTE should cleanup jets AND their endvertices if not keeping them
      goodJets.reserve(jets.size());
      while ( !jets.empty() ) {
        std::auto_ptr<LHCb::Particle> jet(jets.back());
        jets.pop_back();

        addJetIDInfo(jet.get());

        if ( m_JECtool ) {
          m_JECtool->reFit(*(jet.get()));
        }

        if ( HASPOINTINGINFO(jet.get()) && JETIDCUT(jet.get()) ) {
          if (msgLevel(MSG::DEBUG)) { debug() << "Good jet, keeping for candidate" << endmsg; }
          jet->setReferencePoint(vertex->position());
          // jet endVertex = stripping vertex but with jet daughters
          std::auto_ptr<LHCb::Vertex> jetVertex(new LHCb::Vertex());
          jetVertex->setPosition(vertex->position());
          jetVertex->setCovMatrix(vertex->covMatrix());
          jetVertex->setChi2(vertex->chi2());
          jetVertex->setNDoF(vertex->nDoF());
          jetVertex->setOutgoingParticles(jet->daughters());
          jet->setEndVertex(jetVertex.release());
          goodJets.push_back(jet.release());
        } else {
          debug() << "Jet without PV info (or failing JetID)" << endmsg;
        }
      }

      // make a candidate out of the good jets
      if ( ( goodJets.size() >= m_minJet ) && makeCandidate( goodJets, dvCand ) ) {
        setFilterPassed(true);
      } else { // clean up
        if (msgLevel(MSG::DEBUG)) { debug() << "Not enough good jets, cleaning up" << endmsg; }
        BOOST_FOREACH( LHCb::Particle* j, goodJets ) {
          delete j->endVertex();
          delete j;
        }
      }
    }
  }

  return StatusCode::SUCCESS;
}

// Helper method: combine the jets, construct the candidate and put it on the TES
bool DisplacedVertexJetCandidateMakerS20p3::makeCandidate( const LHCb::Particle::Vector& daughters, const LHCb::Particle* strippingCandidate )
{
  if (msgLevel(MSG::DEBUG)) { debug() << "Creating candidate from " << daughters.size() << " jets" << endmsg; }
  bool accepted = false;
  std::auto_ptr<LHCb::Particle> candidate ( new LHCb::Particle() );
  std::auto_ptr<LHCb::Vertex>   tmp_vertex( new LHCb::Vertex()   );

  // conversion to make types match
  LHCb::Particle::ConstVector daughtersConst = LHCb::Particle::ConstVector(daughters.begin(), daughters.end());
  StatusCode sc = m_combiner->combine( daughtersConst >> LoKi::sort<const LHCb::Particle*>( -PT ) , *candidate , *tmp_vertex ) ;
  if (!sc.isSuccess()) {
    Error( "Error from momentum combiner -> skip" , sc ) ;
  } else {
    candidate->setReferencePoint(strippingCandidate->endVertex()->position());
    candidate->setEndVertex(strippingCandidate->endVertex());
    candidate->setParticleID(m_candProp->particleID());
    // copy extra infos from vertexing, if present
    for ( int iKey = 51; iKey < 58; ++iKey ) {
      if ( strippingCandidate->hasInfo(iKey) ) {
        candidate->addInfo(iKey, strippingCandidate->info(iKey, -1.));
      }
    }
    candidate->addInfo(58, strippingCandidate->measuredMass());

    if (msgLevel(MSG::DEBUG)) { debug() << "Saving candidate " << *candidate << endmsg; }

    /* markNewTree (DaVinci::Utils::findDecayTree in fact) checks
     * - the head (candidate, added)
     * - the endVertex (stripping vertex, already stored)
     * - the daughters (jets, added) recursively (with endvertices; PF inputs are already stored)
     * - the outgoing particles of the endVertex (stripping tracks, already stored)
     *
     * the candidate is cleaned up here and jets (with endvertices) in the calling method above
     */
    markNewTree(candidate.release());
    accepted = true;
  }
  return accepted;
}

// Helper method: actually save the extrainfos calculated by the JetID tool
void DisplacedVertexJetCandidateMakerS20p3::addJetIDInfo(LHCb::Particle* jet)
{
  m_jetIDInfoTool->calculateExtraInfo(0, jet);
  std::string name;
  double value;
  for ( int k = m_jetIDInfoTool->getFirstIndex(), k_end = k + m_jetIDInfoTool->getNumberOfParameters()
      ; k < k_end ; ++k )
  {
    if ( m_jetIDInfoTool->getInfo(k, value, name) != 0 ) {
      jet->addInfo(k, value);
    } else {
      Error("Could not retrieve JetID extra info");
    }
  }
}
