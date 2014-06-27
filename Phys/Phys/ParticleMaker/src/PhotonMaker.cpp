// $Id: 
// ============================================================================
// Include files
#include "GaudiKernel/DeclareFactoryEntries.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "Kernel/IParticlePropertySvc.h"
#include "Kernel/ParticleProperty.h"
#include "CaloUtils/CaloParticle.h"
#include "CaloDet/DeCalorimeter.h"
#include "Event/CaloDataFunctor.h"
//#include "CaloUtils/CaloHypoFilter.h"
// local
#include "PhotonMaker.h"

/** @file
 *
 *  Implementation file for class : PhotonMaker
 *
 *  @date 2003-01-19
 *  @author Vanya Belyaev Ivan.Belyaev@itep.ru
 *
 *  @ modified 2006-08-25 for DC06
 *  @ Olivier Deschamps odescham@in2p3.fr
 */
// ============================================================================
/** @var PhotonMakerFactory
 *  Declaration of mandatory  Tool Factory
 */
// ============================================================================

DECLARE_TOOL_FACTORY(PhotonMaker)

// ============================================================================
/** Standard constructor
 *  @param type   tool type
 *  @param name   tool name
 *  @param parent tool parent
 */
// ============================================================================
  PhotonMaker::PhotonMaker
( const std::string& type   ,
  const std::string& name   ,
  const IInterface*  parent )
  : GaudiTool           ( type, name , parent )
  , m_input            ()
// Origin vertex
  , m_point            ()
  , m_pointErr         ()
// PID technque
  , m_useCaloTrMatch   ()
  , m_useIsNotH        ()
  , m_useIsNotE        ()
  , m_useIsPhoton      ()
  , m_usePhotonDLL     ()
// Filters
  , m_clCut            ()
  , m_ptCut            ()
  , m_converted        ()
  , m_unconverted      ()
  ,m_mas               (0.){
  declareProperty ( "Input"                      , m_input = LHCb::ProtoParticleLocation::Neutrals ) ;
  declareProperty ( "Particle"                   , m_part="gamma");
  //
  declareProperty ( "UseCaloTrMatch"             , m_useCaloTrMatch   = false ) ;
  declareProperty ( "UseIsNotH"                  , m_useIsNotH        = true  ) ;
  declareProperty ( "UseIsNotE"                  , m_useIsNotE        = false ) ;
  declareProperty ( "UseIsPhoton"                , m_useIsPhoton      = false ) ;
  declareProperty ( "UsePhotonDLL"               , m_usePhotonDLL     = false ) ;
  //
  declareProperty ( "ConfLevelCut"               , m_clCut       = -1.0   ) ;
  declareProperty ( "PtCut"                      , m_ptCut       = 150.0  ) ;
  declareProperty ( "ConvertedPhotons"           , m_converted   = true   ) ;
  declareProperty ( "UnconvertedPhotons"         , m_unconverted = true   ) ;
  declareProperty ( "PrsThreshold"               , m_minPrs = -1.   ) ;
  //
  declareProperty ( "AddHcalEnergy"              , m_addHcal = false);
  declareProperty ( "MaxHcalRatio"               , m_maxHcal = -1.);
  declareProperty ( "MinHcalRatio"               , m_minHcal = -1.);
  declareProperty ( "MaxPrsEnergy"               , m_maxPrs  = -1.);
  // declare new interface
  declareInterface<ICaloParticleMaker> (this);
}
// ============================================================================


// ============================================================================
/// destructor
// ============================================================================
PhotonMaker::~PhotonMaker() {}
// ============================================================================

