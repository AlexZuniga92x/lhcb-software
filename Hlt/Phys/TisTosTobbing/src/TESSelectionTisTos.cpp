// $Id: TESSelectionTisTos.cpp,v 1.5 2010-07-21 21:22:16 tskwarni Exp $
// Include files 
#include <algorithm>
#include <vector>
#include <sstream>

// from Gaudi    
#include "GaudiKernel/StatusCode.h"
#include "GaudiKernel/StringKey.h"
#include "GaudiKernel/ToolFactory.h"
#include "GaudiKernel/IIncidentSvc.h"

// local
#include "TESSelectionTisTos.h"
#include "ParticleTisTos.h"

using namespace LHCb;

//-----------------------------------------------------------------------------
// Implementation file for class : TESSelectionTisTos
//
// 2009-12-03 : Tomasz Skwarnicki
//-----------------------------------------------------------------------------

// Declaration of the Tool Factory
DECLARE_TOOL_FACTORY( TESSelectionTisTos );


//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
TESSelectionTisTos::TESSelectionTisTos( const std::string& type,
                                                const std::string& name,
                                                const IInterface* parent )
  : ParticleTisTos ( type, name , parent )
  , m_objectSummaries(0)
{
  declareInterface<ITriggerSelectionTisTos>(this);
  
   declareProperty("PrefixInTES",
                   m_PrefixInTES = "/Event/" ); 
   
  m_cached_SelectionNames.reserve(500);
  m_cached_tisTosTob.reserve(500);
}


//=============================================================================
// Destructor
//=============================================================================
TESSelectionTisTos::~TESSelectionTisTos() {} 

//=============================================================================
// Initialization
//=============================================================================
StatusCode TESSelectionTisTos::initialize() {
  StatusCode sc = ParticleTisTos::initialize();
  if ( sc.isFailure() ) return sc;  // error printed already by GaudiAlgorithm

  debug() << "==> Initialize" << endmsg;

  IIncidentSvc*                incidentSvc(0);     
  if (!service( "IncidentSvc", incidentSvc).isSuccess()) return StatusCode::FAILURE;
  // add listener to be triggered by first BeginEvent
  bool rethrow = false; bool oneShot = false; long priority = 0;
  incidentSvc->addListener(this,IncidentType::BeginEvent,priority,rethrow,oneShot);
  incidentSvc->release();

  m_newEvent =true;
   
  setOfflineInput();
 
  return StatusCode::SUCCESS;

}

void TESSelectionTisTos::handle(const Incident& ) 
{

  setOfflineInput( );

  m_newEvent=true;  

  m_objectSummaries =0;
  
}

// ------------------------------------------------------------------------------------
void TESSelectionTisTos::getHltSummary()
{
}

 
//=============================================================================
// -------------- offline inputs -----------------------------------------------
//=============================================================================

// erase previous input ---------------------------------------------------------
void TESSelectionTisTos::setOfflineInput( )
{
  setSignal();  clearCache();
}
   

//    hit list input ---------------------------------------------------------------
void TESSelectionTisTos::addToOfflineInput( const std::vector<LHCb::LHCbID> & hitlist )
{
  if( addToSignal( hitlist ) )clearCache();
}

//    Track input ---------------------------------------------------------------
void TESSelectionTisTos::addToOfflineInput( const LHCb::Track & track )
{
  if( addToSignal( track ) )clearCache();
}

//    Proto-particle input -----------------------------------------------------------------------
void TESSelectionTisTos::addToOfflineInput( const LHCb::ProtoParticle & protoParticle )
{
  if( addToSignal( protoParticle ) )clearCache();
}

//    Particle input -----------------------------------------------------------------------
void TESSelectionTisTos::addToOfflineInput( const LHCb::Particle & particle )
{
  if( addToSignal( particle ) )clearCache();
}


 
//=============================================================================


//=============================================================================
// -------------- outputs:
//=============================================================================


