// $Id: HltSelReportsReader.cpp,v 1.1.1.1 2008-08-02 16:40:07 tskwarni Exp $
// Include files 

// from Gaudi
#include "GaudiKernel/AlgFactory.h" 

#include "Event/HltSelReports.h"
#include "Event/HltObjectSummary.h"
#include "Event/RawEvent.h"

// bank structure
#include "HltSelRepRawBank.h"
#include "HltSelRepRBHits.h"
#include "HltSelRepRBSubstr.h"
#include "HltSelRepRBObjTyp.h"
#include "HltSelRepRBExtraInfo.h"
#include "HltSelRepRBStdInfo.h"

// local
#include "HltSelReportsReader.h"

#include "Event/Track.h"
#include "Event/Particle.h"
#include "Event/RecVertex.h"
#include "Event/CaloCluster.h"


using namespace LHCb;


//-----------------------------------------------------------------------------
// Implementation file for class : HltSelReportsReader
//
// 2008-08-01 : Tomasz Skwarnicki
//-----------------------------------------------------------------------------

// Declaration of the Algorithm Factory
DECLARE_ALGORITHM_FACTORY( HltSelReportsReader );


//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
HltSelReportsReader::HltSelReportsReader( const std::string& name,
                                          ISvcLocator* pSvcLocator)
    : GaudiAlgorithm ( name , pSvcLocator )
  //  : HltBaseAlg( name , pSvcLocator )
{

  declareProperty("OutputHltSelReportsLocation",
    m_outputHltSelReportsLocation= LHCb::HltSelReportsLocation::Default);  
  declareProperty("InputRawEventLocation",
    m_inputRawEventLocation= LHCb::RawEventLocation::Default);

    m_hltANNSvc = 0;

}
//=============================================================================
// Destructor
//=============================================================================
HltSelReportsReader::~HltSelReportsReader() {} 

//=============================================================================
// Initialization
//=============================================================================
StatusCode HltSelReportsReader::initialize() {
  StatusCode sc = GaudiAlgorithm::initialize(); // must be executed first
  // StatusCode sc = HltBaseAlg::initialize(); // must be executed first
  if ( sc.isFailure() ) return sc;  // error printed already by GaudiAlgorithm

  if ( msgLevel(MSG::DEBUG) ) debug() << "==> Initialize" << endmsg;

  m_hltANNSvc = svc<IANNSvc>("HltANNSvc");

  return StatusCode::SUCCESS;
}

