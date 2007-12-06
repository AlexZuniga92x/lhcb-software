#include "MBM/Producer.h"
#include "WT/wtdef.h"
#include <stdexcept>

// Initializing constructor
MBM::Producer::Producer(const std::string& buffer_name, const std::string& client_name, int partition_id)
: Client(buffer_name, client_name, partition_id)  {
}

// Initializing constructor
MBM::Producer::Producer(BMID bmid, const std::string& client_name, int partition_id)
: Client(bmid, client_name, partition_id)  {
}

// Standard destructor
MBM::Producer::~Producer()    {
}

// Switch to non-blocking execution mode
void MBM::Producer::setNonBlocking(int facility, bool subscribe) {
  Client::setNonBlocking(facility,false);
  if ( subscribe ) {
    int sc = wtc_subscribe(facility, spaceRearm, spaceAction, this);
    if( sc != WT_SUCCESS ) {
      throw std::runtime_error("Failed to subscribe action:"+m_buffName+" [Internal Error]");
    }
  }
}

/// Cancel pending request
int MBM::Producer::cancel()   {
  if ( m_bmid != (BMID)-1 ) {
    int sc = ::mbm_cancel_request(m_bmid);
    if ( sc == MBM_NORMAL )  {
      return sc;
    }
    throw std::runtime_error("Failed to cancel request to MBM buffer:"+m_buffName+" [Internal Error]");
  }
  throw std::runtime_error("Failed to cancel request to MBM buffer:"+m_buffName+" [Buffer not connected]");
}

int MBM::Producer::spaceAst(void* param) {
  Producer* prod = (Producer*)param;
  return prod->spaceAst();
}

// Ast to be called on event receival (may be overloaded by clients)
int MBM::Producer::spaceAst() {
  int sc = mbm_get_space_ast(m_bmid);
  if ( sc == MBM_NORMAL ) {
    if ( !m_blocking ) {
      sc = ::wtc_insert(m_facility, this);
      if( sc == WT_SUCCESS ) {
        return MBM_NORMAL;
      }
      throw std::runtime_error("Failed to wtc_insert on get space AST:"+m_buffName+" [Internal Error]");
    }
    return MBM_NORMAL;
  }
  throw std::runtime_error("Failed mbm_get_space_ast MBM buffer:"+m_buffName+" [Internal Error]");
}

// Static action to be called on space receival
int MBM::Producer::spaceAction(unsigned int facility, void* param) {
  Producer* prod = (Producer*)param;
  if ( facility != prod->m_facility ) {
    // Error ?
  }
  return prod->spaceAction();
}

// Action to be called on space receival
int MBM::Producer::spaceAction() {
  void *fadd;
  EventDesc& e = m_event;
  int sc = MBM_NORMAL, flen;
  if ( m_bmid != MBM_INV_DESC ) {
    sc = ::mbm_declare_event(m_bmid, e.len, e.type, e.mask, 0, &fadd, &flen, m_partID);
    if ( sc == MBM_REQ_CANCEL ) {
      ::lib_rtl_printf("Failed to declare event for MBM buffer %s. Request was cancelled.\n",
        m_buffName.c_str());
      return sc;
    }
    else if ( sc == MBM_NORMAL )  {
      sc = ::mbm_send_space(m_bmid);
      if ( sc == MBM_REQ_CANCEL ) {
        ::lib_rtl_printf("Failed to declare event for MBM buffer %s. Request was cancelled.\n",
          m_buffName.c_str());
        return sc;
      }
      else if ( sc == MBM_NORMAL )  {
        return sc;
      }
      ::lib_rtl_printf("Failed to send space for MBM buffer %s. status = %d %08X.\n",
        m_buffName.c_str(),sc,sc);
      throw std::runtime_error("Failed to send space for MBM buffer:"+m_buffName+" [Internal Error]");
    }
    else {
      ::lib_rtl_printf("Failed to declare event for MBM buffer %s. status = %d %08X.\n",
        m_buffName.c_str(),sc,sc);
    }
    throw std::runtime_error("Failed to declare event for MBM buffer:"+m_buffName+" [Internal Error]");
  }
  throw std::runtime_error("Failed to declare event for MBM buffer:"+m_buffName+" [Buffer not connected]");
}

// Action to be called on space receival
int MBM::Producer::declareEvent() {
  void *fadd;
  EventDesc& e = m_event;
  int sc = MBM_NORMAL, flen;
  if ( m_bmid != MBM_INV_DESC ) {
    sc = ::mbm_declare_event(m_bmid, e.len, e.type, e.mask, 0, &fadd, &flen, m_partID);
    if ( sc == MBM_NORMAL )  {
      return MBM_NORMAL;
    }
    throw std::runtime_error("Failed to declare event for MBM buffer:"+m_buffName+" [Internal Error]");
  }
  throw std::runtime_error("Failed to declare event for MBM buffer:"+m_buffName+" [Buffer not connected]");
}

// Action to be called on space receival
int MBM::Producer::sendSpace() {
  if ( m_bmid != MBM_INV_DESC ) {
    if ( ::mbm_send_space(m_bmid) == MBM_NORMAL )  {
      return MBM_NORMAL;
    }
    throw std::runtime_error("Failed to send space for MBM buffer:"+m_buffName+" [Internal Error]");
  }
  throw std::runtime_error("Failed to declare event for MBM buffer:"+m_buffName+" [Buffer not connected]");
}

// Rearm action to be called on space receival
int MBM::Producer::spaceRearm(unsigned int facility, void* param) {
  Producer* prod = (Producer*)param;
  if ( facility != prod->m_facility ) {
    // Error ?
  }
  return prod->spaceRearm(prod->m_event.len);
}

// Space receival rearm
int MBM::Producer::spaceRearm(int new_length) {
  if ( m_bmid != MBM_INV_DESC ) {
    EventDesc& e = m_event;
    e.len = new_length;
    int sc = ::mbm_get_space_a(m_bmid, e.len, &e.data, spaceAst, this);
    if ( sc == MBM_NORMAL )  {
      sc = m_blocking ? ::mbm_wait_space(m_bmid) : ::mbm_wait_space_a(m_bmid);
      if ( sc == MBM_NORMAL || sc == MBM_REQ_CANCEL )  {
        return sc;
      }
      throw std::runtime_error("Failed to wait space for MBM buffer:"+m_buffName+" [Internal Error]");
    }
    throw std::runtime_error("Failed to get event for MBM buffer:"+m_buffName+" [Internal Error]");
  }
  throw std::runtime_error("Failed to declare event for MBM buffer:"+m_buffName+" [Buffer not connected]");
}

// Get space call to fill event data
int MBM::Producer::getSpace(int len)  {
  return spaceRearm(len);
}

/// send and declare event to consumers
int MBM::Producer::sendEvent()  {
  return spaceAction();
}

