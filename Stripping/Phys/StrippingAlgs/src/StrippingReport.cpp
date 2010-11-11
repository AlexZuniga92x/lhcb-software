// Include files 

// from Gaudi
#include "GaudiKernel/AlgFactory.h" 
#include "Event/Particle.h" 
//#include "GaudiAlg/GaudiTuples.h" 

#include "Event/HltDecReports.h"
#include "Event/HltDecReport.h"
#include "GaudiKernel/IAlgManager.h"
#include "GaudiKernel/IChronoStatSvc.h"
#include "GaudiKernel/ChronoEntity.h"
#include "GaudiKernel/IIncidentListener.h"
#include "GaudiKernel/IIncidentSvc.h"
#include "Kernel/SelectionLine.h"

// local
#include "StrippingReport.h"

//-----------------------------------------------------------------------------
// Implementation file for class : StrippingReport
//
// 2010-03-15 : Anton Poluektov
//-----------------------------------------------------------------------------

// Declaration of the Algorithm Factory
DECLARE_ALGORITHM_FACTORY( StrippingReport );

//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
StrippingReport::StrippingReport( const std::string& name,
                                              ISvcLocator* pSvcLocator)
  : GaudiTupleAlg ( name , pSvcLocator )
{
  declareProperty("HDRLocation", m_hdrLocation = "Strip/Phys/DecReports");
  declareProperty("Selections", m_selections);
  declareProperty("OnlyPositive", m_onlyPositive = true);
  declareProperty("EveryEvent", m_everyEvent = false);
  declareProperty("ReportFrequency", m_reportFreq = 100);
  declareProperty("PrintNonResponding", m_printNonResponding = true);
  declareProperty("PrintHot", m_printHot = true);
  declareProperty("HotThreshold", m_hotThreshold = 0.5);
  declareProperty("NormalizeByGoodEvents", m_normalizeByGoodEvents = true);
  declareProperty("Latex", m_latex = false);
}

//=============================================================================
// Destructor
//=============================================================================
StrippingReport::~StrippingReport() {} 

//=============================================================================
// Initialization
//=============================================================================
StatusCode StrippingReport::initialize() {
  StatusCode sc = GaudiTupleAlg::initialize(); // must be executed first
  if ( sc.isFailure() ) return sc;  // error printed already by GaudiAlgorithm

  if ( msgLevel(MSG::DEBUG) ) debug() << "==> Initialize" << endmsg;
  
  m_algMgr = svc<IAlgManager>   ( "ApplicationMgr" );
  
  m_chronoSvc = svc<IChronoStatSvc> ( "ChronoStatSvc" );

  m_incSvc = svc<IIncidentSvc> ( "IncidentSvc" ); 

  if ( !m_incSvc ) {
    error() << "Could not retrieve 'IncidentSvc'" << endmsg;
    return StatusCode::FAILURE;
  }
  m_incSvc->addListener ( this , IncidentType::BeginEvent );
  
  std::vector<std::string>::iterator i;

  for (i = m_selections.begin(); i != m_selections.end(); i++) {
    ReportStat stat;
    stat.name = *i;
    stat.candidates = 0;
    stat.decisions = 0;
    stat.errors = 0;
    stat.incidents = 0;
    stat.slow_events = 0;
    m_stat.push_back(stat);
  }
  
  m_event = 0;
  m_goodEvent = 0;

  return StatusCode::SUCCESS;
}

