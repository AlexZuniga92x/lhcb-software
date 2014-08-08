#include "GaudiKernel/ToolFactory.h" 
#include "Event/Particle.h"

// kernel
#include "GaudiKernel/PhysicalConstants.h"
#include "Kernel/RelatedInfoNamed.h"

// local
#include "RelInfoConeVariablesForEW.h"
//#include "Event/RecVertex.h"
//#include <Kernel/GetIDVAlgorithm.h>
//#include <Kernel/IDVAlgorithm.h>
//#include <Kernel/IDistanceCalculator.h>
//#include "TrackInterfaces/IPVOfflineTool.h"

// chi2
//#include "gsl/gsl_cdf.h"

//-----------------------------------------------------------------------------
// Implementation file for class : RelInfoConeVariablesForEW
//
// 2009-05-06 : Michel De Cian
//-----------------------------------------------------------------------------

// Declaration of the Algorithm Factory
DECLARE_TOOL_FACTORY( RelInfoConeVariablesForEW )

//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
RelInfoConeVariablesForEW::RelInfoConeVariablesForEW( const std::string &type,
                                          const std::string &name,
                                          const IInterface *parent)
    : GaudiTool ( type, name, parent )
{
  declareInterface<IRelatedInfoTool>( this );

  declareProperty( "ConeAngle", m_coneAngle = 0.,
                   "Set the deltaR of the cone around the seed" );

  declareProperty( "TrackType", m_trackType = 3,
                   "Set the type of tracks which are considered inside the cone" );

  declareProperty( "ExtreParticlesLocation", m_extraParticlesLocation = "StdAllNoPIDsMuons",
                   "Set the type of particles which are considered in the charged cone" );
  declareProperty( "ExtrePhotonsLocation", m_extraPhotonsLocation = "StdLooseAllPhotons",
                   "Set the type of photons which are considered in the neutral cone" );
  declareProperty( "Variables", m_variables, 
                   "List of variables to store (store all if empty)");
}

//=============================================================================
// Destructor
//=============================================================================
RelInfoConeVariablesForEW::~RelInfoConeVariablesForEW() {}


StatusCode RelInfoConeVariablesForEW::initialize() 
{
  const StatusCode sc = GaudiTool::initialize();
  if ( sc.isFailure() ) return sc;

  m_keys.clear(); 

  for ( const auto& var : m_variables )
  {
    short int key = RelatedInfoNamed::indexByName( var ); 
    if (key != RelatedInfoNamed::UNKNOWN) {
      m_keys.push_back( key );
      debug() << "Adding variable " << var << ", key = " << key << endmsg; 
    } else {
      warning() << "Unknown variable " << var << ", skipping" << endmsg; 
    }
  }

  return sc; 
}


