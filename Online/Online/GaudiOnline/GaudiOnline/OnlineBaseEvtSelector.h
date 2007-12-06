// $Id: OnlineBaseEvtSelector.h,v 1.1 2007-12-06 14:39:35 frankb Exp $
//====================================================================
//  OnlineBaseEvtSelector.h
//--------------------------------------------------------------------
//
//  Package    : GaudiOnline
//
//  Description: The OnlineBaseEvtSelector component is able
//               to produce a list of event references given a set of "selection
//               criteria".
//
//  Author     : M.Frank
//  Created    : 4/01/99
//
//====================================================================
// $Header: /afs/cern.ch/project/cvs/reps/lhcb/Online/GaudiOnline/GaudiOnline/OnlineBaseEvtSelector.h,v 1.1 2007-12-06 14:39:35 frankb Exp $
#ifndef GAUDIONLINE_ONLINEBASEEVTSELECTOR_H
#define GAUDIONLINE_ONLINEBASEEVTSELECTOR_H 1

// Include files
#include "GaudiKernel/IIncidentListener.h"
#include "GaudiKernel/IEvtSelector.h"
#include "GaudiOnline/OnlineService.h"
#include "GaudiOnline/ISuspendable.h"
#include "MDF/RawEventDescriptor.h"
#include "MBM/Requirement.h"
#include "RTL/rtl.h"

/*
 *  LHCb namespace declaration
 */
namespace LHCb  {

  // Forward declarations
  class RawBank;
  class OnlineContext;
  class OnlineBaseEvtSelector;

  /** @class OnlineContext
    *
    * @author M.Frank
    */
  class OnlineContext : public IEvtSelector::Context   {
  public:
    /// Bank container definition
    typedef std::vector<LHCb::RawBank*> Banks;
    /// Container to bank referentes
    Banks              m_banks;
    /// Event descriptor
    RawEventDescriptor m_evdesc;

  protected:
    /// Reference to event selector
    const OnlineBaseEvtSelector* m_sel;
    /// Flag to indicate that current event needs to be freed.
    bool                         m_needFree;

  public:
    /// Standard constructor
    OnlineContext(const OnlineBaseEvtSelector* s) : m_sel(s), m_needFree(false) {}
    /// Standard destructor 
    virtual ~OnlineContext() {}
    /// IEvtSelector::Context overload; context identifier
    virtual void* identifier() const                     { return (void*)m_sel; }
    /// Access container to bank referentes
    virtual const Banks& banks() const                   { return m_banks;      }
    /// Access to event descriptor
    LHCb::RawEventDescriptor& descriptor()               { return m_evdesc;     }
    /// Access to event descriptor (CONST)
    const LHCb::RawEventDescriptor& descriptor() const   { return m_evdesc;     }
    /// Connect to event data source
    virtual StatusCode connect(const std::string& input) = 0;
    /// Receive event information
    virtual StatusCode receiveEvent() = 0;
    /// Rearm event data request
    virtual StatusCode rearmEvent() = 0;
    /// Free event resources
    virtual StatusCode freeEvent() = 0;
    /// Close connection to event data source
    virtual void close() = 0;
  };