// single complete Trigger Selection TisTos  (define Offline Input before calling)
unsigned int TESSelectionTisTos::tisTosSelection( const std::string & selectionName )
{
  unsigned int result=0;
  if( findInCache( selectionName, result ) )return result;

  bool decision(false);  

  std::string path;  
  if( selectionName.find('/') == 0 ){
    path = selectionName;
  } else {
    path = m_PrefixInTES.value() + selectionName;
  }
  
  if( exist<Particles>(path) || exist<Particles>(path+"/Particles") ){
    
    Particles* cParticles;
    if( exist<Particles>(path) ){      
      cParticles =  get<Particles>(path);  
    } else {
      cParticles = get<Particles>(path+"/Particles");
    }    
    std::vector<Particle*> particles;    
    for( Particles::iterator icand =  cParticles->begin(); icand !=  cParticles->end(); icand++ ) {
      particles.push_back(*icand);
    }
    decision = (0!=particles.size());    
    if( decision > 0 ){
       result = IParticleTisTos::tisTos<Particle>(particles);
    }

  } else if( exist<Vertices>(path) || exist<Vertices>(path+"/Vertices")  ){

    Vertices* cVertices;    
    if( exist<Vertices>(path) ){      
      cVertices =  get<Vertices>(path);  
    } else {
      cVertices = get<Vertices>(path+"/Vertices");
    }    
    std::vector<Vertex*> vtxs;    
    for( Vertices::iterator icand =  cVertices->begin(); icand !=  cVertices->end(); icand++ ) {
      vtxs.push_back(*icand);
    } 
    decision = (0!=vtxs.size());    
    if( decision > 0 ){
      result = IParticleTisTos::tisTos<Vertex>(vtxs);
   }
    
  } else if( exist<RecVertex::Container>(path) ){ 
    
    RecVertex::Container* cVertices =  get<RecVertex::Container>(path);    
    std::vector<RecVertex*> vertices;
    for( RecVertex::Container::iterator icand =  cVertices->begin(); icand !=  cVertices->end(); icand++ ) {
      vertices.push_back(*icand);
    }
    decision = (0!=vertices.size());    
    if( decision ){
      result = IParticleTisTos::tisTos<RecVertex>(vertices);
    }
    
  } else if( exist<Tracks>(path) ){
    
    Tracks* cTracks =  get<Tracks>(path);    
    std::vector<Track*> tracks;
    for( Tracks::iterator icand =  cTracks->begin(); icand !=  cTracks->end(); icand++ ) {
      tracks.push_back(*icand);
    }
    decision = (0!=tracks.size());    
    if( decision ){
      result = IParticleTisTos::tisTos<Track>(tracks);
    }
    
  }

  if( decision )result |= kDecision; 
  storeInCache(selectionName,result);
  return result;  
}

// single complete Trigger Selection TisTos  (define Offline Input before calling)
std::string TESSelectionTisTos::analysisReportSelection( const std::string & selectionName )
{

  unsigned int result=0;

  bool decision(false);  

  std::string path;  
  if( selectionName.find('/') == 0 ){
    path = selectionName;
  } else {
    path = m_PrefixInTES.value() + selectionName;
  }
  
  std::ostringstream report;
  report << offset() << " Selection path "+path;  

  if( exist<Particles>(path) || exist<Particles>(path+"/Particles") ){
    
    Particles* cParticles;
    if( exist<Particles>(path) ){      
      cParticles =  get<Particles>(path);  
    } else {
      cParticles = get<Particles>(path+"/Particles");
    }    
    std::vector<Particle*> particles;    
    for( Particles::iterator icand =  cParticles->begin(); icand !=  cParticles->end(); icand++ ) {
      particles.push_back(*icand);
    }
    report << " Particles size= " << particles.size() << std::endl;
    decision = (0!=particles.size());    
    if( decision > 0 ){
       result = IParticleTisTos::tisTos<Particle>(particles);
       report<< analysisReport<Particle>(particles);
    }

  } else if( exist<Vertices>(path) || exist<Vertices>(path+"/Vertices")  ){

    Vertices* cVertices;    
    if( exist<Vertices>(path) ){      
      cVertices =  get<Vertices>(path);  
    } else {
      cVertices = get<Vertices>(path+"/Vertices");
    }    
    std::vector<Vertex*> vtxs;    
    for( Vertices::iterator icand =  cVertices->begin(); icand !=  cVertices->end(); icand++ ) {
      vtxs.push_back(*icand);
    } 
    report << " Vertcies size= " << vtxs.size() << std::endl;
    decision = (0!=vtxs.size());    
    if( decision > 0 ){
      result = IParticleTisTos::tisTos<Vertex>(vtxs);
      report<< analysisReport<Vertex>(vtxs);
   }
    
  } else if( exist<RecVertex::Container>(path) ){ 
    
    RecVertex::Container* cVertices =  get<RecVertex::Container>(path);    
    std::vector<RecVertex*> vertices;
    for( RecVertex::Container::iterator icand =  cVertices->begin(); icand !=  cVertices->end(); icand++ ) {
      vertices.push_back(*icand);
    }
    report << " RecVerticies size= " << vertices.size() << std::endl;
    decision = (0!=vertices.size());    
    if( decision ){
      result = IParticleTisTos::tisTos<RecVertex>(vertices);
      report<< analysisReport<RecVertex>(vertices);
   }
    
  } else if( exist<Tracks>(path) ){
    
    Tracks* cTracks =  get<Tracks>(path);    
    std::vector<Track*> tracks;
    for( Tracks::iterator icand =  cTracks->begin(); icand !=  cTracks->end(); icand++ ) {
      tracks.push_back(*icand);
    }
    report << " Tracks size= " << tracks.size() << std::endl;
    decision = (0!=tracks.size());    
    if( decision ){
      result = IParticleTisTos::tisTos<Track>(tracks);
      report<< analysisReport<Track>(tracks);
   }
    
  }

  if( decision )result |= kDecision; 
  TisTosTob res( result );
  report << offset() << " Selection " + selectionName + " "  
         << " TIS= " << res.tis() << " TOS= " << res.tos() << " TPS= " << res.tps() 
         << " decision= " << res.decision() << std::endl;
  return report.str();
}