StatusCode PhotonMaker::initialize    ()
{
  // initialize the base class
  StatusCode sc = GaudiTool::initialize();
  if( sc.isFailure() ) { return Error(" Unable to initialize GaudiTool",sc);}

  // ParticleProperty
  LHCb::IParticlePropertySvc* ppSvc = 0;
  sc = service("LHCb::ParticlePropertySvc", ppSvc);
  if( sc.isFailure() ) {
    fatal() << "    Unable to locate Particle Property Service"   << endmsg;
    return sc;
  }
  const LHCb::ParticleProperty* partProp;
  partProp = ppSvc->find( m_part );
  m_Id = (*partProp).pdgID().pid();
  m_mas = partProp->mass();
  //
  m_count[0]=0;
  m_count[1]=0;
  m_count[2]=0;



  // CL techniques
  debug() << " Following techniques will be used for CL evaluation : " << endmsg;
  if( m_useCaloTrMatch)debug() << "CL uses CaloTrMatch : matching with reconstructed tracks " << endmsg ; 
  if( m_useIsNotH   )debug() << "CL uses IsNotH    : Tracking/CALO NN-based combined Photon ID (anti-h)   " << endmsg ; 
  if( m_useIsNotE   )debug() << "CL uses IsNotE    : Tracking/CALO NN-based combined Photon ID (anti-e)   " << endmsg ; 
  if( m_useIsPhoton   )debug() << "CL uses IsPhoton separation variable " << endmsg;
  if( m_usePhotonDLL  )debug() << "CL uses PhotonDLL   : Tracking/CALO DLL-based combined Photon id   " << endmsg ; 

  if( !m_useCaloTrMatch   &&
      !m_useIsNotH        &&
      !m_useIsNotE        &&
      !m_useIsPhoton      &&
      !m_usePhotonDLL      )
  { Warning(" No PID techniques are selected for CL evaluation" ) ; }

  if( m_useCaloTrMatch    ){ debug()<< "  For CaloTrMatch assume Gauss distribution (wrong?)"<< endmsg; }

  if( m_converted && m_unconverted){debug()<< "BOTH converted (SPD) and unconverted (no SPD) photons are to be created"<< endmsg;}
  else if ( m_converted ){debug()<< "ONLY converted (SPD) photons are to be created"<< endmsg;}
  else if ( m_unconverted ){debug()<< "ONLY unconverted (no SPD) photons are to be created"<< endmsg;}
  else {warning()<<" BOTH converted and unconverted photon are to be skipped !!!" << endmsg; }

  return StatusCode::SUCCESS ;
}
// ============================================================================

StatusCode PhotonMaker::finalize      ()
{
  double f = (m_count[0] == 0 ) ? 0. :  (float) ( m_count[1]+m_count[2])/(float)m_count[0];
  double fc =  (m_count[1]+m_count[2] ==0)? 0 : (float) m_count[2]/(float)(m_count[1] + m_count[2])*100.0;

  info() << name() << " -  Summary -----" << endmsg;
  info() << " Created : " << f << " " << m_part << " per event" << endmsg;
  info() << " " << fc  << "% have SPD hits"<< endmsg;
  info() << " ---------------------------" << endmsg;
  // finalize the base class
  return GaudiTool::finalize ();
}

// ============================================================================
/** Make the particles
 *  @see ICaloParticleMaker
 *  @param particles  vector of particles
 */
