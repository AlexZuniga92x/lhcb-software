#include "MEPSvc.h"
#include "GaudiKernel/SvcFactory.h"
#include "GaudiKernel/IIncidentSvc.h"
#include "GaudiKernel/IHistogramSvc.h"

#include "Gaucho/HistAdder.h"
#include "Gaucho/CounterAdder.h"
#include "Gaucho/SaveTimer.h"
#include "Gaucho/AdderSys.h"
#include "Gaucho/Utilities.h"
#include "Gaucho/IGauchoMonitorSvc.h"
#include "AIDA/IHistogram.h"
#include "Gaucho/MonCounter.h"
#include "Trending/ITrendingTool.h"
#include "Trending/ISimpleTrendWriter.h"

DECLARE_SERVICE_FACTORY(MEPSvc)
static std::string DetNames[] = {"TDet","VeloA","VeloC","TT","IT","OTA","OTC","RICH1","RICH2","ECal","HCal",
    "MuonA","MuonC","L0DU","TMuA","TMuC","TPU","PRS","TCalo","LHCb"};
static std::vector<std::string> s_counterTypes;

static std::map<std::string,std::string> s_nammap;
namespace MEPSVC
{
  void Analyze(void *arg, void* buff ,int siz, MonMap *mmap, MonAdder *)
  {
    MEPSvc *tis = (MEPSvc*)arg;
    tis->analyze(buff,siz, mmap);
  }
}

class MyErrh : public DimErrorHandler
{
public:
  bool m_flag;
  void errorHandler(int severity, int code, char *msg)
  {
    if (m_flag)
      {
        ::lib_rtl_output(LIB_RTL_INFO,"DIM(MEPSvc): Code %d.%x %s\n",severity,code,msg);
      }
    return;
  }
  MyErrh () : DimErrorHandler()
  {
    m_flag = true;
  }
  void start()
  {
    m_flag = true;
  }
  void stop()
  {
    m_flag = false;
  }
};

void MEPSvc::filldata(const std::string &cnam,MonMap *mmap)
{
  filldata("Runable/",cnam,mmap);
}
void MEPSvc::filldata(const std::string &prefix,const std::string &cnam,MonMap *mmap)
{
  MonMap::iterator it;
  DetData_T<long long>& lb = m_DetMap.find("LHCb")->second;
  it = mmap->find(prefix+cnam);
  if ( it != mmap->end() )
  {
    CntrDescr *h = (CntrDescr*)MonCounter::de_serialize(it->second);
    long long *p = (long long*)h->ptr;
    for (int i=0;i<h->nel;i++)
    {
      TellMap_T::iterator t=m_TellMap.find(i);
      if (t != m_TellMap.end())
      {
//        printf("%d %s %s\n", i, t->second.det.c_str(),t->second.name.c_str());
        DetData_T<long long>& d= m_DetMap.find(t->second.det)->second;
        DetData_T<long long>::iterator k = d.find(cnam);

        if (k != d.end())
        {
          k->second += p[i];
          lb[cnam] += p[i];
        }
        else
        {
//          printf ("Cannot find %s in Detector Data Map\n",cnam.c_str());
        }
      }
    }
  }
}
void MEPSvc::fillrate(const std::string &prefix,const std::string &cnam,MonMap *mmap)
{
  MonMap::iterator it;
  DetData_T<double>& lb = m_DetMap_rate.find("LHCb")->second;
  it = mmap->find(prefix+cnam);
  if ( it != mmap->end() )
  {
    CntrDescr *h = (CntrDescr*)MonCounter::de_serialize(it->second);
    double *p = (double*)h->ptr;
    for (int i=0;i<h->nel;i++)
    {
      TellMap_T::iterator t=m_TellMap.find(i);
      if (t != m_TellMap.end())
      {
//        printf("%d %s %s\n", i, t->second.det.c_str(),t->second.name.c_str());
        DetData_T<double>& d= m_DetMap_rate.find(t->second.det)->second;
        DetData_T<double>::iterator k = d.find(cnam);

        if (k != d.end())
        {
          double pp;
          pp = p[i];
          k->second += pp;
          lb[cnam] += pp;
        }
        else
        {
//          printf ("Cannot find %s in Detector Data Map\n",cnam.c_str());
        }
      }
    }
  }
}
void MEPSvc::fillsums()//const std::string &cnam,MonMap *mmap)
{
  DetMap_T<long long>::iterator i;
  DetMap_T<double>::iterator k;
//  DetData_T<long long>& lb = m_DetMap.find("LHCb")->second;
  for (i=m_DetMap.begin();i!=m_DetMap.end();i++)
  {
    for (int j=0;j<5;j++)
    {
      i->second["TLostMEP"] += i->second[s_counterTypes[j]];
    }
  }
  for (k=m_DetMap_rate.begin();k!=m_DetMap_rate.end();k++)
  {
    for (int j=0;j<5;j++)
    {
      k->second["TLostMEP"] += k->second[s_counterTypes[j]];
    }
  }
}
StatusCode MEPSvc::initialize()
{
  StatusCode sc = PubSvc::initialize();
  if (sc.isSuccess() && m_enableTrending && m_trender == 0)
  {
    SmartIF<IToolSvc> tools;
    sc = serviceLocator()->service("ToolSvc", tools.pRef());
    if ( !sc.isSuccess() ) {
      ::lib_rtl_output(LIB_RTL_FATAL,"DIM(OvrSvc): Failed to access ToolsSvc.\n");
      return sc;
    }
    sc = tools->retrieveTool("SimpleTrendWriter","MEPSvc",m_trender,this);
    if (sc.isSuccess() && m_trender != 0)
    {
      std::string nnam("MEPRx_Trends");
      m_trender->setPartitionAndName(this->m_PartitionName,nnam);
      m_trender->setMaxTimeNoWrite(600);
    }
  }
  return sc;
}

