// Include files
#include "GaudiKernel/DeclareFactoryEntries.h"
#include "Event/Particle.h"
// kernel

#include "TMath.h"
#include "TFile.h"
#include "TString.h"
#include <TROOT.h>
#include <TObject.h>
#include "TH1D.h"
#include <iostream>
//#include "TMVA/Reader.h"
#include "Kernel/IRelatedPVFinder.h"
#include "Math/Boost.h"

#include "RelInfoBs2MuMuBIsolations.h"


//-----------------------------------------------------------------------------
// Implementation file for class : RelInfoBs2MuMuBIsolations
//
// 2014-08-18 : Fatima Soomro
//-----------------------------------------------------------------------------

// Declaration of the Algorithm Factory
DECLARE_TOOL_FACTORY( RelInfoBs2MuMuBIsolations )

//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
RelInfoBs2MuMuBIsolations::RelInfoBs2MuMuBIsolations( const std::string& type,
                                                    const std::string& name,
                                                    const IInterface* parent)
  : GaudiTool( type, name , parent )
{
  declareInterface<IRelatedInfoTool>(this);
  declareProperty( "tracktype", m_tracktype = 3,
                   "Set the type of tracks which are considered inside the cone (default = 3)");
  declareProperty("angle"     , m_angle  = 0.27     ); // 
  declareProperty("fc"        , m_fc  = 0.60     ); // 
  declareProperty("doca_iso"  , m_doca_iso  = 0.13     ); // 
  declareProperty("ips"       , m_ips  = 3.0     ); // 
  declareProperty("svdis"     , m_svdis  = -0.15     ); // 
  declareProperty("svdis_h"   , m_svdis_h  = 30.     ); // 
  declareProperty("pvdis"     , m_pvdis  = 0.5     ); // 
  declareProperty("pvdis_h"   , m_pvdis_h  = 40.    ); // 
  declareProperty("clone_cut"   , m_clone_cut  = -9999. ); // 
  declareProperty("ghost_cut"   , m_ghost_cut  = 0.3 ); // 
  declareProperty("trchi2_cut"   , m_trchi2_cut  = 3.0 ); // 
  declareProperty("makeTrackCuts", m_makeTrackCuts = false);
  declareProperty("IsoTwoBody", m_IsoTwoBody = false);
  declareProperty("ParticlePath",   m_ParticlePath="/Event/Phys/StdAllNoPIDsPions/Particles");
  declareProperty ( "TrackContainer",   m_TracksPath = LHCb::TrackLocation::Default); // default is "Rec/Track/Best "
  declareProperty("Variables", m_variables, "List of variables to store (store all if empty)");
  declareProperty("PVInputLocation",m_PVInputLocation = LHCb::RecVertexLocation::Primary);

  m_keys.clear();

}

//=============================================================================
// Destructor
//=============================================================================
RelInfoBs2MuMuBIsolations::~RelInfoBs2MuMuBIsolations() {}

//=============================================================================
// Initialization
//=============================================================================
StatusCode RelInfoBs2MuMuBIsolations::initialize() {

  StatusCode sc = GaudiTool::initialize() ;
  if ( sc.isFailure() ) return sc ;

  //get from DV algorithm
  m_dva = Gaudi::Utils::getIDVAlgorithm ( contextSvc() ) ;
  if (0==m_dva) return Error("Couldn't get parent DVAlgorithm", StatusCode::FAILURE);

  m_dist  = tool<IDistanceCalculator>("LoKi::DistanceCalculator",this);
  if( !m_dist ){
    Error("Unable to retrieve the IDistanceCalculator tool");
    return StatusCode::FAILURE;
  }
 
  m_combiner  = m_dva->particleCombiner();
  //m_combiner = tool<IParticleCombiner>("LoKi::", this)
 
  info()<<" ==> Initialize"<<endmsg;
  debug()<<" ==> Initialize"<<endmsg;

  //configure keys
  m_keys.push_back(RelatedInfoNamed::BSMUMUCDFISO);
  m_keys.push_back(RelatedInfoNamed::BSMUMUOTHERBMAG);
  m_keys.push_back(RelatedInfoNamed::BSMUMUOTHERBANGLE);
  m_keys.push_back(RelatedInfoNamed::BSMUMUOTHERBBOOSTMAG);
  m_keys.push_back(RelatedInfoNamed::BSMUMUOTHERBBOOSTANGLE);
  m_keys.push_back(RelatedInfoNamed::BSMUMUOTHERBTRACKS);

  return sc;
}

