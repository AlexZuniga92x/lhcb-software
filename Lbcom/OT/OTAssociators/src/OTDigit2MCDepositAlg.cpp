// $Id: OTDigit2MCDepositAlg.cpp,v 1.1 2003-06-10 09:04:16 jvantilb Exp $

// Event
#include "Event/OTDigit.h"
#include "Event/MCOTDigit.h"
#include "Event/MCOTDeposit.h"
#include "Event/MCTruth.h"

// Gaudi
#include "GaudiKernel/AlgFactory.h"
#include "GaudiKernel/MsgStream.h" 
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/SmartDataPtr.h"

// CLHEP
#include "CLHEP/Units/SystemOfUnits.h"

// local
#include "OTDigit2MCDepositAlg.h"
#include "OTAssociators/OTDigit2MCDepositAsct.h"

/** @file OTDigit2MCDepositAlg.cpp 
 *
 *  Implementation of the OTDigit2MCDepositAlg class
 *  
 *  @author J. van Tilburg
 *  @date   05/06/2003
 */

// Declaration of the Algorithm Factory
static const  AlgFactory<OTDigit2MCDepositAlg>          s_factory ;
const        IAlgFactory& OTDigit2MCDepositAlgFactory = s_factory ; 

OTDigit2MCDepositAlg::OTDigit2MCDepositAlg( const std::string& name,
                                                ISvcLocator* pSvcLocator)
  : Algorithm (name,pSvcLocator) 
{
  // constructor
  declareProperty( "OutputData", m_outputData  = OTDigit2MCDepositLocation );
  declareProperty( "acceptTime", m_acceptTime  = 7.8*ns );
}

OTDigit2MCDepositAlg::~OTDigit2MCDepositAlg() {
  // destructor
}

StatusCode OTDigit2MCDepositAlg::initialize() {

  MsgStream msg(msgSvc(), name());
  msg << MSG::DEBUG << "==> Initialise" << endmsg;
 
  return StatusCode::SUCCESS;
}

StatusCode OTDigit2MCDepositAlg::execute() 
{

  typedef Relation1D<OTDigit, MCOTDeposit>    Table;

  // get OTDigits
  SmartDataPtr<OTDigits> digitCont(eventSvc(),OTDigitLocation::Default);
  if (0 == digitCont){ 
    MsgStream msg(msgSvc(), name());
    msg << MSG::WARNING << "Failed to find OTDigits" << endmsg;
    return StatusCode::FAILURE;
  }

  // get MCOTDeposits to be able to find other deposits killed by dead-time
  if (m_acceptTime > 0.0 ) {
    SmartDataPtr<MCOTDeposits> deposits(eventSvc(), 
                                        MCOTDepositLocation::Default);
    if (0 == deposits){ 
      MsgStream msg(msgSvc(), name());
      msg << MSG::WARNING << "Failed to find MCOTDeposits" << endmsg;
      return StatusCode::FAILURE;
    }
    m_deposits = deposits;
  }
  
  // create an association table 
  Table* aTable = new Table();

  // loop and link OTDigits to MC truth
  OTDigits::const_iterator iterDigit;
  for(iterDigit = digitCont->begin(); 
      iterDigit != digitCont->end(); iterDigit++){
    std::vector<MCOTDeposit*> depVector;
    associateToTruth(*iterDigit, depVector);
    std::vector<MCOTDeposit*>::iterator iDep = depVector.begin();
    while (iDep != depVector.end()) {
      aTable->relate(*iterDigit, *iDep);
      iDep++;
    }
  } // loop iterDigit

  // register table in store
  StatusCode sc = eventSvc()->registerObject(outputData(), aTable);
  if( sc.isFailure() ) {
    MsgStream msg(msgSvc(), name());
    msg << MSG::FATAL << "     *** Could not register " << outputData()
        << endmsg;
    delete aTable;
    return StatusCode::FAILURE;
  }
 
  return StatusCode::SUCCESS;
}

StatusCode OTDigit2MCDepositAlg::finalize() {

  MsgStream msg(msgSvc(), name());
  msg << MSG::DEBUG << "==> Finalize" << endmsg;

  return StatusCode::SUCCESS;
}

StatusCode 
OTDigit2MCDepositAlg::associateToTruth(const OTDigit* aDigit,
                                       std::vector<MCOTDeposit*>& depVector) {
  // make link to truth  to MCOTDeposit
  StatusCode sc = StatusCode::SUCCESS;

  // link digit to truth
  MCOTDeposit* deposit = 0;
  const MCOTDigit* mcDigit = mcTruth<MCOTDigit>(aDigit);
  if (0 != mcDigit) {

    // link to deposits
    SmartRefVector<MCOTDeposit> depCont = mcDigit->deposits();
    if ( 0 == depCont.size()) return StatusCode::FAILURE;
    SmartRefVector<MCOTDeposit>::iterator iterDep;
    for ( iterDep = depCont.begin(); iterDep != depCont.end(); ++iterDep ) {
      deposit = (*iterDep);
      depVector.push_back(deposit);

      // find other deposits killed by dead-time,but within certain time window.
      if (m_acceptTime > 0.0 ) {
        bool keepAdding = true;
        while (keepAdding) {
          // Go to the next MCOTDeposit (they should be ordered)
          const int key = deposit->key();
          MCOTDeposit* nextDeposit = m_deposits->object(key+1);
          if (nextDeposit == 0) {
            keepAdding = false;
            continue;
          }
          // Check for same channel and within acceptTime cut
          if ( nextDeposit->channel() == deposit->channel() &&
               nextDeposit->tdcTime() < deposit->tdcTime() + m_acceptTime ) {
            depVector.push_back(nextDeposit);
            deposit = nextDeposit;
          } else {
            keepAdding = false;
          }
        }    
      }
    } 
  }


  return sc;
}
