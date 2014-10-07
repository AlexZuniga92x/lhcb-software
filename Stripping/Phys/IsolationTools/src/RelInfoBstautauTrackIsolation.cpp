#include "GaudiKernel/ToolFactory.h"
#include "RelInfoBstautauTrackIsolation.h"
#include "Kernel/RelatedInfoNamed.h"
#include "Event/Particle.h"
// kernel
#include "GaudiAlg/Tuple.h"
#include "GaudiAlg/TupleObj.h"
#include "GaudiKernel/PhysicalConstants.h"
#include <Kernel/IDistanceCalculator.h>
#include "Kernel/IPVReFitter.h"
//#include "Kernel/IOnOffline.h"
#include "Kernel/IDVAlgorithm.h"
#include <Kernel/GetIDVAlgorithm.h>


#include "TMath.h"
#include "TFile.h"
#include "TString.h"
#include <TROOT.h>
#include <TObject.h>
#include "TH1D.h"
//#include "TMVA/Reader.h"
#include "Kernel/IRelatedPVFinder.h"
#include "Math/Boost.h"

//-----------------------------------------------------------------------------
// Implementation file for class : RelInfoBstautauTrackIsolation
// Converted from ConeVariables by A. Shires
//
// 2014-07-25 : Giampiero Mancinelli
//-----------------------------------------------------------------------------

// Declaration of the Algorithm Factory
DECLARE_TOOL_FACTORY( RelInfoBstautauTrackIsolation )

//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
RelInfoBstautauTrackIsolation::RelInfoBstautauTrackIsolation( const std::string& type,
                                                              const std::string& name,
                                                              const IInterface* parent) : GaudiTool ( type, name , parent )
{
  declareInterface<IRelatedInfoTool>(this);

  declareProperty( "tracktype", m_tracktype = 3,
                   "Set the type of tracks which are considered inside the cone (default = 3)");

  declareProperty("PVInputLocation",m_PVInputLocation = LHCb::RecVertexLocation::Primary);
  declareProperty ( "TrackContainer",   m_TracksPath = LHCb::TrackLocation::Default); // default is "Rec/Track/Best "
  declareProperty("angle"     , m_angle  = 0.27     ); //
  declareProperty("fc"        , m_fc  = 0.60     ); //
  declareProperty("doca_iso"  , m_doca_iso  = 0.13     ); //
  declareProperty("ips"       , m_ips  = 3.0     ); //
  declareProperty("svdis"     , m_svdis  = -0.15     ); //
  declareProperty("svdis_h"   , m_svdis_h  = 30.     ); //
  declareProperty("pvdis"     , m_pvdis  = 0.5     ); //
  declareProperty("pvdis_h"   , m_pvdis_h  = 40.    ); //
  declareProperty( "Variables", m_variables,
                   "List of variables to store (store all if empty)");
  declareProperty(    "PVInputLocation"
                      , m_PVInputLocation = LHCb::RecVertexLocation::Primary
                      , " PV input location"
                      );

  //sort these keys out by adding all
  m_keys.clear();
  /*    std::vector<std::string>::const_iterator ivar;
        for (ivar = m_variables.begin(); ivar != m_variables.end(); ivar++) {
        short int key = RelatedInfoNamed::indexByName( *ivar );
        if (key != RelatedInfoNamed::Unknown) {
        m_keys.push_back( key );
        } else {
        }
        }
  */

  m_keys.push_back( RelatedInfoNamed::BSTAUTAUTRKISOFIRSTVALUETAUPPIM );
  m_keys.push_back( RelatedInfoNamed::BSTAUTAUTRKISOFIRSTVALUETAUPPIP1 );
  m_keys.push_back( RelatedInfoNamed::BSTAUTAUTRKISOFIRSTVALUETAUPPIP2 );
  m_keys.push_back( RelatedInfoNamed::BSTAUTAUTRKISOFIRSTVALUETAUMPIP );
  m_keys.push_back( RelatedInfoNamed::BSTAUTAUTRKISOFIRSTVALUETAUMPIM1 );
  m_keys.push_back( RelatedInfoNamed::BSTAUTAUTRKISOFIRSTVALUETAUMPIM2 );

}

//=============================================================================
// Destructor
//=============================================================================
RelInfoBstautauTrackIsolation::~RelInfoBstautauTrackIsolation() {}


