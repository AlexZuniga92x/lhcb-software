#ifndef MONSYS_H
#define MONSYS_H
#include "RTL/rtl.h"
#include <vector>
#include <string>
//#include "MonSubSys.h"
class MonSubSys;
class MonSys
{
public:
  std::string m_name;
//  BRTLLock m_lockid;
  std::vector<MonSubSys*> *Subsyslist;
public:
  MonSys();
  virtual ~MonSys();
  void addSubSys(MonSubSys *);
  void remSubSys(MonSubSys *);
  void Lock();
  void unLock();
  static MonSys& m_instance();
  MonSys *setup(char *n);
  void start();
  void Clear();
  void EORUpdate(int runo);
  void Update();
  void List();
  void stop();
  void setRunNo(int runo);
  std::string m_state;
  std::vector<MonSubSys*> *getSubsyslist(){return Subsyslist;};
};
#endif
