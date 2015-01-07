// $Id: HltSelReportsMaker.cpp,v 1.27 2010-08-17 08:45:05 graven Exp $
// #define DEBUGCODE
// Include files 
#include <memory>

#include "boost/algorithm/string/replace.hpp"
#include "boost/algorithm/string/predicate.hpp"
#include "boost/format.hpp"

// from Gaudi
#include "GaudiKernel/StatusCode.h"
#include "GaudiKernel/StringKey.h"
#include "GaudiKernel/AlgFactory.h" 
#include "GaudiKernel/IIncidentSvc.h"
#include "GaudiKernel/IRegistry.h"

#include "Kernel/Particle2Vertex.h"

#include "Event/HltSelReports.h"
#include "Event/HltObjectSummary.h"
#include "Event/HltDecReports.h"

#include "Event/RecVertex.h"
#include "Event/Particle.h"
#include "Event/Vertex.h"
#include "Event/RichPID.h"
#include "Event/MuonPID.h"
#include "Event/HltStage.h"
#include "Event/HltCandidate.h"


//#include "Kernel/CaloCellCode.h"

#include "HltBase/HltSelection.h"
// local
#include "HltSelReportsMaker.h"

static const  Gaudi::StringKey Hlt1SelectionID{"Hlt1SelectionID"};
static const  Gaudi::StringKey Hlt2SelectionID{"Hlt2SelectionID"};
static const  Gaudi::StringKey InfoID{"InfoID"};

using namespace LHCb;

namespace {
    template <typename E> const ContainedObject* getFirst(const Hlt::Selection* sel ) {
        if ( sel->classID() != E::classID() ) return nullptr;
        auto& s = dynamic_cast<const Hlt::TSelection<E>&>(*sel);
        return (!s.empty()) ? s.front() : nullptr;
    }
}

//-----------------------------------------------------------------------------
// Implementation file for class : HltSelReportsMaker
//
// 2008-07-23 : Tomasz Skwarnicki
//-----------------------------------------------------------------------------

// Declaration of the Algorithm Factory
DECLARE_ALGORITHM_FACTORY( HltSelReportsMaker )


//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
HltSelReportsMaker::HltSelReportsMaker( const std::string& name,
                                                      ISvcLocator* pSvcLocator)
  : GaudiAlgorithm ( name , pSvcLocator )
  , m_hltANNSvc {nullptr}
  , m_hltSvc{nullptr}
  , m_regSvc{nullptr}
  , m_inspectionSvc{nullptr}
  , m_intPVSelID(0)
  , m_Turbo(false)
  , m_presentInfoLevelTrack(kMaxInfoLevel)
  , m_presentInfoLevelRecVertex(kMaxInfoLevel)
  , m_presentInfoLevelParticle(kMaxInfoLevel)
  , m_presentInfoLevelCaloCluster(kMaxInfoLevel)
  , m_presentInfoLevelSelection(kMaxInfoLevel)
{

  declareProperty("OutputHltSelReportsLocation",
    m_outputHltSelReportsLocation= LHCb::HltSelReportsLocation::Default);  
  declareProperty("MuonIDSuffix", m_muonIDSuffix = "");
  declareProperty("InputHltDecReportsLocation",
    m_inputHltDecReportsLocation= LHCb::HltDecReportsLocation::Default);  

  declareProperty("DebugEventPeriod",m_debugPeriod = 0 );

  declareProperty("MaxCandidatesDecision", m_maxCandidatesDecision = 1000);
  declareProperty("MaxCandidatesDecisionDebug", m_maxCandidatesDecisionDebug = 5000);
  
  declareProperty("MaxCandidatesNonDecision", m_maxCandidatesNonDecision = 0);
  declareProperty("MaxCandidatesNonDecisionDebug", m_maxCandidatesNonDecisionDebug = 5000);
  
  declareProperty("SelectionMaxCandidates", m_maxCandidates );
  declareProperty("SelectionMaxCandidatesDebug", m_maxCandidatesDebug );

  // changed on 2010/7/17  declareProperty("InfoLevelDecision", m_infoLevelDecision = ((unsigned int)kStandardInfoLevel) );
  declareProperty("InfoLevelDecision", m_infoLevelDecision = ((unsigned int)kMinInfoLevel ));
  declareProperty("InfoLevelDecisionDebug", m_infoLevelDecisionDebug = ((unsigned int)kMaxInfoLevel) );
  
  declareProperty("InfoLevelNonDecision", m_infoLevelNonDecision = ((unsigned int)kMinInfoLevel) );
  declareProperty("InfoLevelNonDecisionDebug", m_infoLevelNonDecisionDebug = ((unsigned int)kMaxInfoLevel) );
  
  declareProperty("SelectionInfoLevel", m_infoLevel );
  declareProperty("SelectionInfoLevelDebug", m_infoLevelDebug );

  declareProperty("InfoLevelTrack", m_infoLevelTrack = ((unsigned int)kMaxInfoLevel) );
  declareProperty("InfoLevelTrackDebug", m_infoLevelTrackDebug = ((unsigned int)kMaxInfoLevel) );

  declareProperty("InfoLevelRecVertex", m_infoLevelRecVertex = ((unsigned int)kMaxInfoLevel) );
  declareProperty("InfoLevelRecVertexDebug", m_infoLevelRecVertexDebug = ((unsigned int)kMaxInfoLevel) );

  declareProperty("InfoLevelParticle", m_infoLevelParticle = ((unsigned int)kMaxInfoLevel) );
  declareProperty("InfoLevelParticleDebug", m_infoLevelParticleDebug = ((unsigned int)kMaxInfoLevel) );

  declareProperty("InfoLevelCaloCluster", m_infoLevelCaloCluster = ((unsigned int)kMaxInfoLevel) );
  declareProperty("InfoLevelCaloClusterDebug", m_infoLevelCaloClusterDebug = ((unsigned int)kMaxInfoLevel) );

  declareProperty("InfoLevelSelection", m_infoLevelSelection = ((unsigned int)kMaxInfoLevel) );
  declareProperty("InfoLevelSelectionDebug", m_infoLevelSelectionDebug = ((unsigned int)kMaxInfoLevel) );

  declareProperty("SuppressPostscale", m_SuppressPostscale = true  );
  declareProperty("SuppressPostscaleDebug", m_SuppressPostscaleDebug = false  );

  declareProperty("DebugIncident", m_debugIncidentName = "RequestDebugEvent" );


  m_debugMode=false;
  m_event=0;

}

// add incident listener, and set m_event 
void HltSelReportsMaker::handle ( const Incident& /*incident*/ ) {
    m_event = 0u-1; // yes, it is unsigned, but all we need is
                    // ++m_event to be zero to force a debug event;-)
}


//=============================================================================
// Destructor
//=============================================================================
HltSelReportsMaker::~HltSelReportsMaker() {} 

//=============================================================================
// Initialization
//=============================================================================
StatusCode HltSelReportsMaker::initialize() {
  StatusCode sc = GaudiAlgorithm::initialize(); // must be executed first
  if ( sc.isFailure() ) return sc;  // error printed already by GaudiAlgorithm

  if ( msgLevel(MSG::DEBUG) ) debug() << "==> Initialize" << endmsg;

  m_hltANNSvc = svc<IANNSvc>("HltANNSvc");
  m_hltSvc = svc<Hlt::IData> ( "Hlt::Service" , true ) ;
  m_regSvc = svc<Hlt::IRegister> ( "Hlt::Service" , true ) ;
  m_inspectionSvc = svc<Hlt::IInspector> ( "Hlt::Service" , true ) ;

  if (!m_debugIncidentName.empty() ) {
    svc<IIncidentSvc>("IncidentSvc")->addListener(this,m_debugIncidentName);
  }


 // get string-to-int selection ID map
  std::map<IANNSvc::minor_key_type,IANNSvc::minor_mapped_type> selectionNameToIntMap;
  for( const IANNSvc::minor_value_type& p: m_hltANNSvc->items(Hlt1SelectionID) ) {
      selectionNameToIntMap.insert( p );
  }
  for( const IANNSvc::minor_value_type& p: m_hltANNSvc->items(Hlt2SelectionID) ) {
      selectionNameToIntMap.insert( p );
  }
 
  // initialize maps
  m_selectionInfo.clear();

  // get trigger selection names 
  Hlt::IInspector::KeyList selectionIDs;
  m_inspectionSvc->selections( selectionIDs );
  Hlt::IRegister::Lock lock(m_regSvc,this);
  for ( const auto& s : selectionIDs ) {
     struct selectionInfo info; info.id = s;
     if (lock->registerInput(info.id,this).isFailure()) {
       Warning( " selectionName="+s+" could not register as input. Skipped. " , StatusCode::SUCCESS, 10 );
       continue;
     }
     auto im = selectionNameToIntMap.find( s );
     if( im==selectionNameToIntMap.end() ){
       Warning( " selectionName="+s+" not found in HltANNSvc. Skipped. " , StatusCode::SUCCESS, 10 );
       continue;
     }
     info.intId =  im->second ;
     info.selection = m_hltSvc->selection(info.id,this);
     if (info.selection==0) {
       Warning( " selectionName="+s+" not found in HltDataSvc. Skipped. " , StatusCode::SUCCESS, 10 );
       continue;
     }
     m_debugMode = true;  info.maxCandDebug = maximumNumberOfCandidatesToStore( s );
     m_debugMode = false; info.maxCand      = maximumNumberOfCandidatesToStore( s );
     m_selectionInfo.push_back(info);

  }

  m_infoIntToName.clear();
  for( const  auto&  i: m_hltANNSvc->items(InfoID)  ) {
    if( ( 0<=i.second ) && ( i.second<=65535 ) ){
      m_infoIntToName.insert( i.second, i.first );
    }
  }
 
  m_lastPVSelectionName.clear();
  m_intPVSelID=0;

  return StatusCode::SUCCESS;
}