//rel infor methods
LHCb::RelatedInfoMap* RelInfoBs2MuMuBIsolations::getInfo(void) {
  return &m_map;
}

std::string RelInfoBs2MuMuBIsolations::infoPath(void){ // check with Alex
  std::stringstream ss;
  // // this function is redundant (Anton)
  ss << std::string("Particle2VertexIsolationRelations");
  //ss << std::string("Particle2TrackIsolationRelations");
  return ss.str();
}

//=============================================================================
// Fill the related info with track variables
//=============================================================================
StatusCode RelInfoBs2MuMuBIsolations::calculateRelatedInfo(const LHCb::Particle* top, const LHCb::Particle *part){
 
  if ( msgLevel(MSG::DEBUG) )  debug() << " part is "<<part->particleID().pid()<<" while top is "<< top->particleID().pid()<< endmsg;

  m_otherB_mag=0.;
  m_otherB_angle=-1.;
  m_otherB_boost_mag=0.;
  m_otherB_boost_angle=-1.;
  otherBtracks= 0;
  m_CDFIso = -1;
 
  // -- The vector m_decayParticles contains all the particles that belong to the given decay
  // -- according to the decay descriptor.
  // -- Clear the vector with the particles in the specific decay Probably not needed here
  m_decayParticles.clear();
  m_decayParticles.push_back( top );
  saveDecayParticles( top );
  info()<<"this should be printed. I am looking at particle "<<part->particleID().pid()<<endmsg;
  
  StatusCode testcode ;

  LHCb::Tracks* tracks = get<LHCb::Tracks>(m_TracksPath);
  if ( tracks->empty() )
  {
    if ( msgLevel(MSG::WARNING) ) Warning( std::string("Could not retrieve tracks at location: ")+m_TracksPath+ " .... Skipping" );
    return StatusCode::FAILURE;
  }

  // If xthis 
  /// check that it is a 2 body decay
  const LHCb::Particle::ConstVector& daughterVec = part->daughtersVector();
  int idx = 0;
  for (LHCb::Particle::ConstVector::const_iterator ipart=daughterVec.begin();ipart!=daughterVec.end();++ipart)
  {
    if ( NULL==(*ipart)->proto() ) continue;
    idx++;
  }
  if(idx != 2 )  {
    info()<<"well, daughters were "<<idx<<endreq;
    //return StatusCode::SUCCESS;
  }

  //StatusCode testcode ;
  // Call functions to do the work and assign values

  if ( msgLevel(MSG::DEBUG) ) debug() << "Going to call CDFIsolation computation" << endmsg ;
  testcode = CDFIsolation(top, part);
  if ( msgLevel(MSG::DEBUG) ) debug() << "Computed CDFIsolation: "<< m_CDFIso <<" with statuscode "<<testcode <<endmsg;
  if(!testcode) return StatusCode::FAILURE;

  if ( msgLevel(MSG::DEBUG) ) debug() << "Going to call OtherB computation" << endmsg ;
  testcode = OtherB(top, part);
  if ( msgLevel(MSG::DEBUG) )
    debug() << "Computed quantities "<< m_otherB_mag<<" "<< m_otherB_angle<< " "<<m_otherB_boost_mag<<" "<<m_otherB_boost_angle <<" "<< otherBtracks<<" with statuscode "<< testcode <<endmsg;
  if(!testcode) return StatusCode::FAILURE;

  m_map.clear();
  info()<<" Map cleared.... "<<endreq;
  std::vector<short int>::const_iterator ikey;
  for (ikey = m_keys.begin(); ikey != m_keys.end(); ikey++) {

    double value = 0;

    switch (*ikey) {
    case RelatedInfoNamed::BSMUMUCDFISO           : value = m_CDFIso; break;
    case RelatedInfoNamed::BSMUMUOTHERBMAG        : value = m_otherB_mag; break;
    case RelatedInfoNamed::BSMUMUOTHERBANGLE      : value = m_otherB_angle; break;
    case RelatedInfoNamed::BSMUMUOTHERBBOOSTMAG   : value = m_otherB_boost_mag; break;
    case RelatedInfoNamed::BSMUMUOTHERBBOOSTANGLE : value = m_otherB_boost_angle; break;
    case RelatedInfoNamed::BSMUMUOTHERBTRACKS     : value = otherBtracks; break;
    
    }
    if (msgLevel(MSG::DEBUG)) debug() << "  Inserting key = " << *ikey << ", value = " << value << " into map" << endreq;
    info () << "  Inserting key = " << *ikey << ", value = " << value << " into map" << endreq;
    m_map.insert( std::make_pair( *ikey, value) );
  } //iter over keys

  return StatusCode(testcode);

}

