// Gaudi
#include "GaudiKernel/AlgFactory.h"
// local
#include "bankKiller.h"

// ============================================================================
/** @file bankKiller.cpp
 *
 *    @author: Olivier Deschamps
 *    @date:   O6 September 2006
 */
// ============================================================================

DECLARE_ALGORITHM_FACTORY( bankKiller )

//=============================================================================
// Standard creator, initializes variables
//=============================================================================
bankKiller::bankKiller( const std::string& name, ISvcLocator* pSvcLocator)
  : GaudiAlgorithm       ( name , pSvcLocator            )
  , m_bankTypes()
  , m_rawEvt(0)
{
  declareProperty( "RawEventLocations", m_rawEventLocations,
                   "List of possible locations of the RawEvent object in the"
                   " transient store. By default it is LHCb::RawEventLocation::Default");
  declareProperty("BankTypes"     , m_bankTypes, "List of bank names"    ) ;
  declareProperty("DefaultIsKill" , m_defaultIsKill = false, 
                  "Main behaviour switch. If false (default), kill only given banks. If true, kill all BUT given banks." ) ;

}

//=============================================================================
// Standard destructor
//=============================================================================
bankKiller::~bankKiller() {}

//=============================================================================
// Initialisation. Check parameters
//=============================================================================
StatusCode bankKiller::initialize() {

  StatusCode sc = GaudiAlgorithm::initialize();
  if( sc.isFailure() ) return sc;
  
  if( m_defaultIsKill ) {
    if(m_bankTypes.empty()) warning() << "bankKiller : nothing specified to be kept - all will be removed " 
                                         << endmsg;
    
    for(std::vector< std::string >::iterator ityp = m_bankTypes.begin() ; 
        ityp!= m_bankTypes.end() ; ++ ityp ){
      always() << "bankKiller : all banks of type '" << *ityp << "' will be kept." <<endmsg;
    }
  } else {
    if(m_bankTypes.empty()) warning() << "bankKiller : nothing to be removed " << endmsg;
    
    for(std::vector< std::string >::iterator ityp = m_bankTypes.begin() ; 
        ityp!= m_bankTypes.end() ; ++ ityp ){
      always() << "bankKiller : all banks of type '" << *ityp << "' will be removed." <<endmsg;
    }
  }

  // Initialise the RawEvent locations
  bool usingDefaultLocation = m_rawEventLocations.empty();
  if (std::find(m_rawEventLocations.begin(), m_rawEventLocations.end(), LHCb::RawEventLocation::Default)
      == m_rawEventLocations.end()) {
    // append the default to the search path
    m_rawEventLocations.push_back(LHCb::RawEventLocation::Default);
  }

  if (!usingDefaultLocation) {
    info() << "Using '" << m_rawEventLocations << "' as search path for the RawEvent object" << endmsg;
  }
  
  return StatusCode::SUCCESS;
}

//=============================================================================
// Main execution
//=============================================================================
StatusCode bankKiller::execute() {

  m_rawEvt = NULL;
  for (std::vector<std::string>::const_iterator p = m_rawEventLocations.begin(); p != m_rawEventLocations.end(); ++p) {
    m_rawEvt = getIfExists<LHCb::RawEvent>(*p);
    if( m_rawEvt != NULL ) break;
  }
  if( m_rawEvt == NULL ) return Error("Failed to find raw data");

  if( m_defaultIsKill ) {
    for( unsigned int ibank = 0 ; ibank < (unsigned int) LHCb::RawBank::LastType ; ++ibank){
      LHCb::RawBank::BankType bankType = (LHCb::RawBank::BankType) ibank;
      bool keep = false;
      for(std::vector<std::string>::const_iterator ityp = m_bankTypes.begin(); 
          ityp != m_bankTypes.end(); ++ityp ){
        if( *ityp == LHCb::RawBank::typeName( bankType ) ) {
          keep = true;
          break;
        }
      }
      if ( !keep ) {
        // kill al banks of this type
        killBankType( bankType, false );
      }
    }
  } else {
    for(std::vector<std::string>::const_iterator ityp = m_bankTypes.begin(); 
        ityp != m_bankTypes.end(); ++ityp ){
      
      LHCb::RawBank::BankType bankType = LHCb::RawBank::LastType;
      for( unsigned int ibank = 0 ; ibank < (unsigned int) LHCb::RawBank::LastType ; ++ibank){
        bankType = (LHCb::RawBank::BankType) ibank;
        if( *ityp == LHCb::RawBank::typeName( bankType ) ){
          // kill all banks of this type
          killBankType( bankType, true );
          break;
        }
      }
    }
  }
  return StatusCode::SUCCESS;
}

//=============================================================================

void  bankKiller::killBankType( LHCb::RawBank::BankType bankType, 
                                bool warningmsg ) {
  const std::string bankTypeName = LHCb::RawBank::typeName( bankType );
  // look for all banks of this type and remove them 
  std::vector<LHCb::RawBank*> banks = m_rawEvt->banks( bankType );
  if( bankType == LHCb::RawBank::LastType || 0 == banks.size() ){
    if (warningmsg) {
      std::stringstream s("");
      s<< " No bank of type '" << bankTypeName		<< "' has been found - nothing to be removed";
      Warning(s.str(),StatusCode::SUCCESS,0).ignore();
    }
    return;
  }
  
  if (msgLevel(MSG::DEBUG)) debug() << "All banks of type '" << bankTypeName 
                                    << "'  are to be removed - banks size =  " << banks.size() << endmsg;
  
  
  for(std::vector<LHCb::RawBank*>::const_iterator itB = banks.begin() ; itB !=  banks.end() ; ++itB ) {
    
    bool success = m_rawEvt -> removeBank ( *itB );
    if( !success && warningmsg){
      if (msgLevel(MSG::DEBUG)) debug() << "The bank " << * itB << " of type '" << bankTypeName 
                                        << "' has not been found to be removed " << endmsg;
      Warning("The bank of type '"+bankTypeName+"' has not been found",StatusCode::SUCCESS,0).ignore();
    }
  }
  if (msgLevel(MSG::DEBUG)) debug() << "All banks of type '" << bankTypeName 
                                    << "' have been removed  - " 
                                    << " banks size is now : " << ( m_rawEvt->banks( bankType ) ).size() << endmsg;
  
}
