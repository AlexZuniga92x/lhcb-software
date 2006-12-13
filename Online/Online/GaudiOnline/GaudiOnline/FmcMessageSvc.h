#ifndef GAUDIONLINE_FMCMESSAGESVC_H
#define GAUDIONLINE_FMCMESSAGESVC_H
#include <string>

// Framework include files
#include "GaudiOnline/IErrorLogger.h"
#include "GaudiOnline/OnlineMessageSvc.h"

namespace LHCb 
{
  /***************************************************************************/
  class FmcMessageSvc:public OnlineMessageSvc,virtual public IErrorLogger
  {
  public:
    /// Default constructor.
    FmcMessageSvc(const std::string& name, ISvcLocator* svcloc);
    /// Destructor.
    virtual ~FmcMessageSvc();
    /// IInterface implementation : queryInterface
    StatusCode queryInterface(const InterfaceID& riid,void** ppIf);
    /// Implementation of IService::initialize()
    virtual StatusCode initialize();
    /// Implementation of IService::finalize()
    virtual StatusCode finalize();
    /// Implementation of IMessageSvc::reportMessage()
    virtual void reportMessage(const Message& message);
    /// Error logger implementation: report message
    virtual void report(int typ,const std::string& src,const std::string& msg);
    /// Number of dropped messages (in congestion-proof mode)
    virtual int getDroppedN();
    /// True if last messages was dropped (in congestion-proof mode)
    virtual bool isDropped();
  private:
    bool m_noDrop;
    std::string m_fifoPath;
    int m_tryN;
    char hostName[80];
    char *pName;
    char *utgid;
    int fifoFD;
    int dfltFifoFD;
    int droppedN;
    bool dropped;
    virtual void getPName();
    virtual int printM(int out,int severity,const char* fName,
                       char *format,...);
    virtual int dimLoggerMsgSend(char *buf,int fifoFD);
    virtual int stdErrMsgSend(char *buf);
    virtual int sysLogMsgSend(char *buf,int severity);
  };
  /***************************************************************************/
}       // End namespace LHCb
#endif  // GAUDIONLINE_FMCMESSAGESVC_H