//=============================================================================
// Save the particles in the decay chain (recursive function)
//=============================================================================
void RelInfoBs2MuMuBIsolations::saveDecayParticles( const LHCb::Particle *top){

  // -- Fill all the daugthers in m_decayParticles
  for ( const auto& dau : top->daughters() ) {
    // -- If the particle is stable, save it in the vector, or...
    if ( dau->isBasicParticle() ) {
      if ( msgLevel(MSG::DEBUG) ) debug() << "Filling particle with ID " << dau->particleID().pid() << endmsg;
      m_decayParticles.push_back( dau );
    }
    else{
      // -- if it is not stable, call the function recursively
      m_decayParticles.push_back( dau );
      if ( msgLevel(MSG::DEBUG) ) debug() << "Filling particle with ID " << dau->particleID().pid() << endmsg;
      saveDecayParticles( dau );
    } //else 
  } // for loop

}

bool RelInfoBs2MuMuBIsolations::isTrackInDecay(const LHCb::Track* track){
  
  bool isInDecay = false;
  for ( const LHCb::Particle * part : m_decayParticles ) {
    const LHCb::ProtoParticle* proto = part->proto();
    if(proto) {
      const LHCb::Track* myTrack = proto->track();
      if(myTrack) {
	if(myTrack == track)
	  {
	    if ( msgLevel(MSG::DEBUG) ) debug() << "Track is in decay, skipping it" << endmsg;
	    isInDecay = true;
	  } 
      }// if myTrack
    }// if proto 
  } // for loop
  
  return isInDecay;

}

//=============================================================================
// Other B quantities 
// From /afs/cern.ch/user/s/serranoj/public/Bs2MuMu/Bs2MuMuAnalysis.cpp
//=============================================================================

StatusCode RelInfoBs2MuMuBIsolations::OtherB(const LHCb::Particle *top, 
					     const LHCb::Particle *part){

  const LHCb::VertexBase* goodPV = m_dva->bestVertex(top);

  LHCb::Particles OTHERB_parts;
  
  // in cppm code m_ParticlePath="/Event/Phys/StdAllNoPIDsPions/Particles";
  LHCb::Particles* m_allparts = get<LHCb::Particles>(m_ParticlePath);
  for ( LHCb::Particles::const_iterator ipp = m_allparts->begin(); ipp != m_allparts->end() ; ++ipp) {

    bool isInList = isTrackInDecay((*ipp)->proto()->track());
    if ( msgLevel(MSG::DEBUG) ) debug() <<" check isInList = "<<isInList<< endreq;    
    if (isInList) continue;
    
    if ( msgLevel(MSG::DEBUG) ) debug() <<"particle inNOTinlist of siblings... will continue with computation"<< endreq;

    Gaudi::XYZVector p_track = ((*ipp)->proto()->track()->momentum());
    double pttrack = p_track.rho();
    double ptrackmag = p_track.R();
    double imp = 0.;
    double ips = -1.;
    m_dist->distance ( (*ipp), goodPV, imp, ips );
    ips = TMath::Sqrt(ips);

    
    if ( (ips>4.0) && (ips<40.0) && (pttrack> 200.0) && (pttrack< 2000.0) && (ptrackmag< 30000.0) )
    {
      LHCb::Particle* track = *(ipp);
      OTHERB_parts.add(track) ;
      otherBtracks++;
    }
  }
      

  if ( otherBtracks>0  && (m_decayParticles.size()>2))
  {

    Gaudi::XYZVector ptproj(0.,0.,0.);
    LHCb::Particles::const_iterator ipp;
    for ( ipp = (&OTHERB_parts)->begin(); ipp != (&OTHERB_parts)->end() ; ++ipp) {
      Gaudi::XYZVector ptrack = ((*ipp)->proto()->track()->momentum());
      ptproj += ptrack;
    }

    m_otherB_mag = ptproj.R();
    m_otherB_angle=arcosine( ptproj, part->slopes());


    info()<<" size of decayparts "<<m_decayParticles.size()<<endreq;


    const LHCb::Particle *vdau1 = m_decayParticles[1];
    const LHCb::Particle *vdau2 = m_decayParticles[2];
    info()<<"found daughters with charges "<<vdau1->charge()<<"  "<<vdau2->charge()<<endreq;

    const LHCb::Particle *myDau = NULL;
    if (vdau1->charge()>0) myDau = vdau1;
    else if (vdau2->charge()>0) myDau = vdau2;
    if(vdau1->charge() == vdau2->charge() ) myDau = vdau2;


    Gaudi::XYZVector ptproj2(0.,0.,0.);
    const Gaudi::LorentzVector p2(part->momentum());
    ROOT::Math::Boost boostToB( p2.BoostToCM() );
    const Gaudi::LorentzVector p3(myDau->momentum());
    const Gaudi::LorentzVector boostedMu = boostToB( p3 );
    const Gaudi::XYZVector pmuplus = Gaudi::XYZVector( boostedMu ) ;
    for ( ipp = (&OTHERB_parts)->begin(); ipp != (&OTHERB_parts)->end() ; ++ipp) {
      const Gaudi::LorentzVector p1((*ipp)->momentum());
      const Gaudi::LorentzVector boostedTrk = boostToB( p1 );
      const Gaudi::XYZVector ptrack = Gaudi::XYZVector( boostedTrk ) ;
      ptproj2 += ptrack;
    }

    m_otherB_boost_mag= ptproj2.R();
    m_otherB_boost_angle =  arcosine(pmuplus,ptproj2);

    
  } // if ntracks>0

  else {
    m_otherB_mag=-1.;
    m_otherB_angle= -1;
    m_otherB_boost_mag=-1.;
    m_otherB_boost_angle= -1;
  }

  return StatusCode::SUCCESS;

}




