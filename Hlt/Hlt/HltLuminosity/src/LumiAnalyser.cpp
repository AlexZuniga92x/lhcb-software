// $Id: LumiAnalyser.cpp,v 1.12 2009-08-06 21:53:18 panmanj Exp $
// Include files 
#include "GaudiKernel/AlgFactory.h" 
#include "GaudiKernel/IAlgManager.h"
#include "GaudiKernel/IHistogramSvc.h"

#include "Event/ODIN.h"

#include "AIDA/IHistogram1D.h"
#include "AIDA/IAxis.h"

#include "Event/LumiCounters.h"
#include "Event/HltLumiSummary.h"
#include "LumiAnalyser.h"

//-----------------------------------------------------------------------------
// Implementation file for class : LumiAnalyser
//
// 2008-10-01 : Jaap Panman
//-----------------------------------------------------------------------------

// Declaration of the Algorithm Factory
DECLARE_ALGORITHM_FACTORY( LumiAnalyser );

LumiAnalyser::LumiAnalyser( const std::string& name,
                            ISvcLocator* pSvcLocator )
  : HltBaseAlg ( name , pSvcLocator )
{
  declareProperty("HistogramUpdatePeriod" , m_histogramUpdatePeriod = 1 );
  declareProperty("InputDataContainer",     m_DataName = LHCb::HltLumiSummaryLocation::Default );
  declareProperty("InputVariables",         m_Variables );
  declareProperty("Thresholds",             m_Thresholds );
  declareProperty("Threshold",              m_Threshold = 5 );
  declareProperty("MaxBins",                m_MaxBins );
  declareProperty("MaxBin",                 m_MaxBin = 500 );
  declareProperty("NumBin",                 m_NumBin = 50 );
  declareProperty("RawHistos",              m_rawHistos = true );
  declareProperty("InHistos",               m_inHistos = true );
  declareProperty("TrendSize"             , m_trendSize = 100 );
  declareProperty("TrendInterval"         , m_trendInterval = 100 );
  declareProperty("ForceTrend"            , m_forceTrend = false );
  declareProperty("BXTypes"               , m_BXTypes ) ;
  declareProperty("AddBXTypes"            , m_addBXTypes ) ;
  declareProperty("SubtractBXTypes"       , m_subtractBXTypes ) ;
  declareProperty("PublishToDIM"          , m_publishToDIM = true ) ;
}

LumiAnalyser::~LumiAnalyser()
{

} 


