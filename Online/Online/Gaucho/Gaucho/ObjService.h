#ifndef ObjService_h
#define ObjService_h
#include "dis.hxx"
#include "ObjSerializer.h"
#include "ObjService.h"
#include "SerialHeader.h"
//class MonObj;

class ObjService : public DimService
{
protected:
  void **m_buffer;
  int *m_buffersize;
  bool m_extBuffer;
  ObjSerializer *m_ser;
  bool m_EORservice;
  SerialHeader m_hdr;
public:
  //void *p;
  bool m_expandservice;
  long long m_runnr;
  ObjService ();
  virtual ~ObjService();
  //ObjService (void *h, const char *name, char *format, void *buff, int siz);
  ObjService (ObjSerializer *,const char *name, char *format, void *buff, int siz);
  ObjService (ObjSerializer *,const char *name, char *format, void *buff, int siz, void **extbuff, int *extbufs);
//  ObjService (const char *name, char *format, void *buff, int siz, void **extbuff, int *extbufs);
  void Updater();
//  void setHeader(SerialHeader &hdr){m_hdr=hdr;};
  void setRunNo(int runno){m_hdr.run_number=runno;};
  void setTime(long long tim){m_hdr.ser_tim=tim;};
  void setEORflag(bool val);
  void setup(void);
};
#endif