// ============================================================================
StatusCode PhotonMaker::makeParticles (LHCb::Particle::Vector & particles )
{
  // avoid some long names

  if ( !particles.empty() )
  {
    Warning( "makeParticles(): extend non-empty vector of Particles" ).ignore();
  }

  // locate input data
  const LHCb::ProtoParticles* pps = getIfExists<LHCb::ProtoParticles>(evtSvc(),m_input);
  if ( !pps )
  {
    return Warning("No ProtoParticles at " + m_input + " 0 photons created",StatusCode::SUCCESS);
  }

  unsigned long long nConverted = 0 ;
  unsigned long long nUnconverted = 0 ;
  unsigned long long nSelConverted = 0 ;
  unsigned long long nSelUnconverted = 0 ;
  unsigned long long nSkip = 0;
  m_count[0] += 1;

  // Loop over PP
  for ( LHCb::ProtoParticles::const_iterator ipp = pps->begin() ;pps->end() != ipp ; ++ipp ){
    
    const LHCb::ProtoParticle* pp = *ipp ;

    //  ---- skip invalid and charged
    if ( 0 == pp || 0 != pp->track() )   { continue ; }
    const SmartRefVector<LHCb::CaloHypo>& hypos  = pp->calo() ;
    if ( hypos.empty() ) { continue ; }

    //  ---- Check the hypothesis
    const LHCb::CaloHypo*   hypo  = *(hypos.begin());
    if ( 0 == hypo || LHCb::CaloHypo::Photon != hypo->hypothesis() ) { continue ; }
    
    // ---- skip negative energy
    if( hypo->e() <= 0 ) continue;

    // == Evaluate Photon conversion (Spd-based for late conversion after magnet)
    bool cnv = ( pp->info(LHCb::ProtoParticle::CaloDepositID,0.) < 0 ) ;  
    //    bool cnv = (bool) pp->info(LHCb::ProtoParticle::CaloNeutralSpd, 0.) ;
    if( UNLIKELY( msgLevel(MSG::DEBUG) ) )debug() << " Conversion "
                                                  << cnv << " " << pp->info(LHCb::ProtoParticle::CaloDepositID,0.) << endmsg;
    
    // ---- selected (un)converted photons
    if( !cnv && !m_unconverted  ) { continue ; }
    if( cnv  && !m_converted    ) { continue ; }

    // ---- apply Prs filter
    double ePrs =  pp->info(LHCb::ProtoParticle::CaloNeutralPrs,0);
    if( m_minPrs >= 0 && ePrs < m_minPrs )continue;
    if( m_maxPrs >= 0 && ePrs > m_maxPrs )continue;
    cnv ? ++nConverted : ++nUnconverted;

    // ---- apply Hcal filter
    double eHcal=pp->info(LHCb::ProtoParticle::CaloNeutralHcal2Ecal,0);
    if( m_maxHcal >=0 && eHcal/(1.+eHcal) > m_maxHcal)continue;
    if( m_minHcal >=0 && eHcal/(1.+eHcal) < m_minHcal)continue;

    // == evaluate kinematical properties
    LHCb::CaloMomentum momentum(pp , m_point , m_pointErr);
    if ( 0 != momentum.status()  ) { continue ; }
    double pT = momentum.pt();
    double E  = momentum.e();
    double px = momentum.px();
    double py = momentum.py();
    double pz = momentum.pz();
    double p  = E;

    // -- apply hcal correction if requested
    if ( m_addHcal || m_mas > 0 ){
      if( m_addHcal ){
        E *= ( 1. + eHcal );
        px *= (1. + eHcal);
        py *= (1. + eHcal);
        pz *= (1. + eHcal);
        pT *= (1. + eHcal);
        p  = E;
      }
      if ( m_mas > 0 ){
        p  = std::sqrt( E*E - m_mas*m_mas );
        px *= (p/E);
        py *= (p/E);
        pz *= (p/E);
        pT *= (p/E);
      }
    }
    if ( pT < m_ptCut ) { continue; }

    // ---- apply CL filter (must be after pT cut to match neutralID definition range)
    const double CL = confLevel( pp );
    if ( CL  < m_clCut ) continue ;
    cnv ? ++nSelConverted : ++nSelUnconverted;

    // ===== create new particle and fill it
    LHCb::Particle* particle = new LHCb::Particle( ); // photon constructor
    particle -> setParticleID( LHCb::ParticleID (m_Id) );
    particle -> setProto( pp ) ;

    // === set  mass and mass uncertainties
    particle -> setMeasuredMass( m_mas   ) ;
    particle -> setMeasuredMassErr( 0   ) ; // the mass is EXACT zero!

    // === set confidence level
    particle  -> setConfLevel  ( CL    ) ;

    // === set photon parameters (4-momentum, vertex and correlations)
    LHCb::CaloParticle calopart(particle, m_point , m_pointErr);
    calopart.updateParticle();

    // ===  Massive object or Hcal correction
    // Warning : covariant matrix should be modified accordingly -> to be included in CaloParticle ...
    if( m_addHcal || m_mas > 0 )particle->setMomentum( Gaudi::LorentzVector(px,py,pz,E) );

    if ( calopart.status() !=0 ){
      delete particle ;
      ++nSkip;
      warning() << "Unable to fill photon parameters, skip particle [" << nSkip << "]"<< endmsg;
      warning() << " CaloParticle status/flag : " << calopart.status() << "/" << calopart.flag();
      cnv ? --nSelConverted : --nSelUnconverted;
      continue ;
    }

    // === printout
    if (msgLevel(MSG::VERBOSE)){
      verbose() << "----- Single "<<m_part << " found [" << nSelConverted+nSelUnconverted << "]"<<endmsg;
      verbose() << "Pt : " << momentum.pt() << endmsg;
      verbose() << "CL : " << CL << endmsg;
      verbose() << "Chi2 " << pp->info(LHCb::ProtoParticle::CaloTrMatch,-999.) << endmsg;
      verbose() << "Converted (SPD digits)? : " << cnv << endmsg;
      verbose() << "CaloDeposit : " <<pp->info(LHCb::ProtoParticle::CaloDepositID,-999.) <<endmsg;
      if( m_addHcal)verbose() << " Hcal factor : " << (1.+eHcal) << endmsg;
      if( m_mas> 0.)verbose() << " Mass : " << m_mas << endmsg;
      verbose() << " "<< endmsg;
    }

    // === push the particle into the container
    particles.push_back( particle );
  }
  m_count[1] += nSelUnconverted ;
  m_count[2] += nSelConverted ;
  counter("Created converted photons")+= nSelConverted;
  counter("Created unConverted photons")+= nSelUnconverted;


  // === debug
  if (msgLevel(MSG::DEBUG) ){
    debug() << " " << endmsg;
    debug() << "-----------------------" << endmsg;
    debug() << " Filtered and created :" << endmsg;
    debug() << " --> " << nSelConverted + nSelUnconverted << " photons (among "
            << nConverted + nUnconverted <<")"<< endmsg;
    debug() << "   -> " << nSelConverted
            << " Converted Photons (among " << nConverted <<")"<< endmsg;
    debug() << "   -> " << nSelUnconverted
            << " Unconverted Photons (among " << nUnconverted <<")"<< endmsg;
    debug() << " Skipped : " << nSkip << endmsg;
    debug() << "-----------------------" << endmsg;
  }
  return StatusCode::SUCCESS ;
}