//=============================================================================
// Main execution
//=============================================================================
StatusCode HltSelReportsMaker::execute() {

  if ( msgLevel(MSG::DEBUG) ) debug() << "==> Execute" << endmsg;

  // determine output mode 
  ++m_event;
  m_debugMode = ( m_event == m_debugPeriod );
  if( m_debugMode ) m_event =0;    

  m_HLTmuonTracks=nullptr;  // will get them only if needed

  // create output container and put it on TES
  HltSelReports* outputSummary = new HltSelReports();
  put( outputSummary, m_outputHltSelReportsLocation );

  // create output container for Object Summaries and put it on TES
  m_objectSummaries = new HltObjectSummary::Container();
  put( m_objectSummaries, m_outputHltSelReportsLocation.value() + "/Candidates" );
  
  // get input
  const HltDecReports* decReports = getIfExists<HltDecReports>(m_inputHltDecReportsLocation);
  if( !decReports ) {  
    Warning( " No HltDecReports at " + m_inputHltDecReportsLocation.value(), StatusCode::SUCCESS, 10 );
  }


#ifdef DEBUGCODE
  if ( msgLevel(MSG::VERBOSE) ){
    verbose() <<" Selection Names Found =" ;
    for( const auto&  i : selectionIDs) verbose() << i.str() << " ";
    verbose() << endmsg;
  }
#endif  
 

  // data compression requires that we store objects from early processing stages first
  // order selections accordingly 
  //     filtering of selections for persistency also happens in this loop

  std::vector< RankedSelection > sortedSelections;
  sortedSelections.reserve(m_selectionInfo.size());
 
  // loop over selection summaries
  for( const auto&  i : m_selectionInfo ) {
     const std::string& selName = i.id.str(); 

     // see if marked for persistency
     int maxCandidates = m_debugMode ? i.maxCandDebug :  i.maxCand;
     if( !maxCandidates )continue;

     // find first candidate for ranking
     const Hlt::Selection* sel = i.selection;

#ifdef DEBUGCODE
     if ( msgLevel(MSG::VERBOSE) ){
       verbose() << " Selection " << selname <<  " found in dataSvc decison=" << sel->decision() << endmsg;          
     }
#endif
     // unsuccessful selections can't save candidates
     if( !sel->decision() )continue;

     // first see if we have have
     // HltSelReportsMaker                      WARNING HltSelReportsMaker::  Unsupported data type CLID=7561 - skip selection ID=Hlt1TrackAllL0Decision
     // EventSelector                           SUCCESS Reading Event record 401. Record number within stream 1: 401
     const ContainedObject* candidate{ nullptr };
     if ( sel->classID() == Hlt::Candidate::classID() ) {
        const auto& csel = dynamic_cast<const Hlt::TSelection<Hlt::Candidate>&>(*sel);
        const Hlt::Candidate* cand = csel.front();
        if (cand) {
            const Hlt::Stage* stage = cand->currentStage();
            // let's first try track...
            candidate = stage->get<LHCb::Track>();
            if (!candidate)  candidate = stage->get<LHCb::RecVertex>();
            if (!candidate)  candidate = stage->get<LHCb::Particle>();
            if (!candidate)  candidate = stage->get<LHCb::CaloCluster>();
            if (!candidate)  Warning("Stage is neither track nor vertex", StatusCode::SUCCESS,10);
        }
     } 
     if (!candidate) candidate = getFirst<LHCb::Track>(sel);
     if (!candidate) candidate = getFirst<LHCb::RecVertex>(sel);
     if (!candidate) candidate = getFirst<LHCb::Particle>(sel);
     if (!candidate) candidate = getFirst<LHCb::CaloCluster>(sel);
     if (!candidate) {
       
       Warning( " Unsupported data type CLID=" + std::to_string( sel->classID()) + " - skip selection ID=" +selName,
                StatusCode::SUCCESS, 10 );
       continue;
       
     }
       
#ifdef DEBUGCODE
     if ( msgLevel(MSG::VERBOSE) ){
          verbose() << "Selection " << selName <<  " has candidates " << endmsg;
     } 
#endif

     // save selection ---------------------------
     setPresentInfoLevel( selName );

     // classify according to first candidate
     int rnk(0);
     // go through all cases of supported candidates
     if (  !rank<Track>       (candidate, rnk)
        && !rank<RecVertex>   (candidate, rnk)
        && !rank<Particle>    (candidate, rnk)
        && !rank<CaloCluster> (candidate, rnk) )
         Warning( " Unsupported data type among candidates, CLID= " 
             + std::to_string(sel->classID()) + " - skip selection ID="     
                          +selName,StatusCode::SUCCESS, 2 );

     sortedSelections.emplace_back( rnk, i );
  }
  
  if( sortedSelections.empty() )return StatusCode::SUCCESS;

  std::sort( std::begin(sortedSelections), std::end(sortedSelections), rankSelLess() );

#ifdef DEBUGCODE
  if (msgLevel(MSG::VERBOSE)) {
    unsigned e=0;
    for( auto i : sortedSelections ) {
      verbose() << e++ << " Selection " << i.second.id << " rank " << i.first << endmsg;
    }
  }
#endif


  // loop over ranked selection summaries in HltSummary
  for( const auto&  s : sortedSelections ) {
    
     const std::string& selName(s.second.id.str());     
     // don't bother if duplicate selection 
     if( outputSummary->hasSelectionName(selName) ){
         Warning( " duplicate selection ignored selectionName=" + selName,StatusCode::SUCCESS, 10 );
         continue;        
     }
     // unsuccessful selections can't save candidates
     const Hlt::Selection* sel = s.second.selection;
     if( !sel->decision() )continue;

     //FIXME: need to invert the next if -- i.e. the m_Suppress... test should be done in 
     //       'AND' with the decision check in order to continue... as we always need the
     //       turbo signature (I think)

     bool turbo_signature { false };
     if( ( !m_debugMode && m_SuppressPostscale ) ||  ( m_debugMode && m_SuppressPostscaleDebug ) ){
     // must also check its decision in HltDecReports since it might have been killed by postscale
       if( decReports ){
         const HltDecReport* decReport = decReports->decReport(selName);
         if( decReport ){
           turbo_signature = ( decReport->executionStage() & 0x80 ); 
           if( !(decReport->decision()) )continue;
         }
       }
     }
     // Check if the Hlt line has been marked for Turbo level output
     m_Turbo = turbo_signature;


     std::vector<const ContainedObject*> candidates; candidates.reserve( sel->size() );
     
     if( sel->classID() == Hlt::Candidate::classID() ) {
       auto& csel = dynamic_cast<const Hlt::TSelection<Hlt::Candidate>&>(*sel);
       for (const auto& cand : csel ) {
         const Hlt::Stage* stage = cand->currentStage();
         const ContainedObject *obj = stage->get<LHCb::Track>();
         if (!obj) obj = stage->get<LHCb::RecVertex>();
         if (!obj) obj = stage->get<LHCb::Particle>();
         if (!obj) obj = stage->get<LHCb::CaloCluster>();
         if (obj) {
            candidates.push_back( obj );
         }  else {
            Warning( " Unsupported data type in stage - skip selection ID=" +selName,StatusCode::SUCCESS, 10 );
         }
       }
     } else if( sel->classID() == LHCb::Track::classID() ) {
       
       const auto& tsel = dynamic_cast<const Hlt::TrackSelection&>(*sel);   
       std::copy( std::begin(tsel), std::end(tsel), std::back_inserter( candidates ) );

     } else if( sel->classID() == LHCb::RecVertex::classID() ) {

       const auto& tsel = dynamic_cast<const Hlt::VertexSelection&>(*sel);   
       std::copy( std::begin(tsel), std::end(tsel), std::back_inserter(candidates) );

     } else if( sel->classID() == LHCb::Particle::classID() ) {

       const auto& psel = dynamic_cast<const Hlt::TSelection<LHCb::Particle>&>(*sel);   
       std::copy( std::begin(psel), std::end(psel), std::back_inserter(candidates) );

     } else {
       
       Warning( " Unsupported data type  - skip selection ID=" 
                +selName,StatusCode::SUCCESS, 10 );
       continue;
       
     }
       

     // trim number of candidates if too large
     int maxCandidates = m_debugMode ? s.second.maxCandDebug :  s.second.maxCand;
     int noc = std::min(int(candidates.size()),maxCandidates);

     // save selection ---------------------------

     HltObjectSummary selSumOut;    
     selSumOut.setSummarizedObjectCLID( 1 ); // use special CLID for selection summaries (lowest number for sorting to the end)
     
     // integer selection id 
     selSumOut.addToInfo("0#SelectionID",float(s.second.intId));
    
     setPresentInfoLevel( selName );
  
     // must also save candidates 
     int nocc(0);

     // save PV association info with selection if found
     std::string pvSelectionName;
     std::vector<unsigned int> packedPVKeys;
     unsigned int nPackedPVKeys(0);
     unsigned int iWord(0);
     Particle2Vertex::Table* table{nullptr};

     for (const auto&  c : candidates ) {
       if( ++nocc>noc ) break;

       const HltObjectSummary* hos{ nullptr };
       // go through all cases of supported candidates
       hos = store<LHCb::Track>(c);
       if (!hos) hos = store<LHCb::RecVertex>(c);
       if (!hos) {
           hos = store<LHCb::Particle>(c);
           if (hos) {//TODO: move into store_<Particle> ??? Oops, this shares info amongst candidates...
                     //      that bit needs to be moved out of this loop, and 'bound' into the relevant store_...
             if( kStandardInfoLevel &  m_presentInfoLevelSelection & m_presentInfoLevel ){
               if( pvSelectionName != "_" ){
                 bool keyFound = false;             
                 const DataObject* container = c->parent();
                 IRegistry* registry = ( container ? container->registry() : nullptr ) ;
                 if( registry ){
                   if( !table ) {
                       std::string path = registry->identifier() ;
                       boost::replace_last(path,"/Particles","/Particle2VertexRelations");
                       table = getIfExists< Particle2Vertex::Table >(path);
                   }
                   if( table ){                     
                     auto rels= table->relations( dynamic_cast<const LHCb::Particle*>(c) );
                     if( !rels.empty() ){
                       const VertexBase* pv = (rels[0]).to();
                       if(  !(pvSelectionName.length()) ){
                         const DataObject* containerPV = pv->parent();
                         IRegistry* registryPV = containerPV ? containerPV->registry() : nullptr;
                         if( registryPV ){
                           std::string pathPV = registryPV ->identifier();
                           auto found = pathPV.find_last_of("/");
                           if( found != std::string::npos ){
                             pvSelectionName = pathPV.substr(found+1);
                           } else {
                             pvSelectionName = pathPV;
                           }
                         }
                       }
                       ++nPackedPVKeys;
                       if( 5 == nPackedPVKeys ){
                         nPackedPVKeys = 1;
                         packedPVKeys.push_back( iWord );
                         iWord = 0;
                       }
                       auto key = std::min( unsigned(pv->key()) , 254u );
                       iWord |= ( key << ((nPackedPVKeys-1)*8) ) ;
                       keyFound = true;
                     }
                   }
                 }
                 if( !keyFound && !pvSelectionName.empty() ){
                   Warning("Only some particles in the trigger selection " +
                           selName + " have PV association - none stored",
                           StatusCode::SUCCESS, 10 );
                   pvSelectionName = "_";
                 }
               }
             }
           } 
        }
        if (!hos) hos = store<LHCb::CaloCluster>(c);
        if (!hos) {
               Warning(" Unsupported data type in Hlt Trigger Summary - skip remaining candidates too, selection ID=" 
                       +selName,StatusCode::SUCCESS, 10 );
               break;       
        }
       selSumOut.addToSubstructure(hos);
     }

     // insert PV relations if any
     if( !pvSelectionName.empty()  ){
       packedPVKeys.push_back( iWord );
       int intPVSelID=0;       
       if( pvSelectionName == m_lastPVSelectionName ){
         intPVSelID = m_intPVSelID;
       } else {
         const auto& items = m_hltANNSvc->items(Hlt1SelectionID);
         auto si = std::find_if( std::begin(items), std::end(items),
                                 [&](const std::pair<std::string,int>& i) { return i.first == pvSelectionName; } );
         if( si != std::end(items) ) {
           intPVSelID=si->second;
           m_lastPVSelectionName = pvSelectionName;
           m_intPVSelID = intPVSelID;
         } else {
           const auto& hlt2 = m_hltANNSvc->items(Hlt2SelectionID);
           auto si = std::find_if( std::begin(hlt2), std::end(hlt2),
                                   [&](const std::pair<std::string,int>& i) { return i.first == pvSelectionName; } );
           if( si != std::end(hlt2) ) { 
             intPVSelID=si->second;
             m_lastPVSelectionName = pvSelectionName;
             m_intPVSelID = intPVSelID;
           }
         }
       }
       if( !intPVSelID ){       
         Warning("PV selection " + pvSelectionName + 
                 " found via Particle2Vertex for " + selName +
                 " has unregistered name - no associations stored ",
                 StatusCode::SUCCESS, 10 );                 
       } else {
         selSumOut.addToInfo("10#" + pvSelectionName ,float(intPVSelID));
         // store PV keys only if at least one was not zero
         if( std::any_of( std::begin(packedPVKeys), std::end(packedPVKeys), 
                          [](unsigned int w) -> bool { return w; } ) ){
           int j=0;
           for( const auto& Word : packedPVKeys ) {
             union IntFloat { unsigned int mInt; float mFloat; };
             IntFloat a; a.mInt = Word;
             selSumOut.addToInfo( std::string{  "11#" } + boost::str( boost::format("%1$=08X") % j++),
                                  a.mFloat );              
           }
         }         
       }
     }

     // insert selection into the container
     if( outputSummary->insert(selName,selSumOut) == StatusCode::FAILURE ){
       Warning(" Failed to add Hlt selection name "+selName
               +" to its container ",StatusCode::SUCCESS, 10 );

     }
     m_Turbo = false; 
  }
  
  // -------------------------------------------------------------------------------------
  // clone HltObjectSummary-s of selection and insert them into object store (needed for raw data storage
  // and global selections)
  // -------------------------------------------------------------------------------------
  std::transform( std::begin(*outputSummary), std::end(*outputSummary),
                  std::back_inserter(*m_objectSummaries),
                  [](HltSelReports::Container::const_reference i) { return new HltObjectSummary( i.second ); } );

  // -------------------------------------------------------------------------------------
  // create global selections ------------------------------------------------------------
  // -------------------------------------------------------------------------------------
  if( !outputSummary->hasSelectionName("Hlt1Global") ){
    
     HltObjectSummary selSumOut;    
     selSumOut.setSummarizedObjectCLID( 1 ); // use special CLID for selection summaries (lowest number for sorting to the end)
     
     // integer selection id 
     selSumOut.addToInfo("0#SelectionID",float(kHlt1GlobalID));

     // see which decisions contributed to it
     for( const auto& it : *outputSummary) {
       const std::string & selName = it.first;
       if( !boost::algorithm::starts_with( selName, "Hlt1") || 
           !boost::algorithm::ends_with(   selName, "Decision") ) continue;
       const LHCb::HltObjectSummary& selRep = it.second;
       // must find corresponding HltObjectSummary in the object store
       auto i = selRep.numericalInfo().find("0#SelectionID");
       if( i==selRep.numericalInfo().end() ){               
           Error(" (1) HltObjectSummary of selection-summary-type has no SelectionID info ",StatusCode::SUCCESS,20);
           continue;
       }
       int id = (int)(i->second+0.1);
       for( const auto& pObj : *m_objectSummaries) {
         if( pObj->summarizedObjectCLID()!=1 ) continue; 
         auto j=pObj->numericalInfo().find("0#SelectionID");
         if( j==pObj->numericalInfo().end() ){
           Error(" (2) HltObjectSummary of selection-summary-type has no SelectionID info ",StatusCode::SUCCESS,20);
           continue;
         }
         if( id == (int)(j->second+0.1) ){
           // must also check its decision in HltDecReports since it might have been killed by postscale
           if( decReports ){
             const HltDecReport* decReport = decReports->decReport(selName);
             if( decReport && !decReport->decision() ) break; 
           }
           selSumOut.addToSubstructure( (const SmartRef<HltObjectSummary>)(pObj) );
           break;                     
         }
       }
     }
     if( !selSumOut.substructure().empty() ){
       
       // insert selection into the container
       if( outputSummary->insert("Hlt1Global",selSumOut) == StatusCode::FAILURE ){
         Error( "  Failed to add Hlt selection name Hlt1Global to its container ", StatusCode::SUCCESS, 10 );
       } 
       m_objectSummaries->push_back(new HltObjectSummary{ selSumOut });
     }
  }

  if( !outputSummary->hasSelectionName("Hlt2Global") ){

     HltObjectSummary selSumOut;    
     selSumOut.setSummarizedObjectCLID( 1 ); // use special CLID for selection summaries (lowest number for sorting to the end)
     
     // integer selection id 
     selSumOut.addToInfo("0#SelectionID",float(kHlt2GlobalID));

     // see which decisions contributed to it
     for(const  auto&  it : *outputSummary ) {
       const std::string & selName = it.first;
       if( !boost::algorithm::starts_with(selName, "Hlt2")  || 
           !boost::algorithm::ends_with( selName, "Decision") ) continue;
       auto& selRep = it.second;
       // must find corresponding HltObjectSummary in the object store
       auto i = selRep.numericalInfo().find("0#SelectionID");
       if( i==selRep.numericalInfo().end() ){               
         Error(" (1) HltObjectSummary of selection-summary-type has no SelectionID info ",StatusCode::SUCCESS,20);
         continue;
       } 
       int id = (int)(i->second+0.1);
       for( const auto& pObj : *m_objectSummaries ){
         if( pObj->summarizedObjectCLID()!=1 )continue; 
         auto j=pObj->numericalInfo().find("0#SelectionID");
         if( j==pObj->numericalInfo().end() ){
           Error(" (2) HltObjectSummary of selection-summary-type has no SelectionID info ",StatusCode::SUCCESS,20);
           continue;
         } 
         if( id == (int)(j->second+0.1) ){
           // must also check its decision in HltDecReports since it might have been killed by postscale
           if( decReports ){
             const HltDecReport* decReport = decReports->decReport(selName);
             if( decReport && !(decReport->decision()) )break;
           }                 
           selSumOut.addToSubstructure( pObj );
           break;                     
         }
       }
     }
     if( selSumOut.substructure().size() ){
       
       // insert selection into the container
       if( outputSummary->insert("Hlt2Global",selSumOut) == StatusCode::FAILURE ){
         Error( "  Failed to add Hlt selection name Hlt2Global to its container ", StatusCode::SUCCESS, 10 );
       } 
       m_objectSummaries->push_back( new HltObjectSummary{ selSumOut } );    
     }
  }

  // ----------------- printout ------------------------
  if ( msgLevel(MSG::VERBOSE) ){

   verbose() << " ======= HltSelReports size= " << outputSummary->size() << endmsg;  
   verbose() << *outputSummary << endmsg;

   verbose() << " ======= HltObjectSummary container size= " << m_objectSummaries->size() << endmsg;
   for( const auto& pHos : *m_objectSummaries ) {
     verbose() << " key " << pHos->index();
     const auto& selby = outputSummary->selectedAsCandidateBy(pHos);
     if( !selby.empty() ){
       verbose() << " selectedAsCandidateBy=";       
       for( const auto&  i : selby ) verbose()  << i << " ";
       std::pair<std::string,int> pvInfo = outputSummary->pvSelectionNameAndKey(pHos);
       if( pvInfo.second > -1 ){
         verbose() << " pvSelectionName= " << pvInfo.first << " pvKey= " << pvInfo.second << " ";
       }
     }     
     verbose() << *pHos << endmsg;    
   }
  }
  return StatusCode::SUCCESS;
}

