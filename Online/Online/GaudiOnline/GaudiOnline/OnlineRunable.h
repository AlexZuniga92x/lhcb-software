//	============================================================
//
//	OnlineRunable.h
//	------------------------------------------------------------
//
//	Package   : GaudiOnline
//
//	Author    : Markus Frank
//
//	===========================================================
#ifndef GAUDIONLINE_ONLINERUNABLE_H
#define GAUDIONLINE_ONLINERUNABLE_H 1

// Framework include files
#include "GaudiKernel/Service.h"
#include "GaudiKernel/IRunable.h"
#include "GaudiKernel/IEventProcessor.h"

// Forward declarations
class ISvcLocator;

/*
 *  LHCb namespace declaration
 */
namespace LHCb  {

  /** @class OnlineRunable OnlineRunable.h GaudiOnline/OnlineRunable.h
    *
    *  @author Markus Frank
    *  @version 1.0
    */
  class OnlineRunable : public Service, 
                        virtual public IRunable,
                        virtual public IEventProcessor
  {
    std::string m_call;
    std::string m_dll;
    std::string m_initFlags;
    int         m_wait;
    bool        m_warnings;
    StatusCode wait();
  public:
    /// Standard Constructor
    OnlineRunable(const std::string& name, ISvcLocator* svc);
    /// Standard Destructor
    virtual ~OnlineRunable();
    /// IInterface implementation: Query interface
    virtual StatusCode queryInterface(const InterfaceID& riid, void** ppvInterface);
    /// IRunable implementation: Run the object
    virtual StatusCode run();
    /// IRunable overload: Process single event
    virtual StatusCode executeEvent(void* par=0);
    /// IEventProcessor overload: Process the maxevt events as a Run (beginRun() and endRun() called)
    virtual StatusCode executeRun(int maxevt);
    /// IEventProcessor overload: Process the next maxevt events
    virtual StatusCode nextEvent(int maxevt);
    /// IEventProcessor overload: Schedule a stop of the current event processing
    virtual StatusCode stopRun();
  };
}      // End namespace LHCb
#endif //  GAUDIONLINE_ONLINERUNABLE_H