// fast check for TOS
#define TOSTISTPS( FUN )\
{\
  std::string path;  \
  bool decision(false);\
  if( selectionName.find('/') == 0 ){\
    path = selectionName;\
  } else {\
    path = m_PrefixInTES.value() + selectionName;\
  }\
  \
  if( exist<Particles>(path) || exist<Particles>(path+"/Particles") ){\
    \
    Particles* cParticles;\
    if( exist<Particles>(path) ){      \
      cParticles =  get<Particles>(path);  \
    } else {\
      cParticles = get<Particles>(path+"/Particles");\
    }    \
    std::vector<Particle*> particles;    \
    for( Particles::iterator icand =  cParticles->begin(); icand !=  cParticles->end(); icand++ ) {\
      particles.push_back(*icand);\
    }\
    decision = (0!=particles.size());    \
    if( decision > 0 ){\
       return IParticleTisTos::FUN<Particle>(particles);\
    }    \
\
  } else if( exist<Vertices>(path) || exist<Vertices>(path+"/Vertices")  ){\
\
    Vertices* cVertices;    \
    if( exist<Vertices>(path) ){      \
      cVertices =  get<Vertices>(path);  \
    } else {\
      cVertices = get<Vertices>(path+"/Vertices");\
    }    \
    std::vector<Vertex*> vtxs;    \
    for( Vertices::iterator icand =  cVertices->begin(); icand !=  cVertices->end(); icand++ ) {\
      vtxs.push_back(*icand);\
    } \
    decision = (0!=vtxs.size());    \
    if( decision > 0 ){\
      return IParticleTisTos::FUN<Vertex>(vtxs);\
   }\
    \
  } else if( exist<RecVertex::Container>(path) ){ \
    \
    RecVertex::Container* cVertices =  get<RecVertex::Container>(path);    \
    std::vector<RecVertex*> vertices;\
    for( RecVertex::Container::iterator icand =  cVertices->begin(); icand !=  cVertices->end(); icand++ ) {\
      vertices.push_back(*icand);\
    }\
    decision = (0!=vertices.size());    \
    if( decision ){\
      return IParticleTisTos::FUN<RecVertex>(vertices);\
    }\
    \
  } else if( exist<Tracks>(path) ){\
    \
    Tracks* cTracks =  get<Tracks>(path);    \
    std::vector<Track*> tracks;\
    for( Tracks::iterator icand =  cTracks->begin(); icand !=  cTracks->end(); icand++ ) {\
      tracks.push_back(*icand);\
    }\
    decision = (0!=tracks.size());    \
    if( decision ){\
      return IParticleTisTos::FUN<Track>(tracks);\
    }\
    \
  }\
  return false;  \
}\


bool TESSelectionTisTos::tosSelection( const std::string & selectionName )
TOSTISTPS( tos )

bool TESSelectionTisTos::tisSelection( const std::string & selectionName )
TOSTISTPS( tis )

bool TESSelectionTisTos::tusSelection( const std::string & selectionName )
TOSTISTPS( tus )

// ------------ auxiliary output:  list of LHCbIDs corresponding to present offline input
std::vector<LHCb::LHCbID> TESSelectionTisTos::offlineLHCbIDs() 
{
  return signal();
}

