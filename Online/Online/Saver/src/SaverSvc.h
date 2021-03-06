#ifndef SaverSvc_SaverSvc_H
#define SaverSvc_SaverSvc_H 1

// Include files
#include "CPP/Interactor.h"
#include "GaudiKernel/Service.h"
//#include "GaudiKernel/IMonitorSvc.h"
#include "Gaucho/IGauchoMonitorSvc.h"
#include "GaudiKernel/IIncidentListener.h"

class IIncidentSvc;
class ProcessManager;
namespace LHCb {

  class SaverSvc : virtual public IIncidentListener, public Service, public Interactor  {
  public:
    /// Constructor of this form must be provided
    SaverSvc(const std::string& name, ISvcLocator* pSvcLocator); 
    /// Default destructor
    virtual ~SaverSvc();
    /// Iservice overload: Initialize service
    StatusCode initialize();
    /// Iservice overload: Finalize service
    StatusCode finalize();
    /// Call to save the histograms
    StatusCode save(ProcessMgr* processMgr);
    /// Incident handler callback
    virtual void handle(const Incident& inc);
    /// Interactor handler
    virtual void handle(const Event& ev);

    /** Declare named monitoring information items
      * @param nam    Monitoring information name known to the external system
      * @param var    Monitoring variable (by reference!)
      * @param dsc    Description of monitoring variable
      */
     template <class T>
     void declareInfo(const std::string& nam,const T& var,const std::string& dsc) const {
       m_pGauchoMonitorSvc->declareInfo(nam, var, dsc, this);
     }
     //std::vector<std::string> m_file;
     std::string* m_file;

private:
  /// Reference to monitoring service
  //IMonitorSvc*  m_monitorSvc;
  IGauchoMonitorSvc* m_pGauchoMonitorSvc; ///< Online Gaucho Monitoring Service
  /// Reference to incident service
  IIncidentSvc* m_incidentSvc;
  
  //ProcessMgr *m_processMgr;
  //std::string m_file;
  //int m_fileSize;
  std::vector<ProcessMgr *> m_processMgr;
  
  std::string m_utgid;
  std::string m_nodeName;
  bool m_monitoringFarm;
  
  std::string m_tmpPart;
  std::vector<std::string> m_partName;
  std::vector<std::string> m_taskName;
  std::vector<std::string> m_algorithmName;
  std::vector<std::string> m_objectName;
  std::string m_dimClientDns;
  int m_refreshTime;
  std::string m_saveDir;
  int m_saveDiff;
  bool m_enablePostEvents;
    
  };
}  //end namespace LHCb  

#endif    // SaverSvc_SaverSvc_H