StatusCode LumiAnalyser::initialize() {

  StatusCode sc = HltBaseAlg::initialize(); 
  if ( sc.isFailure() ) return sc;

  // initialize lists
  if (m_BXTypes.empty()) {
      m_BXTypes.push_back("NoBeam");
      m_BXTypes.push_back("Beam1");
      m_BXTypes.push_back("Beam2");
      m_BXTypes.push_back("BeamCrossing");
  }
  if (m_addBXTypes.empty()) {
      m_addBXTypes.push_back("NoBeam");
      m_addBXTypes.push_back("BeamCrossing");
  }
  if (m_subtractBXTypes.empty()) {
      m_subtractBXTypes.push_back("Beam1");
      m_subtractBXTypes.push_back("Beam2");
  }

  if (m_Variables.empty()) {
      m_Variables.push_back("SPDMult");
      m_Variables.push_back("PUMult");
      m_Variables.push_back("CaloEt");
  }

  // ------------------------------------------
  m_call_counter = 0;

  if (m_Variables.size()!=m_MaxBins.size() && !m_MaxBins.empty()) {
    error() << "wrong numer of histogram maximum bin values" << endmsg;
    return StatusCode::FAILURE;
  }
  if (m_Variables.size()!=m_Thresholds.size() && !m_Thresholds.empty()) {
    error() << "wrong numer of threshold values" << endmsg;
    return StatusCode::FAILURE;
  }

  // set the default threshold
  if (m_Thresholds.empty()) {
    for(std::vector< std::string >::iterator ivar = m_Variables.begin() ; 
        ivar!= m_Variables.end() ; ++ivar ){  
      m_Thresholds.push_back(m_Threshold);
    }
  }

  // keep a double list of averages (w and w/o threshold)
  for(std::vector< std::string >::iterator ivar = m_Variables.begin() ; 
      ivar!= m_Variables.end() ; ++ivar ){  
    m_Averages.push_back(*ivar);
  }
  for(std::vector< std::string >::iterator ivar = m_Variables.begin() ; 
      ivar!= m_Variables.end() ; ++ivar ){  
    m_Averages.push_back(*ivar+"_threshold");
  }

  debug()<< "going to set up"<<endmsg;
  setupStore();

  // to publish to DIM 
  m_size = m_Variables.size();
  m_means = new double[m_size];            // create a fixed location for DIM to look at
  m_thresholds = new double[m_size];       // create a fixed location for DIM to look at

  int k = 0;
  for(std::vector< std::string >::iterator ivar = m_Variables.begin() ; 
      ivar!= m_Variables.end() ; ++ivar, ++k ){  
    std::string name  = *ivar;
    // announce the values
    if (m_publishToDIM ) {
      //declareInfo("COUNTER_TO_RATE["+name+"_mean]", m_means[k], "mean of "+name);
      //declareInfo("COUNTER_TO_RATE["+name+"_threshold]", m_thresholds[k], "fraction over threshold of "+name);
      declareInfo(name+"_mean", m_means[k], "mean of "+name);
      declareInfo(name+"_threshold", m_thresholds[k], "fraction over threshold of "+name);
      info() << "counter " << name << " declared to DIM (" << k << ") with threshold " << m_Thresholds[k] << endmsg;
    }
  }

  // BXType counting
  m_sizeBxTypes = m_BXTypes.size();
  m_countBxTypes = new double[m_sizeBxTypes];     // create a fixed location for DIM to look at
  int ib = 0;
  for (std::vector<std::string>::iterator iBx = m_BXTypes.begin(); iBx != m_BXTypes.end(); ++iBx, ++ib) {
    std::string bx=*iBx;
    if (m_publishToDIM ) {
      declareInfo(bx, m_countBxTypes[ib], "number of "+bx);
    }
  }
  m_allBxTypes = 0;
  if (m_publishToDIM ) {
    declareInfo("allBXTypes", m_allBxTypes, "all BX Types");
  }
  
  debug() << "Initialised Lumi Analyser" << endmsg ;
  return sc;

}

//---------------------------------------------------------

//---------------------------------------------------------
StatusCode LumiAnalyser::execute() {

  counter("#Entries") += 1;
  counter("HistoInputs") += 1;
  m_call_counter++;

  // accumulate the counters with their normalization
  accumulate();

  if ( m_forceTrend ) {
    // analyse at fixed event count intervals (testing only)
    if (  m_call_counter%m_trendInterval == 0 ) analyse();
  } else {
    // analyse at fixed time intervals
    if ( gpsTimeInterval() ) analyse();
  }
  setDecision(true);
  return StatusCode::SUCCESS;  
}

//---------------------------------------------------------
void LumiAnalyser::setDecision(bool ok) {
  setFilterPassed(ok);
}

//---------------------------------------------------------
StatusCode LumiAnalyser::finalize() {
  debug() << "LumiAnalyser::finalize()" << endmsg;
  // report avergaes of all counters
  for (std::vector<std::string>::iterator iVar = m_Averages.begin(); iVar != m_Averages.end(); ++iVar) {
    std::string var=*iVar;
    info() << "R/N for " << var;
    for (std::vector<std::string>::iterator iBx = m_BXTypes.begin(); iBx != m_BXTypes.end(); ++iBx) {
      std::string bx=*iBx;
      iiMap *theMap=m_theStore[bx];
      iiPair *thePair = (*theMap)[var];
      // take average
      double norm = thePair->first;
      double sums = thePair->second;
      double R = 0;
      if (norm) {R = sums/norm;}
      info() << "  " << bx << ": " << R << "/" << thePair->first;
    }
    info() << endmsg;
  }


  // release storage
  delete[] m_means;
  delete[] m_thresholds;
  delete[] m_countBxTypes;

  return HltBaseAlg::finalize();
}

