#ifndef ONLINE_GAUDIONLINE_CLASS1TASK_H
#define ONLINE_GAUDIONLINE_CLASS1TASK_H

// Framework includes
#include "GaudiOnline/GaudiTask.h"

/*
 * LHCb namespace declaration
 */
namespace LHCb {
  /** @class Class1Task Class1Task.h GaudiOnline/Class1Task.h
    *
    * Object implementing the functionality of a Class 1 task
    * 
    * @author  M.Frank
    * @version 1.0
    */
  class Class1Task : public GaudiTask {
  protected:
    /// Internal helper variable for state transitions
    int m_myState;
  public:
    /// Standard constructor
    Class1Task(IInterface* svc);
    /// Default destructor
    virtual ~Class1Task() {}
    /// Callback on configure transition
    virtual StatusCode configure();
    /// Callback on start transition
    virtual StatusCode initialize();
    /// Callback on activate transition
    virtual StatusCode activate();
    /// Callback on start transition
    virtual StatusCode start();
    /// Callback on stop transition
    virtual StatusCode stop();
    /// Callback on stop transition
    virtual StatusCode finalize();
    /// Callback on reset transition
    virtual StatusCode terminate();
    /// Callback on disable event processing
    virtual StatusCode disable();
    /// Callback on I/O cancel
    virtual StatusCode cancel();
  };
}
#endif // ONLINE_GAUDIONLINE_CLASS1TASK_H