//===============================================================================
// Function to show selection statistics 
//===============================================================================
void StrippingReport::report(bool onlyPositive) {

  std::vector<ReportStat>::iterator i;

  char str[128];
  
  sprintf(str," |%51.51s|%8.8s|%10.10s|%7.7s|%8.8s|%7.7s|%7.7s|%7.7s|", "*Decision name*", "*Rate,%*", "*Accepted*", "*Mult*","*ms/evt*","*Errs*","*Incds*","*Slow*");

  info() << "Event " << m_event << ", Good event " << m_goodEvent << "\n" << str << "\n";

  for (i = m_stat.begin(); i != m_stat.end(); i++) {

    double rate = 0.;
    
    if (m_normalizeByGoodEvents) {
      if (m_goodEvent > 0) rate = 100.*(double)i->decisions/(double)m_goodEvent;
    }
    else {
      if (m_event > 0) rate = 100.*(double)i->decisions/(double)m_event;
    }

    if (i->candidates >= 0) {
      std::string outputName = "!" + i->name;
      double mult = 0;
      if (i->decisions > 0) mult = (double)i->candidates/(double)i->decisions;
      if (i->decisions > 0 || !onlyPositive) {
        if (i->avgtime > 0) 
          sprintf(str," |%-51.51s|%8.4f|%10.1d|%7.3f|%8.3f|%7d|%7d|%7d|", outputName.data(), 
                  rate, i->decisions, mult, i->avgtime, i->errors, i->incidents, i->slow_events);
        else 
          sprintf(str," |%-51.51s|%8.4f|%10.1d|%7.3f|        |%7d|%7d|%7d|", outputName.data(), 
                  rate, i->decisions, mult, i->errors, i->incidents, i->slow_events);
        info() << str << "\n";
      }
    } else if (i->decisions > 0 || !onlyPositive) {
      std::string outputName = "_" + i->name + "_";

      // Not a Selection::Line
      if (i->avgtime > 0) 
        sprintf(str," |%-51.51s|%8.4f|%10.1d|       |%8.3f|       |       |       |", outputName.data(), rate, i->decisions, i->avgtime);
      else 
        sprintf(str," |%-51.51s|%8.4f|%10.1d|       |        |       |       |       |", outputName.data(), rate, i->decisions);
      info() << str << "\n";
    }
      
  }

  info() << endmsg;

}

void StrippingReport::reportLatex(bool onlyPositive) {

  std::vector<ReportStat>::iterator i;

  char str[128];
  
  sprintf(str," %51.51s&%8.8s&%10.10s&%7.7s&%8.8s\\\\ \\hline", "Decision name", "Rate, \\%", "Accepted", "Mult","ms/evt");

  info() << "Event " << m_event << ", Good event " << m_goodEvent << "\n" << str << "\n";

  for (i = m_stat.begin(); i != m_stat.end(); i++) {

    double rate = 0.;
    
    if (m_normalizeByGoodEvents) {
      if (m_goodEvent > 0) rate = 100.*(double)i->decisions/(double)m_goodEvent;
    }
    else {
      if (m_event > 0) rate = 100.*(double)i->decisions/(double)m_event;
    }

    if (i->candidates >= 0) {
      std::string outputName = i->name;
      double mult = 0;
      if (i->decisions > 0) mult = (double)i->candidates/(double)i->decisions;
      if (i->decisions > 0 || !onlyPositive) {
        if (i->avgtime > 0) 
          sprintf(str," %-51.51s&%8.4f&%10.1d&%7.3f&%8.3f\\\\", outputName.data(), 
                  rate, i->decisions, mult, i->avgtime);
        else 
          sprintf(str," %-51.51s&%8.4f&%10.1d&%7.3f&        \\\\", outputName.data(), 
                  rate, i->decisions, mult);
        info() << str << "\n";
      }
    } else if (i->decisions > 0 || !onlyPositive) {
      std::string outputName = i->name;

      // Not a Selection::Line
      if (i->avgtime > 0) 
        sprintf(str," %-51.51s&%8.4f&%10.1d&       &%8.3f\\\\", outputName.data(), rate, i->decisions, i->avgtime);
      else 
        sprintf(str," %-51.51s&%8.4f&%10.1d&       &       \\\\", outputName.data(), rate, i->decisions);
      info() << str << "\n";
    }
      
  }

  info() << endmsg;

}



