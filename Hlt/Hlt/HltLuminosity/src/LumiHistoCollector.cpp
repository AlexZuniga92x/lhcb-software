// $Id: LumiHistoCollector.cpp,v 1.3 2008-12-20 17:58:31 graven Exp $
// Include files 
#include "GaudiKernel/AlgFactory.h" 
#include "GaudiKernel/IAlgManager.h"
#include "GaudiKernel/IHistogramSvc.h"

#include "Event/ODIN.h"

#include "AIDA/IHistogram1D.h"
#include "AIDA/IAxis.h"

#include "HltBase/ANNSvc.h"
#include "LumiHistoCollector.h"
#include "HltBase/ESequences.h"

// for histogram retrieval
#include "GaudiKernel/GaudiException.h" 
#include "GaudiKernel/Bootstrap.h" 
#include "GaudiKernel/IUpdateManagerSvc.h"

#include "GaudiKernel/Kernel.h"
#include "GaudiKernel/IDataManagerSvc.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/IMessageSvc.h"

#include "GaudiKernel/ISvcManager.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/IRegistry.h"
#include "GaudiKernel/LinkManager.h"
#include "GaudiKernel/SmartDataPtr.h"

//-----------------------------------------------------------------------------
// Implementation file for class : LumiHistoCollector
//
// 2008-10-01 : Jaap Panman
//-----------------------------------------------------------------------------

// Declaration of the Algorithm Factory
DECLARE_ALGORITHM_FACTORY( LumiHistoCollector );

LumiHistoCollector::LumiHistoCollector( const std::string& name,
                            ISvcLocator* pSvcLocator )
  : HltBaseAlg ( name , pSvcLocator )
{
  declareProperty("HistogramUpdatePeriod" , m_histogramUpdatePeriod = 1 );
  declareProperty("TrendSize"             , m_trendSize = 100 );
  declareProperty("TrendInterval"         , m_trendInterval = 100 );
  declareProperty("BXTypes"               , m_BXTypes ) ;
  declareProperty("AddBXTypes"            , m_addBXTypes ) ;
  declareProperty("SubtractBXTypes"       , m_subtractBXTypes ) ;
  declareProperty("InputVariables"        , m_Variables );
  declareProperty("prevDir"               , m_prevDir  = "Previous" );
  declareProperty("resDir"                , m_resDir   = "Results" );
  declareProperty("PathBase"              , m_pathBase = "/stat/Histo" );
  
}

LumiHistoCollector::~LumiHistoCollector()
{
} 


StatusCode LumiHistoCollector::initialize() {

  StatusCode sc = HltBaseAlg::initialize(); 
  if ( sc.isFailure() ) return sc;

  // initialize lists for default behaviour
  if (m_BXTypes.empty()) {
      m_BXTypes.push_back("BeamCrossing");
      m_BXTypes.push_back("SingleBeamRight");
      m_BXTypes.push_back("SingleBeamLeft");
      m_BXTypes.push_back("NoBeam");
  }
  if (m_addBXTypes.empty()) {
      m_addBXTypes.push_back("BeamCrossing");
      m_addBXTypes.push_back("NoBeam");
  }
  if (m_subtractBXTypes.empty()) {
      m_subtractBXTypes.push_back("SingleBeamRight");
      m_subtractBXTypes.push_back("SingleBeamLeft");
  }
  if (m_Variables.empty()) {
      m_Variables.push_back("SPDMult");
      m_Variables.push_back("PUMult");
      m_Variables.push_back("CaloEt");
      m_Variables.push_back("SPDMult_threshold");
      m_Variables.push_back("PUMult_threshold");
      m_Variables.push_back("CaloEt_threshold");
  }

  // set up the hisogram lookup tables and book the result histos
  setupStore();
  
  // get list of all histograms and their directories
  if ( msgLevel( MSG::DEBUG ) ) {
    printHistoList();
  }

  debug() << "Initialised Lumi Histo Collector" << endmsg ;
  return sc;

}

