#ifndef CNTER_H
#define CNTER_H
#include "Gaucho/MonTypes.h"
#include "Gaucho/MonObj.h"
#include <vector>
#include <string>
#include "dim/dis.hxx"
#include "Utilities.h"
//#include "Gaucho/RateMgr.h"
//#define AddPtr(ptr,offs) (void*)((char*)ptr +offs)


class CntrDescr
{
public:
  std::string name;
  int type;
  long long data;
};

class MonCounter : public MonObj
{
//friend class HistService;
protected:
  MONTYPE m_type;
  void *m_contents;
  int m_namelen;
  char *m_title;      /** Pointer to Histogram Title **/
  int m_titlen;       /* Allocated length in Bytes for the title */
  int buffersize;     /* buffer size of the data if serialized*/
  unsigned int m_contsiz;      /* Allocated length in Bytes for the bin contents */
  int m_addoff;


  // Methods

  //void CopyData(int*,int*);
  //void CopyData(long long *,long long*);
  //void CopyData(float*,float*);
  //void CopyData(double*,double*);
public:
  char* m_name;    /** Name of the histogram **/
  const char *name() const;
  bool m_expandService;
  DimService *m_service;
  std::string m_srvcprefix;
  std::string m_fmt;
  //HistService *serv;
  int type() const {return (int)m_type;};
  MonCounter(char *name, char *title, int *data );
  MonCounter(char *name, char *title, long *data );
  MonCounter(char *name, char *title, long long *data );
  MonCounter(char *name, char *title, float *data );
  MonCounter(char *name, char *title, double *data );
  MonCounter(char *name, char *title, std::string fmt, void *data , int size);
  MonCounter();
  void setup(MONTYPE typ, void *ext, char *name, char *title);
  virtual ~MonCounter();
  int setname ( char* name);
  void clear(void);
  int titlen();
  int namelen();
  void *cpytitle(void *ptr);
  int Init(char *title);
  void *getextid (void);
  bool nameeq(char *nam, int namlen);
  int namelength(void) const
  {
    return m_namelen+1;
  }
  void *cpyName(void *ptr);
  int datasize();
  int xmitbuffersize();
  int hdrlen();
  int serialize(void* &ptr);
  void List();
  static void *de_serialize(void *, char *nam = 0);
  void SetExpand(bool expand){this->m_expandService = expand;return;};
  void create_OutputService(std::string);
  void delete_OutputService();
  DimService *getDimService();
};
//#ifdef __cplusplus
//extern "C"{
//#endif
//  void *cccpc_init(char* n);
//#ifdef __cplusplus
//}
//#endif
#endif
