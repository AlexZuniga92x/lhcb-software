// $Id: HltFunctionFactory.cpp,v 1.3 2007-06-27 06:01:49 hernando Exp $
// Include files
// from Gaudi
#include "GaudiKernel/ToolFactory.h" 

// local
#include "HltFunctionFactory.h"
#include "HltBase/HltConfigurationHelper.h"
#include "Event/HltSummary.h"
#include "HltBase/EParser.h"

using namespace LHCb;

//-----------------------------------------------------------------------------
// Implementation file for class : HltFunctionFactory
//
// 2007-03-22 : Jose Angel Hernando Morata
//-----------------------------------------------------------------------------

// Declaration of the Tool Factory
DECLARE_TOOL_FACTORY( HltFunctionFactory );


//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
HltFunctionFactory::HltFunctionFactory( const std::string& type,
                                        const std::string& name,
                                        const IInterface* parent )
  : GaudiTool ( type, name , parent )
{
  declareInterface<IHltFunctionFactory>(this);

  declareProperty("DataSummaryLocation", 
                  m_dataSummaryLocation = LHCb::HltSummaryLocation::Default);

  declareProperty("Smart", m_smart = true);
}

StatusCode HltFunctionFactory::initialize() {
  
  IDataProviderSvc* hltsvc = NULL;
  StatusCode sc = serviceLocator()->service("HltDataSvc/EventDataSvc",hltsvc);
  if (!hltsvc) fatal() << " not able to create Hlt Svc provider " << endreq;

  m_conf = NULL;
  std::string loca = m_dataSummaryLocation+"/Configuration";
  Hlt::DataHolder<Hlt::Configuration>* holder = 
    get<Hlt::DataHolder<Hlt::Configuration> >(hltsvc,loca);
  if (!holder) fatal() << " not able to retrieve configuration " << endreq;

  m_conf = &(holder->object());
  
  return sc;
}

Hlt::TrackFunction* HltFunctionFactory::trackFunction(const std::string& fn) 
{
  Hlt::TrackFunction* fun = NULL;
  std::vector<std::string> cromos = EParser::parse(fn,",");
  std::string name = cromos[0];
  debug() << " function name " << name << endreq;
  if (name == "rIP") {
    if (!m_vertices) error() << " vertices not set in factory " << endreq;
    fun =  new Estd::binder_function<Track,RecVertex>(Hlt::rIP(),*m_vertices,
                                                      Estd::abs_min());
    debug() << " created function rIP " << name << endreq;
  } else if (name == "IP") {
    if (!m_vertices) error() << " vertices not set in factory " << endreq;
    fun =  new Estd::binder_function<Track,RecVertex>(Hlt::IP(), *m_vertices,
                                                      Estd::abs_min());
    debug() << " created function IP " << name << endreq;
  } else if (name == "PT") {
    fun = new Hlt::PT();
    debug() << " created function PT " << name << endreq;
  } else if (name == "DOCA") {
    if (!m_vertices) error() << " tracks not set in factory " << endreq;
    fun =  new Estd::binder_function<Track,Track>(Hlt::DOCA(),*m_tracks,
                                                  Estd::abs_min());
    debug() << " created function DOCA " << name << endreq;
  } else if (name == "IDsFraction") {
    if (!m_tracks) error() << " tracks [2] not set in factory " << endreq;
    fun =  new Estd::binder_function<Track,Track>(Hlt::MatchIDsFraction(), 
                                                  *m_tracks, Estd::abs_max());
    debug() << " created function IDsFrunction " << name << endreq;
  }
  if (m_smart && fun) {
    int id = HltConfigurationHelper::getID(*m_conf,"InfoID",name);
    Hlt::TrackFunction* fun1 = fun;
    fun = new Hlt::SmartFunction<Track>(id,*fun1);
    debug() << " created smart function " << name << " id " << id << endreq;
    delete fun1;
  }
  if (!fun) fatal() << " requested track function " << name 
                    << " not in factory " << endreq;
  return fun;
}


Hlt::VertexFunction* HltFunctionFactory::vertexFunction(const std::string& fn) {
  Hlt::VertexFunction* fun = NULL;
  std::vector<std::string> cromos = EParser::parse(fn,",");
  std::string name = cromos[0];
  if (name == "VertexDz") {
    if (!m_vertices) error() << " vertices not set in factory " << endreq;
    fun =  new Estd::binder_function<RecVertex,RecVertex>(Hlt::DZ(),*m_vertices,
                                                          Estd::abs_min());    
  } else if ( name == "VertexPointing") {
    if (!m_vertices) error() << " vertices not set in factory " << endreq;
    fun =  new Estd::binder_function<RecVertex,RecVertex>(Hlt::FC(),*m_vertices,
                                                          Estd::abs_min());
  } else if (name == "VertexMinPT") {
    fun = new Hlt::minPT();
  } else if (name == "VertexMaxPT") {
    fun = new Hlt::maxPT();
  }
  if (m_smart && fun) {
    int id = HltConfigurationHelper::getID(*m_conf,"InfoID",name);
    Hlt::VertexFunction* fun1 = fun;
    fun = new Hlt::SmartFunction<RecVertex>(id,*fun1);
    debug() << " created smart function " << name << " id " << id << endreq;
    delete fun1;
  }
  if (!fun) fatal() << " requested vertex function " << name
                    << " not in factory " << endreq;
  return fun;
}

Hlt::TrackBiFunction* HltFunctionFactory::trackBiFunction(const std::string& fn) {
  Hlt::TrackBiFunction* bfun = NULL;
  std::vector<std::string> cromos = EParser::parse(fn,",");
  std::string name = cromos[0];
  if (name == "DOCA")
    bfun = new Hlt::DOCA();
  if (!bfun) fatal() << " requested track bifunction " << name
                     << " not in factory " << endreq;
  return bfun;
}

Hlt::TrackFilter* HltFunctionFactory::trackFilter(const std::string& name) {

  std::string funname= "";
  std::string mode= "";
  float x0 = 0.;
  float xf = 1e6;
  if (!EParser::parseFilter(name,funname,mode,x0,xf)) {
    fatal() << " not able to parse " << name << endreq;
    return NULL;  
  }
  if (m_smart) {
    int id = HltConfigurationHelper::getID(*m_conf,"InfoID",funname);
    Hlt::Info<Track> info(id);
    return makeFilter(info,mode,x0,xf);
  } 
  Hlt::TrackFunction* fun = trackFunction(funname);
  Hlt::TrackFilter* filter = makeFilter(*fun,mode,x0,xf);
  delete fun;
  return filter;
}

Hlt::VertexFilter* HltFunctionFactory::vertexFilter(const std::string& name) {

  std::string funname= "";
  std::string mode= "";
  float x0 = 0.;
  float xf = 1e6;
  if (!EParser::parseFilter(name,funname,mode,x0,xf)) {
    fatal() << " not able to parse " << name << endreq;
    return NULL;  
  }
  if (m_smart) {
    int id = HltConfigurationHelper::getID(*m_conf,"InfoID",funname);
    Hlt::Info<RecVertex> info(id);
    return makeFilter(info,mode,x0,xf);
  }
  Hlt::VertexFunction* fun = vertexFunction(funname);
  Hlt::VertexFilter* filter = makeFilter(*fun,mode,x0,xf);
  delete fun;  
  return filter;
}


//=============================================================================
// Destructor
//=============================================================================
HltFunctionFactory::~HltFunctionFactory() {} 

//=============================================================================