// ============================================================================
double PhotonMaker::confLevel( const LHCb::ProtoParticle* pp ) const{
  if( 0 == pp ){ Error("confLevel(): ProtoParticle* points to NULL!"); return -1 ; };

  double CL = 1.0 ;

  // track matching
  if ( m_useCaloTrMatch ){
    if ( pp->hasInfo(LHCb::ProtoParticle::CaloTrMatch) ){
      // assume gaussian distribution (it is wrong!)
      CL *= ( 1.0 - std::exp( -0.5 * pp->info(LHCb::ProtoParticle::CaloTrMatch,-999.) )) ;
    }else{
      CL = 1.; //
      if( UNLIKELY( msgLevel(MSG::DEBUG) ) )debug() << "confLevel(): CaloTrMatch is not available " << endmsg ; 
    }
  }

  // NN-based photonID (IsNotH)
  if( m_useIsNotH ){
    if( pp->hasInfo(LHCb::ProtoParticle::IsNotH )){
      double v = pp->info(LHCb::ProtoParticle::IsNotH , 0. );
      if( v > 1. ) v=1.;
      if( v < 0. ) v=0.;
      CL *= v;
    }else{
      Warning("confLevel(): NN-based PhotonID IsNotE is not available" ) ;
      counter("unavailable IsNotH") += 1;
    }
  }

  // NN-based photonID (IsNotE)
  if( m_useIsNotE ){
    if( pp->hasInfo(LHCb::ProtoParticle::IsNotE )){
      double v = pp->info(LHCb::ProtoParticle::IsNotE , 0. );
      if( v > 1. ) v=1.;
      if( v < 0. ) v=0.;
      CL *= v;
    }else{
      Warning("confLevel(): NN-based PhotonID IsNotE is not available" ) ;
      counter("unavailable IsNotE") += 1;
    }
  }

  // IsPhoton
  if( m_useIsPhoton ){
    if( pp->hasInfo(LHCb::ProtoParticle::IsPhoton )){
      double v =  pp->info(LHCb::ProtoParticle::IsPhoton,+1.);
      if( v > 1. ) v = 1.;
      if( v < 0. ) v = 0.;
      CL *= (1-v);
    }else{
      Warning("confLevel(): NN-based IsPhoton is not available" ) ;
      counter("unavailable IsPhoton") += 1;
    }    
  }

  //(obsolete) DLL-based photonID
  if( m_usePhotonDLL ){
    if ( pp->hasInfo(LHCb::ProtoParticle::PhotonID) ){
      double pid = pp->info(LHCb::ProtoParticle::PhotonID, -1.  ) ;
      CL *= 0.5*(std::tanh(pid)+1);
  }
    else
      Warning("confLevel(): DLL-based PhotonID is not available" ) ;
      counter("unavailable PhotonDLL") += 1;
  }

  // return
  return CL ;
}