#define TISTOSSELECTSTORE() \
      unsigned int result = tisTos(*object);\
      bool tis,tos,tps;\
      tis = result & kTIS;\
      tos = result & kTOS;\
      tps = result & kTPS;\
      if(    ((tisRequirement>=kAnything)||(tis==tisRequirement)) \
             && ((tosRequirement>=kAnything)||(tos==tosRequirement)) \
             && ((tpsRequirement>=kAnything)||(tps==tpsRequirement)) \
             ){\
        HltObjectSummary* hos(0);\
        for( HltObjectSummarys::iterator ppHos=m_objectSummaries->begin();\
             ppHos!=m_objectSummaries->end();++ppHos){\
          HltObjectSummary* pHos(*ppHos);    \
          if( pHos->summarizedObjectCLID() == object->clID() ){\
            if( pHos->summarizedObject() == object ) hos = pHos;\
          }\
        }\
        if( !hos ){          \
          hos = new HltObjectSummary();\
          m_objectSummaries->push_back(hos);\
          hos->setSummarizedObjectCLID( object->clID() );\
          hos->setSummarizedObject(object);\
        }        \
        matchedObjectSummaries.push_back( hos );\
      }\

// ------------  additional functionality:  lists of object summaries for tracks/vertices/particles from trigger selections
//               satisfying TIS, TOS requirements

std::vector<const LHCb::HltObjectSummary*> TESSelectionTisTos::hltSelectionObjectSummaries(
                      const std::string & selectionName,
                      unsigned int tisRequirement,
                      unsigned int tosRequirement,
                      unsigned int tpsRequirement)
{
  std::vector<const LHCb::HltObjectSummary*> matchedObjectSummaries;

  if( !m_objectSummaries ){
    if( exist<HltObjectSummary::Container>("/Event/Hlt/TESSelectionTisTosStore") ){
      m_objectSummaries =  get<HltObjectSummary::Container>("/Event/Hlt/TESSelectionTisTosStore");
    } else  {      
      m_objectSummaries = new HltObjectSummary::Container();
      put( m_objectSummaries,"/Event/Hlt/TESSelectionTisTosStore");
    }      
  }  

  std::string path;  
  if( selectionName.find('/') == 0 ){
    path = selectionName;
  } else {
    path = m_PrefixInTES.value() + selectionName;
  }
  
  if( exist<Tracks>(path) ){
    
    Tracks* sel =  get<Tracks>(path);    
    if( !sel )return matchedObjectSummaries;
    if( !(sel->size()) )return matchedObjectSummaries;
    for( Tracks::iterator icand =  sel->begin(); icand !=  sel->end(); icand++ ) {
      Track* object= *icand; 
      TISTOSSELECTSTORE()
    }

  } else if( exist<RecVertex::Container>(path) ){

    RecVertex::Container* sel =  get<RecVertex::Container>(path);    
    if( !sel )return matchedObjectSummaries;
    if( !(sel->size()) )return matchedObjectSummaries;
    for( RecVertex::Container::iterator icand =  sel->begin(); icand !=  sel->end(); icand++ ) {
      RecVertex* object= *icand; 
      TISTOSSELECTSTORE()
    }

  } else if( exist<Particles>(path) ||  exist<Particles>(path+"/Particles") ){
    
    Particles* sel;
    if( exist<Particles>(path) ){      
      sel =  get<Particles>(path);
    } else {
      sel =  get<Particles>(path+"/Particles" );
    }    
    if( !sel )return matchedObjectSummaries;
    if( !(sel->size()) )return matchedObjectSummaries;
    for( Particles::iterator icand =  sel->begin(); icand !=  sel->end(); icand++ ) {
      Particle* object= *icand; 
      TISTOSSELECTSTORE()
    }
    
  } else if( exist<Vertices>(path) ||  exist<Vertices>(path+"/Vertices") ){
    
    Vertices* sel;
    if( exist<Vertices>(path) ){      
      sel =  get<Vertices>(path);
    } else {
      sel =  get<Vertices>(path+"/Vertices" );
    }    
    if( !sel )return matchedObjectSummaries;
    if( !(sel->size()) )return matchedObjectSummaries;
    for( Vertices::iterator icand =  sel->begin(); icand !=  sel->end(); icand++ ) {
      Vertex* object= *icand; 
      TISTOSSELECTSTORE()
    }
    
  }
  
  return matchedObjectSummaries;
}
