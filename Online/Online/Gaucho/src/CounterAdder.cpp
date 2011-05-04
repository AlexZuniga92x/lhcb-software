#include "Gaucho/CounterAdder.h"

#include "Gaucho/AdderSys.h"
#include "Gaucho/Utilities.h"
#include "Gaucho/IGauchoMonitorSvc.h"
#include "AIDA/IHistogram1D.h"

CounterAdder::CounterAdder(const char *myName, const char *serviceName)
{
  m_serviceName = std::string("/Counter/") + serviceName;
  m_MyName      = myName;
  m_type        = ADD_COUNTER;
  AdderSys::Instance().gg_AdderList.push_back(this);
}

CounterAdder::~CounterAdder()
{
}
void CounterAdder::add(void *buff, int siz, MonInfo *h)
{
  addBuffer(buff,siz,h);
}
void CounterAdder::addBuffer(void *buff, int siz, MonInfo *)
{
  //printf("Counter Adder: UNLocking MonitorSvc\n");
  MonMap hmap;
  void *bend   = AddPtr(buff,siz);
  void *hstart = AddPtr(buff,sizeof(SerialHeader));
  DimHistbuff1 *pp = (DimHistbuff1*)hstart;
  while (pp<bend)
  {
    char *nam = (char*)AddPtr(pp,pp->nameoff);
    hmap.insert(std::make_pair(nam,pp));
    pp=(DimHistbuff1*)AddPtr(pp,pp->reclen);
  }
  for (MonIter i=hmap.begin();i!=hmap.end();i++)
  {
    MonIter j = m_hmap.find(i->first);
    if (j!=m_hmap.end())
    {
      //printf("Counter Adder: Locking MAP\n");
      LockMap();
      DimHistbuff1 *sumh = (DimHistbuff1*)(j->second);
      DimHistbuff1 *srch = (DimHistbuff1*)(i->second);
      double *ps = (double*)AddPtr(sumh,sumh->dataoff);
      double *ph = (double*)AddPtr(srch,srch->dataoff);
      switch(srch->type)
      {
        case C_INT:
          {
            *(long long *)ps += *(long long*)ph;
            break;
          }
        case C_LONGLONG:
          {
            *(long long*)ps += *(long long*)ph;
            break;
          }
        case C_FLOAT:
          {
            *(float*)ps += *(float*)ph;
            break;
          }
        case C_DOUBLE:
        case C_RATE:
          {
            *(double*)ps += *(double*)ph;
            break;
          }
        case C_INTSTAR:
        {
          int n = (srch->reclen - srch->dataoff)/sizeof(int);
          int indx;
          int *sum = (int*)ps;
          int *src = (int*)ph;
          for (indx = 0;indx<n;indx++)
          {
            sum[indx]+=src[indx];
          }
          break;
        }
        case C_FLOATSTAR:
        {
          int n = (srch->reclen - srch->dataoff)/sizeof(float);
          int indx;
          float *sum = (float*)ps;
          float *src = (float*)ph;
          for (indx = 0;indx<n;indx++)
          {
            sum[indx]+=src[indx];
          }
          break;
        }
        case C_DOUBLESTAR:
        {
          int n = (srch->reclen - srch->dataoff)/sizeof(double);
          int indx;
          double *sum = (double*)ps;
          double *src = (double*)ph;
          for (indx = 0;indx<n;indx++)
          {
            sum[indx]+=src[indx];
          }
          break;
        }
        case C_LONGSTAR:
        {
          int n = (srch->reclen - srch->dataoff)/sizeof(long);
          int indx;
          long *sum = (long*)ps;
          long *src = (long*)ph;
          for (indx = 0;indx<n;indx++)
          {
            sum[indx]+=src[indx];
          }
          break;
        }
        default:
          {
            break;
          }
      }
      //printf("Counter Adder: UNLocking MAP\n");
      UnLockMap();
    }
    else
    {
      DimHistbuff1 *srch = (DimHistbuff1*)(i->second);
      int csiz = m_usedSize;
      int hsiz = srch->reclen;
      //printf("Counter Adder: Locking MAP\n");
      LockMap();
      void *p = ReAllocate(hsiz);
      if (p!=0)
      {
        p = AddPtr(p,csiz);
        memcpy(p,srch,hsiz);
        m_usedSize += hsiz;
        char *nam = (char*)AddPtr(srch,srch->nameoff);
        m_hmap.insert(std::make_pair(nam,p));
      }
      //printf("Counter Adder: UNLocking MAP\n");
      UnLockMap();
    }
  }
  m_received++;
  m_added++;
}
void CounterAdder::Update()
{
  if (m_received >= m_expected)
  {
    this->m_timer->Stop();
    //    //printf("Finished one cycle. Updating our service... %d %d\n", m_received,expected);
    if (m_isSaver)
    {
      //printf("Counter Adder: UNLocking\n");
      UnLock();
    }
    m_received = 0;
    ////printf(" %d %d\n", m_received,expected);
    if (m_outservice != 0)
    {
      m_outservice->Serialize();
      m_outservice->Update();
    }
    fflush(stdout);
    if (CycleFn!= 0)
    {
      (*CycleFn)(CycleCBarg, m_buffer, m_buffersize, &m_hmap, this);
    }
  }
}