//=============================================================================
// Main execution
//=============================================================================
StatusCode HltSelReportsReader::execute() {

  if ( msgLevel(MSG::DEBUG) ) debug() << "==> Execute" << endmsg;


  // get inputs
  if( !exist<RawEvent>(m_inputRawEventLocation) ){    
    error() << " No RawEvent at " << m_inputRawEventLocation << endmsg;
    return StatusCode::SUCCESS;  
  }  
  RawEvent* rawEvent = get<RawEvent>(m_inputRawEventLocation);

  // create output container and put it on TES
  HltSelReports* outputSummary = new HltSelReports();
  put( outputSummary, m_outputHltSelReportsLocation );

  // create output container for Object Summaries and put it on TES
  LHCb::HltObjectSummary::Container* objectSummaries = new HltObjectSummary::Container();
  put( objectSummaries, m_outputHltSelReportsLocation.value() + "/Candidates" );


  // ----------------------------------------------------------
  // get the bank from RawEvent
  // ----------------------------------------------------------

  const std::vector<RawBank*> hltselreportsRawBanks = rawEvent->banks( RawBank::HltSelReports );
  if( !hltselreportsRawBanks.size() ){
    warning() << " No HltSelReports RawBank in RawEvent. Quiting. " << endmsg;
    return StatusCode::SUCCESS;  
  } else if( hltselreportsRawBanks.size() != 1 ){
    warning() << " More then one HltSelReports RawBanks in RawEvent. Will process only the first one. " << endmsg;
  }
  const RawBank* hltselreportsRawBank = *(hltselreportsRawBanks.begin());
  if( hltselreportsRawBank->version() > kVersionNumber ){
  }
  if( hltselreportsRawBank->sourceID() != kSourceID ){
    warning() << " HltSelReports RawBank has unexpected source ID. Will try to decode it anyway." << endmsg;
  }

  unsigned int bankSize = (hltselreportsRawBank->size()+3)/4; // from bytes to words
  // need to copy it to local array to get rid of const restriction
  unsigned int* pBank = new unsigned int[bankSize];
  HltSelRepRawBank hltSelReportsBank( pBank );

  for(unsigned int i=0; i!=bankSize; ++i){
    (*pBank) = hltselreportsRawBank->data()[i]; ++pBank;
  }


  HltSelRepRBHits hitsSubBank( hltSelReportsBank.subBankFromID( HltSelRepRBEnums::kHitsID ) );
  HltSelRepRBObjTyp objTypSubBank( hltSelReportsBank.subBankFromID( HltSelRepRBEnums::kObjTypID ) );
  HltSelRepRBSubstr substrSubBank( hltSelReportsBank.subBankFromID( HltSelRepRBEnums::kSubstrID ) );
  HltSelRepRBStdInfo stdInfoSubBank( hltSelReportsBank.subBankFromID( HltSelRepRBEnums::kStdInfoID ) );
  HltSelRepRBExtraInfo extraInfoSubBank( hltSelReportsBank.subBankFromID( HltSelRepRBEnums::kExtraInfoID ) );

  // ----------------------------------------- integrity checks -------------------------
  bool errors=false;
  bool exInfOn=true;
  unsigned int ic;
  unsigned int nObj = objTypSubBank.numberOfObj();

  if( bankSize < hltSelReportsBank.size() ){
    error() << " HltSelReportsRawBank internally reported size " << hltSelReportsBank.size()
	    << " less than bank size delivered by RawEvent " << bankSize
            << " Quitting! " << endmsg;
    errors=true;

  } else {

  ic = hltSelReportsBank.integrityCode();
  if( ic ){
    error() << " HltSelReportsRawBank fails integrity check with code " 
	    << ic << " " << HltSelRepRBEnums::IntegrityCodesToString(ic) << endmsg;
    errors=true;
  }

  ic = hitsSubBank.integrityCode();
  if( ic ){
    error() << " HltSelRepRBHits fails integrity check with code " 
	    << ic << " " << HltSelRepRBEnums::IntegrityCodesToString(ic) << endmsg;
    errors=true;
  }

  ic = objTypSubBank.integrityCode();
  if( ic ){
    error() << " HltSelRepRBObjTyp fails integrity check with code " 
	    << ic << " " << HltSelRepRBEnums::IntegrityCodesToString(ic) << endmsg;
    errors=true;
  }

  ic = substrSubBank.integrityCode();
  if( ic ){
    error() << " HltSelRepRBSubstr fails integrity check with code " 
	    << ic << " " << HltSelRepRBEnums::IntegrityCodesToString(ic) << endmsg;
    errors=true;
  }
  if( nObj != substrSubBank.numberOfObj() ){
    error() << " HltSelRepRBSubstr has number of objects " 
	    << substrSubBank.numberOfObj()
            << " which is different than HltSelRepRBObjTyp " << nObj << endmsg;
    errors=true;
  }

  ic = stdInfoSubBank.integrityCode();
  if( ic ){
    error() << " HltSelRepRBStdInfo fails integrity check with code " 
	    << ic << " " << HltSelRepRBEnums::IntegrityCodesToString(ic) << endmsg;
    errors=true;
  }
  if( nObj != stdInfoSubBank.numberOfObj() ){
    error() << " HltSelRepRBStdInfo has number of objects " 
	    << stdInfoSubBank.numberOfObj()
            << " which is different than HltSelRepRBObjTyp " << nObj << endmsg;
    errors=true;
  }

  ic = extraInfoSubBank.integrityCode();
  if( ic ){
    error() << " HltSelRepRBExtraInfo fails integrity check with code " 
	    << ic << " " << HltSelRepRBEnums::IntegrityCodesToString(ic) << endmsg;
    exInfOn=false; // the only non-fatal info corruption
  }
  if( nObj != extraInfoSubBank.numberOfObj() ){
    error() << " HltSelRepRBExtraInfo has number of objects " 
	    << extraInfoSubBank.numberOfObj()
            << " which is different than HltSelRepRBObjTyp " << nObj << endmsg;
    exInfOn=false;
  }

  }

  if ( msgLevel(MSG::VERBOSE) ){

    // print created bank and subbanks inside
    verbose() << hltSelReportsBank << endmsg;
    
    verbose() << HltSelRepRBHits( hltSelReportsBank.subBankFromID( HltSelRepRBEnums::kHitsID ) ) << endmsg;
    verbose() << HltSelRepRBObjTyp( hltSelReportsBank.subBankFromID( HltSelRepRBEnums::kObjTypID ) ) << endmsg;
    verbose() << HltSelRepRBSubstr( hltSelReportsBank.subBankFromID( HltSelRepRBEnums::kSubstrID ) ) << endmsg;
    verbose() << HltSelRepRBStdInfo( hltSelReportsBank.subBankFromID( HltSelRepRBEnums::kStdInfoID ) ) << endmsg;
    verbose() << HltSelRepRBExtraInfo( hltSelReportsBank.subBankFromID( HltSelRepRBEnums::kExtraInfoID ) ) << endmsg;

  } 


  if( errors ){
    hltSelReportsBank.deleteBank();
    return StatusCode::SUCCESS;
  }


  std::vector<IANNSvc::minor_value_type> hltinfos = m_hltANNSvc->items("InfoID"); 
  // get string-to-int selection ID map
  std::vector<IANNSvc::minor_value_type> selectionNameToIntMap;  
  //    std::vector<IANNSvc::minor_value_type> hlt = m_hltANNSvc->items("SelectionID"); // old style
  //    selectionNameToIntMap.insert( selectionNameToIntMap.end(),hlt.begin(),hlt.end() );
  std::vector<IANNSvc::minor_value_type> hlt1 = m_hltANNSvc->items("Hlt1SelectionID"); // new style
  selectionNameToIntMap.insert( selectionNameToIntMap.end(),hlt1.begin(),hlt1.end() );
  std::vector<IANNSvc::minor_value_type> hlt2 = m_hltANNSvc->items("Hlt2SelectionID");
  selectionNameToIntMap.insert( selectionNameToIntMap.end(),hlt2.begin(),hlt2.end() );


  // -----------------------------------------------------------------
  // create object summaries
  // -----------------------------------------------------------------

  // put them in local vector until they are finished
  std::vector< HltObjectSummary* > objects;

  for(unsigned int iObj=0; iObj!= nObj; ++iObj){
  
    HltObjectSummary* hos = new HltObjectSummary();

    // =========== class ID
    hos->setSummarizedObjectCLID( objTypSubBank.next() );


    // =========== numerical info
    HltObjectSummary::Info infoPersistent;


    //           ============== standard 
    HltSelRepRBStdInfo::StdInfo stdInfo = stdInfoSubBank.next();
    if( stdInfo.size() )
    switch( hos->summarizedObjectCLID() )
      {
      case LHCb::CLID_Track:
        {      
          if( stdInfo.size()==6 ){

            infoPersistent.insert( "0#Track.firstState.z", floatFromInt(stdInfo[0]) ); 
            infoPersistent.insert( "1#Track.firstState.x", floatFromInt(stdInfo[1]) ); 
            infoPersistent.insert( "2#Track.firstState.y", floatFromInt(stdInfo[2]) ); 
            infoPersistent.insert( "3#Track.firstState.tx", floatFromInt(stdInfo[3]) ); 
            infoPersistent.insert( "4#Track.firstState.ty", floatFromInt(stdInfo[4]) ); 
            infoPersistent.insert( "5#Track.firstState.qOverP", floatFromInt(stdInfo[5]) ); 
	    
          } else {

            warning() << " wrong number of StdInfo on Track " << stdInfo.size() << endmsg;
            for(HltSelRepRBStdInfo::StdInfo::const_iterator i=stdInfo.begin();
                i!=stdInfo.end(); ++i){
              std::stringstream ss;
              ss << "z#Track.unknown" << int(i-stdInfo.begin() );
              infoPersistent.insert( ss.str(), floatFromInt(*i) );        
            }      
          }
        }
        break;
      case LHCb::CLID_RecVertex:
        {  
          if( stdInfo.size()==3 ){    
 
            infoPersistent.insert( "0#RecVertex.position.x", floatFromInt(stdInfo[0]) ); 
            infoPersistent.insert( "1#RecVertex.position.y", floatFromInt(stdInfo[1]) ); 
            infoPersistent.insert( "2#RecVertex.position.z", floatFromInt(stdInfo[2]) ); 

          } else {
            
            warning() << " wrong number of StdInfo on RecVertex " << stdInfo.size() << endmsg;
            for(HltSelRepRBStdInfo::StdInfo::const_iterator i=stdInfo.begin();
                i!=stdInfo.end(); ++i){
              std::stringstream ss;
              ss << "z#RecVertex.unknown" << int(i-stdInfo.begin() );
              infoPersistent.insert( ss.str(), floatFromInt(*i) );        
            }      
            
          }
        }
	break;
      case LHCb::CLID_Particle:
        {      
          if( stdInfo.size()==8 ){    
            
            infoPersistent.insert( "0#Particle.particleID.pid", floatFromInt(stdInfo[0]) ); 
            infoPersistent.insert( "1#Particle.measuredMass", floatFromInt(stdInfo[1]) ); 
            infoPersistent.insert( "2#Particle.referencePoint.z", floatFromInt(stdInfo[2]) ); 
            infoPersistent.insert( "3#Particle.referencePoint.x", floatFromInt(stdInfo[3]) ); 
            infoPersistent.insert( "4#Particle.referencePoint.y", floatFromInt(stdInfo[4]) );
            infoPersistent.insert( "5#Particle.slopes.x", floatFromInt(stdInfo[5]) );
            infoPersistent.insert( "6#Particle.slopes.y", floatFromInt(stdInfo[6]) );
            infoPersistent.insert( "7#Particle.1/p", floatFromInt(stdInfo[7]) );
            
          } else {

            warning() << " wrong number of StdInfo on Particle " << stdInfo.size() << endmsg;
            for(HltSelRepRBStdInfo::StdInfo::const_iterator i=stdInfo.begin();
                i!=stdInfo.end(); ++i){
              std::stringstream ss;
              ss << "z#Particle.unknown" << int(i-stdInfo.begin() );
              infoPersistent.insert( ss.str(), floatFromInt(*i) );        
            }      
            
          }
        }
        break;
      case LHCb::CLID_CaloCluster:
        {      
          
          if( stdInfo.size()==4 ){    
 
            infoPersistent.insert( "0#CaloCluster.e", floatFromInt(stdInfo[0]) );
            infoPersistent.insert( "1#CaloCluster.position.x", floatFromInt(stdInfo[1]) );
            infoPersistent.insert( "2#CaloCluster.position.y", floatFromInt(stdInfo[2]) );
            infoPersistent.insert( "3#CaloCluster.position.z", floatFromInt(stdInfo[3]) );
          } else {

            warning() << " wrong number of StdInfo on CaloCluster " << stdInfo.size() << endmsg;
            for(HltSelRepRBStdInfo::StdInfo::const_iterator i=stdInfo.begin();
                i!=stdInfo.end(); ++i){
              std::stringstream ss;
              ss << "z#CaloCluster.unknown" << int(i-stdInfo.begin() );
              infoPersistent.insert( ss.str(), floatFromInt(*i) );        
            }      
            
          }
        }
        break;
      case 1:
        {      

          if( stdInfo.size()!=1 ){    
            error() << " wrong number of StdInfo on Trigger Selection " << stdInfo.size() << endmsg;
            for(HltSelRepRBStdInfo::StdInfo::const_iterator i=(stdInfo.begin()+1);
                i!=stdInfo.end(); ++i){
              std::stringstream ss;
              ss << "z#unknown" << int(i-stdInfo.begin() );
              infoPersistent.insert( ss.str(), floatFromInt(*i) );        
            }      
          } 
          infoPersistent.insert( "0#SelectionID", floatFromInt(stdInfo[0]) );
        }
        break;
      default:
        { 
          warning() << " StdInfo on unsupported class type " << hos->summarizedObjectCLID() << endmsg; 
          for(HltSelRepRBStdInfo::StdInfo::const_iterator i=stdInfo.begin();
              i!=stdInfo.end(); ++i){
            std::stringstream ss;
            ss << "z#Unknown.unknown" << int(i-stdInfo.begin() );
            infoPersistent.insert( ss.str(), floatFromInt(*i) );        
          }      
        }    
      }

    //           ============== extra 

    if( exInfOn ){
      HltSelRepRBExtraInfo::ExtraInfo extraInfo = extraInfoSubBank.next();
      for( HltSelRepRBExtraInfo::ExtraInfo::const_iterator i=extraInfo.begin();
           i!=extraInfo.end(); ++i){
        // convert int to string
        bool found=false;
        for( std::vector<IANNSvc::minor_value_type>::const_iterator j= hltinfos.begin();j!=hltinfos.end();++j){
          if( j->second == i->first  ){
            infoPersistent.insert( j->first, i->second );
            found=true;
            break;
          }
        }
        if( !found )warning() << " String key for Extra Info item in storage not found" << endmsg;
      }
    }
    hos->setNumericalInfo( infoPersistent );

    objects.push_back(hos);
  }

  // -----------------------------------------------------------------
  // reloop to add substructure or hits
  // -----------------------------------------------------------------
  for( unsigned int iObj=0; iObj!= nObj; ++iObj){
    
    HltObjectSummary* & hos = objects[iObj];
    
    HltSelRepRBSubstr::Substr sub=substrSubBank.next();

    if( sub.first ){
      // hits
      unsigned int nSeq = hitsSubBank.numberOfSeq();
      std::vector< LHCb::LHCbID > hits;
      for( HltSelRepRBSubstr::Substrv::const_iterator i=sub.second.begin();i!=sub.second.end();++i){
        unsigned int iSeq=*i;
        if( iSeq<nSeq ){
          std::vector< LHCb::LHCbID > hitseq = hitsSubBank.sequence( iSeq );
          hits.insert( hits.end(), hitseq.begin(), hitseq.end() );
        } else {
          error() << " Hit sequence index out of range " << endmsg;
        }
      }
      hos->setLhcbIDs( hits );

    } else {
      // pointers
      SmartRefVector<LHCb::HltObjectSummary> thisSubstructure; 
      for( HltSelRepRBSubstr::Substrv::const_iterator i=sub.second.begin();i!=sub.second.end();++i){
        unsigned int jObj=*i;     
        if( jObj<nObj ){
          thisSubstructure.push_back( &(*(objects[jObj])) );
        } else {
          error() << " Substructure object index out of range " << endmsg;
        }
      }
      hos->setSubstructure( thisSubstructure );
    }

    // give ownership to TES
    objectSummaries->push_back(hos);
  }

  // clean-up
  hltSelReportsBank.deleteBank();

  // ---------------------------------------------------------
  // ------- special container for selections ----------------
  // ---------------------------------------------------------

  for( unsigned int iObj=0; iObj!= nObj; ++iObj){
    
    HltObjectSummary* & hos = objects[iObj];        
    if( hos->summarizedObjectCLID()!=1 )continue; 

    std::string selName="Dummy";
    for( HltObjectSummary::Info::const_iterator i=hos->numericalInfo().begin();
         i!=hos->numericalInfo().end();++i ){
      if( i->first == "0#SelectionID" ){
        int id = (int)(i->second+0.1);

        for( std::vector<IANNSvc::minor_value_type>::const_iterator si=selectionNameToIntMap.begin();
             si!=selectionNameToIntMap.end();++si){
          if( si->second == id ){
            selName = si->first;
            break;
          }
        }
        break;
      }
    }
    if( selName != "Dummy" ){
      
      // clone hos
      HltObjectSummary* selSumOut = new HltObjectSummary();
      selSumOut->setSummarizedObjectCLID( hos->summarizedObjectCLID() );
      selSumOut->setNumericalInfo( hos->numericalInfo() );
      selSumOut->setSubstructure( hos->substructure() ); 

      // insert selection into the container
      if( outputSummary->insert(selName,*selSumOut) == StatusCode::FAILURE ){
        error() << " Failed to add Hlt selection name " << selName
                << " to its container "
                << endmsg;
      }

    } else {    

      error() << " Did not find string key for trigger selection in storage " << endmsg;

    }

  }

  if ( msgLevel(MSG::VERBOSE) ){

    verbose() << " ======= HltSelReports size= " << outputSummary->size() << endmsg;  
    verbose() << *outputSummary << endmsg;

    verbose() << " ======= HltObjectSummary container size= " << objectSummaries->size() << endmsg;
    for( HltObjectSummary::Container::const_iterator ppHos=objectSummaries->begin();
         ppHos!=objectSummaries->end();++ppHos){
      const HltObjectSummary* pHos=*ppHos;    
      verbose() << " key " << pHos->index() << *pHos << endmsg;    
    }
   
  }

 return StatusCode::SUCCESS;
}

//=============================================================================
//  Finalize
//=============================================================================
StatusCode HltSelReportsReader::finalize() {

  if ( msgLevel(MSG::DEBUG) ) debug() << "==> Finalize" << endmsg;

  return GaudiAlgorithm::finalize();  // must be called after all other actions
  //  return HltBaseAlg::finalize();  // must be called after all other actions
}

//=============================================================================
    
  
float HltSelReportsReader::floatFromInt(unsigned int i)
{
        union IntFloat { unsigned int mInt; float mFloat; };
        IntFloat a; a.mInt=i;
        return a.mFloat;
}