//=============================================================================
// Initialize
//=============================================================================
StatusCode RelInfoBstautauTrackIsolation::initialize() {
  StatusCode sc = GaudiTool::initialize() ;
  if ( sc.isFailure() ) return sc ;

  //get from DV algorithm
  m_dva = Gaudi::Utils::getIDVAlgorithm ( contextSvc() ) ;
  if (0==m_dva) return Error("Couldn't get parent DVAlgorithm",
                             StatusCode::FAILURE);
  m_dist       = tool<IDistanceCalculator>("LoKi::DistanceCalculator",this);
  if( !m_dist ){
    Error("Unable to retrieve the IDistanceCalculator tool");
    return StatusCode::FAILURE;
  }
  
  m_descend = tool<IParticleDescendants> ( "ParticleDescendants", this );
  if( ! m_descend ) {
    fatal() << "Unable to retrieve ParticleDescendants tool "<< endreq;
    return StatusCode::FAILURE;
  }

  return sc;
}

//=============================================================================
// Fill Extra Info structure
//=============================================================================
StatusCode RelInfoBstautauTrackIsolation::calculateRelatedInfo( const LHCb::Particle *top,
                                                                const LHCb::Particle *top_bis )
{

  if ( msgLevel(MSG::DEBUG) ) debug() << "Calculating TrackIso extra info" << endmsg;
  m_bdt1 = 0 ;
 m_bdt1_TauP_piM = 0 ;
 m_bdt1_TauP_piP1 =0  ;
 m_bdt1_TauP_piP2 = 0;
 m_bdt1_TauM_piP = 0;
 m_bdt1_TauM_piM1 = 0;
 m_bdt1_TauM_piM2 = 0;


 if ( top->isBasicParticle() || top!=top_bis)
    {
      if ( msgLevel(MSG::DEBUG) ) 
        debug() << "top != top_bis" << endmsg;
      return StatusCode::SUCCESS ;
    }





  // -- The vector m_decayParticles contains all the particles that belong to the given decay
  // -- according to the decay descriptor.

  // -- Clear the vector with the particles in the specific decay
  m_decayParticles.clear();

  bool test = true;

  //set PV and SV of the mother
  //
  const LHCb::VertexBase* PV = m_dva->bestVertex(top);
   const SmartRefVector< LHCb::Particle > & daughters = top->daughters();
  LHCb::Particles* tracks = get<LHCb::Particles>("/Event/Phys/StdAllNoPIDsPions/Particles");
  //    LHCb::Tracks* tracks = get<LHCb::Tracks>(LHCb::TrackLocation::Default);
  if ( tracks->empty() )
  {
    if ( msgLevel(MSG::WARNING) ) Warning(  "Could not retrieve tracks. Skipping" );
    return StatusCode::FAILURE;
  }

  saveDecayParticles(top);
  if(exist<LHCb::RecVertex::Container>(m_PVInputLocation)){
    m_vertices = get<LHCb::RecVertex::Container>(m_PVInputLocation);
  }



  for( SmartRefVector< LHCb::Particle >::const_iterator idau = daughters.begin() ; idau != daughters.end() ; ++idau){//
    const LHCb::Particle* Part = *idau;
    if (Part->isBasicParticle()){
      if ( msgLevel(MSG::DEBUG) ) debug()<<"Trying to compute pions isolation on particle of ID "<<Part->particleID().pid()<<endmsg;
    }
    else{
      LHCb::Particle::ConstVector Daughters_2 = m_descend->descendants(Part);
      if ( msgLevel(MSG::VERBOSE) ) verbose() << "number of PID "<<Part->particleID().pid()<<"'s daughters : "<<Daughters_2.size()<<endmsg;//
      LHCb::Particle::ConstVector::const_iterator i_daug;
      for(i_daug = Daughters_2.begin(); i_daug!=Daughters_2.end();++i_daug){
          const LHCb::Particle* part = *i_daug;
          if(! part ) {
            if ( msgLevel(MSG::WARNING) ) Warning(  "The particle in question is not valid" );
            return StatusCode::FAILURE;
          }
          else{
            const LHCb::Particle* mother = NULL;
            // -- Fill all the daugthers in m_decayParticles
            for( SmartRefVector< LHCb::Particle >::const_iterator idau = daughters.begin() ; idau != daughters.end() ; ++idau){//
              const SmartRefVector< LHCb::Particle > & daughters2 = (*idau)->daughters();//
              // -- Fill all the daugthers in m_decayParticles
              for( SmartRefVector< LHCb::Particle >::const_iterator idau2 = daughters2.begin() ; idau2 != daughters2.end() ; ++idau2){//
                const LHCb::ProtoParticle* proto = (*idau2)->proto();
                if(proto){
                  const LHCb::Track* myTrack = proto->track();
                  if(myTrack){
                    if(myTrack == part->proto()->track()) mother=(*idau);
                  }
                }
              }
            }
            
            if (mother==NULL)
              {
                if ( msgLevel(MSG::WARNING) ) Warning( "Mother of part not found. Skipping");
                return StatusCode::FAILURE;
              }
            
          
            const LHCb::VertexBase *SV = mother->endVertex();
            
            if ( msgLevel(MSG::VERBOSE) ) verbose() << "Filling variables with particle " << part << endmsg;
            calcValue( part, tracks, PV, SV ) ;
            if ( msgLevel(MSG::DEBUG) ) debug() << m_bdt1 << '\t' << endmsg ;

            if ( msgLevel(MSG::DEBUG) ) debug()  << "after \"calcBDTValue\" "  << endmsg ;

            const LHCb::Particle* part2 = NULL;
            LHCb::Particle::ConstVector::const_iterator i_daug_2;
            for(i_daug_2=Daughters_2.begin(); i_daug_2!=Daughters_2.end();++i_daug_2 ){//
              const LHCb::Particle* part_2 = *(i_daug_2);
              if((i_daug_2!=i_daug)&&(part_2->charge()== part->charge())) part2= *(i_daug_2);//
              
            }
            
            
          
         
            if(Part->charge()>0)
              {
                if ( msgLevel(MSG::VERBOSE) ) verbose() << "Part ID "<<Part->particleID().pid()<<endmsg;//
                
                if (part->charge()<0)
                  {
                    if ( msgLevel(MSG::VERBOSE) ) verbose() << "part ID "<<part->particleID().pid()<<endmsg;//
                    m_bdt1_TauP_piM = m_bdt1;
                  
                  }else {
                  if(part->p() >= part2->p())
                    { 
                      if ( msgLevel(MSG::VERBOSE) ) verbose() << "part ID "<<part->particleID().pid()<<endmsg;//
                      m_bdt1_TauP_piP1 = m_bdt1;
                       }else
                    {
                      m_bdt1_TauP_piP2 = m_bdt1;
                     }
                }
                
              }else
              {
                if ( msgLevel(MSG::VERBOSE) ) verbose() << "Part ID "<<Part->particleID().pid()<<endmsg;//

                if (part->charge()>0)
                  {
                    if ( msgLevel(MSG::VERBOSE) ) verbose() << "part ID "<<part->particleID().pid()<<endmsg;//
                 
                    m_bdt1_TauM_piP = m_bdt1;
                     }else {
                    
                     
                     if(part->p() >= part2->p())
                     { 
                     m_bdt1_TauM_piM1 = m_bdt1;
                      }else
                     {
                     m_bdt1_TauM_piM2 = m_bdt1;
                     }
                }
                
              }
          }
      }
    }
  }
  

  /*

  const LHCb::Particle* mother  = NULL;

  const SmartRefVector< LHCb::Particle > & daughters = top->daughters();

  // -- Fill all the daugthers in m_decayParticles
  for( SmartRefVector< LHCb::Particle >::const_iterator idau = daughters.begin() ; idau != daughters.end() ; ++idau){

    const SmartRefVector< LHCb::Particle > & daughters2 = (*idau)->daughters();

    // -- Fill all the daugthers in m_decayParticles
    for( SmartRefVector< LHCb::Particle >::const_iterator idau2 = daughters2.begin() ; idau2 != daughters2.end() ; ++idau2){
      const LHCb::ProtoParticle* proto = (*idau2)->proto();
      if(proto){
        const LHCb::Track* myTrack = proto->track();

        if(myTrack){

          if(myTrack == part->proto()->track()) mother=(*idau);
        }
      }
    }
  }

  if (mother==NULL)
  {
    if ( msgLevel(MSG::WARNING) ) Warning(  "Mother of part not found. Skipping" );
    return StatusCode::FAILURE;
  }


  bool test = true;

  //set PV and SV of the mother
  //

  const LHCb::VertexBase* PV = m_dva->bestVertex(top);
  const LHCb::VertexBase *SV = mother->endVertex();

  if(exist<LHCb::RecVertex::Container>(m_PVInputLocation)){
    m_vertices = get<LHCb::RecVertex::Container>(m_PVInputLocation);
  }

  if( part )
  {

    if ( msgLevel(MSG::VERBOSE) ) verbose() << "Filling variables with particle " << part << endmsg;

    // -- process -- iterate over tracks
    //
  debug()<<" '*ciao*' "<<endmsg;

    calcValue( part, tracks, PV, SV ) ;
    if ( msgLevel(MSG::DEBUG) ) debug() << m_bdt1 << '\t'  << endmsg ;
   debug()<<" *ciao** after \"calcValue\" "<<endmsg;
 
    //
    //store
    m_map.clear();

    std::vector<short int>::const_iterator ikey;
    for (ikey = m_keys.begin(); ikey != m_keys.end(); ikey++) {

      float value = 0;
      switch (*ikey) {
      case RelatedInfoNamed::BSTAUTAUTRKISOFIRSTVALUE : value = m_bdt1; break;
      }
      if (msgLevel(MSG::DEBUG)) debug() << "  Inserting key = " << *ikey << ", value = " << value << " into map" << endreq;

      m_map.insert( std::make_pair( *ikey, value) );
    }

  }

  else
  {
    if ( msgLevel(MSG::WARNING) ) Warning( "The particle in question is not valid" );
    return StatusCode::FAILURE;
  }
*/



  
  m_map.clear();
  
  std::vector<short int>::const_iterator ikey;
  for (ikey = m_keys.begin(); ikey != m_keys.end(); ikey++) {
    
    float value = 0;
    switch (*ikey) {
    case RelatedInfoNamed::BSTAUTAUTRKISOFIRSTVALUETAUPPIM : value = m_bdt1_TauP_piM ; break;//
    case RelatedInfoNamed::BSTAUTAUTRKISOFIRSTVALUETAUPPIP1 : value = m_bdt1_TauP_piP1 ; break;//
    case RelatedInfoNamed::BSTAUTAUTRKISOFIRSTVALUETAUPPIP2 : value = m_bdt1_TauP_piP2 ; break;//
    case RelatedInfoNamed::BSTAUTAUTRKISOFIRSTVALUETAUMPIP : value = m_bdt1_TauM_piP ; break;//
    case RelatedInfoNamed::BSTAUTAUTRKISOFIRSTVALUETAUMPIM1 : value = m_bdt1_TauM_piM1 ; break;//
    case RelatedInfoNamed::BSTAUTAUTRKISOFIRSTVALUETAUMPIM2 : value = m_bdt1_TauM_piM2 ; break;//
    }
     if (msgLevel(MSG::DEBUG)) debug() << "  Inserting key = " << *ikey << ", value = " << value << " into map" << endreq;
    m_map.insert( std::make_pair( *ikey, value) );
  }


  return StatusCode(test);
}