//---------------------------------------------------------
void LumiAnalyser::setupStore() {
  debug() << "LumiAnalyser::setupStore()0" << endmsg;

  // hard-wired BX types histo
  m_bxHisto = initializeHisto("BXTypes", 0, 4, 4);

  // set up store of pairs for averaging and for raw R histos
  bool useMaxBins = (m_Variables.size()==m_MaxBins.size());
  for (std::vector<std::string>::iterator iBx = m_BXTypes.begin(); iBx != m_BXTypes.end(); ++iBx) {
    std::string bx=*iBx;
    iiMap *theMap = new iiMap();
    iiMap *prevMap = new iiMap();
    m_theStore[bx] = theMap;
    m_prevStore[bx] = prevMap;
    histoMap* hMap=new histoMap();     // create storage for the existing histograms 
    m_histoStore[bx]=hMap;
    int i=0;
    for (std::vector<std::string>::iterator iVar = m_Averages.begin(); iVar != m_Averages.end(); ++iVar, ++i) {
      std::string var=*iVar;
      iiPair *thePair = new iiPair();
      iiPair *prevPair = new iiPair();
      (*theMap)[var] = thePair;
      (*prevMap)[var] = prevPair;

      // the input histos
      if ( m_inHistos ) {
        std::string prefix="In_";
        // defaults for threshold histos
        double x_min = 0;
        double x_max = 1.;
        unsigned int nbin = m_NumBin;
        if ( var.find("_threshold") == std::string::npos ) {
          // direct counters - add a bin for negatives
	  int bins = m_MaxBin;
	  if (useMaxBins) {
	    bins = m_MaxBins[i];
	  }
	  double x0 = -0.5;
	  x_max = bins + x0;
          int n = std::min(m_NumBin, bins);
	  double width = (double) bins/n;
          x_min = x0 - width;
          nbin = n+1;
        }
        AIDA::IHistogram1D* theHisto = initializeHisto(prefix+var+'_'+bx, x_min, x_max, nbin);
        (*hMap)[prefix+var]=theHisto;  // store in map
      }

      // the raw histos for R
      if ( m_rawHistos ) {
        std::string prefix="RawR_";
        // defaults for threshold histos
        double x_min = 0;
        double x_max = 1.;
        unsigned int nbin = m_NumBin;
        if ( var.find("_threshold") == std::string::npos ) {
          // direct R
	  int bins = m_MaxBin;
	  if (useMaxBins) {
	    bins = m_MaxBins[i];
	  }
	  double x0 = -0.5;
	  x_max = bins + x0;
          int n = std::min(m_NumBin, bins);
	  double width = (double) bins/n;
          x_min = x0 - width;
          nbin = n+1;
        }
        AIDA::IHistogram1D* theHisto = initializeHisto(prefix+var+'_'+bx, x_min, x_max, nbin);
        (*hMap)[var]=theHisto;  // store in map
      }
    }
  }

  // the trend histos
  std::string prefix="Trend_";
  for (std::vector<std::string>::iterator iVar = m_Averages.begin(); iVar != m_Averages.end(); ++iVar) {
    std::string var=*iVar;
    AIDA::IHistogram1D* trendHisto = initializeHisto(prefix+var, -m_trendSize*m_trendInterval, 0, m_trendSize);
    m_trendMap[var] = trendHisto;  // store in trend map
  }
}

//---------------------------------------------------------
StatusCode LumiAnalyser::accumulate() {
  debug() << "LumiAnalyser::accumulate()" << endmsg;

  // get data container
  if( !exist<LHCb::HltLumiSummary>(m_DataName) ){
    StatusCode sc = Warning("LumiSummary cannot be loaded",StatusCode::FAILURE);
    return sc;
  }
  LHCb::HltLumiSummary* HltLumiSummary = get<LHCb::HltLumiSummary>(m_DataName);

  // get ODIN to determine BXType
  if( !exist<LHCb::ODIN>(LHCb::ODINLocation::Default) ){
    StatusCode sc = Error("ODIN cannot be loaded",StatusCode::FAILURE);
    return sc;
  }
  LHCb::ODIN* odin = get<LHCb::ODIN> (LHCb::ODINLocation::Default);

  // bunch crossing type
  std::stringstream sbxType("");
  sbxType << (LHCb::ODIN::BXTypes) odin->bunchCrossingType();
  std::string bxType = sbxType.str();
  ++m_allBxTypes;  // count all types

  // protection
  if (m_theStore.find(bxType) == m_theStore.end() ) {
    warning() << "unknown BX Type: " << bxType << endmsg;    
    return StatusCode::SUCCESS;  
  }
  int ib = 0;
  for (std::vector<std::string>::iterator iBx = m_BXTypes.begin(); iBx != m_BXTypes.end(); ++iBx, ++ib) {
    std::string bx=*iBx;
    if ( bx == bxType ) {
      m_bxHisto->fill((double) ib);
      ++m_countBxTypes[ib];
    }
  }

  // use the storage belonging to the bxType
  iiMap *theMap = m_theStore[bxType];
  
  int i = 0;
  std::string prefix="In_";
  for (std::vector<std::string>::iterator iVar = m_Variables.begin(); iVar != m_Variables.end(); ++iVar,++i) {
    std::string var=*iVar;
    int counter = LHCb::LumiCounters::counterKeyToType(var);
    if ( counter == LHCb::LumiCounters::Unknown ) {
      warning() << "LumiCounter not found with name: " << var <<  endmsg;
    }
    else {
      int ivalue = HltLumiSummary->info(counter,-1);
      if (ivalue > -1) {
        ((*theMap)[var])->first += 1;
        ((*theMap)[var])->second +=(long) ivalue;
        ((*theMap)[var+"_threshold"])->first += 1;
        if ( ivalue > m_Thresholds[i] ) {
          ((*theMap)[var+"_threshold"])->second += 1;
        }
      }
      // fill the input histos
      if ( m_inHistos ) {
        ((*(m_histoStore[bxType]))[prefix+var])->fill((double) ivalue);
      }
    }
  }
  return StatusCode::SUCCESS;
}

