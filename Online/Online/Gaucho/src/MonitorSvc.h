#ifndef GAUDIKERNEL_MONITORSVC_H
#define GAUDIKERNEL_MONITORSVC_H

#include "GaudiKernel/Service.h"
#include "GaudiKernel/IMonitorSvc.h"
#include <string>
#include <map>
#include <set>

// Forward declarations
class ISvcLocator;
class DimPropServer;
class DimEngine;
class DimCmdServer;


namespace AIDA { class IBaseHistogram; }

/** @class MonitorSvc MonitorSvc.h GaudiKernel/MonitorSvc.h
    
This class implements the IMonitorSvc interface, and publishes Gaudi variables
to outside monitoring processes with Dim.

An internal DimEngine is used for this purpose. A DimPropServer is started 
which takes string commands in the format Algorithm.Property and returns 
the value of the property.

@author Philippe Vannerem
@author Jose Helder Lopes Jan. 2005
@author Jose Helder Lopes 2006/12/26: Modified to publish general c-structures and AIDA::IProfile1D  
*/

class MonitorSvc : public Service, virtual public IMonitorSvc {
public:
  MonitorSvc(const std::string& name, ISvcLocator* sl);
  virtual ~MonitorSvc() {}
  //IInterface pure virtual member functions
  StatusCode queryInterface(const InterfaceID& riid, void** ppvIF);
  // Service pure virtual member functions
  StatusCode initialize();
  StatusCode finalize();
  
  /** Declare monitoring information
      @param name Monitoring information name knwon to the external system
      @param var Monitoring Listener address
      @param desc Textual description
      @param owner Owner identifier of the monitoring information 
      (needed to peform clean up 
  */
  void declareInfo(const std::string& name, const bool&  var, 
                   const std::string& desc, const IInterface* owner) ;
  void declareInfo(const std::string& name, const int&  var, 
                   const std::string& desc, const IInterface* owner) ;
  void declareInfo(const std::string& name, const long&  var, 
                   const std::string& desc, const IInterface* owner) ;
  void declareInfo(const std::string& name, const double& var, 
                   const std::string& desc, const IInterface* owner) ;
  void declareInfo(const std::string& name, const std::string& var, 
                   const std::string& desc, const IInterface* owner) ;
  void declareInfo(const std::string& name, const std::pair<double,double>&var,
                   const std::string& desc, const IInterface* owner) ;
  void declareInfo(const std::string& name, const AIDA::IBaseHistogram* var, 
                   const std::string& desc, const IInterface* owner) ;
  void declareInfo(const std::string& name, const std::string& format, const void * var, 
                   int size, const std::string& desc, const IInterface* owner) ;
  
  /** Undeclare monitoring information
      @param name Monitoring information name knwon to the external system
      @param owner Owner identifier of the monitoring information
  */
  void undeclareInfo( const std::string& name, const IInterface* owner ) ;
  
  /** Undeclare monitoring information
      @param owner Owner identifier of the monitoring information
  */
  void undeclareAll( const IInterface* owner ) ;
  
  /** Get the names for all declared monitoring informations for a given
      owner. If the owner is NULL, then it returns for all owners
  */
  std::set<std::string> * getInfos(const IInterface* owner = 0);

private:
  // Map associating to each algorithm name a set with the info 
  // names from this algorithm 
  typedef std::map<const IInterface*, std::set<std::string> > InfoNamesMap;
  InfoNamesMap m_InfoNamesMap;
  InfoNamesMap::iterator m_InfoNamesMapIt;

  // The container below is necessary because usually the descriptions of 
  // the services are constant strings, not variables. DimService expects 
  // a variable address to monitor as its second argument and we are using 
  // a DimService for strings to publish the descriptions.
  std::map<std::string,std::string> m_infoDescriptions;

  DimPropServer* m_dimpropsvr;
  DimEngine* m_dimeng;
  DimCmdServer* m_dimcmdsvr;
  std::string infoOwnerName( const IInterface* owner );
};

#endif // GAUDIKERNEL_MONITORSVC_H
