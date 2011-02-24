#include "Gaucho/MonSubSys.h"
//#include "HistServer.h"
#include "Gaucho/ObjService.h"
#include "Gaucho/MonTimer.h"
#include "Gaucho/ObjSerializer.h"
#include "Gaucho/MonObj.h"
#include "Gaucho/MonSys.h"
#include "Gaucho/MonCounter.h"
//static int mpty;
typedef std::pair<std::string, MonObj*> SysPair;
typedef ObjMap::iterator SysIter;
static long mpty;
MonSubSys::MonSubSys(int intv)
{
  //m_serv = new ObjServer();
//  bufsiz = 0;
//  buffersize = 0;
  m_numObjs = 0;
  start_done = false;
  m_updateTimer = new MonTimer(this,intv);
  char pid[255];
  sprintf(pid,"MON_SUBSYS_%0X_%0lX",lib_rtl_pid(),(unsigned long)this);
  m_lockcnt = 0;
  m_lockcount=0;
  m_unlockcount=0;
//  lib_rtl_create_lock(pid,&m_lockid);
  lib_rtl_create_lock(0,&m_lockid);
  m_rpc = 0;
  m_ser = 0;
  m_genSrv = 0;
  m_EORsvc = 0;

}
MonSubSys::~MonSubSys()
{
  delete m_updateTimer;
  SysIter i;
  this->Lock();
  if (m_expandnames)
  {
    if (m_type == MONSUBSYS_Counter)
    {
      for (SysIter i =m_Objmap.begin();i!=m_Objmap.end();i++)
      {
        MonCounter *h = (MonCounter*)i->second;
        h->delete_OutputService();
      }
    }
  }
  for (i =m_Objmap.begin();i!=m_Objmap.end();i++)
  {
    MonObj *h = i->second;
    delete h;
  }
  m_Objmap.clear();
  if (m_rpc != 0) {delete m_rpc;m_rpc=0;}
  if ( m_genSrv != 0) {delete m_genSrv;m_genSrv=0;}
  if (m_EORsvc != 0) {delete m_EORsvc;m_EORsvc=0;}

  if (m_ser != 0) {delete m_ser;m_ser=0;}

  this->unLock();
  MonSys::m_instance().remSubSys(this);
  lib_rtl_delete_lock (m_lockid);

}
void MonSubSys::start()
{
  if (!start_done)
  {
    //DimServer::start(m_name.c_str());
    m_updateTimer->Start();
    start_done = true;
  }
}
void MonSubSys::stop()
{
  if (start_done)
  {
    //DimServer::start(m_name.c_str());
    m_updateTimer->Stop();
    start_done = false;
  }
}
void MonSubSys::addObj(MonObj* h)
{
  std::string n(h->name());
//  this->Lock();
  m_Objmap.insert(SysPair(n,h));
  m_numObjs++;
//  m_Objmap.push_back(h);
//  calcBufferSize();
//  this->unLock();
}
void MonSubSys::removeAll()
{
  ObjMap::iterator i;
  for (i=m_Objmap.begin();i!= m_Objmap.end();i++)
  {
    delete i->second;
  }
  m_Objmap.clear();
  m_numObjs = 0;
}
void MonSubSys::removeObj(MonObj* h)
{
  if (h == 0) return;
  std::string n(h->name());
//  this->Lock();
  m_Objmap.erase(n);
//  calcBufferSize();
  m_numObjs--;
//  this->unLock();
}
/*
void MonSubSys::calcBufferSize()
{
//  std::vector <int>::size_type i;
  SysIter i;
//  bufsiz = 0;
  for (i =m_Objmap.begin();i!=m_Objmap.end();i++)
  {
    MonObj *h = i->second;//m_Objmap[i];
    bufsiz += h->xmitbuffersize();
  }
}
*/
MonObj *MonSubSys::findobj(const char *nam)
{
  SysIter i;
  //std::vector <int>::size_type i;
  //int namlen;
  std::string n(nam);
  i = m_Objmap.find(n);
  if (i == m_Objmap.end())
  {
    return 0;
  }
  else
  {
    MonObj *h;
    std::string x;
    h = i->second;
    x = i->first;
    return h;
  }
}
/*
 void *MonSubSys::Allocate(int siz)
{
  if (buffersize < siz)
  {
//    printf("re-allocaing %d\n",siz);
    free(buffer);
    buffer=malloc(siz);
    buffersize = siz;
  }
  return buffer;
}
*/
void MonSubSys::setup(char *n, bool expandnames)
{
  m_expandnames = expandnames;
  char procname[64];
  lib_rtl_get_process_name(procname,sizeof(procname));
  std::string nam;
  m_name  = n;
  m_pname = procname;
  std::string nodename;
  nodename = RTL::nodeNameShort();
  nam = /*nodename+"_*/std::string("MON_")+m_pname+"/"+m_name+"/HistCommand";
  if (m_ser == 0) m_ser = new ObjSerializer(&m_Objmap,m_expandnames);
  if (m_expandnames)
  {
    if (m_type == MONSUBSYS_Counter)
    {
      for (SysIter i =m_Objmap.begin();i!=m_Objmap.end();i++)
      {
        MonCounter *h = (MonCounter*)i->second;
        h->create_OutputService(m_expandInfix);
      }
    }
  }
  if (m_rpc == 0) m_rpc = new ObjRPC(m_ser, (char*)nam.c_str(), (char*)"I:1;C",(char*)"C");
  nam = /*nodename+"_*/std::string("MON_")+m_pname+"/"+m_name+"/Data";
  if ( m_genSrv == 0) m_genSrv = new ObjService(m_ser,(char*)nam.c_str(),(char*)"C",(void*)&mpty, 4);
  if (m_expandnames)
  {
    m_genSrv->m_expandservice = m_expandnames;
  }
  nam = m_name;
  nam = /*nodename+"_*/std::string("MON_")+m_pname+"/"+m_name+"/EOR";
  if (m_EORsvc == 0) m_EORsvc = new ObjService(m_ser,(char*)nam.c_str(),(char*)"C",(void*)&mpty,4);
  m_EORsvc->setEORflag(true);
  MonSys::m_instance().addSubSys(this);
//  unLock();
  return;
}