StatusCode MEPSvc::finalize()
{
  StatusCode sc;
  if (m_trender != 0)
  {
//    m_trender->close();
    SmartIF<IToolSvc> tools;
    sc = serviceLocator()->service("ToolSvc", tools.pRef());
    if ( !sc.isSuccess() ) {
      ::lib_rtl_output(LIB_RTL_FATAL,"DIM(RateSvc): Failed to access ToolsSvc.\n");
      return sc;
    }
    sc = tools->releaseTool(m_trender);
    m_trender = 0;
  }
  Service::finalize();
  return StatusCode::SUCCESS;
}

StatusCode MEPSvc::start()
{
  PubSvc::start();
  fillTellMap();
  m_DetMap_old = m_DetMap;
  this->m_DetMap_rate.setServiceName("Stat/"+m_myservicename+"/Rate");
  this->m_DetMap.setServiceName("Stat/"+m_myservicename+"/Count");
  m_adder->SetCycleFn(MEPSVC::Analyze,this);
  return StatusCode::SUCCESS;
}
void MEPSvc::analyze(void *, int ,MonMap* mmap)
{
  m_DetMap.Zero();
  m_DetMap_rate.Zero();
  for(size_t i=0; i<s_counterTypes.size();i++)
  {
    filldata(s_counterTypes[i],mmap);
    fillrate(std::string("R_Runable/"),s_counterTypes[i],mmap);
  }
  this->fillsums();
  m_DetMap.Update();
//  m_DetMap.dump();
//  m_DetMap_rate.dump();
  m_DetMap_rate.Update();
  DetMap_T<double>::iterator ii = m_DetMap_rate.find(m_PartitionName/*"LHCb"*/);
  if (ii!=m_DetMap_rate.end())
  {
    DetData_T<double>::iterator jj = ii->second.find("rxOct");
    if (jj!=ii->second.end())
    {
      m_DataRate = (float)jj->second;
      if (m_LHCbDataRate == 0)
      {
        m_LHCbDataRate = new DimService((char*)(m_PartitionName+"_DataRate").c_str(),m_DataRate);
      }
      m_LHCbDataRate->updateService(m_DataRate);
//      printf("%s Total Data Rate %f\n",m_PartitionName.c_str(),m_DataRate);
    }
  }
  if (m_enableTrending)
  {
    m_trender->startEvent();
    m_trender->addEntry("TotalDataRate", (double)m_DataRate/1.0e09);
    DetMap_T<double>::iterator ii;// = m_DetMap_rate.find(m_PartitionName/*"LHCb"*/);
    for (ii=m_DetMap_rate.begin();ii!=m_DetMap_rate.end();ii++)
    {
      DetData_T<double>::iterator jj = ii->second.find("TLostMEP");
      if (jj!=ii->second.end())
      {
        double d = (double)jj->second;
        m_trender->addEntry(ii->first, d);
      }
    }
    m_trender->saveEvent();
  }
}