// -------------------------------------
// store Track in HltObjectSummary store
// -------------------------------------
const LHCb::HltObjectSummary* HltSelReportsMaker::store_(const LHCb::Track& object)
{
  // not yet in object store; create new one
  std::unique_ptr<HltObjectSummary> hos { new HltObjectSummary() };
  hos->setSummarizedObjectCLID( object.clID() );
  hos->setSummarizedObject(&object);
 
  // find mutations of the same object
  HltObjectSummary::Info theseInfo = infoToSave( *hos );
  std::vector<LHCbID> theseHits = object.lhcbIDs() ;
  
  if( !m_Turbo ){
    std::vector<LHCbID>::const_iterator rbegin1=theseHits.end(); --rbegin1;
    std::vector<LHCbID>::const_iterator rend1=theseHits.begin(); --rend1;        

    //    look for objects with the same lhcbids
    //      if many then the one which leaves least amount info on this one
    int smallestSize = theseInfo.size();
    const HltObjectSummary* pBestHos(0);

    //   couldn't get reverse operator to work
    HltObjectSummarys::const_iterator rbegin=m_objectSummaries->end();  --rbegin;
    HltObjectSummarys::const_iterator rend  =m_objectSummaries->begin(); --rend;  
    for( auto  ppHos=rbegin;ppHos!=rend;--ppHos){
      const HltObjectSummary* pHos(*ppHos);    
      if( pHos->summarizedObjectCLID() == object.clID() ){
        std::vector<LHCbID> otherHits = pHos->lhcbIDsFlattened();      
        bool different=false;      
        if( otherHits.size() == theseHits.size() ){
          std::vector<LHCbID>::const_iterator h2=otherHits.end(); --h2;        
          for( std::vector<LHCbID>::const_iterator h1=rbegin1;h1!=rend1;--h1,--h2){
            if( h1->lhcbID() != h2->lhcbID() ){
              different=true;
              break;            
            }
          }
          if( different )continue;
          // found object of the same type with the same lhcbid content!
          // now check if its Info content is a subset of this one
          HltObjectSummary::Info otherInfo = pHos->numericalInfoFlattened(); 
          if( otherInfo.size() <= theseInfo.size() ){
            bool notcontained=false;          
            for( auto i1=otherInfo.begin();i1!=otherInfo.end();++i1 ){
              auto i2 = theseInfo.find( i1->first );
              if( i2==theseInfo.end() ){
                notcontained = true;
                break;
              }
              if( i1->second != i2->second ) break;
            }
            if( notcontained ) continue;
            if( otherInfo.size() == theseInfo.size() ){
              smallestSize=0;
              pBestHos=pHos;
              // don't need to look any more since subobject has all the info
              break;            
            } else {
              int presentSize = theseInfo.size()-otherInfo.size();
              if( presentSize < smallestSize ){
                smallestSize = presentSize;
                pBestHos=pHos;
              }
            }
          }
        }
      }
    }
    // found subobject
    if( pBestHos ){
      if( smallestSize==0 ){      
        theseInfo.clear();
      } else {
        for( const auto&  i : pBestHos->numericalInfoFlattened() ) {
          theseInfo.erase( i.first );
        }
      }
      // lhcbids are stored at the botton object
      theseHits.clear();
      // set subobject link
      hos->addToSubstructure( pBestHos );          
    }
  }  
  hos->setLhcbIDs( std::move(theseHits) );
  hos->setNumericalInfo( theseInfo );

  m_objectSummaries->push_back(hos.release());
  return m_objectSummaries->back();
}