//rel infor methods

LHCb::RelatedInfoMap* RelInfoBstautauTrackIsolation::getInfo(void) {
  return &m_map;
}

std::string RelInfoBstautauTrackIsolation::infoPath(void) {
  std::stringstream ss;
  ss << std::string("Particle2TrackIsolationRelations");
  return ss.str();
}

//=============================================================================
// Save the particles in the decay chain (recursive function)
//=============================================================================
void RelInfoBstautauTrackIsolation::saveDecayParticles( const LHCb::Particle *top)
{

  // -- Get the daughters of the top particle
  const SmartRefVector< LHCb::Particle > & daughters = top->daughters();

  // -- Fill all the daugthers in m_decayParticles
  for( SmartRefVector< LHCb::Particle >::const_iterator idau = daughters.begin() ; idau != daughters.end() ; ++idau){

    // -- If the particle is stable, save it in the vector, or...
    if( (*idau)->isBasicParticle() ){
      if ( msgLevel(MSG::DEBUG) ) debug() << "Filling particle with ID " << (*idau)->particleID().pid() << endmsg;
      m_decayParticles.push_back( (*idau) );
    }else{
      // -- if it is not stable, call the function recursively
      m_decayParticles.push_back( (*idau) );
      if ( msgLevel(MSG::DEBUG) ) debug() << "Filling particle with ID " << (*idau)->particleID().pid() << endmsg;
      saveDecayParticles( (*idau) );
    }

  }

}