void MEPSvc::dump()
{
  printf("Dump of cumulated values\n");
  m_DetMap.dump();
  printf("Dump of differential values\n");
  m_DetMap_diff.dump();
}
MEPSvc::MEPSvc(const std::string& name, ISvcLocator* sl) : PubSvc(name,sl)
{
  declareProperty("Tell1List",       m_tell1List);
  declareProperty("TrendingOn",  m_enableTrending = true);
  m_trender = 0;

//  m_started     = false;
//  m_SaveTimer   = 0;
//  m_errh        = 0;
//  m_pMonitorSvc = 0;
//  m_phistsvc    = 0;
//  m_arrhist     = 0;
  if (s_nammap.empty())
  {
    s_nammap["ectell"]  = "ECAL";
    s_nammap["hctell"]  = "HCAL";
    s_nammap["tttell"]  = "TT";
    s_nammap["ittell"]  = "IT";
    s_nammap["mutella"] = "MUONA";
    s_nammap["mutellc"] = "MUONC";
    s_nammap["ottella"] = "OTA";
    s_nammap["ottellc"] = "OTC";
    s_nammap["pstell"]  = "PRS";
    s_nammap["tcatell"] = "TCALO";
    s_nammap["trgtell1"] = "L0DU";
    s_nammap["tputell"] = "TPU";
    s_nammap["r1ukl1"]    ="RICH1";
    s_nammap["r2ukl1"]    = "RICH2";
    s_nammap["vetella"] = "VELOA";
    s_nammap["vetellc"] = "VELOC";
    s_nammap["dumtell"] = "TDET";
    s_nammap["tmutellq01"] = "TMUA";
    s_nammap["tmutellq02"] = "TMUA";
    s_nammap["tmutellq12"] = "TMUA";
    s_nammap["tmutellq03"] = "TMUC";
    s_nammap["tmutellq04"] = "TMUC";
    s_nammap["tmutellq34"] = "TMUC";
    s_nammap["tfcodin"] = "TFC";
  }
  if (s_counterTypes.empty())
  {
    s_counterTypes.push_back("badLenPkt");
    s_counterTypes.push_back("misPkt");
    s_counterTypes.push_back("badPckFktPkt");
    s_counterTypes.push_back("truncPkt");
    s_counterTypes.push_back("multipleEvt");
    s_counterTypes.push_back("rxOct");
    s_counterTypes.push_back("rxPkt");
    s_counterTypes.push_back("rxEvt");
    s_counterTypes.push_back("rxMEP");
    s_counterTypes.push_back("TLostMEP");
  }
  m_LHCbDataRate = 0;
}

MEPSvc::~MEPSvc()
{
  deletePtr(m_LHCbDataRate);
}

StatusCode MEPSvc::queryInterface(const InterfaceID& riid, void** ppvIF)
{
  if ( IIncidentListener::interfaceID().versionMatch(riid) )
  {
    *ppvIF = (IIncidentListener*)this;
  }
  else
  {
    return Service::queryInterface(riid, ppvIF);
  }
  addRef();
  return StatusCode::SUCCESS;
}

void MEPSvc::fillTellMap()
{
  typedef std::map<std::string,std::string> StringMap;
  unsigned int i;
  for (i = 0;i<m_tell1List.size();i+=3)
  {
    std::string nam = m_tell1List[i+1];
    std::string snam = nam.substr(0,nam.length()-2);
    if (snam == "tmutellq")
    {
      snam = nam;
    }
    if (nam.substr(0,7) == "tfcodin") snam = "tfcodin";
    StringMap::const_iterator j = s_nammap.find(snam);
    m_TellMap.insert(std::make_pair(i/3,TellDesc(nam,j==s_nammap.end() ? "UNKNOWN" : (*j).second)));
  }

  DetData_T<long long> a;
  DetData_T<double> f;
  for(size_t i=0; i<s_counterTypes.size();i++)
  {
    a[s_counterTypes[i]] = 0;
  }
  for(size_t i=0; i<s_counterTypes.size();i++)
  {
    f[s_counterTypes[i]] = 0.0;
  }
  for(StringMap::const_iterator i=s_nammap.begin(); i!=s_nammap.end();++i)
  {
    m_DetMap.insert(std::make_pair((*i).second,a));
    m_DetMap_rate.insert(std::make_pair((*i).second,f));
  }
  m_DetMap.insert(std::make_pair("LHCb",a));
  m_DetMap_rate.insert(std::make_pair("LHCb",f));
}

