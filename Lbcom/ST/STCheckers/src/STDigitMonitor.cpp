// $Id: STDigitMonitor.cpp,v 1.4 2008-01-18 10:15:10 mneedham Exp $

// Gaudi
#include "GaudiKernel/AlgFactory.h"

// LHCbKernel
#include "Kernel/STDetSwitch.h"

// STDet
#include "STDet/DeSTDetector.h"
#include "STDet/DeSTSector.h"

// Event
#include "Event/STDigit.h"

// local
#include "STDigitMonitor.h"

using namespace LHCb;

DECLARE_ALGORITHM_FACTORY( STDigitMonitor );

//--------------------------------------------------------------------
//
//  STDigitMonitor : Monitor the STDigits
//
//--------------------------------------------------------------------

STDigitMonitor::STDigitMonitor( const std::string& name, 
                                ISvcLocator* pSvcLocator ) :
  GaudiHistoAlg(name, pSvcLocator),
  m_tracker(0)
{
  // constructer
  declareProperty("DetType", m_detType = "TT" );
  declareProperty("InputData",m_dataLocation = STDigitLocation::TTDigits );
}

STDigitMonitor::~STDigitMonitor()
{
  // destructer
}

StatusCode STDigitMonitor::initialize()
{
  // Set the top directory to IT or TT.
  if( "" == histoTopDir() ) setHistoTopDir(m_detType+"/");

  // Initialize GaudiHistoAlg
  StatusCode sc = GaudiHistoAlg::initialize();
  if (sc.isFailure()) return Error("Failed to initialize", sc);
    
  // detector element     
  m_tracker = getDet<DeSTDetector>(DeSTDetLocation::location(m_detType));
 
  // flip  
  STDetSwitch::flip(m_detType,m_dataLocation); 
 
  return StatusCode::SUCCESS;
}

StatusCode STDigitMonitor::execute()
{
  // retrieve STDigits
  const STDigits* digitsCont = get<STDigits>(m_dataLocation);
  
  // number of digits
  plot((double)digitsCont->size(), 1, "Number of digits", 0.,10000., 500);

  // histos per digit
  STDigits::const_iterator iterDigit = digitsCont->begin();
  for( ; iterDigit != digitsCont->end(); ++iterDigit){
    this->fillHistograms(*iterDigit);
  }

  return StatusCode::SUCCESS;
}

void STDigitMonitor::fillHistograms(const STDigit* aDigit)
{
  // histogram by station
  const int iStation = aDigit->channelID().station();
  plot((double)iStation, 2, "Number of digits per station" , -0.5, 4.5, 5);
  
  // by layer
  const int iLayer = aDigit->channelID().layer();
  plot((double)(10*iStation+iLayer), 3, "Number of digits per layer",
       -0.5, 40.5, 41);

  if ( fullDetail() ){
    const DeSTSector* aSector = m_tracker->findSector(aDigit->channelID());
    if (aSector != 0) {
      plot(aDigit->depositedCharge(),aSector->type()+"/1", 
            "Deposited charge ", 0., 128., 128);
    }
  }

}
