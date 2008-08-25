// $Id: LumiHistoMaker.h,v 1.2 2008-08-25 10:59:04 panmanj Exp $
#ifndef LUMIHISTOMAKER_H 
#define LUMIHISTOMAKER_H 1

// Include files
#include "Event/HltSummary.h"
#include "Event/RecVertex.h"
#include "HltBase/HltTypes.h"
#include "HltBase/HltBaseAlg.h"
#include "Event/HltLumiSummary.h"
/** @class LumiHistoMaker 
 *  
 *
 *  functionality: Fill Histos with info from HltLumiSummary
 *
 *
 *  options:
 *  
 *
 *  @author Antonio Perez-Calero
 *  @date   2008-07-24
 */
class LumiHistoMaker : public HltBaseAlg {
public:

  // Standard constructor
  LumiHistoMaker( const std::string& name, ISvcLocator* pSvcLocator );

  // Standard destructor
  virtual ~LumiHistoMaker( ); 


  // initialize algorithm
  virtual StatusCode initialize();

  // finalize algorithm
  virtual StatusCode finalize  ();

protected:

  // initialize the counters
  virtual void initCounters();


private:  

  
  // driver of the execute()
  StatusCode execute();

  //protected:


  // force decision to the value of decision
  void setDecision(bool decision);
    

    
private:

  // period to update filling of histogram
  int m_histogramUpdatePeriod;


protected:

  // name of the input data to lumi histos->should become a vector of strings (option)
  std::string m_DataName;
  LHCb::HltLumiSummarys* m_HltLumiSummarys;

  std::vector<std::string> m_Variables;
  
  // counter with all the entries of the algorithm
  Hlt::Counter m_counterEntries;
  // counter with all the events with fine inputs
  Hlt::Counter m_counterHistoInputs;

  std::vector<Hlt::Histo*> m_Histos;
  std::vector<std::string> m_names;
  std::vector<int> m_keys;
  
};
#endif // LUMIHISTOMAKER_H