//=============================================================================
// Check if the track is already in the decay
//=============================================================================
bool RelInfoBstautauTrackIsolation::isTrackInDecay(const LHCb::Track* track){

  bool isInDecay = false;

  for(  std::vector<const LHCb::Particle*>::iterator it = m_decayParticles.begin() ; it != m_decayParticles.end() ; ++it ){

    const LHCb::ProtoParticle* proto = (*it)->proto();
    if(proto){
      const LHCb::Track* myTrack = proto->track();

      if(myTrack){

        if(myTrack == track){
          if ( msgLevel(MSG::DEBUG) ) debug() << "Track is in decay, skipping it" << endmsg;
          isInDecay = true;
        }
      }
    }
  }
  return isInDecay;
}


///============================================================================
/// Track isolation method
///============================================================================



bool RelInfoBstautauTrackIsolation::calcValue( const LHCb::Particle * part
                                               , const LHCb::Particles * tracks
                                               , const LHCb::VertexBase * PV
                                               , const LHCb::VertexBase * SV
                                               )
{
  //double bdtval = 0 ;
  //double bdtmin = 0 ;
  double doca   = 0;
  double angle  = 0;
  double fc     = 0;
  double pvDistGeometric(0) ;
  double svDistGeometric(0) ;
  double ipchisqany(0);

  int isolation1 = 0;

  LHCb::Particles::const_iterator tr_it, tr_it_end( tracks->end() ) ;
  for ( tr_it = tracks->begin() ; tr_it != tr_it_end ; ++tr_it )
  {
    const LHCb::Track * track = (* tr_it)->proto()->track() ;
    //pointer comparison
    //        if ( track == part->proto()->track() ) continue ;
    //        if (isTrackInDecay(track)) Indecay = true ;
    if (isTrackInDecay(track)) continue;

    Gaudi::XYZVector trackMomentum = track->momentum();
    Gaudi::XYZPoint trackPosition = track->position();
    //exception handling?
    Gaudi::XYZVector partMomentum = part->proto()->track()->momentum();
    Gaudi::XYZPoint partPosition = part->proto()->track()->position();

    // --  takes seven variables
    //
    angle         = enclosedAngle(trackMomentum, partMomentum);
    Gaudi::XYZPoint perpFoot_track, perpFoot_daughter;

    Gaudi::XYZPoint vertex_mu_track;
    bool failed = false;
    // Get Mu + Track "vertex"
    getPerpFeet(  trackPosition, trackMomentum, partPosition, partMomentum,
                  perpFoot_track, perpFoot_daughter, vertex_mu_track, failed);
    if(failed && msgLevel(MSG::DEBUG) ) debug() << "Vertex calculation ( perpendicular feet ) failed: denom == 0!" << endmsg;
    // FC
    fc = calcFC(  trackMomentum, partMomentum, vertex_mu_track, PV);
    if(fc==-1 && msgLevel(MSG::DEBUG) ) debug() << "FC calculation failed: fc_denom == 0!" << endmsg;
    // DOCA
    StatusCode sc_doca  = m_dist->distance(track,part->proto()->track(),doca);
    if(!sc_doca)  {
      if ( msgLevel(MSG::VERBOSE) )   debug()<<"Tool6*** - failed sc_doca"<<endmsg;
      return StatusCode(sc_doca);
    }
    double pvDist,pvDistChi2 ;
    StatusCode sc_pv    = m_dist->distance(PV, vertex_mu_track, pvDist, pvDistChi2);
    pvDistGeometric     = calcVertexDist(vertex_mu_track, PV);
    if(!sc_pv)  {
      if ( msgLevel(MSG::VERBOSE) )   debug()<<"Tool6*** - failed sc_pv"<<endmsg;
      return StatusCode(sc_pv);
    }
    double svDist, svDistChi2 ;
    StatusCode sc_sv    = m_dist->distance(SV, vertex_mu_track, svDist, svDistChi2);
    svDistGeometric     = calcVertexDist(vertex_mu_track, SV);
    if(!sc_sv)  {
      if ( msgLevel(MSG::VERBOSE) )   debug()<<"Tool6*** - failed sc_sv"<<endmsg;
      return StatusCode(sc_sv);
    }
    
     if ( msgLevel(MSG::VERBOSE) )   {
        debug()<<"Tool6*** - angle <m_angle "<<angle<<"<"<<m_angle<<endreq;//
        debug()<<"Tool6*** - (2*doca)<m_doca_iso "<<(2*doca)<<"<"<<m_doca_iso<<endreq;//
        debug()<<"Tool6*** - fc<m_fc "<<fc<<"<"<<m_fc<<endreq;//
        debug()<<"Tool6*** - ipchisqany>m_ips "<<ipchisqany<<">"<<m_ips<<endreq;//
        debug()<<"Tool6*** - svDistGeometric>m_svdis && svDistGeometric<m_svdis_h "<<svDistGeometric<<">"<<m_svdis<<" && "<< svDistGeometric<<"<"<<m_svdis_h<<endreq;//
        debug()<<"Tool6*** - pvDistGeometric>m_pvdis && pvDistGeometric<m_pvdis_h "<<pvDistGeometric<<">"<<m_pvdis <<" && "<<pvDistGeometric<<"<"<<m_pvdis_h<<endreq;//
        debug()<<"Tool6*** - track->type()==m_tracktype "<<track->type()<<"=="<<m_tracktype<<endreq;
     }
     
    ipchisqany = calcIPToAnyPV(track) ;
    if (angle <m_angle && fc<m_fc && (2*doca)<m_doca_iso && ipchisqany>m_ips &&
        svDistGeometric>m_svdis && svDistGeometric<m_svdis_h && pvDistGeometric>m_pvdis && pvDistGeometric<m_pvdis_h
        && track->type()==m_tracktype) {
      if ( msgLevel(MSG::VERBOSE) )   debug()<<"Tool6*** - entered in the if statement "<<endreq;//
          
      
      isolation1+=1;}
}
  m_bdt1 = isolation1 ;

  if ( msgLevel(MSG::VERBOSE) )   debug()<<"Tool6*** - isolation1  : "<<isolation1<<endmsg;//



  return true ;
}

