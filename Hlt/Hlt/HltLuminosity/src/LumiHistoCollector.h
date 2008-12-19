// $Id: LumiHistoCollector.h,v 1.2 2008-12-19 17:35:58 graven Exp $
#ifndef LUMIHISTOCOLLECTOR_H 
#define LUMIHISTOCOLLECTOR_H 1

// Include files
#include "GaudiKernel/IHistogramSvc.h"
#include "Event/HltSummary.h"
#include "Event/RecVertex.h"
#include "Event/HltLumiSummary.h"
#include "HltBase/HltBaseAlg.h"

class IRegistry;
class IDataManagerSvc;

/** @class LumiHistoCollector 
 *  
 *
 *  functionality: Collect Histos filled by the HistoMaker and calculate "R"
 *
 *  options:
 *
 *  @author Jaap Panman
 *  @date   2008-10-01
 */
class LumiHistoCollector : public HltBaseAlg {
public:

  // Standard constructor
  LumiHistoCollector( const std::string& name, ISvcLocator* pSvcLocator );

  // Standard destructor
  virtual ~LumiHistoCollector( ); 

  // initialize algorithm
  virtual StatusCode initialize();

  // finalize algorithm
  virtual StatusCode finalize  ();

protected:
  // analyse histos
  virtual StatusCode analyse  ();
  virtual void storeTrend  (std::string varname, double lumiValue) ;
  virtual StatusCode printHistoList();
  virtual bool printHistoLeaf(IHistogramSvc*, SmartIF<IDataManagerSvc> mgr, IRegistry* pObj);
  virtual void setupStore  ();
  // to set interval
  ulonglong LumiHistoCollector::gpsTime  ();
  bool LumiHistoCollector::gpsTimeInterval  ();

  // force decision to the value of decision
  void setDecision(bool decision);
    
private:  
  // driver of the execute()
  StatusCode execute();

private:
  // period to update filling of histogram
  int m_histogramUpdatePeriod;

protected:
  IHistogramSvc* m_histogramSvc;

  // counter with all the entries of the algorithm
  Hlt::Counter m_counterEntries;
  // counter with all the events with fine inputs
  Hlt::Counter m_counterHistoInputs;

  std::vector< std::string > m_Variables;        // list of variables to look at
  std::vector< std::string > m_BXTypes;          // list of bunch crossing types to look at
  std::vector< std::string > m_addBXTypes;       // list of bunch crossing types to be added
  std::vector< std::string > m_subtractBXTypes;  // list of bunch crossing types to be subtracted

  std::vector< std::string > m_HistoNames;
  std::vector< Hlt::Histo* > m_Histos;
  Hlt::Histo* myhisto;

  std::string m_pathBase;                                         // prefix of pathname
  std::string m_topDir;                                           // topDirectory for local storage
  std::string m_prevDir;                                          // topDirectory for local storage previous state
  std::string m_resDir;                                           // topDirectory for local result storage
  typedef std::map< std::string, AIDA::IHistogram1D* > histoMap;  // simple map of histos
  typedef std::map< std::string, histoMap* > histoStore;          // map of maps
  histoStore m_histoStore;                                        // main store
  histoStore m_previousStore;                                     // store previous values
  histoMap m_trendMap;                                            // trends
  histoMap m_resultMap;                                           // to keep results of subtractions
  histoMap m_tempMap;
  long m_trendSize;                                               // size of trend histos
  long m_trendInterval;                                           // interval for trending
  unsigned long m_gpsPrevious;                                    // to keep previous time when analysed
};
#endif // LUMIHISTOCOLLECTOR_H