  /** @class OnlineBaseEvtSelector  OnlineBaseEvtSelector.h
    *
    * Base class for online event selectors.
    *
    * Concrete implementations get input from buffer managers
    * or from the network.
    *
    * Input requests may be specified by means of requirements.
    *
    * @author  M.Frank
    * @vrsion  1.0
    */
  class OnlineBaseEvtSelector : public OnlineService, 
                                virtual public ISuspendable,
                                virtual public IEvtSelector,
                                virtual public IIncidentListener
  {
    friend class OnlineContext;

  public:
    /// IInterface implementation: query interfaces
    virtual StatusCode queryInterface(const InterfaceID& riid, void** ppvInterface);

    /// IService implementation: Db event selector override
    virtual StatusCode initialize();

    /// IService implementation: Service finalization
    virtual StatusCode finalize();

    /// ISuspendable implementation: suspend operation
    virtual StatusCode suspend()  const;

    /// ISuspendable implementation: resume operation
    virtual StatusCode resume()  const;

    /// Incident handler implemenentation: Inform that a new incident has occured
    virtual void handle(const Incident& inc);

    /** Access last item in the iteration
      * @param refContext [IN/OUT] Reference to the Context object.
      */
    virtual StatusCode last(Context& refContext) const;

    /// Get next iteration item from the event loop context
    /** @param refCtxt   [IN/OUT]  Reference to the context
      * 
      * @return StatusCode indicating success or failure
      */
    virtual StatusCode next(Context& refCtxt) const;

    /// Get next iteration item from the event loop context, but skip jump elements
    /** @param refCtxt   [IN/OUT]  Reference to the context
      * 
      * @return StatusCode indicating success or failure
      */
    virtual StatusCode next(Context& refCtxt,int jump) const;

    /// Get previous iteration item from the event loop context
    /** @param refCtxt   [IN/OUT]  Reference to the context
      * @param jump      [IN]      Number of events to be skipped
      * 
      * @return StatusCode indicating success or failure
      */
    virtual StatusCode previous(Context& refCtxt) const;

    /// Get previous iteration item from the event loop context, but skip jump elements
    /** @param refCtxt   [IN/OUT]  Reference to the context
      * @param jump      [IN]      Number of events to be skipped
      * 
      * @return StatusCode indicating success or failure
      */
    virtual StatusCode previous(Context& refCtxt,int jump) const;

    /// Rewind the dataset
    /** @param refCtxt   [IN/OUT]  Reference to the context
      * 
      * @return StatusCode indicating success or failure
      */
    virtual StatusCode rewind(Context& refCtxt) const;

    /// Create new Opaque address corresponding to the current record
    /** @param refCtxt   [IN/OUT]  Reference to the context
      * 
      * @return StatusCode indicating success or failure
      */
    virtual StatusCode createAddress(const Context& refCtxt, IOpaqueAddress*&) const;

    /// Release existing event iteration context
    /** @param refCtxt   [IN/OUT]  Reference to the context
      * 
      * @return StatusCode indicating success or failure
      */
    virtual StatusCode releaseContext(Context*& refCtxt) const;

    /** Will set a new criteria for the selection of the next list of events and will change
      * the state of the context in a way to point to the new list.
      * 
      * @param cr The new criteria string.
      * @param c  Reference pointer to the Context object.
      */
    virtual StatusCode resetCriteria(const std::string& cr,Context& c)const;

    /// Service Constructor
    OnlineBaseEvtSelector(const std::string& name, ISvcLocator* svcloc);

    /// Standard destructor
    virtual ~OnlineBaseEvtSelector()    {}

    /// Increase event request counter
    void increaseReqCount()    const     {  m_reqCount++;     }
    /// Increase event receive counter
    void increaseEvtCount()    const     {  m_evtCount++;     }
    /// Flag to indicate that event decoding is required
    bool mustDecode() const              {  return m_decode;  }
    /// Number of active requests required by the job options
    int numReq() const                   {  return m_nreqs;   }
    /// Access requests given by the job option data
    const MBM::Requirement& req(int i) const { return m_Reqs[i]; }

  protected:
    /// Lock handle to suspend/resume operations
    lib_rtl_event_t               m_suspendLock;
    /// Allow suspending event access
    bool                          m_allowSuspend;
    /// Input buffer name (default="EVENT")
    std::string                   m_input;
    /// Decode data buffer
    bool                          m_decode;
    /// Requirement properties
    std::string                   m_Rqs[8];
    /// Decoded requirements
    mutable MBM::Requirement      m_Reqs[8];
    /// Number of requirements
    mutable int                   m_nreqs;
    /// Property to indicate that the connection goes to a partitioned buffer
    bool                          m_partitionBuffer;
    /// Monitoring quantity: Number of events received
    mutable int                   m_evtCount;
    /// Monitoring quantity: Number of events requested
    mutable int                   m_reqCount;
    /// Flag indicating if event selector is suspended and waiting for an event
    mutable bool                  m_isWaiting;
  };
}
#endif  // GAUDIONLINE_ONLINEBASEEVTSELECTOR_H
