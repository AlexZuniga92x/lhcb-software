#include "GaudiOnline/GaudiDeamon.h"
#include "GaudiKernel/IMessageSvc.h"
#include "GaudiKernel/IAppMgrUI.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/Bootstrap.h"
#include "GaudiKernel/SmartIF.h"
#include "WT/wt_facilities.h"
#include "WT/wtdef.h"

LHCb::GaudiDeamon::GaudiDeamon(IInterface*) : DimTaskFSM(0), m_appMgr(0)  {
  propertyMgr().declareProperty("Runable",        m_runable     = "LHCb::OnlineRunable");
  propertyMgr().declareProperty("EventLoop",      m_evtloop     = "MinimalEventLoopMgr");
  propertyMgr().declareProperty("MessageSvcType", m_msgsvcType  = "MessageSvc");
  propertyMgr().declareProperty("JobOptionsPath", m_mainOptions = "");
}

LHCb::GaudiDeamon::~GaudiDeamon()  {
  if ( m_appMgr ) m_appMgr->release();
  m_appMgr= 0;
}

StatusCode LHCb::GaudiDeamon::run()  {
  SmartIF<IAppMgrUI> ui(Gaudi::createInstance("ApplicationMgr","ApplicationMgr","GaudiSvc"));
  if ( ui )  {
    SmartIF<IProperty> ip(ui);
    if ( ip )  {
      ip->setProperty(IntegerProperty("OutputLevel",8));
      ip->setProperty(StringProperty("EvtSel","NONE"));
      ip->setProperty(StringProperty("HistogramPersistency","NONE"));
      ip->setProperty(StringProperty("Runable",        m_runable));
      ip->setProperty(StringProperty("EventLoop",      m_evtloop));
      ip->setProperty(StringProperty("MessageSvcType", m_msgsvcType));
      if ( m_mainOptions.empty() )  {
        ip->setProperty(StringProperty("JobOptionsType", "NONE"));
      }
      else  {
        ip->setProperty(StringProperty("JobOptionsPath", m_mainOptions));
      }
      m_appMgr = ui;
      return m_appMgr->run();
    }
  }
  return DimTaskFSM::unload();
}

StatusCode LHCb::GaudiDeamon::unload()  {
  m_appMgr->finalize();
  m_appMgr->terminate();
  return DimTaskFSM::unload();
}