//---------------------------------------------------------
StatusCode LumiHistoCollector::execute() {

  StatusCode sc = StatusCode::SUCCESS;  
  counter("#Entries") += 1;
  counter("HistoInputs") +=1;

  // analyse the data at with time intervals
  if ( gpsTimeInterval() ) analyse();

  setDecision(true);
  return sc;
}

//---------------------------------------------------------
void LumiHistoCollector::setDecision(bool ok) {
  setFilterPassed(ok);
}

//---------------------------------------------------------
void LumiHistoCollector::setupStore() {
  debug() << "LumiHistoCollector::setupStore()0" << endmsg;

  //  get histogram objects
  ISvcLocator* svclocator = Gaudi::svcLocator();
  StatusCode sc2 = svclocator->service("HistogramDataSvc", m_histogramSvc, true);

  // simple method - using known paths
  AIDA::IHistogram1D* theHisto;
  AIDA::IHistogram1D* mydataobject;
  // loop over requested histogram paths and retrieve them
  int countMaps = 0;
  std::string prefix = "";
  for (std::vector<std::string>::iterator iBx = m_BXTypes.begin(); iBx != m_BXTypes.end(); ++iBx) {
    countMaps++;
    std::string bx = *iBx;
    /* create storage for the existing histograms */
    histoMap* hMap=new histoMap();
    m_histoStore[bx]=hMap;
    /* create storage for the histograms  which should hold the previous cycle */
    histoMap* prevMap = new histoMap();
    m_previousStore[bx]=prevMap;
    for (std::vector<std::string>::iterator iVar = m_Variables.begin(); iVar != m_Variables.end(); ++iVar) {
      std::string var = *iVar;
      std::string path = m_pathBase + bx + "/" + var;
      StatusCode sc=m_histogramSvc->retrieveObject(path, mydataobject);
      if (sc.isSuccess()) {
        theHisto=dynamic_cast<AIDA::IHistogram1D*>(mydataobject);
        (*hMap)[var]=theHisto;  // store in map
        double sumBins=theHisto->sumBinHeights();
        double mean=theHisto->mean();
        const AIDA::IAxis& axis = theHisto->axis();   // assuming 1Dhisto
        double lowerEdge=axis.lowerEdge();
        double upperEdge=axis.upperEdge();
        long bins=axis.bins();
        /* create histogram with same name (add prefix anyway) */
        prefix = "Previous_"+ bx +"_";
        AIDA::IHistogram1D* prevHisto = initializeHisto(prefix+var, lowerEdge, upperEdge, bins);
        (*prevMap)[var] = prevHisto;  // store in other map
        /* the first time around create a single map to keep the results of the subtraction and so */
        if ( countMaps == 1 ) {
          // the result histos
          prefix="Results_";
          AIDA::IHistogram1D* resultHisto = initializeHisto(prefix+var, lowerEdge, upperEdge, bins);
          m_resultMap[var] = resultHisto;  // store in result map
          // the temp histos
          prefix="Temp_";
          AIDA::IHistogram1D* tempHisto = initializeHisto(prefix+var, lowerEdge, upperEdge, bins);
          m_tempMap[var] = tempHisto;  // store in temp map
          // the trend histos
          prefix="Trend_";
          AIDA::IHistogram1D* trendHisto = initializeHisto(prefix+var, -m_trendSize*m_trendInterval, 0, m_trendSize);
          m_trendMap[var] = trendHisto;  // store in trend map
        }
        debug() << theHisto->title() << " at path = " << path << " "
                << " axis: " << bins << " " << lowerEdge << " " << upperEdge
                << " sumBins: " << sumBins << " mean: " << mean
                << endreq;
      }
      else {
        error() << MSG::INFO << ">>>Could not retrieve object from TES (" << path << ")"
                << endreq;
      }
    }
  }
}