///============================================================================
/// helper functions
///============================================================================


/// ------------------
/// calc ips to any PV
/// ------------------

double RelInfoBstautauTrackIsolation::calcIPToAnyPV( const LHCb::Track * track )
{
  LHCb::RecVertex::Container::const_iterator iv;
  double ips(-1),imp(-1),impchi2(-1);
  ips = 6.0e5;
  for(iv = m_vertices->begin();iv!=m_vertices->end();iv++){
    StatusCode sc_ips = m_dist->distance(track,(*iv),imp,impchi2);
    if(!sc_ips) return StatusCode(sc_ips);
    if(ips>impchi2) ips = impchi2;
  }
  return sqrt(ips);
}



///============================================================================
/// Calculates the perpendicular feet on the track's and muon's trajectory
///
/// The formulae are derived from:
/// Let p1 and p2 be the points where the intersect (p1-p2) is equal to the
/// doca-vector of the track and the muon satisfying:
///    ( p1 - p2 ) * track_mom == 0
/// && ( p1 - p2 ) * mu_mom == 0
/// solve equations for scalars mu1 and mu2 which are the translations
/// along the unit momentum vectors
/// => p1 = track_pos + mu1 * Unit(track_mom) and
///    p2 = mu_pos + mu2 * Unit(mu_mom)
///
/// Additionally, a "vertex" is calculated as the mid-point
/// on the doca line segment
///============================================================================
void RelInfoBstautauTrackIsolation::getPerpFeet(Gaudi::XYZPoint track_pos,
                                                Gaudi::XYZVector track_p,
                                                Gaudi::XYZPoint mu_pos,
                                                Gaudi::XYZVector mu_p,
                                                Gaudi::XYZPoint& perpFootTrack,
                                                Gaudi::XYZPoint& perpFootMu,
                                                Gaudi::XYZPoint& vertex, bool& fail)
{
  // def difference of positions
  Gaudi::XYZVector  diffPos(track_pos - mu_pos);
  // Get unit vectors
  Gaudi::XYZVector  unitTrack_p(track_p.unit());
  Gaudi::XYZVector  unitMu_p(mu_p.unit());
  Gaudi::XYZPoint   temp1(0.,0.,0.);
  Gaudi::XYZPoint   temp2(0.,0.,0.);
  fail = false;
  // def used scalar-products
  double  d_DiffTr  = diffPos.Dot( unitTrack_p);
  double  d_DiffMu  = diffPos.Dot( unitMu_p);
  double  d_MuTrack = unitMu_p.Dot( unitTrack_p);
  double  d_TrTr    = unitTrack_p.Dot( unitTrack_p);
  double  d_MuMu    = unitMu_p.Dot( unitMu_p);
  double  denom     = d_MuTrack * d_MuTrack - d_MuMu * d_TrTr;
  if (fabs(denom)<1E-27) {
    perpFootTrack = temp1;
    perpFootMu    = temp2;
    fail = true;
  }
  else {
    double numer  = d_DiffTr * d_MuTrack - d_TrTr * d_DiffMu;
    double mu2    = numer / denom;
    double mu1    = ( mu2 * d_MuTrack - d_DiffTr ) / d_TrTr ;
    perpFootTrack = track_pos + unitTrack_p *mu1;
    perpFootMu    = mu_pos    + unitMu_p    *mu2;
  }
  if ( fail && msgLevel(MSG::DEBUG) ) debug() << d_DiffTr << '\t' << d_DiffMu << '\t' << d_MuTrack << '\t' << d_TrTr << '\t' << denom << endmsg ;
  //
  vertex = ( perpFootTrack + ( perpFootMu - perpFootTrack ) * 0.5 );
}
///============================================================================
/// Calculates fc value which is a combination of momenta, transverse
/// momenta and angles
///
/// For more information see Bs2mumu roadmap or
/// G. Mancinelli & J. Serrano LHCb-INT-2010-011
/// XYZVector::R() equals length of the vector,
/// XYZVector::Rho() equals length projection on X-Y plane -> like
/// transverse momentum
///============================================================================

