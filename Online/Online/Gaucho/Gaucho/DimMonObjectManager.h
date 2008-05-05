#ifndef DIMMONOBJECTMANAGER_H
#define DIMMONOBJECTMANAGER_H 1

#include "TH1.h"
#include "Gaucho/MonObject.h"
#include "GaudiKernel/IMessageSvc.h"
#include "GaudiKernel/MsgStream.h"
#include "RTL/rtl.h"
#include "dic.hxx"
#include "dis.hxx"
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>

#include <cstring>
#include <string>
#include <vector>
#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <map>

class DimTimerMonObjectAdder;
class DimServiceMonObject;
class DimInfoMonObject;

class DimMonObjectManager {
private:
  IMessageSvc*  m_msgSvc;
  std::string   m_source;
  std::string   m_name;
  DimBrowser*   m_dimBrowser;
  std::string   m_utgid;

  //MonitorSvc
  typedef std::map<std::string, DimServiceMonObject*, std::less<std::string> > DimServiceMonObjectMap;
  typedef DimServiceMonObjectMap::iterator DimServiceMonObjectMapIt;
  DimServiceMonObjectMap    m_dimMonObjects;
  DimServiceMonObjectMapIt  m_dimMonObjectsIt;

  //Saver
  std::vector<std::vector<MonObject*> >  m_monObjectVect2;

  //Adder & Saver
  std::vector<std::vector<std::string> >  m_dimInfoNamesVect2;

  //Adder
  std::vector<std::string>     m_dimSvcNamesAdderVect1;

  std::vector<std::vector<DimInfoMonObject*> >  m_dimInfoMonObjectVect2;
  std::vector<DimServiceMonObject*>    m_dimSvcMonObjectAdderVect1;
  DimTimerMonObjectAdder*       m_dimTimerMonObjectAdder;

public:
  DimMonObjectManager();
  DimMonObjectManager(std::string source);
  virtual ~DimMonObjectManager();
  void setDimClientDns(std::string dimClientDns);
  void setDimBrowser(DimBrowser* dimBrowser);
  //void setMsgSvc(IMessageSvc*  m_msgSvc);
  //void setSource(const std::string& source);
  void deleteDimBrowser();
  void printSource(){
    std::cout << "printSource() --> m_name="<<m_name << std::endl;
    std::cout << "printSource() --> m_source="<<m_source << std::endl;
  }
  MonObject* createMonObject(std::string  monObjectTypeName);
  void deleteMonObject(MonObject* monObject);
  int monObjectSize(MonObject* monObject);
  void loadMonObject(char* c, int size, MonObject *monObject);
  void saveMonObject(char* c, int& size, MonObject *monObject);

  void msg(std::string msg, std::string source, int msgLevel);

  IMessageSvc* msgSvc(){return m_msgSvc;}
  void setMsgSvc(IMessageSvc* msgSvc){m_msgSvc = msgSvc;}
  
  std::string source(){return m_source;}

  void dimWait();
  void setUTGID(std::string utgid){m_utgid = utgid;};

  static int serviceParts(std::string serviceStr, std::string &svctype, std::string &task, std::string &utgid, std::string &algo, std::string &object);
  //MonitorSvc
  void declServiceMonObject(std::string infoName, const MonObject* monObject);
  void undeclServiceMonObject(std::string infoName);
  void updateServiceMonObject(std::string infoName, bool endOfRun);

  //Saver
  void saverCreateMonObjects(int refreshTime);
  void saverDeleteMonObjects();
  void saverCreateDimNames(std::string nodeName, std::vector<std::string> taskName, std::vector<std::string> algorithmName, std::vector<std::string> objectName);
  void saverWriteMonObjects();

  //Adder
  int adderCreateDimNames(std::vector<std::string> nodeNames, std::vector<std::string> taskName, std::vector<std::string> algorithmName, std::vector<std::string> objectName, std::string serverName);
  int adderCreateDimMonObjects(std::string procName, int refreshTime);
  //void adderDeleteDimMonObjects();
  void adderCreateDimTimerMonObject(int refreshTime);
  void adderDeleteDimTimerMonObject();

  //Presenter
  int  presenterGetDimMonObjectSvcList(std::vector<std::string> &dimMonObjectServicesList);
  TH1* presenterInitHistogram(DimInfoMonObject* dimInfoMonObject, int localInstance);
  void presenterFillHistogram(DimInfoMonObject* dimInfoMonObject);

  //MonObjectTextPrinter 
  MonObject *monObjectTextPrinterGetMonObject(int refreshTime, std::string svcName);

};

#endif // DIMMONOBJECTMANAGER_H