//---------------------------------------------------------
StatusCode LumiAnalyser::analyse() {
  debug() << "LumiAnalyser::analyse()" << endmsg;

  // take delta from all counters
  for (std::vector<std::string>::iterator iVar = m_Averages.begin(); iVar != m_Averages.end(); ++iVar) {
    std::string var=*iVar;
    double meanR = 0;  // for R calculation
    for (std::vector<std::string>::iterator iBx = m_BXTypes.begin(); iBx != m_BXTypes.end(); ++iBx) {
      std::string bx=*iBx;
      iiMap *theMap=m_theStore[bx];
      iiMap *prevMap=m_prevStore[bx];
      iiPair *thePair = (*theMap)[var];
      iiPair *prevPair = (*prevMap)[var];
      // subtract the previous results from the present ones and store in the previous
      iiPair deltaPair;
      deltaPair.first = thePair->first-prevPair->first;
      deltaPair.second = thePair->second-prevPair->second;
      debug() << " subtraction result: " << bx << "/" << var 
              << " entries: " << thePair->first << " - " << prevPair->first << " = " << deltaPair.first
              << " sums: " << thePair->second << " - " << prevPair->second << " = " << deltaPair.second
              << endmsg;
      // add and subtract according to crossing type
      double norm = deltaPair.first;
      double sums = deltaPair.second;
      double R = 0;
      if (norm) {R = sums/norm;}
      // add or subtract this crossing type
      for (std::vector<std::string>::iterator iABx = m_addBXTypes.begin(); iABx != m_addBXTypes.end(); ++iABx) {
        if ( (*iABx) == bx ) {
          meanR += R;
          break;
        }
      }
      for (std::vector<std::string>::iterator iSBx = m_subtractBXTypes.begin(); iSBx != m_subtractBXTypes.end(); ++iSBx) {
        if ( (*iSBx) == bx ) {
          meanR -= R;
          break;
        }
      }
      // save the data in store for the next time
      prevPair->first = thePair->first;
      prevPair->second = thePair->second;
      // fill the rawR histos
      if ( m_rawHistos ) {
        (*(m_histoStore[bx]))[var]->fill(R);
      }
      
    }
    // R-calculation
    m_resultMap[var] = meanR;
    
    debug() << var << ": R from pairs: " << meanR << endmsg;
    // store trend
    storeTrend(var, meanR);
  }

  // update DIM locations
  int k = 0;
  for(std::vector< std::string >::iterator ivar = m_Variables.begin() ; 
      ivar!= m_Variables.end() ; ++ivar, ++k ){  
    m_means[k] = m_resultMap[*ivar];
    m_thresholds[k] = m_resultMap[(*ivar)+"_threshold"];
    debug() << "DIM data: " << *ivar << " " << m_means[k] << " and " << m_thresholds[k] << endmsg;
  }
  return StatusCode::SUCCESS;
  
}

//=============================================================================
ulonglong LumiAnalyser::gpsTime() {

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
bool LumiAnalyser::gpsTimeInterval() 
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
void LumiAnalyser::storeTrend(std::string varname, double lumiValue) 
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