//=============================================================================
// Main execution
//=============================================================================
StatusCode StrippingReport::execute() {

  if ( msgLevel(MSG::DEBUG) ) debug() << "==> Execute" << endmsg;
  
  StatusCode result = StatusCode::SUCCESS;
  
  std::vector< ReportStat >::iterator i;

  char str[128];

  LHCb::HltDecReports* reports = get<LHCb::HltDecReports>(m_hdrLocation);

  if (m_everyEvent) { 
    sprintf(str,"%-51.51s: %4s %5s", "Decision name", "Decn", "Cands");
 
    info() << "----------------------------------------------------------------" << endmsg;
    info() << str << endmsg;
  }

  for (i = m_stat.begin(); i != m_stat.end(); i++) {
    
    int passed = 0;
    int cand = 0;
    
    i->avgtime = -1;

    const LHCb::HltDecReport* report = 0;
    if (reports) { 
      report = reports->decReport(i->name + "Decision");
      verbose() << "HDRs found at " << m_hdrLocation << endmsg;
    }
    
    if (report) {
      if (report->errorBits() & 0x01) i->errors++;
      if (report->errorBits() & 0x02) i->incidents++;
      if (report->errorBits() & 0x04) i->slow_events++;
      verbose() << "HDR " << i->name << " found" << endmsg;
    } else {
      verbose() << "HDR " << i->name << " NOT found" << endmsg;    
    }
    
    IAlgorithm* myIAlg(0);

    StatusCode result = m_algMgr->getAlgorithm( i->name, myIAlg );
    if ( result.isSuccess() ) {
        Algorithm* myAlg = dynamic_cast<Algorithm*>(myIAlg);
        if (myAlg) passed = myAlg->filterPassed();
        Selection::Line* strAlg = dynamic_cast<Selection::Line*>(myIAlg);
        if (strAlg) cand = strAlg->numberOfCandidates();
        else {
          // Not a Selection::Line
          cand = -1;
        }
        
        if (m_chronoSvc) {
    	  const ChronoEntity* chrono = m_chronoSvc->chrono(i->name + ":execute");
    	  if (chrono) {
    	    i->avgtime = chrono->uMeanTime()/1.e3;
          } 
        } 
    }
    
    if (report) {

      // The decision is valid only if the line has been run, i.e. if the corresponding HDR exists. 
      // Otherwise, it was an event marked as "bad". 
      i->candidates += cand;
      i->decisions += passed;

    } else if (cand < 0) {

      // It is a sequencer
      i->candidates = -1;
      i->decisions  += passed;

    }

    if (m_everyEvent && (passed != 0 || cand < 0 || !m_onlyPositive)) { 
      if (cand >= 0) {
        sprintf(str,"-- %-48.48s: %4.1d %5.1d", i->name.data(), passed, cand);
        info() << str << endmsg;
      } else {  
        sprintf(str,"%-51.51s: %4.1d", i->name.data(), passed);
        info() << "----------------------------------------------------------------" << endmsg;
        info() << str << endmsg;
      }
    }

  }

  if (m_everyEvent) { 
    info() << "----------------------------------------------------------------" << endmsg;
  }
  
  m_goodEvent++;
  
  if (m_reportFreq > 0 && (m_event % m_reportFreq == 0) ) { 
    if (m_latex) {
      reportLatex(m_onlyPositive);
    } else { 
      report(m_onlyPositive);
    }
  }
  
  return result;
}

//=============================================================================
//  Finalize
//=============================================================================
StatusCode StrippingReport::finalize() {

  if ( msgLevel(MSG::DEBUG) ) debug() << "==> Finalize" << endmsg;

  // Produce selection statistics report (always show all lines, even non-responding)
  if (m_latex) {
    reportLatex(false);
  } else { 
    report(false);
  }

  std::vector<ReportStat>::iterator i;

  char str[128];

  // List non-responding lines
  if (m_printNonResponding) {

    sprintf(str,"%-51.51s", "Non-responding lines:");

    warning() << endmsg;
    warning() << "-----------------------------------------------------------------" << endmsg;
    warning() << str << endmsg;

    for (i = m_stat.begin() + 1; i != m_stat.end(); i++) {
      std::string strippedName = i->name;

      if (i->candidates >= 0) {
        if (i->decisions == 0) {
          sprintf(str,"-- %-48.48s", strippedName.data());
          warning() << str << endmsg;
        } 
      } 
    }
    warning() << "-----------------------------------------------------------------" << endmsg;
  }

  // List hot lines
  if (m_printHot) {

    sprintf(str,"%-51.51s", "Hot lines:");

    warning() << endmsg;
    warning() << "-----------------------------------------------------------------" << endmsg;
    warning() << str << endmsg;

    for (i = m_stat.begin() + 1; i != m_stat.end(); i++) {
      std::string strippedName = i->name;

      double rate = 0.;
      if (m_normalizeByGoodEvents) {
        if (m_goodEvent > 0) rate = (double)i->decisions/(double)m_goodEvent;
      }
      else {
        if (m_event > 0) rate = (double)i->decisions/(double)m_event;
      }

      if (i->candidates >= 0) {
        if (rate > m_hotThreshold) {
          sprintf(str,"-- %-48.48s %8.6f", strippedName.data(), rate);
          warning() << str << endmsg;
        }
      } 
    }
    warning() << "-----------------------------------------------------------------" << endmsg;
  }


  return GaudiTupleAlg::finalize();  // must be called after all other actions
}

//=======================================================================
//  Handler for BeginEvent incident: increment event counter. 
//=======================================================================

void StrippingReport::handle ( const Incident& i ) {
  if ( IncidentType::BeginEvent == i.type () ) {
    m_event++;
  }
}

//=============================================================================