// -------------------------------------
// store RecVertex in HltObjectSummary store
// -------------------------------------
const LHCb::HltObjectSummary* HltSelReportsMaker::store_(const LHCb::RecVertex& object)
{  
  // not yet in object store; create new one
  std::unique_ptr<HltObjectSummary> hos { new HltObjectSummary() };
  hos->setSummarizedObjectCLID( object.clID() );
  hos->setSummarizedObject(&object);

  // find mutations of the same object
  HltObjectSummary::Info theseInfo = infoToSave( *hos );

  SmartRefVector<LHCb::HltObjectSummary> thisSubstructure;  
  const auto& tracks = object.tracks();
  std::transform( std::begin(tracks), std::end(tracks), 
                  std::back_inserter(thisSubstructure),
                  [&](const LHCb::Track* t) { return store_(*t); } );
  SmartRefVector<LHCb::HltObjectSummary>::const_iterator rbegin1=thisSubstructure.end(); --rbegin1;
  SmartRefVector<LHCb::HltObjectSummary>::const_iterator rend1=thisSubstructure.begin(); --rend1;        

  //    look for objects with the same lhcbids
  //      if many then the one which leaves least amount info on this one
  int smallestSize = theseInfo.size();
  const HltObjectSummary* pBestHos { nullptr };

  //   couldn't get reverse operator to work
  HltObjectSummarys::iterator rbegin=m_objectSummaries->end();  --rbegin;
  HltObjectSummarys::iterator rend  =m_objectSummaries->begin(); --rend;  
  for( auto ppHos=rbegin;ppHos!=rend;--ppHos){ 
    const HltObjectSummary* pHos{*ppHos};
    if( pHos->summarizedObjectCLID() == object.clID() ){
      SmartRefVector<LHCb::HltObjectSummary> otherSubstructure = pHos->substructureFlattened();      
      bool different=false;      
      if( otherSubstructure.size() == thisSubstructure.size() ){
        auto h2=otherSubstructure.end(); --h2;        
        for( auto h1=rbegin1;h1!=rend1;--h1,--h2){
          if( (*h1) != (*h2) ){
            different=true;
            break;            
          }
        }
        if( different )continue;
        // found object of the same type with the same (flattened) substructure!
        // now check if its Info content is a subset of this one
        HltObjectSummary::Info otherInfo = pHos->numericalInfoFlattened(); 
        if( otherInfo.size() <= theseInfo.size() ){
          bool notcontained=false;          
          for( const auto&  i1 : otherInfo ) {
            auto i2 = theseInfo.find( i1.first );
            if( i2==theseInfo.end() ){
              notcontained = true;
              break;
            }
            if( i1.second != i2->second ) break;
          }
          if( notcontained )continue;
          if( otherInfo.size() == theseInfo.size() ){
            smallestSize=0;
            pBestHos=pHos;
            // don't need to look any more since subobject has all the info
            break;            
          } else {
            int presentSize = theseInfo.size()-otherInfo.size();
            if( presentSize < smallestSize ){
              smallestSize = presentSize;
              pBestHos=pHos;
            }
          }
        }
      }
    }
  }
  // found subobject
  if( pBestHos ){
    if( smallestSize==0 ){      
      theseInfo.clear();
    } else {
      for( const auto& i1 : pBestHos->numericalInfoFlattened()) {
        theseInfo.erase( i1.first );
      }
    }
    // set subobject link
    thisSubstructure.clear();
    // set substructure link
    thisSubstructure.push_back( pBestHos );    
  }
   
  hos->setSubstructure( thisSubstructure );
  hos->setNumericalInfo( std::move(theseInfo) );
  m_objectSummaries->push_back(hos.release());
  return m_objectSummaries->back();
}

