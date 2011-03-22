#include <string.h>
#include <stdlib.h>
#include "Gaucho/dimhist.h"
//#include "Gaucho/MonSubSys.h"
#include "Gaucho/MonCounter.h"
#define dimtype double
//#include "CCPCHist.h"
//#include "Gaucho/ObjService.h"
//#include "Gaucho/MonServer.h"

#include <math.h>
#include <iterator>

#define MIN( x , y)  =   (( (x) < (y) ) ? (x) : (y) )
#define MAX( x , y)  =   (( (x) > (y) ) ? (x) : (y) )

//static DimHistbuff1 dumbuf1;
//static DimHistbuff2 dumbuf2;

void MonCounter::setup(MONTYPE typ, void *data,char *name, char *title)
{
  m_type  = typ;
  m_titlen = 0;
  m_title = 0;
  m_contsiz    = 0;
  m_addoff = 0;
  m_contents = data;
  m_service = 0;
  setname(name);
  Init(title);
  m_srvcprefix = std::string("");
  switch (m_type)
  {
  case   C_INT:
    {
//      m_srvcprefix = std::string("MonI/");
      break;
    }
  case   C_LONGLONG:
    {
//      m_srvcprefix = std::string("MonL/");
      break;
    }
  case   C_FLOAT:
    {
//      m_srvcprefix = std::string("MonF/");
      break;
    }
  case   C_DOUBLE:
    {
//      m_srvcprefix = std::string("MonD/");
      break;
    }
  case   C_VOIDSTAR:
    {
      m_srvcprefix = std::string("");
      break;
    }
  default:
    {
      break;
    }
  }
  return;
}
MonCounter::MonCounter()
{
  setup(H_ILLEGAL,0,0,0);
}
MonCounter::MonCounter(char *name, char *title, int *data )
{
  setup(C_INT,data,name,title);
}
MonCounter::MonCounter(char *name, char *title, long long *data )
{
  setup(C_LONGLONG,data,name,title);
}
MonCounter::MonCounter(char *name, char *title, float *data )
{
  setup(C_FLOAT,data,name,title);
}
MonCounter::MonCounter(char *name, char *title, double *data )
{
  setup(C_DOUBLE,data,name,title);
}
MonCounter::MonCounter(char *name, char *title, std::string fmt, void *data , int size)
{
  setup(C_VOIDSTAR,data,name,title);
  m_fmt = fmt;
  m_contsiz = size;
  buffersize = 0;
}
int MonCounter::Init(char *title)
{
  m_titlen = strlen(title);
  m_title = (char*)malloc(m_titlen+1);
  strcpy(m_title,title);
  m_title[m_titlen]  = 0;
  if (m_type == C_INT)
  {
    m_contsiz    = 4;
    buffersize = 8;
  }
  else if (m_type == C_LONGLONG)
  {
    m_contsiz    = 8;
    buffersize = 8;
  }
  else if (m_type == C_FLOAT)
  {
    m_contsiz    = 4;
    buffersize = 8;
  }
  else if (m_type == C_DOUBLE)
  {
    m_contsiz    = 8;
    buffersize = 8;
  }
  return 0;
}
MonCounter::~MonCounter()
{
  if (m_titlen != 0)
  {
    free(m_title);
    m_titlen  = 0;
  }
  if (m_namelen != 0)
  {
    free(m_name);
    m_namelen = 0;
  }
  if (this->m_service != 0)
  {
    delete m_service;
    m_service = 0;
  }
}
int MonCounter::setname ( char* name)
{
  int allocsiz = strlen(name)+1;
  m_name = (char*)malloc(allocsiz);
//  memset(m_name,0,allocsiz);
  strcpy(m_name,name);
  m_namelen = strlen(name);
  m_name[m_namelen]=0;
  return 0;
}
const char *MonCounter::name() const
{
  return m_name;
}
void *MonCounter::cpyName(void *p)
{
  char *ptr = (char*)p;
  strncpy(ptr,m_name,this->m_namelen);
  ptr[m_namelen] = 0;
  ptr = (char*)ptr+m_namelen;
  return p;
}
void MonCounter::clear(void)
{
  if (m_contents != 0)
  {
    memset(m_contents,0,m_contsiz);
  }
  return;
}
int MonCounter::datasize()
{
  return buffersize;
}
int MonCounter::hdrlen()
{
  int s;
  s = sizeof(DimBuffBase)+titlen()+1+namelen()+1;
  s = (s + 7)&~7;   //align to 8-byte boundary...
  return s;
}
int MonCounter::xmitbuffersize()
{
  if (m_type == C_VOIDSTAR)
  {
    return 0;
  }
  int s;
  s = hdrlen();
  s += datasize();
  return s;
}
int MonCounter::titlen(void)
{
  return (m_titlen+1);//+3)&(~3); //align to 32-bits
}
int MonCounter::namelen(void)
{
  return (m_namelen+1);//+3)&(~3); //align to 32 bits
}
void *MonCounter::cpytitle(void *p)
{
  char *ptr = (char*)p;
  memcpy(ptr,m_title,m_titlen);
  ptr[m_titlen] = 0;
  return ptr;
}
int MonCounter::serialize(void* &ptr)
{
  if (m_type == C_VOIDSTAR)
  {
    return 0;
  }
  int siz;
  int titl = titlen();
  int naml = namelen();
  int hdrl = hdrlen();
  siz = hdrl+datasize();//2*(p->m_nx+2)*sizeof(dimtype);
  DimBuffBase *pp = (DimBuffBase*)ptr;
  pp->dataoff = hdrl;
  pp->addoffset = m_addoff;
  pp->reclen = siz;
  pp->titlen = titl;
  pp->namelen = naml;
  pp->type = m_type;
  char *tit;
  char *nam;
  pp->nameoff = sizeof(DimBuffBase);
  pp->titoff  = pp->nameoff+namelen();
  nam = (char*)AddPtr(pp,pp->nameoff);
  tit = (char*)AddPtr(pp,pp->titoff);
  cpyName(nam);
  cpytitle(tit);
  switch (m_type)
  {
  case   C_INT:
    {
      long long *dst = (long long *)AddPtr(pp,pp->dataoff);
      *dst = *(int*)m_contents;
      break;
    }
  case   C_LONGLONG:
    {
      long long *dst = (long long *)AddPtr(pp,pp->dataoff);
      *dst = *(long long*)m_contents;
      break;
    }
  case   C_FLOAT:
    {
      double *dst = (double *)AddPtr(pp,pp->dataoff);
      *dst = *(float*)m_contents;
      break;
    }
  case   C_DOUBLE:
    {
      double *dst = (double *)AddPtr(pp,pp->dataoff);
      *dst = *(double*)m_contents;
      break;
    }
  default:
    {
      break;
    }
  }
  ptr = (void*)((char*)ptr+siz);
  return siz;
}
bool MonCounter::nameeq(char *nam, int namlen)
{
  bool r;
  r = (namlen == m_namelen);
  r = r && (strcmp(m_name,nam) == 0);
  return r;
}
void MonCounter::List()
{
  m_srvcprefix = std::string("");
  std::string typ;
  switch (m_type)
  {
  case   C_INT:
    {
      typ = std::string("C_INT");
//      m_srvcprefix = std::string("MonI/");
      break;
    }
  case   C_LONGLONG:
    {
      typ = std::string("C_LONGLONG");
//      m_srvcprefix = std::string("MonL/");
      break;
    }
  case   C_FLOAT:
    {
      typ = std::string("C_FLOAT");
//      m_srvcprefix = std::string("MonF/");
      break;
    }
  case   C_DOUBLE:
    {
      typ = std::string("C_DOUBLE");
//      m_srvcprefix = std::string("MonD/");
      break;
    }
  case   C_VOIDSTAR:
    {
      typ = std::string("C_VOIDSTAR");
//      m_srvcprefix = std::string("");
      break;
    }
  default:
    {
//      typ = std::string("UNKNOWN");
      break;
    }
  }
//  printf("Counter Type: %s Name %s\n",typ.c_str(),name());
}
void MonCounter::create_OutputService(std::string infix)
{
  std::string nam;
  if (m_service != 0) return;
  nam = m_srvcprefix+infix+std::string(m_name);
  switch(m_type)
  {
    case C_INT:
    {
      this->m_service = new DimService(nam.c_str(),*(int*)m_contents);
      break;
    }
    case C_LONGLONG:
    {
      this->m_service = new DimService(nam.c_str(),*(long long*)m_contents);
      break;
    }
    case C_FLOAT:
    {
      this->m_service = new DimService(nam.c_str(),*(float*)m_contents);
      break;
    }
    case C_DOUBLE:
    {
      this->m_service = new DimService(nam.c_str(),*(double*)m_contents);
      break;
    }
    case C_VOIDSTAR:
    {
      this->m_service = new DimService(nam.c_str(),(char*)m_fmt.c_str(),m_contents, m_contsiz);
      break;
    }
    default:
    {
      m_service = 0;
      break;
    }
  }
  return;
}
void MonCounter::delete_OutputService()
{
  if (m_service != 0)
  {
    delete m_service;
    m_service = 0;
  }
}
DimService *MonCounter::getDimService()
{
  return this->m_service;
}

