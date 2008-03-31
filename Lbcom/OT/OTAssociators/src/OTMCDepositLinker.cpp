// $Id: OTMCDepositLinker.cpp,v 1.2 2008-03-31 16:31:20 janos Exp $
// Include files 

// from Gaudi
#include "GaudiKernel/AlgFactory.h"
#include "GaudiKernel/SystemOfUnits.h"  

// from Event
#include "Event/OTTime.h"

// from LHCbKernel
#include "Kernel/OTChannelID.h"

// from Linker
#include "Linker/LinkerTool.h"

// local
#include "OTMCDepositLinker.h"

//-----------------------------------------------------------------------------
// Implementation file for class : OTMCDepositLinker
//
// 2007-07-02 : Jan Amoraal
//-----------------------------------------------------------------------------

// Declaration of the Algorithm Factory
DECLARE_ALGORITHM_FACTORY( OTMCDepositLinker );

//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
OTMCDepositLinker::OTMCDepositLinker( const std::string& name,
                                      ISvcLocator* pSvcLocator)
  : GaudiAlgorithm ( name , pSvcLocator ),
    m_outputLocation(""),
    m_acceptTime(0.0)
{
  declareProperty("OutputLocation", m_outputLocation = LHCb::OTTimeLocation::Default + "2MCDeposits");
  declareProperty("AcceptTime"    , m_acceptTime     = 7.8*Gaudi::Units::ns                         );
}

//=============================================================================
// Destructor
//=============================================================================
OTMCDepositLinker::~OTMCDepositLinker() {} 

//=============================================================================
// Initialization
//=============================================================================
StatusCode OTMCDepositLinker::initialize() {
  StatusCode sc = GaudiAlgorithm::initialize(); // must be executed first
  if ( sc.isFailure() ) return sc;  // error printed already by GaudiAlgorithm

  return StatusCode::SUCCESS;
}

//=============================================================================
// Main execution
//=============================================================================
StatusCode OTMCDepositLinker::execute() {
  /// Get MCOTTimes
  const LHCb::MCOTTimes* mcTimes = get<LHCb::MCOTTimes>(LHCb::MCOTTimeLocation::Default);
  
  /// Create linker instance; link channelID(key) to MCOTDeposit
  LinkerWithKey<LHCb::MCOTDeposit> channelToDeposit(evtSvc(), msgSvc(), outputLocation());
  
  /// loop over MCOTTimes and get deposits
  for (LHCb::MCOTTimes::const_iterator iMCTime = mcTimes->begin(), iMCTimeEnd = mcTimes->end(); 
       iMCTime != iMCTimeEnd; ++iMCTime) {
    const LHCb::MCOTTime* mcTime = (*iMCTime);
    const unsigned int key = unsigned(mcTime->channel());
    OTMCDepositLinker::Deposits deposits;
    StatusCode sc = depositsToLink(mcTime, deposits);
    if (sc.isFailure()) return Error("Failed to find deposits to link!", StatusCode::FAILURE);
    /// loop over deposits and link 'em
    for (OTMCDepositLinker::Deposits::const_iterator iDeposit = deposits.begin(), iDepositEnd = deposits.end(); 
         iDeposit != iDepositEnd; ++iDeposit) channelToDeposit.link(key, (*iDeposit));
  }
  
  return StatusCode::SUCCESS;
}

//=============================================================================
//  depositsToLink
//=============================================================================
StatusCode OTMCDepositLinker::depositsToLink(const LHCb::MCOTTime* aTime, OTMCDepositLinker::Deposits& deposits) const {
  
  /// Get tdc time
  const unsigned int tdcTime = aTime->tdcTime();
  
  /// Get all deposits
  SmartRefVector<LHCb::MCOTDeposit> allDeposits = aTime->deposits();
  /// Check if container is empty
  if (allDeposits.empty()) {
    return Error("There are no OT MCDeposits to link to!", StatusCode::FAILURE);
  } else { /// loop over deposits and check whether to accept or not to accept
    for (SmartRefVector<LHCb::MCOTDeposit>::iterator iDeposit = allDeposits.begin(), iDepositEnd = allDeposits.end(); 
         iDeposit != iDepositEnd; ++iDeposit ) {
      const LHCb::MCOTDeposit* deposit = (*iDeposit);
      if ((deposit->tdcTime()) < (tdcTime + m_acceptTime)) { //accept
        deposits.push_back(deposit);
      }
    }
  }

  return StatusCode::SUCCESS;  
}

//=============================================================================