//---------------------------------------------------------
StatusCode LumiHistoCollector::analyse() {
  debug() << "analyse()" << endmsg;
  // look at what we have in store 
  info () << "summary of store contents" << endreq;  
  std::string prefix = "";
  for ( histoStore::iterator iBx=m_histoStore.begin(); iBx != m_histoStore.end(); ++iBx ) {
    histoMap *theMap=iBx->second;
    std::string bx=iBx->first;
    histoMap *prevMap=m_previousStore[bx];
    for ( histoMap::iterator iVar=theMap->begin(); iVar!=theMap->end(); ++iVar ) {
      std::string var=iVar->first;
      AIDA::IHistogram1D* theHist=iVar->second;
      AIDA::IHistogram1D* prevHist=(*prevMap)[var];
      std::string title=theHist->title();
      double sumBins=theHist->sumBinHeights();
      double sumAllBins=theHist->sumAllBinHeights();
      double prevSumBins=prevHist->sumBinHeights();
      double prevSumAllBins=prevHist->sumAllBinHeights();
      double mean=theHist->mean();
      double prevMean=prevHist->mean();
      debug() << bx << " / " << var << " : " << title
              << " sumBins: " << sumBins << " (" << sumAllBins << ") mean: " << mean
              << " prevSumBins: " << prevSumBins << " (" << prevSumAllBins << ") prevMean: " << prevMean
              << endreq;
    }
  }

  // take delta from all histos
  for (std::vector<std::string>::iterator iVar = m_Variables.begin(); iVar != m_Variables.end(); ++iVar) {
    std::string var=*iVar;
    AIDA::IHistogram1D *resultHist = m_resultMap[var];
    resultHist->reset();
    double meanR = 0;  // for R calculation
    for (std::vector<std::string>::iterator iBx = m_BXTypes.begin(); iBx != m_BXTypes.end(); ++iBx) {
      std::string bx=*iBx;
      histoMap *theMap=m_histoStore[bx];
      histoMap *prevMap=m_previousStore[bx];
      AIDA::IHistogram1D *theHist=(*theMap)[var];
      AIDA::IHistogram1D *prevHist=(*prevMap)[var];
      AIDA::IHistogram1D *tempHist = m_tempMap[var];
      tempHist->reset();
      // subtract the previous results from the present ones and store in the previous
      long prevEntries = prevHist->entries();
      double prevSumAll = prevHist->sumAllBinHeights();
      double theSumAll = theHist->sumAllBinHeights();
      double prevMean = prevHist->mean();
      double theMean = theHist->mean();
      double calcMean = 0;
      if ( theSumAll - prevSumAll ) {
        calcMean = (theSumAll*theMean - prevSumAll*prevMean) / ( theSumAll - prevSumAll );
      }
      prevHist->scale(-1.);
      tempHist->add(*theHist);
      tempHist->add(*prevHist);
      double tempMean=tempHist->mean();
      
      prevHist->add(*theHist);
      debug() << " subtraction result: " << bx << "/" << var << " entries: " 
              << theHist->entries() << " - " << prevEntries << " = " << prevHist->entries() 
              << " *** sum: " 
              << theHist->sumAllBinHeights() << " - " << prevSumAll << " = " << prevHist->sumAllBinHeights() 
              << endmsg;
      // add and subtract according to crossing type
      double deltaSumAll = prevHist->sumAllBinHeights();
      double deltaMean = prevHist->mean();
      debug() << " check means " << deltaMean << " and " << calcMean 
              << " from " << theMean << " and " << prevMean 
              << " temp " << tempMean << endmsg;
      double scale = 0;  // if histo not wanted scale to zero
      for (std::vector<std::string>::iterator iABx = m_addBXTypes.begin(); iABx != m_addBXTypes.end(); ++iABx) {
        if ( (*iABx) == bx ) {
          scale = deltaSumAll;
          meanR += deltaMean;
          break;
        }
      }
      for (std::vector<std::string>::iterator iSBx = m_subtractBXTypes.begin(); iSBx != m_subtractBXTypes.end(); ++iSBx) {
        if ( (*iSBx) == bx ) {
          scale = -1 * deltaSumAll;
          meanR -= deltaMean;
          break;
        }
      }
      // add or subtract this crossing type
      prevHist->scale(scale);
      resultHist->add(*prevHist);
      // dispose of intermediate data and save the data in store for the next time
      prevHist->reset();
      prevHist->add(*theHist);
    }
    // R-calculation
    double histoR = resultHist->mean();
    info() << var << ": R from means: " << meanR << "  R from histos: " << histoR << endmsg;
    // store trend
    storeTrend(var, meanR);
  }

  return StatusCode::SUCCESS;
  
}
  
