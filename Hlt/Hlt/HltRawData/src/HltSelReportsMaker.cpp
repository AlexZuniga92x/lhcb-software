// $Id: HltSelReportsMaker.cpp,v 1.15 2009-11-26 13:08:48 tskwarni Exp $
// #define DEBUGCODE
// Include files 
#include "boost/algorithm/string/replace.hpp"
#include "boost/format.hpp"

// from Gaudi
#include "GaudiKernel/AlgFactory.h" 
#include "GaudiKernel/IIncidentSvc.h"
#include "GaudiKernel/IRegistry.h"

#include "Kernel/Particle2Vertex.h"

#include "Event/HltSelReports.h"
#include "Event/HltObjectSummary.h"
#include "Event/HltDecReports.h"

#include "Event/RecVertex.h"
#include "Event/Particle.h"


//#include "Kernel/CaloCellCode.h"

#include "HltBase/stringKey.h"
#include "HltBase/HltSelection.h"
#include "HltBase/IHltDataSvc.h"
// local
#include "HltSelReportsMaker.h"



using namespace LHCb;


namespace {
    // FUN =  int (HltSelReportsMaker::*)(const T*) const
    template <typename T,typename FUN> bool dispatch(const ContainedObject* obj,HltSelReportsMaker* parent, FUN fun, int& rank) {
        const T* c = dynamic_cast<const T*>(obj);
        bool ret = (c!=0);
        if (ret) rank = (parent->*fun)(c);
        return ret;
    }
}

//-----------------------------------------------------------------------------
// Implementation file for class : HltSelReportsMaker
//
// 2008-07-23 : Tomasz Skwarnicki
//-----------------------------------------------------------------------------

// Declaration of the Algorithm Factory
DECLARE_ALGORITHM_FACTORY( HltSelReportsMaker );


