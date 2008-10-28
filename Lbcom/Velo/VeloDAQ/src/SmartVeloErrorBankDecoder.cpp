// $Id: SmartVeloErrorBankDecoder.cpp,v 1.3 2008-10-28 10:03:52 szumlat Exp $
// Include files 

// from Gaudi
#include "GaudiKernel/AlgFactory.h" 

// local
#include "SmartVeloErrorBankDecoder.h"

//-----------------------------------------------------------------------------
// Implementation file for class : SmartVeloErrorBankDecoder
//
// 2008-08-24 : Tomasz Szumlak
//-----------------------------------------------------------------------------

// Declaration of the Algorithm Factory
DECLARE_ALGORITHM_FACTORY( SmartVeloErrorBankDecoder );

using namespace VeloTELL1;

typedef unsigned int* ErrorBankIT;
typedef std::pair<ErrorBankIT, ErrorBankIT> ITPair;
typedef std::map<unsigned int, ITPair> BANKS;
typedef std::map<unsigned int, ITPair> SECTORS;

//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
SmartVeloErrorBankDecoder::SmartVeloErrorBankDecoder( const std::string& name,
                                                      ISvcLocator* pSvcLocator)
  : GaudiAlgorithm ( name , pSvcLocator ),
    m_rawEvent ( 0 ),
    m_errorBank ( 0 )
{
  declareProperty("PrintBankHeader", m_printBankHeader=0);
  declareProperty("RawEventLocation",
                  m_rawEventLoc=LHCb::RawEventLocation::Default);
  declareProperty("ErrorBankLocation",
                  m_errorBankLoc=VeloErrorBankLocation::Default);
}
//=============================================================================
// Destructor
//=============================================================================
SmartVeloErrorBankDecoder::~SmartVeloErrorBankDecoder() {} 

//=============================================================================
// Initialization
//=============================================================================
StatusCode SmartVeloErrorBankDecoder::initialize() {
  StatusCode sc = GaudiAlgorithm::initialize(); // must be executed first
  if ( sc.isFailure() ) return sc;  // error printed already by GaudiAlgorithm

  debug() << "==> Initialize" << endmsg;

  return StatusCode::SUCCESS;
}

//=============================================================================
// Main execution
//=============================================================================
StatusCode SmartVeloErrorBankDecoder::execute() {

  debug() << "==> Execute" << endmsg;
  //
  StatusCode rawEvtStatus=getRawEvent();
  StatusCode bankStatus;
  if(rawEvtStatus.isSuccess()) bankStatus=cacheErrorRawBanks();
  if(bankStatus.isSuccess()){
    bankStatus=storeErrorRawBanks();
  }else{
    // go to the next event
    bankStatus=StatusCode::SUCCESS;
  }
  //
  return ( bankStatus );
}