// -------------------------------------------
// store CaloCluster in HltObjectSummary store
// -------------------------------------------
const LHCb::HltObjectSummary* HltSelReportsMaker::store_(const LHCb::CaloCluster& object)
{
  std::unique_ptr<HltObjectSummary> hos { new HltObjectSummary{} };
  hos->setSummarizedObjectCLID( object.clID() );
  hos->setSummarizedObject(&object);
  hos->setNumericalInfo(infoToSave( *hos ));
  hos->setLhcbIDs( std::vector<LHCbID> { { object.seed() } } );  
  m_objectSummaries->push_back(hos.release());
  return m_objectSummaries->back();
}

// -------------------------------------------
// store Vertex in HltObjectSummary store
// -------------------------------------------
const LHCb::HltObjectSummary* HltSelReportsMaker::store_(const LHCb::Vertex& object)
{
  std::unique_ptr<HltObjectSummary> hos {new HltObjectSummary{} };
  hos->setSummarizedObjectCLID( object.clID() );
  hos->setSummarizedObject(&object);
  hos->setNumericalInfo( infoToSave( *hos ));
  m_objectSummaries->push_back(hos.release());
  return m_objectSummaries->back();  
}
// -------------------------------------------
// store ProtoParticle in HltObjectSummary store
// -------------------------------------------
const LHCb::HltObjectSummary* HltSelReportsMaker::store_(const LHCb::ProtoParticle& object)
{
  std::unique_ptr<HltObjectSummary> hos { new HltObjectSummary() };
  hos->setSummarizedObjectCLID( object.clID() );
  hos->setSummarizedObject(&object);
  hos->setNumericalInfo( infoToSave( *hos ) );
  m_objectSummaries->push_back(hos.release());
  return m_objectSummaries->back();
}
// -------------------------------------------
// store RichPID in HltObjectSummary store
// -------------------------------------------
const LHCb::HltObjectSummary* HltSelReportsMaker::store_(const LHCb::RichPID& object)
{
  std::unique_ptr<HltObjectSummary> hos { new HltObjectSummary() };
  hos->setSummarizedObjectCLID( object.clID() );
  hos->setSummarizedObject(&object);
  hos->setNumericalInfo(  infoToSave( *hos ));
  m_objectSummaries->push_back(hos.release());
  return m_objectSummaries->back();  
}
// -------------------------------------------
// store MuonPID in HltObjectSummary store
// -------------------------------------------
const LHCb::HltObjectSummary* HltSelReportsMaker::store_(const LHCb::MuonPID& object)
{
  std::unique_ptr<HltObjectSummary> hos{ new HltObjectSummary() };
  hos->setSummarizedObjectCLID( object.clID() );
  hos->setSummarizedObject(&object);
  hos->setNumericalInfo( infoToSave( *hos ));
  m_objectSummaries->push_back(hos.release());
  return m_objectSummaries->back();  
}


// ----------------------------------------
// store Particle in HltObjectSummary store
// ----------------------------------------
const LHCb::HltObjectSummary* HltSelReportsMaker::store_(const LHCb::Particle& object)
{  
  std::unique_ptr<HltObjectSummary> hos { new HltObjectSummary() };
  hos->setSummarizedObjectCLID( object.clID() );
  hos->setSummarizedObject(&object);  

  HltObjectSummary::Info theseInfo = infoToSave( *hos );

  // no subobject code implemented since so far Hlt2 does not save intermediate selections

  const auto& daughters = object.daughtersVector();
  if( !daughters.empty() ){    
    debug() << "Adding non-basic particle" << endmsg;
    for( const auto&  p : daughters ) hos->addToSubstructure( store_( *p ) );
    // If the particle is not basic, then add the vertex to the substructure after the daughters
    if(m_Turbo) hos->addToSubstructure( store_( *object.endVertex() ) ); // SB add
  } else {
    debug() << "Adding basic particle" << endmsg;
    // particles with no daughters have substructure via ProtoParticle 
    // we don't save Protoparticles, only things they lead to
    const ProtoParticle* pp = object.proto();
    if( !pp ){
      std::string w { " Particle with no daughters and no protoparticle " };
      const ObjectContainerBase *p = object.parent();
      if (p) { 
          IRegistry* r = p->registry();
          if (r) {
              w += " with id " ; w += r->identifier();
          } else {
              w += " in " ; w += p->name() ; 
          }
      }
      w+= ", skipped";
      Warning(w,StatusCode::SUCCESS, 10 );
    } else {
      const Track* track=pp->track();
      if( track ){
        // charged track particle
        hos->addToSubstructure( store_( *track ) ); 
        debug() << "requesting track store" << endmsg;
        if(m_Turbo) {
          hos->addToSubstructure( store_( *pp ) );  
          debug() << "requesting proto-particle store" << endmsg;// SB add
        }
        if(m_Turbo && pp->richPID() ) {
          hos->addToSubstructure( store_( *pp->richPID() ) );
          debug() << "requesting RichPID store" << endmsg;// SB add
        }
        if(m_Turbo && pp->muonPID() ) {
          hos->addToSubstructure( store_( *pp->muonPID() ) );
          debug() << "requesting MuonPID store" << endmsg;// SB add
        }
        // if muon add muon stub too        
        const LHCb::MuonPID* muid = pp->muonPID();
        if ( muid && object.particleID().abspid()==13  &&  muid->IsMuon() ){
            const LHCb::Track*  muStub=muid->muonTrack();
            if (!muStub ){
              if( !m_HLTmuonTracks ) { 
                //Now we need to derive the muon segment container from
                //the track container
                m_HltMuonTracksLocation.clear() ;
                const DataObject* obj = track->parent() ;
                const IRegistry* reg = ( obj ? obj->registry() : nullptr );
                if ( reg ) m_HltMuonTracksLocation = reg->identifier() + m_muonIDSuffix;
                m_HLTmuonTracks = ( !m_HltMuonTracksLocation.empty() ? getIfExists<LHCb::Tracks>(m_HltMuonTracksLocation) : nullptr);
                if( !m_HLTmuonTracks) {
                  Warning(" Found Particle which is a muon but no muon tracks at " + m_HltMuonTracksLocation 
                          ,StatusCode::SUCCESS,10 );
                }
              }
              muStub = ( m_HLTmuonTracks ?  m_HLTmuonTracks->object(track->key()) : nullptr );      
            }
            if( muStub ){
              hos->addToSubstructure( store_( *muStub ) );
            } else {
              Warning(" Found Particle which is a muon but could not reach muon lhcbids "
                          ,StatusCode::SUCCESS,10 );
            }            
        }
      } else {
        // neutral particle ?
        // Ecal via CaloHypo
        const auto& caloVec = pp->calo();
        if( !caloVec.empty() ) {
          const LHCb::CaloHypo*   hypo  = caloVec.front();
          if( LHCb::CaloHypo::Photon == hypo->hypothesis() ){
            // Photon
            hos->addToSubstructure( store_( *hypo->clusters().front() ) );
          } else if (  LHCb::CaloHypo::Pi0Merged == hypo->hypothesis() ){
            // Split Photons
            const SmartRefVector<LHCb::CaloHypo>& hypos = hypo->hypos();
            const LHCb::CaloHypo* g1 = hypos.front();
            hos->addToSubstructure( store_( *g1->clusters().front() ) );
            const LHCb::CaloHypo* g2 = *std::next(std::begin((hypos)));
            hos->addToSubstructure( store_( *g2->clusters().front() ) );
          }
        }
      }
    }
  }
  hos->setNumericalInfo( std::move(theseInfo) );
  m_objectSummaries->push_back(hos.release());
  return m_objectSummaries->back();  
}