//---------------------------------------------------------
StatusCode LumiHistoCollector::printHistoList() {
  // walk through the tree recursively and print the nodes
  ISvcLocator* svclocator = Gaudi::svcLocator();
  StatusCode sc = svclocator->service("HistogramDataSvc", m_histogramSvc, true);
  if ( sc.isSuccess() )  {
    std::string  m_rootName;
    SmartDataPtr<DataObject> root(m_histogramSvc, m_rootName);
    std::string store_name = "Unknown";
    IRegistry* pReg = root->registry();
    IRegistry* pObj;
    if ( pReg )  {
      SmartIF<IService> isvc(pReg->dataSvc());
      if ( isvc )  {
        store_name = isvc->name();
      }
    }
    pObj=root->registry();
    SmartIF<IDataManagerSvc> mgr(m_histogramSvc);
    if ( mgr )    {
      printHistoLeaf(m_histogramSvc, mgr, pObj);
    }
  }
  else {
    error() << "printHistoList cannot get HistogramDataSvc" << endmsg;
  }
  return StatusCode::SUCCESS;
}

//---------------------------------------------------------
bool LumiHistoCollector::printHistoLeaf(IHistogramSvc* histogramSvc, SmartIF<IDataManagerSvc> mgr, IRegistry* pObj) {
  // print the node name and work down the tree if new leaves
  typedef std::vector<IRegistry*> Leaves;
  Leaves leaves;
  StatusCode sc = mgr->objectLeaves(pObj, leaves);
  if ( sc.isSuccess() )  {
    if ( leaves.size() ) {
      // this is a leaf
      for ( Leaves::const_iterator i=leaves.begin();
            i != leaves.end(); i++ )   {
        const std::string& id = (*i)->identifier();
        if ( printHistoLeaf(histogramSvc, mgr, (*i)) ) {
          // this is not a histogram
          info() << MSG::INFO << "--Node found with id " << id << endreq;
        }
      }
    }
    else {
      const std::string& id = pObj->identifier();
      info() << MSG::INFO << "**Hist found with id " << id << endreq;
      return false;
    }
  }
  return true;
}

//=============================================================================
ulonglong LumiHistoCollector::gpsTime() {

  // get ODIN bank
  LHCb::ODIN* odin;
  if( exist<LHCb::ODIN>(LHCb::ODINLocation::Default) ){
    odin = get<LHCb::ODIN> (LHCb::ODINLocation::Default);
  }else{
    StatusCode sc = Error("ODIN cannot be loaded",StatusCode::FAILURE);
    return 0;
  }
  return odin->gpsTime();  // in microseconds
}

//=============================================================================
bool LumiHistoCollector::gpsTimeInterval() 
{
  // check if time interval has elapsed
  ulonglong now = gpsTime();
  verbose() << "gps: " << now << endmsg;
  
  if ( m_gpsPrevious ) {
    double delta = (now - m_gpsPrevious);
    double interval = (m_trendInterval);
    
    if ( delta > (interval*(1.0E6)) ) {
      m_gpsPrevious = now;
      return true;
    }
    return false;
  }
  m_gpsPrevious = now;
  return false;
}

//=============================================================================
void LumiHistoCollector::storeTrend(std::string varname, double lumiValue) 
{
  // store trends for
  AIDA::IHistogram1D *theHist = m_trendMap[varname];
  const AIDA::IAxis& axis = theHist->axis();
  long bins = axis.bins();
  debug() << "trend " << varname << ": ";
  for ( long i = 0; i < bins; ++i ) {
    double binValue = theHist->binHeight(i);
    double nextValue;
    if ( i < bins - 1 ) {
      nextValue = theHist->binHeight(i+1);
    }
    else {
      nextValue = lumiValue;
    }
    double x = 0.5*(axis.binUpperEdge(i)+axis.binLowerEdge(i));
    theHist->fill(x, nextValue - binValue);
    debug() << theHist->binHeight(i) << " ";
  }
  debug() << endmsg;
}