double RelInfoBstautauTrackIsolation::calcFC( Gaudi::XYZVector p,
                                              Gaudi::XYZVector p_mu,
                                              Gaudi::XYZPoint vtx,
                                              const LHCb::VertexBase* PV)
{

  Gaudi::XYZVector rv;
  Gaudi::XYZPoint pv = PV->position();
  double fc = -1.;

  rv = vtx - pv;
  double pt=p.Rho()+p_mu.Rho();
  Gaudi::XYZVector ptot(p+p_mu);
  double temp = arcosine(rv,ptot);
  double  num=ptot.R()*sin(temp);
  double  den=(num+pt);
  return fc = num/den;
}

double RelInfoBstautauTrackIsolation::arcosine(Gaudi::XYZVector p1,Gaudi::XYZVector p2) {

  double num    = (p1.Cross(p2)).R();
  double den    = p1.R()*p2.R();
  double seno   = num/den;
  double coseno = p1.Dot(p2)/den;
  double alpha  = asin(fabs(seno));
  if (coseno < 0 ) {
    alpha = ROOT::Math::Pi() - alpha;
  }
  return alpha;
}

///--------------------------------------------
/// calculates angle between two vectors
///--------------------------------------------
double RelInfoBstautauTrackIsolation::enclosedAngle(Gaudi::XYZVector p1,Gaudi::XYZVector p2) {
  double den      = p1.R()*p2.R();
  double cosAngle = p1.Dot(p2)/den;
  double angle    = acos(fabs(cosAngle));
  if (cosAngle < 0 ) {
    angle = ROOT::Math::Pi() - angle;
  }
  return angle;
}

double RelInfoBstautauTrackIsolation::calcVertexDist(Gaudi::XYZPoint muTrack, const LHCb::VertexBase* v){
  if (v==NULL) return 0 ;
  Gaudi::XYZPoint vertex = v->position();
  return ( (muTrack.z()-vertex.z())/fabs(muTrack.z()-vertex.z())*(muTrack-vertex).R() );
}

