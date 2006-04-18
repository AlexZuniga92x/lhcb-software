#include "RTL/rtl.h"
#include "WT/wtdef.h"
#include "MBM/Client.h"
#include <stdexcept>

// Initializing constructor
MBM::Client::Client(const std::string& buffer_name, const std::string& client_name, int partition_id)
  : m_bmid(MBM_INV_DESC), m_buffName(buffer_name), m_name(client_name), m_partID(partition_id), m_facility(0), m_blocking(true)
{
  include();
}

// Initializing constructor
MBM::Client::Client(BMID bmid, const std::string& client_name, int partition_id)
  : m_bmid(bmid), m_name(client_name), m_partID(partition_id), m_facility(0), m_blocking(true)
{
}

// Standard destructor
MBM::Client::~Client()  {
  if ( !m_buffName.empty() ) exclude();
}

// Include MBM client into buffer
int MBM::Client::include()  {
  if ( m_bmid == MBM_INV_DESC )  {
    m_bmid = ::mbm_include(m_buffName.c_str(),m_name.c_str(),m_partID);
    if ( m_bmid == (BMID)-1 ) {
      throw std::runtime_error("Failed to include into MBM buffer:"+m_buffName);
    }
  }
  return MBM_NORMAL;
}

// Exclude MBM client into buffer
int MBM::Client::exclude()  {
  if ( m_bmid != MBM_INV_DESC )  {
    int status = ::mbm_exclude(m_bmid);
    if ( status != MBM_NORMAL )  {
      throw std::runtime_error("Failed to exclude from MBM buffer:"+m_buffName);
    }
    m_bmid = MBM_INV_DESC;
  }
  return MBM_NORMAL;
}

// Access to process id
int MBM::Client::pid()  {
  return lib_rtl_pid();
}

// Switch to non-blocking execution mode
void MBM::Client::setNonBlocking(int facility, bool /* subscribe */) {
  static bool first = true;
  if ( first )  {
    first = false;
    ::wtc_init();
  }
  m_blocking = false;
  m_facility = facility;
}

// Switch to non-blocking execution mode
void MBM::Client::setBlocking() {
  if ( !m_blocking ) {
    ::wtc_remove(m_facility);
  }
  m_blocking = true;
  m_facility = 0;
}


// Run the application
int MBM::Client::run() {
  for(;;)  {
    int sc = (m_blocking) ? runSynchronous() : runAsynchronous();
    if ( sc != MBM_NORMAL ) {
      lib_rtl_printf("Error executing MBM client sc=%d\n",sc);
    }
  }
  return MBM_NORMAL;
}

// Run the application in synchonous mode with WT
int MBM::Client::runAsynchronous() {
  int sub_status;
  void* userpar;
  unsigned int facility;
  int status = ::wtc_wait(&facility, &userpar, &sub_status);
  ::lib_rtl_printf("Exited WAIT>>>> Facility = %d Status=%d Sub-Status = %d\n", 
	   facility, status, sub_status);
  return status;
}

// Run the application in synchonous mode
int MBM::Client::runSynchronous() {
  ::lib_rtl_printf("MBM::Client::runSynchronous> This implementation MUST be overriden.\n");
  return MBM_ERROR;
}