//=============================================================================
// Calculate cone variables
//=============================================================================
StatusCode RelInfoConeVariablesForEW::calculateRelatedInfo( const LHCb::Particle *top,
                                                            const LHCb::Particle *seed)
{

  if ( msgLevel(MSG::DEBUG) )
    debug() << "==> Fill" << endmsg;

  // -- The vector m_decayParticles contains all the particles that belong to the given decay
  // -- according to the decay descriptor.

  // -- Clear the vector with the particles in the specific decay
  m_decayParticles.clear();

  // -- Add the mother (prefix of the decay chain) to the vector
  if ( msgLevel(MSG::DEBUG) )
    debug() << "Filling particle with ID " << top->particleID().pid() << endmsg;
  m_decayParticles.push_back( top );

  // -- Save all particles that belong to the given decay in the vector m_decayParticles
  saveDecayParticles( top );

  // -- Get all particles in the event
  LHCb::Particles *parts = get<LHCb::Particles>( "Phys/" + m_extraParticlesLocation + "/Particles" );
  if ( parts->size() == 0 ) {
    if ( msgLevel(MSG::WARNING) )
      warning() << "Could not retrieve extra-particles. Skipping" << endmsg;
    return StatusCode::FAILURE;
  }

  if ( seed ) {

    // -- Retrieve information in the charged cone
    int multiplicity = 0;
    std::vector < double > vectorP;
    double scalarP = 0.;
    double scalarPt = 0.;
    double minimumPtE = 0.;
    double maximumPtE = 0.;
    double minimumPtMu = 0.;
    double maximumPtMu = 0.;

    StatusCode sc = ChargedCone( seed, parts, m_coneAngle, multiplicity,
                                 vectorP, scalarP, scalarPt,
                                 minimumPtE, maximumPtE,
                                 minimumPtMu, maximumPtMu );
    if ( sc.isFailure() )
      multiplicity = -1;

    // -- Create a vector with the summed momentum of all tracks in the cone
    Gaudi::XYZVector coneMomentum;
    coneMomentum.SetX( vectorP[0] );
    coneMomentum.SetY( vectorP[1] );
    coneMomentum.SetZ( vectorP[2] );

    // -- Create a vector with the summed momentum of all tracks in the cone + seed
    Gaudi::XYZVector totalMomentum;
    totalMomentum.SetX(seed->momentum().X() + coneMomentum.X());
    totalMomentum.SetY(seed->momentum().Y() + coneMomentum.Y());
    totalMomentum.SetZ(seed->momentum().Z() + coneMomentum.Z());

    /*
    // -- Calculate the Eta-Phi difference between the summed momentum of all tracks in the cone and the seed
    double deltaEta = 0.;
    double deltaPhi = 0.;
    if ( multiplicity != 0 ) {
    deltaEta = seed->momentum().Eta() - coneMomentum.Eta();
    deltaPhi = fabs( seed->momentum().Phi() - coneMomentum.Phi() );
    if ( deltaPhi > M_PI )
    deltaPhi = 2 * M_PI - deltaPhi;
    }
    */

    // -- Fill the tuple with the variables
    m_mult = multiplicity;
    m_px = coneMomentum.X();
    m_py = coneMomentum.Y();
    m_pz = coneMomentum.Z();
    m_vp = sqrt( coneMomentum.Mag2() );
    m_vpt = sqrt( coneMomentum.Perp2() );
    m_sp = scalarP;
    m_spt = scalarPt;
    m_tp = sqrt( totalMomentum.Mag2() );
    m_tpt = sqrt( totalMomentum.Perp2() );
    if ( m_coneAngle == 0. ) {
      m_minpte = minimumPtE;
      m_maxpte = maximumPtE;
      m_minptmu = minimumPtMu;
      m_maxptmu = maximumPtMu;
    }

    // -- Retrieve information in the neutral cone
    int nmultiplicity = 0;
    std::vector < double > nvectorP;
    double nscalarP = 0.;
    double nscalarPt = 0.;

    // -- Get all photons in the event
    LHCb::Particles *photons = get<LHCb::Particles>( "Phys/" + m_extraPhotonsLocation + "/Particles" );
    if ( photons->size() != 0 ) {
      StatusCode nsc = NeutralCone( seed, photons, m_coneAngle,
                                    nmultiplicity, nvectorP, nscalarP, nscalarPt );
      if ( nsc.isFailure() )
        nmultiplicity = -1;
    }
    else {
      if ( msgLevel(MSG::WARNING) )
        Warning("Could not retrieve photons");
      nmultiplicity = -1;
      nvectorP.push_back( 0 );
      nvectorP.push_back( 0 );
      nvectorP.push_back( 0 );
    }

    // -- Create a vector with the summed momentum of all tracks in the cone
    Gaudi::XYZVector neutralConeMomentum;
    neutralConeMomentum.SetX( nvectorP[0] );
    neutralConeMomentum.SetY( nvectorP[1] );
    neutralConeMomentum.SetZ( nvectorP[2] );

    /*
    // -- Calculate the Eta-Phi difference between the summed momentum of all tracks in the cone and the seed
    double ndeltaEta = 0.;
    double ndeltaPhi = 0.;
    if ( nmultiplicity != 0 ) {
    ndeltaEta = seed->momentum().Eta() - neutralConeMomentum.Eta();
    ndeltaPhi = fabs( seed->momentum().Phi() - neutralConeMomentum.Phi() );
    if ( ndeltaPhi > M_PI )
    ndeltaPhi = 2 * M_PI - ndeltaPhi;
    }
    */

    // -- Fill the tuple with the variables
    m_nmult = nmultiplicity;
    m_npx =   neutralConeMomentum.X();
    m_npy =   neutralConeMomentum.Y();
    m_npz =   neutralConeMomentum.Z();
    m_nvp =   sqrt( neutralConeMomentum.Mag2() );
    m_nvpt =  sqrt( neutralConeMomentum.Perp2() );
    m_nsp =   nscalarP;
    m_nspt =  nscalarPt;

    m_map.clear();
    
    for ( const auto key : m_keys )
    {

      float value = 0;
      switch (key) {
        case RelatedInfoNamed::EWCONEANGLE : value = m_coneAngle; break;
        case RelatedInfoNamed::EWCONEMULT  : value = m_mult; break;
        case RelatedInfoNamed::EWCONEPX    : value = m_px; break;
        case RelatedInfoNamed::EWCONEPY    : value = m_py; break;
        case RelatedInfoNamed::EWCONEPZ    : value = m_pz; break;
        case RelatedInfoNamed::EWCONEVP    : value = m_vp; break;
        case RelatedInfoNamed::EWCONEVPT   : value = m_vpt; break;
        case RelatedInfoNamed::EWCONESP    : value = m_sp; break;
        case RelatedInfoNamed::EWCONESPT   : value = m_spt; break;
        case RelatedInfoNamed::EWCONETP    : value = m_tp; break;
        case RelatedInfoNamed::EWCONETPT   : value = m_tpt; break;
        case RelatedInfoNamed::EWCONEMINPTE   : value = m_minpte; break;
        case RelatedInfoNamed::EWCONEMAXPTE   : value = m_maxpte; break;
        case RelatedInfoNamed::EWCONEMINPTMU  : value = m_minptmu; break;
        case RelatedInfoNamed::EWCONEMAXPTMU  : value = m_maxptmu; break;
        case RelatedInfoNamed::EWCONENMULT    : value = m_nmult; break;
        case RelatedInfoNamed::EWCONENPX      : value = m_npx; break;
        case RelatedInfoNamed::EWCONENPY      : value = m_npy; break;
        case RelatedInfoNamed::EWCONENPZ      : value = m_npz; break;
        case RelatedInfoNamed::EWCONENVP      : value = m_nvp; break;
        case RelatedInfoNamed::EWCONENVPT     : value = m_nvpt; break;
        case RelatedInfoNamed::EWCONENSP      : value = m_nsp; break;
        case RelatedInfoNamed::EWCONENSPT     : value = m_nspt; break;
        default: value = 0.; break;
      }
      
      debug() << "  Inserting key = " << key << ", value = " << value << " into map" << endreq; 
    
      m_map.insert( std::make_pair( key, value) );
    }


  }
  else {

    if ( msgLevel(MSG::WARNING) )
      warning() << "The seed particle is not valid. Skipping" << endmsg;
    return StatusCode::FAILURE;

  }

  return StatusCode::SUCCESS;

}
//=============================================================================
// Save the particles in the decay chain (recursive function)
//=============================================================================
void RelInfoConeVariablesForEW::saveDecayParticles( const LHCb::Particle *top ) 
{

  // -- Fill all the daugthers in m_decayParticles
  for ( const auto& dau : top->daughters() )
  {

    // -- If the particle is stable, save it in the vector, or...
    if ( dau->isBasicParticle() ) {
      if ( msgLevel(MSG::DEBUG) )
        debug() << "Filling particle with ID " << dau->particleID().pid() << endmsg;
      m_decayParticles.push_back( dau );
    }
    else {
      // -- if it is not stable, call the function recursively
      m_decayParticles.push_back( dau );
      if ( msgLevel(MSG::DEBUG) )
        debug() << "Filling particle with ID " << dau->particleID().pid() << endmsg;
      saveDecayParticles( dau );
    }

  }

  return;

}
//=============================================================================
// Loop over all the tracks in the cone which do not belong to the desired decay
//=============================================================================
StatusCode RelInfoConeVariablesForEW::ChargedCone( const LHCb::Particle *seed,
                                            const LHCb::Particles *parts,
                                            const double rcut,
                                            int &mult,
                                            std::vector < double > &vP,
                                            double &sP, double &sPt,
                                            double &minPtE, double &maxPtE,
                                            double &minPtMu, double &maxPtMu )
{

  // -- Initialize values
  mult = 0;
  double sPx = 0.;
  double sPy = 0.;
  double sPz = 0.;
  sP  = 0.;
  sPt = 0.;
  minPtE = 1.e10;
  int minQE = 0;
  maxPtE = 0.;
  int maxQE = 0;
  minPtMu = 1.e10;
  int minQMu = 0;
  maxPtMu = 0.;
  int maxQMu = 0;

  // -- Get the 4-momentum of the seed particle
  const Gaudi::LorentzVector& seedMomentum = seed->momentum();

  for ( const LHCb::Particle *particle : *parts )
  {

    const LHCb::ProtoParticle *proto = particle->proto();
    if ( proto ) {

      const LHCb::Track *track = proto->track();
      if ( track ) {

        // -- Check if the track belongs to the decay itself
        bool isInDecay = isTrackInDecay( track );
        if ( isInDecay )
          continue;

        // -- Get the 3-momentum of the track
        const Gaudi::XYZVector& trackMomentum = track->momentum();

        // -- Calculate the difference in Eta and Phi between the seed particle and a track
        double deltaPhi = fabs( seedMomentum.Phi() - trackMomentum.Phi() );
        if ( deltaPhi > M_PI )
          deltaPhi = 2 * M_PI - deltaPhi;
        double deltaEta = seedMomentum.Eta() - trackMomentum.Eta();
        double deltaR   = sqrt( deltaPhi * deltaPhi + deltaEta * deltaEta );

        if ( track->type() == m_trackType ) {
          if ( ( rcut == 0. ) || ( deltaR < rcut ) ) {
            // -- Calculate vector information
            sPx += trackMomentum.X();
            sPy += trackMomentum.Y();
            sPz += trackMomentum.Z();

            // -- Calculate scalar information
            sP  += sqrt( trackMomentum.Mag2() );
            sPt += sqrt( trackMomentum.Perp2() );

            mult++;
          }

          if ( rcut == 0. ) {

            // Extra Electron
            double prsE = 50.;
            double eCalEoP = .10;
            double hCalEoP = .05;
            if ( proto->info( LHCb::ProtoParticle::CaloPrsE, -1. ) > prsE ) {
              if ( proto->info( LHCb::ProtoParticle::CaloEcalE, -1. ) / track->p() > eCalEoP ) {
                if ( ( proto->info( LHCb::ProtoParticle::CaloHcalE, -1. ) > 0 ) && ( proto->info( LHCb::ProtoParticle::CaloHcalE, -1. ) / track->p() < hCalEoP ) ) {
                  if ( track->pt() < minPtE ) {
                    minPtE = track->pt();
                    minQE  = track->charge();
                  }
                  if ( track->pt() > maxPtE ) {
                    maxPtE = track->pt();
                    maxQE  = track->charge();
                  }
                }
              }
            }

            // Extra Muon
            double minP = 10.e3;
            if ( track->p() > minP ) {
              const LHCb::MuonPID *muonPID = proto->muonPID();
              if ( muonPID ) {
                if ( muonPID->IsMuon() ) {
                  if ( track->pt() < minPtMu ) {
                    minPtMu = track->pt();
                    minQMu  = track->charge();
                  }
                  if ( track->pt() > maxPtMu ) {
                    maxPtMu = track->pt();
                    maxQMu  = track->charge();
                  }
                }
              }
            }

          }
        }
      }
    }
  }

  vP.push_back( sPx );
  vP.push_back( sPy );
  vP.push_back( sPz );

  if ( minPtE == 1.e10 )
    minPtE = 0.;
  if ( maxPtE == 0. )
    maxPtE = 0.;
  minPtE *= minQE;
  maxPtE *= maxQE;

  if ( minPtMu == 1.e10 )
    minPtMu = 0.;
  if ( maxPtMu == 0. )
    maxPtMu = 0.;
  minPtMu *= minQMu;
  maxPtMu *= maxQMu;

  return StatusCode::SUCCESS;

}
/*
  StatusCode RelInfoConeVariablesForEW::PVCone( const LHCb::Particle *seed, const LHCb::Particles *parts, const double rcut,
  int &mult, std::vector < double > &vP,
  double &sP, double &sPt,
  double &minM, double &totM,
  double &minPt, double &maxPt ) {

  // -- Initialize values
  mult = 0;
  double sPx = 0.;
  double sPy = 0.;
  double sPz = 0.;
  sP  = 0.;
  sPt = 0.;
  minM = 1.e10;
  totM = 0.;
  minPt = 1.e10;
  int minQ = 0;
  maxPt = 0.;
  int maxQ = 0;
  Gaudi::LorentzVector sM(0., 0., 0., 0.);

  // -- Get the 4-momentum of the seed particle
  Gaudi::LorentzVector seedMomentum = seed->momentum();

  LHCb::RecVertex::Container *pvs = get<LHCb::RecVertex::Container>( "Rec/Vertex/Primary" );
  if ( pvs->size() == 0 )
  if ( msgLevel(MSG::WARNING) )
  Warning( "Could not retrieve vertices" );

  IDVAlgorithm *dva = Gaudi::Utils::getIDVAlgorithm( contextSvc(), this ) ;
  if ( !dva )
  if ( msgLevel(MSG::WARNING) )
  Warning( "Could not retrieve DV utilities" );

  const IDistanceCalculator *dist = dva->distanceCalculator();
  if ( !dist )
  if ( msgLevel(MSG::WARNING) )
  Warning( "Could not retrieve distance calculator tool" );

  IPVOfflineTool* pvTool = tool<IPVOfflineTool>( "PVOfflineTool", this );
  if ( !pvTool )
  if ( msgLevel(MSG::WARNING) )
  Warning( "Could not retrieve IPVOfflineTool" );

  if (( pvs->size() == 0 ) || ( !dva || !dist || !pvTool )) {
  vP.push_back( 0 );
  vP.push_back( 0 );
  vP.push_back( 0 );
  minM = 0.;
  totM = 0.;
  minPt = 0.;
  maxPt = 0.;
  return StatusCode::FAILURE;
  }

  double ip, chi2;
  double ipMin = m_ipMin;
  LHCb::RecVertex *pv = NULL;

  std::vector<const LHCb::Track*> tracks2exclude;
  tracks2exclude.clear();
  for ( std::vector<const LHCb::Particle*>::const_iterator ip = m_decayParticles.begin(); ip != m_decayParticles.end(); ++ip ) {
  if ( (*ip)->proto() )
  if ( (*ip)->proto()->track() )
  tracks2exclude.push_back( (*ip)->proto()->track() );
  }

  for ( LHCb::RecVertex::Container::const_iterator iv = pvs->begin(); iv != pvs->end(); ++iv ) {

  LHCb::RecVertex newPV(**iv);

  const Gaudi::XYZPoint seedPosition = newPV.position();

  StatusCode scfit = pvTool->reDoSinglePV( seedPosition, tracks2exclude, newPV );
  if ( !scfit ) {
  Warning( "ReDoPV fails!", StatusCode::SUCCESS, 10 ).ignore();
  continue;
  }

  LHCb::VertexBase* newPVPtr = ( LHCb::VertexBase* ) &newPV;
  dist->distance( seed, newPVPtr, ip, chi2 );
  if ( msgLevel(MSG::DEBUG) )
  debug() << "IP = " << ip << "  CHI2 = " << sqrt( chi2 ) << "  IP_Min = " << ipMin << endmsg;
  if ( ip < ipMin ) {
  ipMin = ip;
  pv = (*iv);
  }
  }

  if ( !pv ) {
  if ( msgLevel(MSG::WARNING) )
  Warning( "Could not retrieve associated PV" );
  vP.push_back( 0 );
  vP.push_back( 0 );
  vP.push_back( 0 );
  minM = 0.;
  totM = 0.;
  minPt = 0.;
  maxPt = 0.;
  return StatusCode::FAILURE;
  }

  if ( pv->isPrimary() ) {

  const SmartRefVector<LHCb::Track> &tracks = pv->tracks();

  for ( LHCb::Particles::const_iterator ip = parts->begin(); ip != parts->end(); ++ip ) {
  const LHCb::Particle *particle = (*ip);

  const LHCb::ProtoParticle *proto = particle->proto();
  if ( proto ) {

  const LHCb::Track *track = proto->track();
  if ( track ) {

  // -- Check if the track belongs to the decay itself
  bool isInDecay = isTrackInDecay( track );
  if ( isInDecay )
  continue;

  // -- Check if the track belong to the same PV
  for ( SmartRefVector<LHCb::Track>::const_iterator it = tracks.begin(); it != tracks.end(); ++it ) {
  if ( track == (*it) ) {

  // -- Check the quality of the track

  // -- Track fit chi2
  if ( track->nDoF() > 0 )
  if ( gsl_cdf_chisq_Q( track->chi2() * m_pchi2Correction, track->nDoF() ) < m_pchi2Cut )
  continue;

  // -- Momentum resolution
  double SPoP_X1 =  50.e3;
  double SPoP_Y1 =    .04;
  double SPoP_X2 = 100.e3;
  double SPoP_Y2 =    .10;
  double Const   = SPoP_Y1 - (SPoP_Y2 - SPoP_Y1) / (SPoP_X2 - SPoP_X1) * SPoP_X1;
  double Slope   = (SPoP_Y2 - SPoP_Y1) / (SPoP_X2 - SPoP_X1);
  double SPoP_Cut;
  if ( sqrt( track->momentum().Mag2() ) < SPoP_X2 )
  SPoP_Cut = SPoP_Y1 > Const+Slope*sqrt(track->momentum().Mag2()) ?
  SPoP_Y1 : Const+Slope*sqrt(track->momentum().Mag2());
  else
  SPoP_Cut = SPoP_Y2;
  if ( sqrt( track->firstState().errP2() / track->momentum().Mag2() ) > SPoP_Cut )
  continue;

  // -- Get the 3-momentum of the track
  Gaudi::XYZVector trackMomentum = track->momentum();

  // -- Calculate the difference in Eta and Phi between the seed particle and a track
  double deltaPhi = fabs( seedMomentum.Phi() - trackMomentum.Phi() );
  if ( deltaPhi > M_PI )
  deltaPhi = 2 * M_PI - deltaPhi;
  double deltaEta = seedMomentum.Eta() - trackMomentum.Eta();
  double deltaR   = sqrt( deltaPhi * deltaPhi + deltaEta * deltaEta );

  if ( track->type() == m_trackType ) {
  if ( ( rcut == 0. ) || ( deltaR < rcut ) ) {
  // -- Calculate vector information
  sPx += trackMomentum.X();
  sPy += trackMomentum.Y();
  sPz += trackMomentum.Z();

  // -- Calculate scalar information
  sP  += sqrt( trackMomentum.Mag2() );
  sPt += sqrt( trackMomentum.Perp2() );

  // -- Calculate invariant masses
  Gaudi::LorentzVector particleMomentum = particle->momentum();
  if ( particle->pt() > m_ptMin ) {
  Gaudi::LorentzVector ppM = seedMomentum + particleMomentum;
  if ( ( minM == -1. ) || ( ppM.M() < minM ) )
  minM = ppM.M();
  sM += particleMomentum;
  }

  mult++;
  }

  if ( rcut == 0. ) {
  double minP = 10.e3;
  if ( track->p() > minP ) {
  const LHCb::MuonPID *muonPID = proto->muonPID();
  if ( muonPID ) {
  if ( muonPID->IsMuon() ) {
  if ( track->pt() < minPt ) {
  minPt = track->pt();
  minQ  = track->charge();
  }
  if ( track->pt() > maxPt ) {
  maxPt = track->pt();
  maxQ  = track->charge();
  }
  }
  }
  }
  }
  }
  }
  }
  }
  }
  }
  }

  vP.push_back( sPx );
  vP.push_back( sPy );
  vP.push_back( sPz );

  totM = sM.M();

  if ( ( mult < 1 ) || ( minM == 1.e10 ) )
  minM = 0.;
  if ( mult < 2 )
  totM = 0.;

  if ( minPt == 1.e10 )
  minPt = 0.;
  if ( maxPt == 0. )
  maxPt = 0.;
  minPt *= minQ;
  maxPt *= maxQ;

  return StatusCode::SUCCESS;

  }
*/
StatusCode RelInfoConeVariablesForEW::NeutralCone( const LHCb::Particle *seed, 
                                                   const LHCb::Particles *photons, 
                                                   const double rcut,
                                                   int &mult, 
                                                   std::vector<double> &vP,
                                                   double &sP, 
                                                   double &sPt )
{

  // -- Initialize values
  mult = 0;
  double sPx = 0.;
  double sPy = 0.;
  double sPz = 0.;
  sP  = 0.;
  sPt = 0.;

  // -- Get the 4-momentum of the seed particle
  const Gaudi::LorentzVector& seedMomentum = seed->momentum();

  for ( const LHCb::Particle *photon : *photons )
  {

    // -- Get the 3-momentum of the photon
    const Gaudi::XYZVector& photonMomentum = photon->momentum().Vect();

    // -- Calculate the difference in Eta and Phi between the seed particle and a photons
    double deltaPhi = fabs( seedMomentum.Phi() - photonMomentum.Phi() );
    if ( deltaPhi > M_PI )
      deltaPhi = 2 * M_PI - deltaPhi;
    double deltaEta = seedMomentum.Eta() - photonMomentum.Eta();
    double deltaR   = sqrt( deltaPhi * deltaPhi + deltaEta * deltaEta );

    if ( ( rcut == 0. ) || ( deltaR < rcut ) ) {
      // -- Calculate vector information
      sPx += photonMomentum.X();
      sPy += photonMomentum.Y();
      sPz += photonMomentum.Z();

      // -- Calculate scalar information
      sP  += sqrt( photonMomentum.Mag2() );
      sPt += sqrt( photonMomentum.Perp2() );

      mult++;
    }
  }

  vP.push_back( sPx );
  vP.push_back( sPy );
  vP.push_back( sPz );

  return StatusCode::SUCCESS;

}
//=============================================================================
// Check if the track is already in the decay
//=============================================================================
bool RelInfoConeVariablesForEW::isTrackInDecay( const LHCb::Track *track ) {

  bool isInDecay = false;

  for ( const LHCb::Particle * part : m_decayParticles )
  {
    const LHCb::ProtoParticle *proto = part->proto();
    if ( proto ) {

      const LHCb::Track *myTrack = proto->track();
      if ( myTrack ) {

        if ( myTrack == track ) {
          if ( msgLevel( MSG::DEBUG ) )
            debug() << "Track is in decay, skipping it" << endmsg;
          isInDecay = true;
        }

      }
    }
  }

  return isInDecay;

}

LHCb::RelatedInfoMap* RelInfoConeVariablesForEW::getInfo(void) 
{
  return &m_map; 
}
