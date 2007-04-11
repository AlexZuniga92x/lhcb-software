// $Header: /afs/cern.ch/project/cvs/reps/lhcb/Online/PVSSInterface/PVSS/PythonCallback.h,v 1.2 2007-04-11 17:45:46 frankb Exp $
//  ====================================================================
//  PythonCallback.h
//  --------------------------------------------------------------------
//
//  Author    : Markus Frank
//
//  ====================================================================
#ifndef ONLINE_PVSS_PYTHONCALLBACK_H
#define ONLINE_PVSS_PYTHONCALLBACK_H

// Framework include files
#include "PVSS/DeviceListener.h"
#if defined(__GCCXML) && !defined(__REAL_COMP)
struct _object;
typedef _object PyObject;
#else
#include "Python.h"
#endif

/*
 * PVSS namespace
 */
namespace PVSS {

  // Forward declarations
  class PythonCall;
  class ControlsManager;

  /** @class PyDeviceListener
    * 
    * @author  M.Frank
    * @version 1.0
    */
  class PyDeviceListener : public DeviceListener {
  protected:
    /// Dressed python callback
    PythonCall* m_call;
  public:
    /// Standard constructor: Argument self: instance to python object implementation
    PyDeviceListener(PyObject* self, ControlsManager* m);
    /// Default destructor
    virtual ~PyDeviceListener();
    /// Hack!
    void setSelf(PyObject* self);
    /// DIM overloaded callback calling python itself.
    virtual void handleDevices();
    /// DIM overloaded callback calling python itself.
    virtual void handleDevice();
  };

  class PyCallbackTest  {
  protected:
    /// Dressed python callback
    PythonCall* m_call;
  public:
    /// Standard constructor: Argument self: instance to python object implementation
    PyCallbackTest(PyObject* self);
    /// Default destructor
    virtual ~PyCallbackTest();
    /// overloaded callback calling python itself.
    virtual void handle();
  };
}      // End namespace PVSS
#endif // ONLINE_PVSS_PYTHONCALLBACK_H