//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
HltSelReportsMaker::HltSelReportsMaker( const std::string& name,
                                                      ISvcLocator* pSvcLocator)
  : GaudiAlgorithm ( name , pSvcLocator )
  , m_hltANNSvc (0)
  , m_hltDataSvc(0)
{

  declareProperty("OutputHltSelReportsLocation",
    m_outputHltSelReportsLocation= LHCb::HltSelReportsLocation::Default);  
  declareProperty("HltMuonTrackLocation",
                  m_HltMuonTracksLocation =  LHCb::TrackLocation::HltMuon);
  declareProperty("InputHltDecReportsLocation",
    m_inputHltDecReportsLocation= LHCb::HltDecReportsLocation::Default);  


  declareProperty("DebugEventPeriod",m_debugPeriod = 0 );

  declareProperty("MaxCandidatesDecision", m_maxCandidatesDecision = 1000);
  declareProperty("MaxCandidatesDecisionDebug", m_maxCandidatesDecisionDebug = 5000);
  
  declareProperty("MaxCandidatesNonDecision", m_maxCandidatesNonDecision = 0);
  declareProperty("MaxCandidatesNonDecisionDebug", m_maxCandidatesNonDecisionDebug = 5000);
  
  declareProperty("SelectionMaxCandidates", m_maxCandidates );
  declareProperty("SelectionMaxCandidatesDebug", m_maxCandidatesDebug );

  declareProperty("InfoLevelDecision", m_infoLevelDecision = ((unsigned int)kStandardInfoLevel) );
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
  m_hltDataSvc = svc<IHltDataSvc>("HltDataSvc");

  if (!m_debugIncidentName.empty() ) {
    svc<IIncidentSvc>("IncidentSvc")->addListener(this,m_debugIncidentName);
  }


  // seup string-to-int selection ID map (vector)
  std::vector<IANNSvc::minor_value_type> hlt1 = m_hltANNSvc->items("Hlt1SelectionID");
  std::vector<IANNSvc::minor_value_type> hlt2 = m_hltANNSvc->items("Hlt2SelectionID");
 
  // get trigger selection names 
  m_selectionIDs = m_hltDataSvc->selectionKeys();

  // initialize maps
  m_selectionIntIDs.clear();
  m_maxCand.clear();
  m_maxCandDebug.clear();

  for( std::vector<stringKey>::const_iterator is=m_selectionIDs.begin();is!=m_selectionIDs.end();++is){
     const stringKey name(*is);
     const std::string selName(name.str()); 
     int intSelID(0);
     for( std::vector<IANNSvc::minor_value_type>::const_iterator si=hlt1.begin();
          si!=hlt1.end();++si){
       if( si->first == selName ){
         intSelID=si->second;
         break;
       }
     }
     if( !intSelID ){
       for( std::vector<IANNSvc::minor_value_type>::const_iterator si=hlt2.begin();
            si!=hlt2.end();++si){
         if( si->first == selName ){
           intSelID=si->second;
           break;
         }
       }
     }
     m_selectionIntIDs.push_back(intSelID);
     m_debugMode = false; m_maxCand.push_back( maximumNumberOfCandidatesToStore( selName ) );
     m_debugMode = true;  m_maxCandDebug.push_back( maximumNumberOfCandidatesToStore( selName ) );

  }

  m_infoIntToName.clear();
  std::vector<IANNSvc::minor_value_type> hltinfos = m_hltANNSvc->items("InfoID"); 
  for( std::vector<IANNSvc::minor_value_type>::const_iterator i= hltinfos.begin();i!=hltinfos.end();++i){
    if( ( 0<= i->second ) && ( i->second<=65535 ) ){
      m_infoIntToName.insert( i->second, i->first );
    }
  }
 
  m_lastPVSelectionName = "";  
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

  m_HLTmuonTracks=0;  // will get them only if needed

  // create output container and put it on TES
  HltSelReports* outputSummary = new HltSelReports();
  put( outputSummary, m_outputHltSelReportsLocation );

  // create output container for Object Summaries and put it on TES
  m_objectSummaries = new HltObjectSummary::Container();
  put( m_objectSummaries, m_outputHltSelReportsLocation.value() + "/Candidates" );
  
  const HltDecReports* decReports(0);
  // get input
  if( exist<HltDecReports>(m_inputHltDecReportsLocation) ){    
    decReports = get<HltDecReports>(m_inputHltDecReportsLocation);
  }  else {
    Warning( " No HltDecReports at " + m_inputHltDecReportsLocation.value(), StatusCode::SUCCESS, 10 );
  }


#ifdef DEBUGCODE
  if ( msgLevel(MSG::VERBOSE) ){
    verbose() <<" Selection Names Found =" ;
    for( std::vector<stringKey>::const_iterator i=selectionIDs.begin();i!=selectionIDs.end();++i){
      verbose() << i->str() << " ";
    }
    verbose() << endmsg;
  }
#endif  
 

  // data compression requires that we store objects from early processing stages first
  // order selections accordingly 
  //     filtering of selections for persistency also happens in this loop

  std::vector< RankedSelection > sortedSelections;
 
  // loop over selection summaries in HltSummary
  int i(0);
  for( std::vector<stringKey>::const_iterator is=m_selectionIDs.begin();is!=m_selectionIDs.end();++is,++i){
     const stringKey name(*is);
     const std::string selName(name.str()); 

     // see if marked for persistency
     //     int maxCandidates = maximumNumberOfCandidatesToStore( selName );      
     int maxCandidates = m_debugMode ? m_maxCandDebug[i] :  m_maxCand[i];

     if( !maxCandidates )continue;

     // find first candidate for ranking
     ContainedObject* candidate(0);
     
     // try dataSvc first
     const Hlt::Selection* sel = m_hltDataSvc->selection(name,this);
     if ( sel != 0 ) {

#ifdef DEBUGCODE
        if ( msgLevel(MSG::VERBOSE) ){
          verbose() << " Selection " << name.str() <<  " found in dataSvc decison=" << sel->decision() << endmsg;          
        }
#endif
       // unsuccessful selections can't save candidates
       if( !sel->decision() )continue;
       
       if( sel->classID() == LHCb::Track::classID() ) {

         const Hlt::TrackSelection& tsel = dynamic_cast<const Hlt::TrackSelection&>(*sel);   
         if( tsel.begin()!=tsel.end() ){
           candidate = (ContainedObject*)(*(tsel.begin()));
         }

       } else if( sel->classID() == LHCb::RecVertex::classID() ) {

         const Hlt::VertexSelection& tsel = dynamic_cast<const Hlt::VertexSelection&>(*sel);   
         if( tsel.begin()!=tsel.end() ){
           candidate = (ContainedObject*)(*(tsel.begin()));
         }

       } else if( sel->classID() == LHCb::Particle::classID() ) {

         const Hlt::TSelection<LHCb::Particle>& psel = dynamic_cast<const Hlt::TSelection<LHCb::Particle>&>(*sel);   
         if( psel.begin()!=psel.end() ){
           candidate = (ContainedObject*)(*(psel.begin()));
         }

       } else {
         
         if( sel->classID()==1 )continue; // skip selections of selections for now
         
         std::ostringstream mess;
         mess << " Unsupported data type CLID=" <<  sel->classID() << " - skip selection ID=" +selName;
         Warning( mess.str(),StatusCode::SUCCESS, 10 );
         continue;
         
       }
       
     } 

     // empty selections have nothing to save
     if( !candidate )continue;
       
#ifdef DEBUGCODE
        if ( msgLevel(MSG::VERBOSE) ){
          verbose() << "Selection " << name.str() <<  " has candidates " << endmsg;
        } 
#endif

    // save selection ---------------------------

     // int selection id
     int intSelID = m_selectionIntIDs[i];
     if( !intSelID ){
       Warning( " selectionName="+selName+" not found in HltANNSvc. Skipped. " , StatusCode::SUCCESS, 10 );
       continue;
     }
 
     setPresentInfoLevel( selName );

     // classify according to first candidate
     int rank(0);
     // go through all cases of supported candidates
     if (  !dispatch<Track>    (candidate,this, &HltSelReportsMaker::rankTrack,       rank)
        && !dispatch<RecVertex>(candidate,this, &HltSelReportsMaker::rankRecVertex,   rank)
        && !dispatch<Particle> (candidate,this, &HltSelReportsMaker::rankParticle,    rank)
        && !dispatch<CaloCluster> (candidate,this, &HltSelReportsMaker::rankCaloCluster, rank) )
         Warning( " Unsupported data type among candidates - skip selection ID=" 
                          +selName,StatusCode::SUCCESS, 10 );

     stringint selNameInt(selName,i);
     sortedSelections.push_back( RankedSelection(rank,selNameInt) );     
  }
  
  if( 0==sortedSelections.size() )return StatusCode::SUCCESS;

  std::sort( sortedSelections.begin(), sortedSelections.end(), rankSelLess() );

  // loop over ranked selection summaries in HltSummary
  for( std::vector<RankedSelection>::const_iterator is=sortedSelections.begin(); is!=sortedSelections.end(); ++is ){
    //    info() << " Selection " << is->second << " rank " << is->first << endmsg;
    
     const std::string selName(is->second.first);     
     const stringKey name(selName);
     const int i=is->second.second;

     std::vector<ContainedObject*> candidates;
     
     // try dataSvc first
     const Hlt::Selection* sel = m_hltDataSvc->selection(name,this);
     if ( sel != 0 ) {


       // unsuccessful selections can't save candidates
       if( !sel->decision() )continue;
       
       if( sel->classID() == LHCb::Track::classID() ) {
         
         const Hlt::TrackSelection& tsel = dynamic_cast<const Hlt::TrackSelection&>(*sel);   
         for (Hlt::TrackSelection::const_iterator it = tsel.begin(); it != tsel.end(); ++it) {
           candidates.push_back( (ContainedObject*)(*it) );
         }

       } else if( sel->classID() == LHCb::RecVertex::classID() ) {

         const Hlt::VertexSelection& tsel = dynamic_cast<const Hlt::VertexSelection&>(*sel);   
         for (Hlt::VertexSelection::const_iterator it = tsel.begin(); it != tsel.end(); ++it) {
           candidates.push_back( (ContainedObject*)(*it) );
         }

       } else if( sel->classID() == LHCb::Particle::classID() ) {

         const Hlt::TSelection<LHCb::Particle>& psel = dynamic_cast<const Hlt::TSelection<LHCb::Particle>&>(*sel);   
         for (Hlt::TSelection<LHCb::Particle>::const_iterator it = psel.begin(); it != psel.end(); ++it) {
          candidates.push_back( (ContainedObject*)(*it) );
         }

       } else {
         
         Warning( " Unsupported data type  - skip selection ID=" 
                  +selName,StatusCode::SUCCESS, 10 );
         continue;
         
       }
       
     }

     // trim number of candidates if too large
     int noc = candidates.size();
     // int maxCandidates = maximumNumberOfCandidatesToStore( selName );      
     int maxCandidates = m_debugMode ? m_maxCandDebug[i] :  m_maxCand[i];
     noc = (noc>maxCandidates)?maxCandidates:noc;

     // don't bother if duplicate selection 
     if( outputSummary->hasSelectionName(selName) ){
         Warning( " duplicate selection ignored selectionName=" + selName,StatusCode::SUCCESS, 10 );
         continue;        
     }

     // save selection ---------------------------

     // int selection id
     int intSelID = m_selectionIntIDs[i];

     HltObjectSummary selSumOut;    
     selSumOut.setSummarizedObjectCLID( 1 ); // use special CLID for selection summaries (lowest number for sorting to the end)
     
     // integer selection id 
     selSumOut.addToInfo("0#SelectionID",float(intSelID));
    
     setPresentInfoLevel( selName );
  
     // must also save candidates 
     int nocc(0);

     // save PV association info with selection if found
     std::string pvSelectionName("");
     std::vector<unsigned int> packedPVKeys;
     unsigned int nPackedPVKeys(0);
     unsigned int iWord(0);
     Particle2Vertex::Table* table(0);

     for (std::vector<ContainedObject*>::const_iterator ic = candidates.begin();
          ic != candidates.end(); ++ic) {

       ++nocc;
       if( nocc>noc )break;

       const HltObjectSummary* hos(0);
       // go through all cases of supported candidates
       Track* candi = dynamic_cast<Track*>(*ic);
       if( candi ){
         hos = storeTrack( candi );
       } else {
         RecVertex* candi = dynamic_cast<RecVertex*>(*ic);
         if( candi ){
           hos = storeRecVertex( candi );
         } else {
           Particle* candi = dynamic_cast<Particle*>(*ic);
           if( candi ){
             hos = storeParticle( candi );

             if( kStandardInfoLevel &  m_presentInfoLevelSelection & m_presentInfoLevel ){ 
               if( pvSelectionName != "_" ){
                 bool keyFound = false;             
                 const DataObject* container = candi->parent();
                 if( container ){
                   IRegistry* registry = container->registry() ;
                   if( registry ){
                     std::string path = registry->identifier() ;
                     boost::replace_last(path,"/Particles","/Particle2VertexRelations");
                     if( !table ){                     
                       if( exist< Particle2Vertex::Table >(path) ){
                         table = get< Particle2Vertex::Table >(path);
                       }
                     }
                     if( table ){                     
                       Particle2Vertex::Table::Range rels= table->relations( candi );
                       if( rels.size() ){
                         const VertexBase* pv = (rels[0]).to();
                         if(  !(pvSelectionName.length()) ){
                           const DataObject* containerPV = pv->parent();
                           if( containerPV ){
                             IRegistry* registryPV = containerPV->registry() ;
                             if( registryPV ){
                               std::string pathPV = registryPV ->identifier();
                               std::size_t found = pathPV.find_last_of("/");
                               if( found != std::string::npos ){
                                 pvSelectionName = pathPV.substr(found+1);
                               } else {
                                 pvSelectionName = pathPV;
                               }
                             }
                           }
                         }
                         unsigned int key( pv->key() );
                         key = ( key < 255 ) ? key : 254;
                         ++nPackedPVKeys;
                         if( 5 == nPackedPVKeys ){
                           nPackedPVKeys = 1;
                           packedPVKeys.push_back( iWord );
                           iWord = 0;
                         }
                         iWord |= ( key >> ( nPackedPVKeys-1) );
                         keyFound = true;                     
                       }
                     }
                   }
                 }
                 if( !keyFound && pvSelectionName.length() ){
                   Warning("Only some particles in the trigger selection " +
                           selName + " have PV association - none stored", 
                           StatusCode::SUCCESS, 10 );                 
                   pvSelectionName = "_";               
                 }
               }
             }
             
           } else { 
             CaloCluster* candi = dynamic_cast<CaloCluster*>(*ic);
             if( candi ){
               hos = storeCaloCluster( candi );
             } else { 
               Warning(" Unsupported data type in Hlt Trigger Summary - skip remaining candidates too, selection ID=" 
                       +selName,StatusCode::SUCCESS, 10 );
               break;       
             }
           }
         }
       }
       if( !hos ){
         Warning(" Could not store supported candidate - skip remaining candidates too ",StatusCode::SUCCESS, 10 );
         break;
       } 
       selSumOut.addToSubstructure(hos);
     }

     // insert PV relations if any
     if( pvSelectionName.length()  ){
       packedPVKeys.push_back( iWord );
       int intPVSelID=0;       
       if( pvSelectionName == m_lastPVSelectionName ){
         intPVSelID = m_intPVSelID;
       } else {
         std::vector<IANNSvc::minor_value_type> hlt1 = m_hltANNSvc->items("Hlt1SelectionID");
         for( std::vector<IANNSvc::minor_value_type>::const_iterator si=hlt1.begin();
              si!=hlt1.end();++si){
           if( si->first == pvSelectionName ){
             intPVSelID=si->second;
             m_lastPVSelectionName = pvSelectionName;
             m_intPVSelID = intPVSelID;
             break;
           }
         }
         if( !intPVSelID ){           
           std::vector<IANNSvc::minor_value_type> hlt2 = m_hltANNSvc->items("Hlt2SelectionID");
           for( std::vector<IANNSvc::minor_value_type>::const_iterator si=hlt2.begin();
                si!=hlt2.end();++si){
             if( si->first == pvSelectionName ){
               intPVSelID=si->second;
               m_lastPVSelectionName = pvSelectionName;
               m_intPVSelID = intPVSelID;
               break;
             }
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
         bool notZero = false;         
         for( std::vector<unsigned int>::iterator pWord = packedPVKeys.begin();
              pWord != packedPVKeys.end(); ++pWord ){
           if( *pWord ){
             notZero = true;
             break;
           }
         }
         // store PV keys only if at least one was not zero
         if( notZero ){           
           int j=0;         
           for( std::vector<unsigned int>::iterator pWord = packedPVKeys.begin();
                pWord != packedPVKeys.end(); ++pWord, ++j ){
             std::stringstream ss;
             ss << "11#" << boost::format("%1$=08X") % j;
             union IntFloat { unsigned int mInt; float mFloat; };
             IntFloat a; a.mInt = *pWord;
             selSumOut.addToInfo( ss.str(), a.mFloat );              
           }         
         }         
       }
     }

     // insert selection into the container
     if( outputSummary->insert(selName,selSumOut) == StatusCode::FAILURE ){
       Warning(" Failed to add Hlt selection name "+selName
               +" to its container ",StatusCode::SUCCESS, 10 );

     }    
     
  }
  
  // -------------------------------------------------------------------------------------
  // clone HltObjectSummary-s of selection and insert them into object store (needed for raw data storage
  // and global selections)
  // -------------------------------------------------------------------------------------
  for( HltSelReports::Container::const_iterator i=outputSummary->begin();i!=outputSummary->end();++i){
    const HltObjectSummary& sHos= i->second;  
    HltObjectSummary* hos= new HltObjectSummary( sHos );    
    m_objectSummaries->push_back(hos);
  }


  // -------------------------------------------------------------------------------------
  // create global selections ------------------------------------------------------------
  // -------------------------------------------------------------------------------------
  if( !outputSummary->hasSelectionName("Hlt1Global") ){

     HltObjectSummary selSumOut;    
     selSumOut.setSummarizedObjectCLID( 1 ); // use special CLID for selection summaries (lowest number for sorting to the end)
     
     // integer selection id 
     selSumOut.addToInfo("0#SelectionID",float(kHlt1GlobalID));

     // see which decisions contributed to it
     for( HltSelReports::Container::const_iterator it=outputSummary->begin();it!=outputSummary->end();++it){
       const std::string & selName = it->first;
       if( ( selName.find("Hlt1") == 0 ) && 
           ( selName.find("Decision") != std::string::npos ) ) {         
         const LHCb::HltObjectSummary& selRep = it->second;
         // must find corresponding HltObjectSummary in the object store
         HltObjectSummary::Info::const_iterator i = selRep.numericalInfo().find("0#SelectionID");
         if( i!=selRep.numericalInfo().end() ){               
           int id = (int)(i->second+0.1);
           for( HltObjectSummary::Container::const_iterator pObj=m_objectSummaries->begin();
                pObj!=m_objectSummaries->end();++pObj){
             if( (*pObj)->summarizedObjectCLID()!=1 )continue; 
             HltObjectSummary::Info::const_iterator j=(*pObj)->numericalInfo().find("0#SelectionID");
             if( j!=(*pObj)->numericalInfo().end() ){
               if( id == (int)(j->second+0.1) ){
                 // must also check its decision in HltDecReports since it might have been killed by postscale
                 if( decReports ){
                   const HltDecReport* decReport = decReports->decReport(selName);
                   if( decReport ){
                     if( !(decReport->decision()) )break;
                   }
                 }                 
                 selSumOut.addToSubstructure( (const SmartRef<HltObjectSummary>)(*pObj) );
                 break;                     
               }
             } else {
               Error(" (2) HltObjectSummary of selection-summary-type has no SelectionID info ",StatusCode::SUCCESS,20);
             }
           }
         } else {               
           Error(" (1) HltObjectSummary of selection-summary-type has no SelectionID info ",StatusCode::SUCCESS,20);
         }
       }
     }
     if( selSumOut.substructure().size() ){
       
       // insert selection into the container
       if( outputSummary->insert("Hlt1Global",selSumOut) == StatusCode::FAILURE ){
         Error( "  Failed to add Hlt selection name Hlt1Global to its container ", StatusCode::SUCCESS, 10 );
       } 
       HltObjectSummary* hos= new HltObjectSummary(  selSumOut );    
       m_objectSummaries->push_back(hos);
       
       
     }
  }

  if( !outputSummary->hasSelectionName("Hlt2Global") ){

     HltObjectSummary selSumOut;    
     selSumOut.setSummarizedObjectCLID( 1 ); // use special CLID for selection summaries (lowest number for sorting to the end)
     
     // integer selection id 
     selSumOut.addToInfo("0#SelectionID",float(kHlt2GlobalID));

     // see which decisions contributed to it
     for( HltSelReports::Container::const_iterator it=outputSummary->begin();it!=outputSummary->end();++it){
       const std::string & selName = it->first;
       if( ( selName.find("Hlt2") == 0 ) && 
           ( selName.find("Decision") != std::string::npos ) ) {         
         const LHCb::HltObjectSummary& selRep = it->second;
         // must find corresponding HltObjectSummary in the object store
         HltObjectSummary::Info::const_iterator i = selRep.numericalInfo().find("0#SelectionID");
         if( i!=selRep.numericalInfo().end() ){               
           int id = (int)(i->second+0.1);
           for( HltObjectSummary::Container::const_iterator pObj=m_objectSummaries->begin();
                pObj!=m_objectSummaries->end();++pObj){
             if( (*pObj)->summarizedObjectCLID()!=1 )continue; 
             HltObjectSummary::Info::const_iterator j=(*pObj)->numericalInfo().find("0#SelectionID");
             if( j!=(*pObj)->numericalInfo().end() ){
               if( id == (int)(j->second+0.1) ){
                 // must also check its decision in HltDecReports since it might have been killed by postscale
                 if( decReports ){
                   const HltDecReport* decReport = decReports->decReport(selName);
                   if( decReport ){
                     if( !(decReport->decision()) )break;
                   }
                 }                 
                 selSumOut.addToSubstructure( (const SmartRef<HltObjectSummary>)(*pObj) );
                 break;                     
               }
             } else {
               Error(" (2) HltObjectSummary of selection-summary-type has no SelectionID info ",StatusCode::SUCCESS,20);
             }
           }
         } else {               
           Error(" (1) HltObjectSummary of selection-summary-type has no SelectionID info ",StatusCode::SUCCESS,20);
         }
       }
     }
     if( selSumOut.substructure().size() ){
       
       // insert selection into the container
       if( outputSummary->insert("Hlt2Global",selSumOut) == StatusCode::FAILURE ){
         Error( "  Failed to add Hlt selection name Hlt2Global to its container ", StatusCode::SUCCESS, 10 );
       } 
       HltObjectSummary* hos= new HltObjectSummary(  selSumOut );    
       m_objectSummaries->push_back(hos);
       
       
     }
  }

  // ----------------- printout ------------------------
  if ( msgLevel(MSG::VERBOSE) ){

   verbose() << " ======= HltSelReports size= " << outputSummary->size() << endmsg;  
   verbose() << *outputSummary << endmsg;

   verbose() << " ======= HltObjectSummary container size= " << m_objectSummaries->size() << endmsg;
   for( HltObjectSummary::Container::const_iterator ppHos=m_objectSummaries->begin();
        ppHos!=m_objectSummaries->end();++ppHos){
     const HltObjectSummary* pHos=*ppHos;    
     verbose() << " key " << pHos->index() << *pHos << endmsg;    
   }
   
  }
  
  return StatusCode::SUCCESS;
}

// -------------------------------------
// store Track in HltObjectSummary store
// -------------------------------------
const LHCb::HltObjectSummary* HltSelReportsMaker::storeTrack(const LHCb::Track* object)
{
  if( !object )return (HltObjectSummary*)(0);  
  for( HltObjectSummarys::const_iterator ppHos=m_objectSummaries->begin();
       ppHos!=m_objectSummaries->end();++ppHos){
    const HltObjectSummary* pHos(*ppHos);    
    if( pHos->summarizedObjectCLID() == object->clID() ){
      if( pHos->summarizedObject() == object ){
        return pHos;
      }
    }
  }
  // not yet in object store; create new one
  HltObjectSummary* hos = new HltObjectSummary();
  hos->setSummarizedObjectCLID( object->clID() );
  hos->setSummarizedObject(object);
 
  // find mutations of the same object
  HltObjectSummary::Info theseInfo = infoToSave( hos );
  std::vector<LHCbID> theseHits = object->lhcbIDs() ;
  std::vector<LHCbID>::const_iterator rbegin1=theseHits.end(); --rbegin1;
  std::vector<LHCbID>::const_iterator rend1=theseHits.begin(); --rend1;        
  
  //    look for objects with the same lhcbids
  //      if many then the one which leaves least amount info on this one
  int smallestSize = theseInfo.size();
  const HltObjectSummary* pBestHos(0);

  //   couldn't get reverse operator to work
  HltObjectSummarys::const_iterator rbegin=m_objectSummaries->end();  --rbegin;
  HltObjectSummarys::const_iterator rend  =m_objectSummaries->begin(); --rend;  
  for( HltObjectSummarys::const_iterator ppHos=rbegin;ppHos!=rend;--ppHos){
    const HltObjectSummary* pHos(*ppHos);    
    if( pHos->summarizedObjectCLID() == object->clID() ){
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
          for( HltObjectSummary::Info::const_iterator i1=otherInfo.begin();i1!=otherInfo.end();++i1 ){
            HltObjectSummary::Info::const_iterator i2 = theseInfo.find( i1->first );
            if( i2==theseInfo.end() ){
              notcontained = true;
              break;
            }
            if( i1->second != i2->second ){
              notcontained = false;
              break;
            }
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
        HltObjectSummary::Info otherInfo = pBestHos->numericalInfoFlattened(); 
        for( HltObjectSummary::Info::const_iterator i1=otherInfo.begin();i1!=otherInfo.end();++i1 ){
          theseInfo.erase( i1->first );
        }
    }
    // lhcbids are stored at the botton object
    theseHits.clear();
    // set subobject link
    hos->addToSubstructure( pBestHos );          
  }
  
  hos->setLhcbIDs( theseHits );
  hos->setNumericalInfo( theseInfo );

  m_objectSummaries->push_back(hos);
  return hos;  
}


// -------------------------------------
// store RecVertex in HltObjectSummary store
// -------------------------------------
const LHCb::HltObjectSummary* HltSelReportsMaker::storeRecVertex(const LHCb::RecVertex* object)
{  
  if( !object )return (HltObjectSummary*)(0);  
  for( HltObjectSummarys::const_iterator ppHos=m_objectSummaries->begin();
       ppHos!=m_objectSummaries->end();++ppHos){
    const HltObjectSummary* pHos(*ppHos);    
    if( pHos->summarizedObjectCLID() == object->clID() ){
      if( pHos->summarizedObject() == object ){
        return pHos;
      }
    }
  }  
  // not yet in object store; create new one
  HltObjectSummary* hos = new HltObjectSummary();
  hos->setSummarizedObjectCLID( object->clID() );
  hos->setSummarizedObject(object);

  // find mutations of the same object
  HltObjectSummary::Info theseInfo = infoToSave( hos );

  SmartRefVector<LHCb::HltObjectSummary> thisSubstructure;  
  const SmartRefVector<LHCb::Track> & tracks = object->tracks();
  for( SmartRefVector<LHCb::Track>::const_iterator it = tracks.begin();it != tracks.end(); ++it) {
    const Track* track = *it;    
    thisSubstructure.push_back( storeTrack(track) );
  }
  SmartRefVector<LHCb::HltObjectSummary>::const_iterator rbegin1=thisSubstructure.end(); --rbegin1;
  SmartRefVector<LHCb::HltObjectSummary>::const_iterator rend1=thisSubstructure.begin(); --rend1;        

  //    look for objects with the same lhcbids
  //      if many then the one which leaves least amount info on this one
  int smallestSize = theseInfo.size();
  const HltObjectSummary* pBestHos(0);

  //   couldn't get reverse operator to work
  HltObjectSummarys::iterator rbegin=m_objectSummaries->end();  --rbegin;
  HltObjectSummarys::iterator rend  =m_objectSummaries->begin(); --rend;  
  for( HltObjectSummarys::iterator ppHos=rbegin;ppHos!=rend;--ppHos){
    HltObjectSummary* pHos(*ppHos);    
    if( pHos->summarizedObjectCLID() == object->clID() ){
      SmartRefVector<LHCb::HltObjectSummary> otherSubstructure = pHos->substructureFlattened();      
      bool different=false;      
      if( otherSubstructure.size() == thisSubstructure.size() ){
        SmartRefVector<LHCb::HltObjectSummary>::const_iterator h2=otherSubstructure.end(); --h2;        
        for( SmartRefVector<LHCb::HltObjectSummary>::const_iterator h1=rbegin1;h1!=rend1;--h1,--h2){
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
          for( HltObjectSummary::Info::const_iterator i1=otherInfo.begin();i1!=otherInfo.end();++i1 ){
            HltObjectSummary::Info::const_iterator i2 = theseInfo.find( i1->first );
            if( i2==theseInfo.end() ){
              notcontained = true;
              break;
            }
            if( i1->second != i2->second ){
              notcontained = false;
              break;
            }
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
        HltObjectSummary::Info otherInfo = pBestHos->numericalInfoFlattened(); 
        for( HltObjectSummary::Info::const_iterator i1=otherInfo.begin();i1!=otherInfo.end();++i1 ){
          theseInfo.erase( i1->first );
        }
    }
    // set subobject link
    thisSubstructure.clear();
    // set substructure link
    thisSubstructure.push_back( pBestHos );    
  }
   
  hos->setSubstructure( thisSubstructure );
  hos->setNumericalInfo( theseInfo );
  m_objectSummaries->push_back(hos);
  return hos;  
}

// -------------------------------------------
// store CaloCluster in HltObjectSummary store
// -------------------------------------------
const LHCb::HltObjectSummary* HltSelReportsMaker::storeCaloCluster(const LHCb::CaloCluster* object)
{
  if( !object )return (HltObjectSummary*)(0);  
  for( HltObjectSummarys::const_iterator ppHos=m_objectSummaries->begin();
       ppHos!=m_objectSummaries->end();++ppHos){
    const HltObjectSummary* pHos(*ppHos);    
    if( pHos->summarizedObjectCLID() == object->clID() ){
      if( pHos->summarizedObject() == object ){
        return pHos;
      }
    }
  }  
  HltObjectSummary* hos = new HltObjectSummary();
  hos->setSummarizedObjectCLID( object->clID() );
  hos->setSummarizedObject(object);
  
  std::vector<LHCbID> clusterSeed; clusterSeed.push_back(object->seed());  
  hos->setLhcbIDs( clusterSeed );

  m_objectSummaries->push_back(hos);
  return hos;  
}


// ----------------------------------------
// store Particle in HltObjectSummary store
// ----------------------------------------
const LHCb::HltObjectSummary* HltSelReportsMaker::storeParticle(const LHCb::Particle* object)
{  
  if( !object )return (HltObjectSummary*)(0);  
  for( HltObjectSummarys::const_iterator ppHos=m_objectSummaries->begin();
       ppHos!=m_objectSummaries->end();++ppHos){
    const HltObjectSummary* pHos(*ppHos);    
    if( pHos->summarizedObjectCLID() == object->clID() ){
      if( pHos->summarizedObject() == object ){
        return pHos;
      }
    }
  }  
  HltObjectSummary* hos = new HltObjectSummary();
  hos->setSummarizedObjectCLID( object->clID() );
  hos->setSummarizedObject(object);  

  HltObjectSummary::Info theseInfo = infoToSave( hos );

  // no subobject code implemented since so fat Hlt2 does not save intermediate selections

  std::vector<const LHCb::Particle*> daughters = object->daughtersVector();
  if( daughters.size() >0 ){    
    for( std::vector<const LHCb::Particle*>::const_iterator p = daughters.begin(); 
         p!=daughters.end(); ++p){
      hos->addToSubstructure( storeParticle( *p ) );
    }
  } else {
    // particles with no daughters have substructure via ProtoParticle 
    // we don't save Protoparticles, only things they lead to
    const ProtoParticle* pp = object->proto();
    if( !pp ){
      Warning(" Particle with no daughters and no protoparticle, skipped ",StatusCode::SUCCESS, 10 );
    } else {
      const Track* track=pp->track();
      if( track ){
        // charged track particle
        hos->addToSubstructure( storeTrack( track ) );
        // if muon add muon stub too        
        const LHCb::MuonPID* muid = pp->muonPID();
        if( muid!=0 && object->particleID().abspid()==13 ){
          if( muid->IsMuon() ){
            if( !m_HLTmuonTracks ){ 
              if( exist<LHCb::Tracks>(m_HltMuonTracksLocation) ){
                m_HLTmuonTracks = get<LHCb::Tracks>(m_HltMuonTracksLocation);
              } else {
                Warning(" Found track which is a muon but no muon tracks at " + m_HltMuonTracksLocation.value()
                        ,StatusCode::SUCCESS,10 );
              }
            }
            if( m_HLTmuonTracks ){          
              const Track* muStub= m_HLTmuonTracks->object(track->key());     
              if( muStub ){
                hos->addToSubstructure( storeTrack( muStub ) );
              }
            }
          }
        }
      } else {
        // neutral particle ?
        // Ecal via CaloHypo
        const SmartRefVector< LHCb::CaloHypo > &caloVec = pp->calo();
        if( caloVec.size() > 0 ){
          const LHCb::CaloHypo*   hypo  = *(caloVec.begin());
          if( LHCb::CaloHypo::Photon == hypo->hypothesis() ){
            // Photon
            const CaloCluster* cluster = *(hypo->clusters().begin());          
            hos->addToSubstructure( storeCaloCluster( cluster ) );
          } else if (  LHCb::CaloHypo::Pi0Merged == hypo->hypothesis() ){
            // Split Photons
            const SmartRefVector<LHCb::CaloHypo>& hypos = hypo->hypos();
            const LHCb::CaloHypo* g1 = *(hypos.begin() );
            const CaloCluster* cluster1 = *(g1->clusters().begin());
            hos->addToSubstructure( storeCaloCluster( cluster1 ) );
            const LHCb::CaloHypo* g2 = *(hypos.begin()+1 );
            const CaloCluster* cluster2 = *(g2->clusters().begin());
            hos->addToSubstructure( storeCaloCluster( cluster2 ) );
          }        
        }
      }
    }    
  }
  
  hos->setNumericalInfo( theseInfo );
  m_objectSummaries->push_back(hos);
  return hos;  
}
 



//=============================================================================
// fetch info to save; to find object follow summerizedObject pointer
//=============================================================================
HltObjectSummary::Info HltSelReportsMaker::infoToSave( const HltObjectSummary* hos ) const
{
  HltObjectSummary::Info infoPersistent;
  if( kMinInfoLevel == m_presentInfoLevel )return infoPersistent; 

  if( !hos )return infoPersistent;  
  if( !(hos->summarizedObject()) )return infoPersistent;  
  switch( hos->summarizedObjectCLID() )
  {
  case LHCb::CLID_Track:
    {      
      const Track* candi = dynamic_cast<const Track*>(hos->summarizedObject());
      if( !candi )return infoPersistent; 
      if( kExtraInfoLevel &  m_presentInfoLevelTrack & m_presentInfoLevel ){ 
	const Track::ExtraInfo & eInfo = candi->extraInfo();
	for( Track::ExtraInfo::const_iterator ei=eInfo.begin();ei!=eInfo.end();++ei){
          if( ( 0<= ei->first ) && ( ei->first<=65535 ) ){
	    GaudiUtils::VectorMap< int, std::string >::const_iterator i=m_infoIntToName.find(ei->first);
	    if( i!=m_infoIntToName.end() ){
	      infoPersistent.insert( i->second, float( ei->second ) );
	    }
	  }
	}
      }
      if( kStandardInfoLevel &  m_presentInfoLevelTrack & m_presentInfoLevel ){ 
        if( candi->nStates() ){
          const State & firstState = candi->firstState();
          infoPersistent.insert( "0#Track.firstState.z", float( firstState.z() ) );
          infoPersistent.insert( "1#Track.firstState.x", float( firstState.x() ) );
          infoPersistent.insert( "2#Track.firstState.y", float( firstState.y() ) );
          infoPersistent.insert( "3#Track.firstState.tx", float( firstState.tx() ) );
          infoPersistent.insert( "4#Track.firstState.ty", float( firstState.ty() ) );
          infoPersistent.insert( "5#Track.firstState.qOverP", float( firstState.qOverP() ) );
        }
      }    
    }    
    break;
  case LHCb::CLID_RecVertex:
    {      
      const RecVertex* candi = dynamic_cast<const RecVertex*>(hos->summarizedObject());
      if( !candi )return infoPersistent; 
      if( kExtraInfoLevel &  m_presentInfoLevelRecVertex & m_presentInfoLevel ){
	const RecVertex::ExtraInfo & eInfo = candi->extraInfo();
	for( RecVertex::ExtraInfo::const_iterator ei=eInfo.begin();ei!=eInfo.end();++ei){
          if( ( 0<= ei->first ) && ( ei->first<=65535 ) ){
	    GaudiUtils::VectorMap< int, std::string >::const_iterator i=m_infoIntToName.find(ei->first);
	    if( i!=m_infoIntToName.end() ){
	      infoPersistent.insert( i->second, float( ei->second ) );
	    }
	  }
	}
      }
      if( kStandardInfoLevel &  m_presentInfoLevelRecVertex & m_presentInfoLevel ){ 
        infoPersistent.insert( "0#RecVertex.position.x", float( candi->position().x() ) );
        infoPersistent.insert( "1#RecVertex.position.y", float( candi->position().y() ) );
        infoPersistent.insert( "2#RecVertex.position.z", float( candi->position().z() ) );
      }      
    }    
    break;
  case LHCb::CLID_Particle:
    {      
      const Particle* candi = dynamic_cast<const Particle*>(hos->summarizedObject());
      if( !candi )return infoPersistent; 
      if( kExtraInfoLevel &  m_presentInfoLevelParticle & m_presentInfoLevel ){
	const Particle::ExtraInfo & eInfo = candi->extraInfo();
	for( Particle::ExtraInfo::const_iterator ei=eInfo.begin();ei!=eInfo.end();++ei){
          if( ( 0<= ei->first ) && ( ei->first<=65535 ) ){
	    GaudiUtils::VectorMap< int, std::string >::const_iterator i=m_infoIntToName.find(ei->first);
	    if( i!=m_infoIntToName.end() ){
	      infoPersistent.insert( i->second, float( ei->second ) );
	    }
	  }
	}
      }
      if( kStandardInfoLevel &  m_presentInfoLevelParticle & m_presentInfoLevel ){ 
        infoPersistent.insert( "0#Particle.particleID.pid", float( candi->particleID().pid() ) );
        infoPersistent.insert( "1#Particle.measuredMass", float( candi->measuredMass() ) );
        infoPersistent.insert( "2#Particle.referencePoint.z", float( candi->referencePoint().z() ) );
        infoPersistent.insert( "3#Particle.referencePoint.x", float( candi->referencePoint().x() ) );
        infoPersistent.insert( "4#Particle.referencePoint.y", float( candi->referencePoint().y() ) );
        infoPersistent.insert( "5#Particle.slopes.x", float( candi->slopes().x() ) );
        infoPersistent.insert( "6#Particle.slopes.y", float( candi->slopes().y() ) );
        double p = candi->p();
        if( p < 1E-9 )p=1E-9;        
        infoPersistent.insert( "7#Particle.1/p", float( 1.0/p ) );
      }      
    }
    break;
  case LHCb::CLID_CaloCluster:
    {      
      const CaloCluster* candi = dynamic_cast<const CaloCluster*>(hos->summarizedObject());
      if( !candi )return infoPersistent; 
      if( kStandardInfoLevel &  m_presentInfoLevelCaloCluster & m_presentInfoLevel ){ 
        infoPersistent.insert( "0#CaloCluster.e", float( candi->e() ) );
        infoPersistent.insert( "1#CaloCluster.position.x", float( candi->position().x() ) );
        infoPersistent.insert( "2#CaloCluster.position.y", float( candi->position().y() ) );
        infoPersistent.insert( "3#CaloCluster.position.z", float( candi->position().z() ) );
      }      
    }
    break;
  default:
    { }    
  }
  return infoPersistent;
}

int HltSelReportsMaker::rankTrack(const LHCb::Track* object) const
{
  if( !object )return 0;
  HltObjectSummary* hos = new HltObjectSummary();
  hos->setSummarizedObjectCLID( object->clID() );
  hos->setSummarizedObject(object);  
  int rank=100*rankLHCbIDs( object->lhcbIDs() ) +  infoToSave( hos ).size();
  delete hos;  
  return rank;
}

int HltSelReportsMaker::rankCaloCluster(const LHCb::CaloCluster* object) const
{
  if( !object )return 0;
  HltObjectSummary* hos = new HltObjectSummary();
  hos->setSummarizedObjectCLID( object->clID() );
  hos->setSummarizedObject(object);  
  std::vector<LHCbID> clusterSeed; clusterSeed.push_back(object->seed()); 
  int rank=100*rankLHCbIDs( clusterSeed ) +  infoToSave( hos ).size();
  delete hos;  
  return rank;
}

int HltSelReportsMaker::rankRecVertex(const LHCb::RecVertex* object) const
{
  if( !object )return 0;
  HltObjectSummary* hos = new HltObjectSummary();
  hos->setSummarizedObjectCLID( object->clID() );
  hos->setSummarizedObject(object);  
  int rank=10000 +  infoToSave( hos ).size();
  delete hos;  
  return rank;
}

int HltSelReportsMaker::rankParticle(const LHCb::Particle* object) const
{
  if( !object )return 0;
  HltObjectSummary* hos = new HltObjectSummary();
  hos->setSummarizedObjectCLID( object->clID() );
  hos->setSummarizedObject(object);  
  int rank=100000 +  infoToSave( hos ).size();
  delete hos;  
  return rank;
}

int HltSelReportsMaker::rankLHCbIDs( const std::vector<LHCbID> & lhcbIDs ) const
{
  int rank(0);
  for(  std::vector<LHCbID>::const_iterator i= lhcbIDs.begin();i!= lhcbIDs.end();++i){
    const LHCbID id= *i;    
    if( id.isVelo() ){
      rank = rank | 8;
    } else if( id.isTT() ){
      rank = rank | 16;      
    } else if ( id.isOT() || id.isIT() ){
      rank = rank | 32;      
    } else if ( id.isMuon() ) {
      rank = rank | 4;      
    } else if ( id.isCalo() ) {
      rank = rank | 2;      
    } else {
      rank = rank | 1;
    }
  }
  return rank;  
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
    for( SelectionSetting::const_iterator i=maxCandidatesDebug.begin();
         i!=maxCandidatesDebug.end(); ++i){
      if( selName == i->first ){
        return i->second;
      }
    }
    
    // not set specifically for this selection ; use default
    if( selName.find("Decision")!=std::string::npos ){
      return m_maxCandidatesDecisionDebug;
    }
    return m_maxCandidatesNonDecisionDebug;

  } else {
    
    // NORMAL MODE

    // set max number of candidates
    const SelectionSetting & maxCandidates =  m_maxCandidates.value();    
    for( SelectionSetting::const_iterator i=maxCandidates.begin();
         i!=maxCandidates.end(); ++i){
      if( selName == i->first ){
        return i->second;
      }
    }
    
    // not set specifically for this selection ; use default
    if( selName.find("Decision")!=std::string::npos ){
      return m_maxCandidatesDecision;
    }
    return m_maxCandidatesNonDecision;
    
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
    return;    

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
    return;    
  }

}

    

bool HltSelReportsMaker::rankSelLess::operator()( const HltSelReportsMaker::RankedSelection & elem1, 
                                                  const HltSelReportsMaker::RankedSelection & elem2) const
{  
    if( elem1.first < elem2.first )return true;
    if( elem1.first > elem2.first )return false;
    // equal ranks; now use selection name; decisions come last
    if( elem1.second.first.find("Decision")!=std::string::npos ){
      if( elem2.second.first.find("Decision")!=std::string::npos ){
        return (elem1.second.first.length()<elem2.second.first.length());
      }
      return false;
    } 
    if( elem2.second.first.find("Decision")!=std::string::npos ){
      return true;
    } 
    return (elem1.second.first.length()<elem2.second.first.length());
}