//=============================================================================
//  Finalize
//=============================================================================
StatusCode SmartVeloErrorBankDecoder::finalize() {

  debug() << "==> Finalize" << endmsg;

  return GaudiAlgorithm::finalize();  // must be called after all other actions
}
//=============================================================================
StatusCode SmartVeloErrorBankDecoder::getRawEvent()
{
  debug()<< " ==> getRawEvent() " <<endmsg;
  debug()<< "--------------------" <<endmsg;
  //
  if(!exist<LHCb::RawEvent>(m_rawEventLoc)){
    error()<< " ==> There is no RawEvent at: "
           << LHCb::RawEventLocation::Default <<endmsg;
    return ( StatusCode::FAILURE );
  }else{  
    // get the RawEvent from default TES location
    m_rawEvent=get<LHCb::RawEvent>(m_rawEventLoc);
    debug()<< " ==> The RawEvent has been read-in from location: "
           << LHCb::RawEventLocation::Default <<endmsg;  
  }
  //
  return ( StatusCode::SUCCESS );
}
//=============================================================================
StatusCode SmartVeloErrorBankDecoder::cacheErrorRawBanks()
{
  debug()<< " ==> cacheErrorRawBanks() " <<endmsg;
  // check if there is error bank present
  const std::vector<LHCb::RawBank*>& errorBank=
    m_rawEvent->banks(LHCb::RawBank::VeloError);
  std::vector<LHCb::RawBank*>::const_iterator bIt;
  // if so write out the banks
  if(errorBank.size()!=0){
    m_errorBank=new VeloErrorBanks();
    //
    debug()<< " --> Error bank detected of size: " << errorBank.size() <<endmsg;
    ITPair data;
    for(bIt=errorBank.begin(); bIt!=errorBank.end(); ++bIt){
      LHCb::RawBank* aBank=(*bIt);
      // get the sensor number == sourceID if there is no full data
      unsigned int tell1=aBank->sourceID();
      m_bankLength=aBank->size();
      m_bankVersion=aBank->version();
      m_bankType=aBank->type();
      m_magicPattern=aBank->magic();
      if(m_printBankHeader){
        info()<< " --> src Id: " << tell1 <<endmsg;
        info()<< " --> type of bank: " << (aBank->typeName()) <<endmsg;
        info()<< " --> bank header size: " << (aBank->hdrSize()) <<endmsg;
        info()<< " --> body size: " << (aBank->size()) <<endmsg;
        info()<< " --> total size: " << (aBank->totalSize()) <<endmsg;
      }
      // get pointer to the bank data
      data.first=aBank->begin<unsigned int>();
      data.second=aBank->end<unsigned int>();
      m_cachedBanks[tell1]=data;
      int dist=std::distance(data.first, data.second);
      debug()<< " --> bank body size: " << (dist*sizeof(unsigned int)) <<endmsg;
    }
  }else{
    Warning(" --> No error bank detected - skipping to the next event ");
    return ( StatusCode::FAILURE );
  }
  debug()<< " --> cached error bank strucure's size:" 
         << (m_cachedBanks.size()) <<endmsg;
  return ( StatusCode::SUCCESS );
}
//=============================================================================
StatusCode SmartVeloErrorBankDecoder::storeErrorRawBanks()
{
  debug()<< " ==> storeErrorRawBanks() " <<endmsg;
  //
  std::map<unsigned int, ITPair>::iterator bankIT;
  bankIT=m_cachedBanks.begin();
  for( ; bankIT!=m_cachedBanks.end(); ++bankIT){
    VeloErrorBank* err=new VeloErrorBank(bankIT->first);
    EvtInfo anEvtInfo(bankIT->first);
    allEvt evtInfoData;
    allError errorInfoData;
    dataVec sources;
    // get information on error sources
    SECTORS sectors=errorDetector(bankIT->first);
    if(sectors.size()!=SOURCES)
      debug()<< " --> Error detected for " << (sectors.size()) 
             << " PPFPGA(s) "<<endmsg;
    // store words in error bank
    SECTORS::iterator secIT=sectors.begin();
    for( ; secIT!=sectors.end(); ++secIT){
      // write fixed words
      errorInfo anErrorInfo;
      ErrorBankIT begin=secIT->second.first-INIT_SHIFT;
      ErrorBankIT end=(secIT->second.first+1);
      for( ; begin!=end; ++begin){
        anErrorInfo.push_back(*begin);
      }
      if(secIT->second.second==0){
        anErrorInfo.push_back(*end);
        anErrorInfo.push_back(*(++end));
      }
      errorInfoData.push_back(anErrorInfo);
      // write optional part
      if(secIT->second.second!=0){
        // error was sent for this fpga build evt info section
        // encode properly the evt section block
        sources.push_back(secIT->first);
        evtInfo anInfo;
        ErrorBankIT begin=secIT->second.first-INIT_SHIFT;
        anInfo.push_back(*begin);
        anInfo.push_back(*(++begin));
        unsigned int word=(*(++begin));
        word=(word>>bitShift16)&bitMask16;
        anInfo.push_back(word);
        begin=secIT->second.second;
        ErrorBankIT end=begin+EVT_WORDS;
        for( ; begin!=end; ++begin){
          anInfo.push_back(*begin);
        }
        evtInfoData.push_back(anInfo);
      }
    }
    anEvtInfo.setEvtInfo(evtInfoData);
    err->setEvtInfoSection(anEvtInfo);
    err->setErrorInfoSection(errorInfoData);
    err->setBankLength(m_bankLength);
    err->setBankType(m_bankType);
    err->setBankVersion(m_bankVersion);
    err->setMagicPattern(m_magicPattern);
    debug()<< " --> Detected errors: " << (sources.size()) <<endmsg;
    err->setErrorSources(sources); 
    m_errorBank->insert(err);       
  }
  //
  put(m_errorBank, m_errorBankLoc);
  //
  return ( StatusCode::SUCCESS );
}
//=============================================================================
std::map<unsigned int, ITPair> SmartVeloErrorBankDecoder::errorDetector(
                                 unsigned int tell1)
{
  debug()<< " ==> errorDetector() " <<endmsg;
  //
  unsigned int fpga=0;
  SECTORS sectors;
  BANKS::const_iterator bankIT;
  bankIT=m_cachedBanks.find(tell1);
  if(bankIT!=m_cachedBanks.end()){
    ITPair iTPair=bankIT->second;
    ErrorBankIT iT=iTPair.first;
    // we can skip first four words - see note EDMS 694818
    iT+=INIT_SHIFT;
    // check if the file has expected format
    ITPair aPair(0, 0);
    for( ; iT!=iTPair.second; ++iT){
      if(((*iT&bitMask8)==EBMarker0)&&(((*iT>>bitShift8)&bitMask8)==EBMarker)){
        // cache this iterator
        aPair.first=iT;
        if(((*iT>>bitShift16)&bitMask16)==ERROR_BANK_SIZE){
          // cache iterator to the first word
          aPair.second=(iT+1);
        }else{
          // store nothing
          aPair.second=0;
        }
        sectors[fpga]=aPair;
        ++fpga;
      }
    }
  }
  //
  return ( sectors );
}
//--