//=============================================================================
// CDF isolation variable for the B
//=============================================================================
StatusCode RelInfoBs2MuMuBIsolations::CDFIsolation(const LHCb::Particle* top, 
						   const LHCb::Particle* part){
  
  bool test = true;
  
  LHCb::Particle::Range  parts = get<LHCb::Particle::Range>(m_ParticlePath);
  if (!parts) {
    Error( " Failed to get particles container " );
    return StatusCode::SUCCESS;
  }


  double pt_bs = part->momentum().rho();

  double iso_giampi = 0.0;
  double iso_giampi_tc = 0.0;

  for(LHCb::Particle::Range::const_iterator ipp=parts.begin();ipp!=parts.end();ipp++){
    const LHCb::ProtoParticle *proto = (*ipp)->proto();
    if(proto) {
      const LHCb::Track* atrack = proto->track();
      if(atrack) {

	bool isInList = isTrackInDecay(atrack);
	if ( msgLevel(MSG::DEBUG) ) debug() <<" check isInList = "<<isInList<< endreq;    
	if (isInList) continue;
	if ( msgLevel(MSG::DEBUG) ) debug() <<"particle inNOTinlist of siblings... will continue with computation"<< endreq;
	
	Gaudi::XYZVector ptrack =  ((*ipp)->proto()->track()->momentum());
        double pttrack = ptrack.rho();
	
	double deta      =  part->momentum().Eta() - (*ipp)->momentum().Eta() ;
	double delta_phi =  part->momentum().Phi() - (*ipp)->momentum().Phi();
	delta_phi = TMath::Abs(delta_phi);
	
	//if(!m_IsoTwoBody){
	  if (delta_phi > TMath::Pi() )  delta_phi = 2*TMath::Pi() - delta_phi;
	  //	}
	
	double rad_cone = TMath::Sqrt(  (delta_phi*delta_phi + deta*deta) );
	if (  rad_cone <=1.0)
          {
            iso_giampi += pttrack;
	    
            if(atrack->ghostProbability()<0.3 && atrack->chi2PerDoF()< 3)
	      {
		iso_giampi_tc += pttrack;
	      }
          }
	
	
      } // atrack
    } //proto
  } // ipp

  iso_giampi = pt_bs/(iso_giampi+pt_bs);  // stored by TupleToolMuonVariables as "_yetanother_CDF_iso"
  m_CDFIso = iso_giampi;

  //Version with track cuts
  iso_giampi_tc = pt_bs/(iso_giampi_tc+pt_bs);  // stored by TupleToolMuonVariables as "_yetanother_CDF_iso_tc"
  if(m_makeTrackCuts) m_CDFIso = iso_giampi_tc;

  debug()<<"isotwobody was "<<m_IsoTwoBody<<" and maketrackcuts was "<<m_makeTrackCuts<<" so I computed "<<iso_giampi<<" and "<<iso_giampi_tc<<" and set "<<m_CDFIso<<endmsg;

  return test;

}


//=============================================================================
// Other functions needed by isolation
//=============================================================================

double RelInfoBs2MuMuBIsolations::arcosine(Gaudi::XYZVector p1,Gaudi::XYZVector p2) {

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