//=============================================================================
// fetch info to save; to find object follow summerizedObject pointer
//=============================================================================
HltObjectSummary::Info HltSelReportsMaker::infoToSave( const HltObjectSummary& hos ) const
{
  HltObjectSummary::Info infoPersistent;
  if( kMinInfoLevel == m_presentInfoLevel )return infoPersistent; 

  if( !(hos.summarizedObject()) )return infoPersistent;  
  switch( hos.summarizedObjectCLID() )
  {
  case LHCb::CLID_Track: {      
    const Track* candi = dynamic_cast<const Track*>(hos.summarizedObject());
    if( !candi ) return infoPersistent; 
    if( kExtraInfoLevel &  m_presentInfoLevelTrack & m_presentInfoLevel ){ 
        for( const auto& ei : candi->extraInfo() ) {
            if( ( 0<= ei.first ) && ( ei.first<=65535 ) ){
                auto i=m_infoIntToName.find(ei.first);
                if( i!=std::end(m_infoIntToName) ){
                    infoPersistent.insert( i->second, float( ei.second ) );
                }
            }
        }
    }
    if( kStandardInfoLevel &  m_presentInfoLevelTrack & m_presentInfoLevel ){
        if( candi->nStates() ){
          const State & firstState = candi->firstState();
          infoPersistent.insert( "0#Track.firstState.z", float( firstState.z() ) ); debug() << "0#Track.firstState.z = " << float( firstState.z() ) << endmsg;
          infoPersistent.insert( "1#Track.firstState.x", float( firstState.x() ) ); debug() << "1#Track.firstState.x = " << float( firstState.x() ) << endmsg;
          infoPersistent.insert( "2#Track.firstState.y", float( firstState.y() ) ); debug() << "2#Track.firstState.y = " << float( firstState.y() ) << endmsg;
          infoPersistent.insert( "3#Track.firstState.tx", float( firstState.tx() ) ); debug() << "3#Track.firstState.tx = " << float( firstState.tx() ) << endmsg;
          infoPersistent.insert( "4#Track.firstState.ty", float( firstState.ty() ) ); debug() << "4#Track.firstState.ty = " << float( firstState.ty() ) << endmsg; 
          infoPersistent.insert( "5#Track.firstState.qOverP", float( firstState.qOverP() ) ); debug() << "5#Track.firstState.qOverP = " << float( firstState.qOverP() ) << endmsg;
          infoPersistent.insert( "6#Track.chi2PerDoF", float( candi->chi2PerDoF() ) ); debug() << "6#Track.chi2PerDoF = " << float( candi->chi2PerDoF() ) << endmsg;
          infoPersistent.insert( "7#Track.nDoF", float( candi->nDoF() ) ); debug() << "7#Track.nDoF = " << float( candi->nDoF() ) << endmsg;
          if(m_Turbo){ // SB additions
            infoPersistent.insert( "8#Track.Likelihood", float( candi->likelihood() ) ); debug() << "8#Track.Likelihood = " << float( candi->likelihood() ) << endmsg;
            infoPersistent.insert( "9#Track.GhostProb", float( candi->ghostProbability() ) ); debug() << "9#Track.GhostProb = " << float( candi->ghostProbability() ) << endmsg;
            infoPersistent.insert( "10#Track.flags", float( candi->flags() ) ); debug() << "10#Track.flags = " << float( candi->flags() ) << endmsg;
            const State* lastState = candi->states().back();
            infoPersistent.insert( "11#Track.lastState.z", float( lastState->z() ) ); debug() << "11#Track.lastState.z = " << float( lastState->z() ) << endmsg;
            infoPersistent.insert( "12#Track.lastState.x", float( lastState->x() ) ); debug() << "12#Track.lastState.x = " << float( lastState->x() ) << endmsg;
            infoPersistent.insert( "13#Track.lastState.y", float( lastState->y() ) ); debug() << "13#Track.lastState.y = " << float( lastState->y() ) << endmsg;
            infoPersistent.insert( "14#Track.lastState.tx", float( lastState->tx() ) ); debug() << "14#Track.lastState.tx = " << float( lastState->tx() ) << endmsg;
            infoPersistent.insert( "15#Track.lastState.ty", float( lastState->ty() ) ); debug() << "15#Track.lastState.ty = " << float( lastState->ty() ) << endmsg;
            infoPersistent.insert( "16#Track.lastState.qOverP", float( lastState->qOverP() ) ); debug() << "16#Track.lastState.qOverP = " << float( lastState->qOverP() ) << endmsg;
          }
        }
      }
    }
    break;
  case LHCb::CLID_RichPID: // SB add RichPID
    {      
      const RichPID* candi = dynamic_cast<const RichPID*>(hos.summarizedObject());
      if( !candi )return infoPersistent; 
      if( kStandardInfoLevel & m_presentInfoLevel ){ 
          infoPersistent.insert( "0#Rich.pidResultCode", float( candi->pidResultCode() ) );
          infoPersistent.insert( "1#Rich.DLLe", float( candi->particleDeltaLL( Rich::ParticleIDType::Electron ) ) );
          infoPersistent.insert( "2#Rich.DLLmu", float( candi->particleDeltaLL( Rich::ParticleIDType::Muon ) ) );
          infoPersistent.insert( "3#Rich.DLLpi", float( candi->particleDeltaLL( Rich::ParticleIDType::Pion ) ) );
          infoPersistent.insert( "4#Rich.DLLK", float( candi->particleDeltaLL( Rich::ParticleIDType::Kaon ) ) ); debug() << "4#Rich.DLLK = " << float( candi->particleDeltaLL( Rich::ParticleIDType::Kaon ) ) << endmsg;
          infoPersistent.insert( "5#Rich.DLLp", float( candi->particleDeltaLL( Rich::ParticleIDType::Proton ) ) );
      }    
    }    
    break;
  case LHCb::CLID_MuonPID: // SB add MuonPID
    {      
      const MuonPID* candi = dynamic_cast<const MuonPID*>(hos.summarizedObject());
      if( !candi )return infoPersistent; 
      if( kStandardInfoLevel & m_presentInfoLevel ){ 
          infoPersistent.insert( "0#Muon.MuonLLMu", float( candi->MuonLLMu() ) ); debug() << "0#Muon.MuonLLMu = " << float( candi->MuonLLMu() ) << endmsg;
          infoPersistent.insert( "1#Muon.MuonLLBg", float( candi->MuonLLBg() ) );
          infoPersistent.insert( "2#Muon.NShared", float( candi->nShared() ) );
          infoPersistent.insert( "3#Muon.Status", float( candi->Status() ) );
          infoPersistent.insert( "4#Muon.IsMuon", float( candi->IsMuon() ) );
          infoPersistent.insert( "5#Muon.IsMuonLoose", float( candi->IsMuonLoose() ) );
          infoPersistent.insert( "6#Muon.IsMuonTight", float( candi->IsMuonTight() ) );
      }    
    }
    break;
  case LHCb::CLID_ProtoParticle: // SB add ProtoParticle
    {      
      const ProtoParticle* candi = dynamic_cast<const ProtoParticle*>(hos.summarizedObject());
      if( !candi )return infoPersistent; 
      if( kStandardInfoLevel & m_presentInfoLevel ){ 
          infoPersistent.insert( "0#Proto.extraInfo.IsPhoton", float( candi->info( 381, -1000 ) ) ); debug() << "0#Proto.extraInfo.IsPhoton = " << float( candi->info( 381, -1000 ) ) << endmsg;
      }    
    }    
    break;
  case LHCb::CLID_Vertex: // SB add Vertex
    {      
      const Vertex* candi = dynamic_cast<const Vertex*>(hos.summarizedObject());
      if( !candi )return infoPersistent; 
      if( kStandardInfoLevel & m_presentInfoLevel ){ 
          infoPersistent.insert( "0#Vertex.chi2", float( candi->chi2() ) ); debug() << "0#Vertex.chi2 = " << float( candi->chi2() ) << endmsg;
          infoPersistent.insert( "1#Vertex.ndf", float( candi->nDoF() ) );
          const auto &position = candi->position();
          infoPersistent.insert( "2#Vertex.position.x", float( position.x() ) );
          infoPersistent.insert( "3#Vertex.position.y", float( position.y() ) );
          infoPersistent.insert( "4#Vertex.position.z", float( position.z() ) );
          infoPersistent.insert( "5#Vertex.technique", float( candi->technique() ) );
          // TODO: normalize off-diagonal elements wrt. diagonal ones, so in range[-1,1]
          const auto& covMatrix = candi->covMatrix();
          infoPersistent.insert( "6#Vertex.cov00", float( covMatrix(0,0) ) );
          infoPersistent.insert( "7#Vertex.cov11", float( covMatrix(1,1) ) );
          infoPersistent.insert( "8#Vertex.cov22", float( covMatrix(2,2) ) );
          infoPersistent.insert( "9#Vertex.cov10", float( covMatrix(1,0) ) );
          infoPersistent.insert( "10#Vertex.cov20", float( covMatrix(2,0) ) );
          infoPersistent.insert( "11#Vertex.cov21", float( covMatrix(2,1) ) );
      }    
    }    
    break;
  case LHCb::CLID_RecVertex:
    {      
      const RecVertex* candi = dynamic_cast<const RecVertex*>(hos.summarizedObject());
      if( !candi )return infoPersistent; 
      if( kExtraInfoLevel &  m_presentInfoLevelRecVertex & m_presentInfoLevel ){
	for( const auto& ei : candi->extraInfo() ) {
          if( ( 0<= ei.first ) && ( ei.first<=65535 ) ){
	    auto i=m_infoIntToName.find(ei.first);
	    if( i!=m_infoIntToName.end() ){
	      infoPersistent.insert( i->second, float( ei.second ) );
	    }
	  }
	}
      }
      if( kStandardInfoLevel &  m_presentInfoLevelRecVertex & m_presentInfoLevel ){ 
        const auto& position = candi->position();
        infoPersistent.insert( "0#RecVertex.position.x", float( position.x() ) );
        infoPersistent.insert( "1#RecVertex.position.y", float( position.y() ) );
        infoPersistent.insert( "2#RecVertex.position.z", float( position.z() ) );
        infoPersistent.insert( "3#RecVertex.chi2", float( candi->chi2() ) );
      }      
    }    
    break;
  case LHCb::CLID_Particle:
    {      
      const Particle* candi = dynamic_cast<const Particle*>(hos.summarizedObject());
      if( !candi )return infoPersistent; 
      if( kExtraInfoLevel &  m_presentInfoLevelParticle & m_presentInfoLevel ){
        for( const auto& ei : candi->extraInfo() ) {
            if( ( 0<= ei.first ) && ( ei.first<=65535 ) ){
                auto i=m_infoIntToName.find(ei.first);
                if( i!=std::end(m_infoIntToName) ) infoPersistent.insert( i->second, float( ei.second ) );
            }
        }
      }
      if( kStandardInfoLevel &  m_presentInfoLevelParticle & m_presentInfoLevel ){ 
        infoPersistent.insert( "0#Particle.particleID.pid", float( candi->particleID().pid() ) );
        infoPersistent.insert( "1#Particle.measuredMass", float( candi->measuredMass() ) );
        const auto& referencePoint = candi->referencePoint();
        infoPersistent.insert( "2#Particle.referencePoint.z", float( referencePoint.z() ) );
        infoPersistent.insert( "3#Particle.referencePoint.x", float( referencePoint.x() ) );
        infoPersistent.insert( "4#Particle.referencePoint.y", float( referencePoint.y() ) );
        const auto& slopes = candi->slopes();
        infoPersistent.insert( "5#Particle.slopes.x", float( slopes.x() ) );
        infoPersistent.insert( "6#Particle.slopes.y", float( slopes.y() ) );
        double p = candi->p();
        if( p < 1E-9 )p=1E-9;        // Note: p is _not_ signed for a Particle. Charge is part of pid...
        infoPersistent.insert( "7#Particle.1/p", float( 1.0/p ) );
        // SB additions
        if(m_Turbo){
          infoPersistent.insert( "8#Particle.conflevel", float( candi->confLevel() ) );
          infoPersistent.insert( "9#Particle.massErr", float( candi->measuredMassErr() ) );
          // TODO: normalize off-diagonal elements to on-diagional ones, and use that they are in [-1,+1] 
          //       to pack them better...
          const auto& momCov = candi->momCovMatrix();
          infoPersistent.insert( "10#Particle.momCov00", float( momCov(0,0) ) );
          infoPersistent.insert( "11#Particle.momCov11", float( momCov(1,1) ) );
          infoPersistent.insert( "12#Particle.momCov22", float( momCov(2,2) ) );
          infoPersistent.insert( "13#Particle.momCov33", float( momCov(3,3) ) );
          infoPersistent.insert( "14#Particle.momCov10", float( momCov(1,0) ) );
          infoPersistent.insert( "15#Particle.momCov20", float( momCov(2,0) ) );
          infoPersistent.insert( "16#Particle.momCov21", float( momCov(2,1) ) );
          infoPersistent.insert( "17#Particle.momCov30", float( momCov(3,0) ) );
          infoPersistent.insert( "18#Particle.momCov31", float( momCov(3,1) ) );
          infoPersistent.insert( "19#Particle.momCov32", float( momCov(3,2) ) );
          const auto& posMomCov = candi->posMomCovMatrix();
          infoPersistent.insert( "20#Particle.posmomCov00", float( posMomCov(0,0) ) );
          infoPersistent.insert( "21#Particle.posmomCov11", float( posMomCov(1,1) ) );
          infoPersistent.insert( "22#Particle.posmomCov22", float( posMomCov(2,2) ) );
          infoPersistent.insert( "23#Particle.posmomCov10", float( posMomCov(1,0) ) );
          infoPersistent.insert( "24#Particle.posmomCov01", float( posMomCov(0,1) ) );
          infoPersistent.insert( "25#Particle.posmomCov20", float( posMomCov(2,0) ) );
          infoPersistent.insert( "26#Particle.posmomCov02", float( posMomCov(0,2) ) );
          infoPersistent.insert( "27#Particle.posmomCov21", float( posMomCov(2,1) ) );
          infoPersistent.insert( "28#Particle.posmomCov12", float( posMomCov(1,2) ) );
          infoPersistent.insert( "29#Particle.posmomCov30", float( posMomCov(3,0) ) );
          infoPersistent.insert( "30#Particle.posmomCov31", float( posMomCov(3,1) ) );
          infoPersistent.insert( "31#Particle.posmomCov32", float( posMomCov(3,2) ) );
          const auto& posCov = candi->posCovMatrix();
          infoPersistent.insert( "32#Particle.posCov00", float( posCov(0,0) ) );
          infoPersistent.insert( "33#Particle.posCov11", float( posCov(1,1) ) );
          infoPersistent.insert( "34#Particle.posCov22", float( posCov(2,2) ) );
          infoPersistent.insert( "35#Particle.posCov10", float( posCov(1,0) ) );
          infoPersistent.insert( "36#Particle.posCov20", float( posCov(2,0) ) );
          infoPersistent.insert( "37#Particle.posCov21", float( posCov(2,1) ) );
        }
      }      
    }
    break;
  case LHCb::CLID_CaloCluster:
    {      
      const CaloCluster* candi = dynamic_cast<const CaloCluster*>(hos.summarizedObject());
      if( !candi )return infoPersistent; 
      if( kStandardInfoLevel &  m_presentInfoLevelCaloCluster & m_presentInfoLevel ){ 
        infoPersistent.insert( "0#CaloCluster.e", float( candi->e() ) );
        const auto& position = candi->position();
        infoPersistent.insert( "1#CaloCluster.position.x", float( position.x() ) );
        infoPersistent.insert( "2#CaloCluster.position.y", float( position.y() ) );
        infoPersistent.insert( "3#CaloCluster.position.z", float( position.z() ) );
      }      
    }
    break;
  default:
    { }    
  }
  return infoPersistent;
}

