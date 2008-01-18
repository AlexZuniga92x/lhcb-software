// $Id: MCSTDepositMonitor.cpp,v 1.3 2008-01-18 10:15:08 mneedham Exp $

// BOOST!
#include "boost/lexical_cast.hpp"

// GaudiKernel
#include "GaudiKernel/AlgFactory.h"

// LHCbKernel
#include "Kernel/STDetSwitch.h"

// xml geometry
#include "STDet/DeSTDetector.h"

// Event
#include "Event/MCSTDeposit.h"
#include "Event/MCHit.h"

// local
#include "MCSTDepositMonitor.h"

using namespace LHCb;

DECLARE_ALGORITHM_FACTORY( MCSTDepositMonitor );

//--------------------------------------------------------------------
//
//  MCSTDepositMonitor : Make plots for MCSTDeposits
//
//--------------------------------------------------------------------

MCSTDepositMonitor::MCSTDepositMonitor( const std::string& name, 
                                        ISvcLocator* pSvcLocator ) :
  GaudiHistoAlg(name, pSvcLocator),
  m_tracker(0)  
{
  // constructer
  declareProperty("DetType", m_detType = "TT");
  declareProperty("InputData",  m_depositLocation = MCSTDepositLocation::TTDeposits );
}

MCSTDepositMonitor::~MCSTDepositMonitor()
{
  // destructer
}

StatusCode MCSTDepositMonitor::initialize()
{
  if( "" == histoTopDir() ) setHistoTopDir(m_detType+"/");

  StatusCode sc = GaudiHistoAlg::initialize();
  if (sc.isFailure()) return Error("Failed to initialize", sc);
      
  // detector element     
  m_tracker = getDet<DeSTDetector>(DeSTDetLocation::location(m_detType));

  // Determine the location of the ST deposits
  STDetSwitch::flip(m_detType,m_depositLocation);

  return StatusCode::SUCCESS;
}

StatusCode MCSTDepositMonitor::execute()
{
  // retrieve Digitizations
  MCSTDeposits* depositsCont = get<MCSTDeposits>(m_depositLocation); 
 
  // number of digits
  plot((double)depositsCont->size(),1,"Number of deposits", 0., 10000., 200);

  // histos per digit
  MCSTDeposits::const_iterator iterDep = depositsCont->begin(); 
  for ( ; iterDep != depositsCont->end(); ++iterDep) {
    this->fillHistograms(*iterDep);
  } // loop iterDep

  return StatusCode::SUCCESS;
}

void MCSTDepositMonitor::fillHistograms(const MCSTDeposit* aDeposit) const
{
  // Plot deposited charge
  plot(aDeposit->depositedCharge(), 2, "Deposited charge", 0., 100., 100);

  // Plot number of deposits per station
  const int iStation = aDeposit->channelID().station();
  plot((double)iStation, 3, "Number of deposits per station", -0.5, 4.5, 5);
  
  // by layer
  plot((double)(10*iStation+aDeposit->channelID().layer()),
       4, "Number of deposits per layer", -0.5, 40.5, 41);
      
   // detailed level histos
  if (fullDetail() == true){
    const MCHit* aHit = aDeposit->mcHit();
    if (0 != aHit) {    
      // take midPoint
      Gaudi::XYZPoint impactPoint = aHit->midPoint();

      // fill x vs y scatter plots
      std::string title = "x vs y " + m_detType + " station " +
        boost::lexical_cast<std::string>(iStation);
      plot2D( impactPoint.x(), impactPoint.y(), 200+iStation, title,
              -1000.,1000., -1000.,1000., 50, 50);
    }
  }

  return;
}