void MonSubSys::Clear(char *name)
{
  MonObj *h =findobj(name);
  if (h != 0)
  {
    h->clear();
  }
}
void MonSubSys::Clear()
{
  SysIter i;
  for (i =m_Objmap.begin();i!=m_Objmap.end();i++)
  {
    MonObj *h = i->second;//m_Objmap[i];
    h->clear();
  }
}
void MonSubSys::EORUpdate(int runo)
{
  m_EORsvc->setRunNo(runo);
  m_EORsvc->Serialize();
  dim_lock();
  m_EORsvc->Update();
  dim_unlock();
}
int MonSubSys::Lock(void)
{
  int status = 0;
  while (status != 1)
  {
    status = lib_rtl_lock(m_lockid);
    if (status != 1)
    {
//      printf("Status from lock not success......\n");
    }
  }
  m_lockcnt++;
  m_lockcount++;
  if (m_lockcnt != 1)
  {
//    printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!Lock Counter != 1 after locking %d %d %d\n",m_lockcnt,m_lockcount,m_unlockcount);
  }
//  printf("+++++++++++Monitor Sub System LockED. Lock Address %0X\n",m_lockid);
  return status;
}
int MonSubSys::unLock(void)
{
//  printf("Monitor Sub System Un-Locking\n");
  int status = 0;
  m_lockcnt--;
  m_unlockcount++;
  while (status != 1)
  {
    status = lib_rtl_unlock(m_lockid);
    if (status != 1)
    {
//      printf("Status from unlock not success......\n");
    }
  }
//  printf("-----------Monitor Sub System Un-LockED. Lock Address %0X\n",m_lockid);
  return status;
}
void MonSubSys::List()
{
  SysIter i;
  for (i =m_Objmap.begin();i!=m_Objmap.end();i++)
  {
    MonObj *h = i->second;//m_Objmap[i];
    h->List();
  }
}
void MonSubSys::setRunNo(int runno)
{
  this->m_runno = runno;
}

