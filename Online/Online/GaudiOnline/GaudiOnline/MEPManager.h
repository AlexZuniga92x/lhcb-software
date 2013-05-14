// $Header: /afs/cern.ch/project/cvs/reps/lhcb/Online/GaudiOnline/GaudiOnline/MEPManager.h,v 1.10 2010-09-21 14:28:25 frankb Exp $
//	====================================================================
//  MEPManager.cpp
//	--------------------------------------------------------------------
//
//	Author    : Markus Frank
//
//	====================================================================
#ifndef GAUDIONLINE_MEPManager_H
#define GAUDIONLINE_MEPManager_H

#include "GaudiKernel/Service.h"
#include "MBM/mepdef.h"
#include <cstring>
#include <map>
#include <set>

namespace MBM {
  class Producer;
  class Consumer;
}

// Declaration of the interface ID. 
static const InterfaceID IID_IMEPManager("IMEPManager", 0, 0); 

/*
 *    LHCb namespace
 */
namespace LHCb    {

  /** @class MEPManager MEPManager.cpp  GaudiOnline/MEPManager.cpp
    *
    * Service mapping/creating MBM and MEP buffers for online data access.
    * 
    * @author  M.Frank
    * @version 1.0
    * @date    01/01/2005
    */
  class MEPManager : public Service  {
    /// Property: Container of buffer names to connect on initialize
    std::vector<std::string>   m_buffers;
    /// Property: Process name used to include into MEP/MBM buffers
    std::string                m_procName;
    /// Property: Initialization flags to possibly install MBM/MEP buffers
    std::string                m_initFlags;
    /// Property: partition identifier used to connect to buffer managers
    int                        m_partitionID;
    /// Property: partition name used to connect to buffer managers (If set overrides partition identifier)
    std::string                m_partitionName;
    /// Property: Allows to choose if the buffers should be connected on "initialize" or "start"
    std::string                m_connectWhen;
    /// Property: Flag to indicate if unused MEP buffers should be mapped
    bool                       m_mapUnused;
    /// Property: Flag to indicate if buffer names should contain partition ID
    bool                       m_partitionBuffers;
    /// Property; Flag to indicate if signals should be handled
    bool                       m_handleSignals;
    /// Buffer to store MBM identifiers
    std::vector<BMID>          m_bmIDs;
    /// Map between buffer identifiers and the corresponding name
    std::map<std::string,BMID> m_buffMap;

    std::string                m_input;

  public:
    /// Retrieve interface ID
    static const InterfaceID& interfaceID() { return IID_IMEPManager; }

    /// Standard service constructor
    MEPManager(const std::string& nam, ISvcLocator* loc);

    /// Standard destructor
    virtual ~MEPManager();

    /// Create producer
    MBM::Producer* createProducer(const std::string& buffer,const std::string& instance);

    /// Create consumer attached to a specified buffer
    MBM::Consumer* createConsumer(const std::string& buffer,const std::string& instance);

    /// Access to optional MBM buffer identifiers
    const std::map<std::string,BMID>& buffers()  const { return m_buffMap;  }

    /// Access to optional MBM buffer identifiers
    const std::vector<BMID>& bmIDs()  const    {   return m_bmIDs;          }

    /// Access to partition ID as specified in the job options
    int partitionID()  const                   {   return m_partitionID;    }
    
    /// Access to partition name as specified in the job options
    const std::string& partitionName() const   {   return m_partitionName;  }
    
    /// Access to process name
    const std::string& processName()  const    {   return m_procName;       }

    /// Connect when ???
    const std::string& connectWhen()  const    {   return m_connectWhen;    }

    /// Internal initialization
    virtual StatusCode i_init();
    
    /// Internal finalization
    virtual StatusCode i_fini();
    
    /** Query interfaces of Interface
        @param riid       ID of Interface to be retrieved
        @param ppvUnknown Pointer to Location for interface pointer
    */
    virtual StatusCode queryInterface(const InterfaceID& riid,
                                      void** ppvInterface);

    /// IService overload: Initialize MEP manager service
    virtual StatusCode initialize();

    /// IService overload: start MEP manager service
    virtual StatusCode start();

    /// IService overload: stop MEP manager service
    virtual StatusCode stop();

    /// IService overload: restart MEP manager service
    virtual StatusCode restart();

    /// IService overload: Finalize MEP manager service
    virtual StatusCode finalize();

    /// Connect to optional MBM buffer
    StatusCode connectBuffer(const std::string& nam);

    /// Initialize buffers for MEP usage
    StatusCode initializeBuffers();

    /// Connect to specified buffers
    StatusCode connectBuffers();

    /// Cancel MBM connections
    StatusCode cancel();

    /// Issues error message and return error
    StatusCode error(const std::string& msg)   const;
  };

  /// Simple and lazy interface definition
  typedef MEPManager IMEPManager;
}      // End namespace LHCb
#endif // GAUDIONLINE_MEPManager_H
