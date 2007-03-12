// $Header: /afs/cern.ch/project/cvs/reps/lhcb/Online/PVSSInterface/PVSS/DeviceListener.h,v 1.1 2007-03-12 10:07:24 frankb Exp $
//  ====================================================================
//  DeviceSensor.h
//  --------------------------------------------------------------------
//
//  Author    : Markus Frank
//
//  ====================================================================
#ifndef ONLINE_PVSS_DEVICELISTENER_H
#define ONLINE_PVSS_DEVICELISTENER_H

// Framework include files
#include "PVSS/Kernel.h"
#include "PVSS/DataPoint.h"
#include "CPP/Interactor.h"

// C++ include files
#include <map>

// Forward declarations
class Event;

/*
 *   PVSS namespace declaration
 */
namespace PVSS {

  // Forward declarations
  class ControlsManager;
  class DeviceSensor;

  /** @class DeviceListener DeviceListener.h PVSS/DeviceListener.h
    *
    *  PVSS DeviceListener base class.
    *  Usage from python to allow python interaction
    *
    *   @author  M.Frank
    *   @version 1.0
    */
  class DeviceListener : public Interactor {
  public:
    /// Definition of device connections
    typedef std::map<DpID,DataPoint*> DevDesc;
    typedef std::vector<DataPoint*>   DevArray;
  protected:
    /// Access to manager
    ControlsManager *m_manager;
    /// Access to device sensor (Only valid during callback!)
    DeviceSensor    *m_sensor;
    /// Access to current device (Only valid during callback!)
    DataPoint       *m_dp;
  public:

    /// Initializing constructor
    DeviceListener(ControlsManager* m);
    /// Standard destructor
    virtual ~DeviceListener();

    /// Access to controls manager type
    ControlsManager* manager() const       {  return m_manager;   }
    /// Access to current device list (Only valid during callback!)
    DevDesc& devices()  const;
    /// Access to device vector (Only valid during callback!)
    DevArray& devArray()  const;
    /// Access to current device (Only valid during callback!)
    DataPoint& dp() const;
    /// Sensor callback
    virtual void handle(const Event& ev);
    /// Interface callback to handle all devices 
    virtual void handleDevices();
    /// Interface callback to handle single devices one by one
    virtual void handleDevice();
  };
}      // End namespace PVSS
#endif // ONLINE_PVSS_DEVICELISTENER_H