int HltSelReportsMaker::rank_(const LHCb::Track& object) const
{
  HltObjectSummary hos;
  hos.setSummarizedObjectCLID( object.clID() );
  hos.setSummarizedObject(&object);  
  return 100*rankLHCbIDs( object.lhcbIDs() ) +  infoToSave( hos ).size();
}

int HltSelReportsMaker::rank_(const LHCb::RichPID& object) const
{
  HltObjectSummary hos;
  hos.setSummarizedObjectCLID( object.clID() );
  hos.setSummarizedObject(&object);  
  return 10000 +  infoToSave( hos ).size();
}

int HltSelReportsMaker::rank_(const LHCb::MuonPID& object) const
{
  HltObjectSummary hos;
  hos.setSummarizedObjectCLID( object.clID() );
  hos.setSummarizedObject(&object);  
  return 10000 +  infoToSave( hos ).size();
}

int HltSelReportsMaker::rank_(const LHCb::ProtoParticle& object) const
{
  HltObjectSummary hos;
  hos.setSummarizedObjectCLID( object.clID() );
  hos.setSummarizedObject(&object);  
  return 10000 +  infoToSave( hos ).size();
}

int HltSelReportsMaker::rank_(const LHCb::Vertex& object) const
{
  HltObjectSummary hos;
  hos.setSummarizedObjectCLID( object.clID() );
  hos.setSummarizedObject(&object);  
  return 10000 +  infoToSave( hos ).size();
}

int HltSelReportsMaker::rank_(const LHCb::CaloCluster& object) const
{
  HltObjectSummary hos;
  hos.setSummarizedObjectCLID( object.clID() );
  hos.setSummarizedObject(&object);  
  return 100*rankLHCbIDs( { { object.seed() } } ) +  infoToSave( hos ).size();
}

int HltSelReportsMaker::rank_(const LHCb::RecVertex& object) const
{
  HltObjectSummary hos;
  hos.setSummarizedObjectCLID( object.clID() );
  hos.setSummarizedObject(&object);  
  return 10000 +  infoToSave( hos ).size();
}

int HltSelReportsMaker::rank_(const LHCb::Particle& object) const
{
  HltObjectSummary  hos;
  hos.setSummarizedObjectCLID( object.clID() );
  hos.setSummarizedObject(&object);  
  return 100000 +  infoToSave( hos ).size();
}

int HltSelReportsMaker::rankLHCbIDs( const std::vector<LHCbID> & lhcbIDs ) const
{
  return std::accumulate( std::begin(lhcbIDs), std::end(lhcbIDs), 0, 
                          [](int rank, const LHCbID& id ) { 
    if ( id.isVelo() )                    rank |=  8;
    else if ( id.isTT() )                 rank |= 16;      
    else if ( id.isOT() || id.isIT() )    rank |= 32;      
    else if ( id.isMuon() )               rank |=  4;      
    else if ( id.isCalo() )               rank |=  2;      
    else                                  rank |=  1;
    return rank;  
  } );
}

    

//=============================================================================
// qualify trigger selections for persistency
//=============================================================================
unsigned int HltSelReportsMaker::maximumNumberOfCandidatesToStore( const std::string & selName  ) const
{
  if( m_debugMode ){
    // DEBUG MODE

    // set max number of candidates
    const SelectionSetting & maxCandidatesDebug =  m_maxCandidatesDebug.value();    
    for( auto i=maxCandidatesDebug.begin(); i!=maxCandidatesDebug.end(); ++i){
      if( selName == i->first ) return i->second;
    }
    
    // not set specifically for this selection ; use default
    return boost::algorithm::ends_with(selName,"Decision") ?
              m_maxCandidatesDecisionDebug :
              m_maxCandidatesNonDecisionDebug;

  } else {
    
    // NORMAL MODE

    // set max number of candidates
    const SelectionSetting & maxCandidates =  m_maxCandidates.value();    
    for( auto i=maxCandidates.begin(); i!=maxCandidates.end(); ++i){
      if( selName == i->first ) return i->second;
    }
    
    // not set specifically for this selection ; use default
    return boost::algorithm::ends_with( selName, "Decision") ?
              m_maxCandidatesDecision :
              m_maxCandidatesNonDecision;
  }

}

//=============================================================================
// set output info level for this selection
//=============================================================================
void HltSelReportsMaker::setPresentInfoLevel( const std::string & selName )
{
  if( m_debugMode ){
    // DEBUG MODE

    m_presentInfoLevelTrack = m_infoLevelTrackDebug;
    m_presentInfoLevelRecVertex = m_infoLevelRecVertexDebug;
    m_presentInfoLevelParticle = m_infoLevelParticleDebug;
    m_presentInfoLevelCaloCluster = m_infoLevelCaloClusterDebug;
    m_presentInfoLevelSelection = m_infoLevelSelectionDebug;
    // set max number of candidates
    const SelectionSetting & infoLevelDebug =  m_infoLevelDebug.value();    
    for( SelectionSetting::const_iterator i=infoLevelDebug.begin();
         i!=infoLevelDebug.end(); ++i){
      if( selName == i->first ){
        m_presentInfoLevel = i->second;
        return;        
      }
    } 
    
    // not set specifically for this selection ; use default
    if( selName.find("Decision")!=std::string::npos ){
      m_presentInfoLevel = m_infoLevelDecisionDebug;
      return;      
    }
    m_presentInfoLevel = m_infoLevelNonDecisionDebug;

  } else {
    
    // NORMAL MODE

    m_presentInfoLevelTrack = m_infoLevelTrack;
    m_presentInfoLevelRecVertex = m_infoLevelRecVertex;
    m_presentInfoLevelParticle = m_infoLevelParticle;
    m_presentInfoLevelCaloCluster = m_infoLevelCaloCluster;
    m_presentInfoLevelSelection = m_infoLevelSelection;
    // set max number of candidates
    const SelectionSetting & infoLevel =  m_infoLevel.value();    
    for( SelectionSetting::const_iterator i=infoLevel.begin();
         i!=infoLevel.end(); ++i){
      if( selName == i->first ){
        m_presentInfoLevel = i->second;
        return;        
      }
    }
    
    // not set specifically for this selection ; use default
    if( selName.find("Decision")!=std::string::npos ){
      m_presentInfoLevel = m_infoLevelDecision;
      return;      
    }
    m_presentInfoLevel = m_infoLevelNonDecision;
  }
}

bool HltSelReportsMaker::rankSelLess::operator()( const HltSelReportsMaker::RankedSelection & elem1, 
                                                  const HltSelReportsMaker::RankedSelection & elem2) const
{  
    if ( elem1.first != elem2.first ) return elem1.first< elem2.first ;

    // equal ranks; now use selection name; decisions come last
    auto decision_1 = boost::algorithm::ends_with(elem1.second.id.str(),"Decision");
    auto decision_2 = boost::algorithm::ends_with(elem2.second.id.str(),"Decision");
    if (decision_1 != decision_2) return decision_2;
    size_t l1 = elem1.second.id.str().length();
    size_t l2 = elem2.second.id.str().length();
    return (l1!=l2) ? (l1 < l2) : ( elem1.second.id.str() < elem2.second.id.str() );
}
