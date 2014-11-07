// $Id: HltBufferedIOReader.h,v 1.12 2009-12-02 10:32:35 apuignav Exp $
#ifndef GAUDIONLINE_DIMRPCFILEREADER_H
#define GAUDIONLINE_DIMRPCFILEREADER_H

// Framework includes
#include "GaudiKernel/IRunable.h"
#include "GaudiKernel/IEvtSelector.h"
#include "GaudiOnline/OnlineService.h"
#include "GaudiKernel/IEventProcessor.h"
#include "MBM/Producer.h"
#include "RTL/rtl.h"

// C/C++ include files
#include <sys/stat.h>
#include <ctime>
#include <set>

// Forward declarations
class IIncidentSvc;

/*
 *   LHCb namespace declaration
 */
namespace LHCb
{

  class MEPManager;

  /** Class definition of HltBufferedIOReader.
   *
   * This is the online extension of the runnable of the application manager.
   * The runable handles the actual run() implementation of the
   * ApplicationMgr object.
   *
   *  @author Markus Frank
   *  @modified Albert Puig
   *
   *  @version 1.5
   */
  class HltBufferedIOReader: public OnlineService, virtual public IRunable
  {
  protected:
    /// Flag indicating that MBM event retrieval is active
    bool                     m_receiveEvts;
    /// Lock handle
    lib_rtl_lock_t           m_lock;
    /// Reference to MEP manager service
    MEPManager*              m_mepMgr;
    /// Reference to buffer manager producer
    MBM::Producer*           m_producer;
    /// Current file name
    std::string              m_current;
    /// List of files to process
    std::set<std::string>    m_files;

    /// Property: Buffer name for data output
    std::string              m_buffer;
    /// Property: Data directory name
    std::string              m_directory;
    /// Property: File prefix to select files from data directory
    std::string              m_filePrefix;
    /// Property: Path to the file containing broken nodes, where no reading should happen
    std::string              m_brokenHostsFile;
    /// Property: List of runs to be processed (as strings!)
    std::vector<std::string> m_allowedRuns;
    /// Property: Maximum number of seconds to wait for consumers (default: 20 seconds)
    int                      m_maxConsWait;

    /// Monitoring quantity: Number of events processed
    int                      m_evtCount;
    /// Flag to indicate if files should be deleted
    bool                     m_deleteFiles;
    /// Flag to indicate if the node is in the broken hosts file and hence disabled
    bool                     m_disabled;

    // Helper routines

    /// Scan directory for matching files
    size_t scanFiles();
    /// Open a new data file
    int openFile();
    /// Save remainder of currently read file
    void safeRestOfFile(int file_handle);

    /// IRunable implementation : Run the class implementation
    virtual StatusCode i_run();

  public:
    /// Standard Constructor
    HltBufferedIOReader(const std::string& nam, ISvcLocator* svcLoc);
    /// Standard Destructor
    virtual ~HltBufferedIOReader();
    /// IInterface implementation : queryInterface
    virtual StatusCode queryInterface(const InterfaceID& riid,
                                      void** ppvInterface);
    /// IService implementation: initialize the service
    virtual StatusCode initialize();
    /// Low level overload from Service base class: sysStart
    virtual StatusCode sysStart();
    /// IService implementation: finalize the service
    virtual StatusCode finalize();
    /// Low level overload from Service base class: sysStop
    virtual StatusCode sysStop();
    /// Incident handler implemenentation: Inform that a new incident has occured
    virtual void handle(const Incident& inc);
    /// IRunable implementation : Run the class implementation
    virtual StatusCode run();
  };
} // End namespace LHCb
#endif // GAUDIONLINE_DIMRPCFILEREADER_H
// $Id: HltBufferedIOReader.cpp,v 1.29 2010-09-21 14:28:25 frankb Exp $
#include "GaudiKernel/SmartIF.h"
#include "GaudiKernel/Incident.h"
#include "GaudiKernel/IAppMgrUI.h"
#include "GaudiKernel/IRegistry.h"
#include "GaudiKernel/SvcFactory.h"
#include "GaudiKernel/DataIncident.h"
#include "GaudiKernel/IIncidentSvc.h"
//#include "GaudiOnline/HltBufferedIOReader.h"
#include "GaudiOnline/MEPManager.h"
#include "MBM/BufferInfo.h"
#include "MDF/MEPEvent.h"
#include "MDF/MDFHeader.h"
#include "RTL/rtl.h"
#include "RTL/readdir.h"
#include <fcntl.h>
#include <cerrno>
#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#define O_BINARY 0
#endif

// Instantiation of a static factory class used by clients to create instances of this service
DECLARE_NAMESPACE_SERVICE_FACTORY(LHCb,HltBufferedIOReader)

using namespace LHCb;
using namespace std;

static int file_read(int file, char* p, int len) {
  int tmp = 0;
  while (tmp < len)   {
    int sc = ::read(file, p + tmp, len - tmp);
    if (sc > 0)
      tmp += sc;
    else if (sc == 0)
      return 0;
    else return -1;
  }
  return 1;
}

static bool file_write(int file, const void* data, int len)   {
  const char* p = (const char*) data;
  int tmp = len;
  while (tmp > 0)  {
    int sc = ::write(file, p + len - tmp, tmp);
    if (sc > 0)
      tmp -= sc;
    else return false;
  }
  return true;
}

/// Standard Constructor
HltBufferedIOReader::HltBufferedIOReader(const string& nam, ISvcLocator* svcLoc)
  : OnlineService(nam, svcLoc), m_receiveEvts(false), m_lock(0), m_mepMgr(0), 
    m_producer(0), m_evtCount(0), m_disabled(false)
{
  declareProperty("Buffer",      m_buffer          = "Mep");
  declareProperty("Directory",   m_directory       = "/localdisk");
  declareProperty("FilePrefix",  m_filePrefix      = "Run_");
  declareProperty("BrokenHosts", m_brokenHostsFile = "");
  declareProperty("DeleteFiles", m_deleteFiles     = true);
  declareProperty("ConsumerWait",m_maxConsWait     = 20);
  declareProperty("AllowedRuns", m_allowedRuns);
  m_allowedRuns.push_back("*");
  ::lib_rtl_create_lock(0, &m_lock);
}

/// Standard Destructor
HltBufferedIOReader::~HltBufferedIOReader()  {
  if ( m_lock ) {
    ::lib_rtl_delete_lock(m_lock);
  }
}

/// IInterface implementation : queryInterface
StatusCode HltBufferedIOReader::queryInterface(const InterfaceID& riid, void** ppIf)    {
  if (IRunable::interfaceID().versionMatch(riid))  {
    *ppIf = (IRunable*) this;
    addRef();
    return StatusCode::SUCCESS;
  }
  return OnlineService::queryInterface(riid, ppIf);
}

/// IService implementation: initialize the service
StatusCode HltBufferedIOReader::initialize()   {
  StatusCode sc;
  string broken_hosts;
  if (!(sc = OnlineService::initialize()).isSuccess())
    return error("Failed to initialize service base class.");
  else if (!(sc = service("MEPManager", m_mepMgr)).isSuccess())
    return error("Failed to access MEP manager service.");
  broken_hosts = m_brokenHostsFile;
  if ( !broken_hosts.empty() ) {
    struct stat file;
    if ( 0 == ::stat(broken_hosts.c_str(),&file) ) {
      const std::string node = RTL::nodeNameShort();
      int   fd   = ::open(broken_hosts.c_str(),O_RDONLY);
      if ( -1 == fd )  {
	return error("Failed to access broken node file:"+broken_hosts+" [Error ignored]");
      }
      char* data = new char[file.st_size+1];
      int rc = file_read(fd,data,file.st_size);
      if ( 1 == rc ) {
        data[file.st_size] = 0;
        for(int i=0; i<file.st_size; ++i) 
          data[i] = ::toupper(data[i]);
        for(char* ptr=(char*)node.c_str(); *ptr; ++ptr)
          *ptr = ::toupper(*ptr);
        if ( ::strstr(data,node.c_str()) ) {
          warning("Node is disabled and will not process any deferred files.");
          m_disabled = true;
        }
      }
      delete [] data;
      ::close(fd);
    }
    else {
      error("Failed to access broken node file:"+broken_hosts+" [Error ignored]");
    }
  }

  m_producer = m_mepMgr->createProducer(m_buffer, RTL::processName());
  if (0 == m_producer)  {
    return error("Fatal error: Failed to create MBM producer object.");
  }
  incidentSvc()->addListener(this, "DAQ_CANCEL");
  declareInfo("EvtCount", m_evtCount = 0, "Number of events processed");
  ::lib_rtl_lock(m_lock);
  m_files.clear();
  m_current = "";
  return sc;
}

/// IService implementation: finalize the service
StatusCode HltBufferedIOReader::finalize()  {
  if (m_producer)  {
    if (m_mepMgr->FSMState() != Gaudi::StateMachine::OFFLINE)    {
      m_producer->exclude();
    }
    delete m_producer;
    m_producer = 0;
  }
  m_files.clear();
  releaseInterface(m_mepMgr);
  return OnlineService::finalize();
}

StatusCode HltBufferedIOReader::sysStart()  {
#if 0
  if ( m_producer )  {
    m_producer->exclude();
    delete m_producer;
    m_producer = 0;
  }
  m_producer = m_mepMgr->createProducer(m_buffer,RTL::processName());
#endif
  m_receiveEvts = true;
  ::lib_rtl_unlock(m_lock);
  return StatusCode::SUCCESS;
}

/// IService implementation: finalize the service
StatusCode HltBufferedIOReader::sysStop()   {
  incidentSvc()->removeListener(this);
  if (m_receiveEvts)  {
    if (m_mepMgr)    {
      m_mepMgr->cancel();
    }
  }
  ::lib_rtl_unlock(m_lock);
  m_receiveEvts = false;
  return StatusCode::SUCCESS;
}

/// Incident handler implemenentation: Inform that a new incident has occured
void HltBufferedIOReader::handle(const Incident& inc)
{
  info("Got incident:" + inc.source() + " of type " + inc.type());
  if (inc.type() == "DAQ_CANCEL")  {
    m_receiveEvts = false;
    if (m_mepMgr)    {
      m_mepMgr->cancel();
    }
    ::lib_rtl_unlock(m_lock);
  }
  else if (inc.type() == "DAQ_ENABLE")  {
    m_receiveEvts = true;
  }
}

/// Scan directory for matching files
size_t HltBufferedIOReader::scanFiles()   {
  m_files.clear();
  if ( !m_disabled )   {
    DIR* dir = opendir(m_directory.c_str());
    if (dir)  {
      struct dirent *entry;
      bool take_all = (m_allowedRuns.size() > 0 && m_allowedRuns[0]=="*");
      while ((entry = ::readdir(dir)) != 0)    {
        //cout << "File:" << entry->d_name << endl;
        if ( 0 != ::strncmp(entry->d_name,m_filePrefix.c_str(),4) ) {
          continue;
        }
        else if ( !take_all )  {
          bool take_run = false;
          for(vector<string>::const_iterator i=m_allowedRuns.begin(); i!=m_allowedRuns.end(); ++i) {
            if ( ::strstr(entry->d_name,(*i).c_str()) != 0 ) {
              take_run = true;
              break;
            }
          }
          if ( !take_run ) continue;
        }
        //cout << "SELECT:" << entry->d_name << endl;
        m_files.insert(m_directory + "/" + entry->d_name);
      }
      ::closedir(dir);
      return m_files.size();
    }
    const char* err = RTL::errorString();
    info("Failed to open directory:" + string(err ? err : "????????"));
  }
  return 0;
}

/// Open a new data file
int HltBufferedIOReader::openFile()   {
  while (m_files.size() > 0)  {
    set<string>::iterator i = m_files.begin();
    string fname = *i;
    m_files.erase(i);
    int fd = ::open(fname.c_str(), O_RDONLY | O_BINARY, S_IREAD);
    if ( -1 != fd )    {
      if (m_deleteFiles)      {
        int sc = ::unlink(fname.c_str());
        if (sc != 0)        {
          error("CANNOT UNLINK file: " + fname + ": " + RTL::errorString());
          //::exit(EBADF);
          m_receiveEvts = false;
          incidentSvc()->fireIncident(Incident(name(),"DAQ_ERROR"));
	  ::close(fd);
          return 0;
        }
      }
      m_current = fname;
      info("Opened file: " + fname + " for deferred HLT processing");
      return fd;
    }
    error("FAILD to open file: " + fname + " for deferred HLT processing: "
          + RTL::errorString());
  }
  return 0;
}

/// Save remainder of currently read file
void HltBufferedIOReader::safeRestOfFile(int file_handle)     {
  if (file_handle)  {
    char buffer[10 * 1024];
    info("Saving rest of file[%d]: %s",file_handle,m_current.c_str());
    int cnt = 0, ret, fd = ::open(m_current.c_str(), O_CREAT | O_BINARY | O_WRONLY, 0777);
    if ( -1 == fd )    {
      error("CANNOT Create file: " + m_current + ": " + RTL::errorString());
      return;
    }
    while ((ret = ::read(file_handle, buffer, sizeof(buffer))) > 0)  {
      if (!file_write(fd, buffer, ret))      {
        error("CANNOT write file: " + m_current + ": " + RTL::errorString());
      }
      cnt += ret;
    }
    info("Wrote %d bytes to file:%s fd:%d",cnt,m_current.c_str(),fd);
    ::close(fd);
    ::close(file_handle);
    m_current = "";
    file_handle = 0;
  }
}

static bool check_consumers(const MBM::BufferInfo& info, int pid, int evtyp)   {
  return info.num_consumers_partid_evtype(pid,evtyp) != 0;
}
static bool check_consumers_partid(const MBM::BufferInfo& info, int pid)   {
  return info.num_consumers_partid(pid) != 0;
}

/// IRunable implementation : Run the class implementation
StatusCode HltBufferedIOReader::i_run()  {
  const RawBank* bank = (RawBank*)this;
  int cons_wait;
  int file_handle = 0;
  int partid = m_mepMgr->partitionID();
  bool files_processed = false;
  bool m_goto_paused = true;
  std::string bm_name = m_mepMgr->bufferName(m_buffer);
  MBM::BufferInfo mbmInfo;
  

  m_receiveEvts = true;

  /// Before we start, we need to check if there are consumers present:
  mbmInfo.attach(bm_name.c_str());

  for(cons_wait = m_maxConsWait; !check_consumers_partid(mbmInfo,partid) && --cons_wait>=0; )
    ::lib_rtl_sleep(1000);

  if ( cons_wait <= 0 )   {
    incidentSvc()->fireIncident(Incident(name(),"DAQ_ERROR"));
    return error("No consumers present to consume event. ERROR & Return!");
  }
  
  while (1)  {
    files_processed = scanFiles() == 0;
    if ( m_goto_paused && files_processed )   {
      /// Go to state PAUSED, all the work is done
      incidentSvc()->fireIncident(Incident(name(),"DAQ_PAUSE"));
    }
    else if ( files_processed ) {
      info("Locking event loop. Waiting for work....");
      ::lib_rtl_lock(m_lock);
    }
    if ( !m_receiveEvts )    {
      info("Quitting...");
      break;
    }
    m_evtCount = 0;
    files_processed = scanFiles() == 0;
    if ( files_processed )    {
      info("Exit event loop. No more files to process.");
      break;
    }
    // loop over the events
    while (m_receiveEvts)   {
      if (file_handle == 0)  {
        file_handle = openFile();
        if ( file_handle == 0 )   {
          files_processed = scanFiles() == 0;
          if ( files_processed )    {
            break;
          }
        }
      }
      if ( file_handle != 0 )  {
        int size_buf[3];
	off_t file_position = ::lseek(file_handle, 0, SEEK_CUR);
        int status = ::file_read(file_handle, (char*)size_buf, sizeof(size_buf));
        if (status <= 0)   {
          ::close(file_handle);
          file_handle = 0;
          m_current = "";
          continue;
        }
	bool is_mdf   = size_buf[0] == size_buf[1] && size_buf[0] == size_buf[2];
	int  evt_size = size_buf[0];
	int  buf_size = evt_size + (is_mdf ? bank->hdrSize() : sizeof(MEPEVENT) + sizeof(int));
        // Careful here: sizeof(int) MUST match me->sizeOf() !
        // The problem is that we do not (yet) have a valid data pointer!
        try   {
          status = m_producer->spaceRearm(buf_size);
        }
        catch (const exception& e)        {
          error("Exception while reading Input files (spaceRearm): "+string(e.what())+
		" Skipping rest of file: "+m_current);
          ::close(file_handle);
          file_handle = 0;
          m_current = "";
          continue;
        }
        if (status == MBM_NORMAL)        {
	  MBM::EventDesc& dsc = m_producer->event();
	  char*  read_ptr = 0;
	  size_t read_len = 0;
	  if ( is_mdf ) {
	    RawBank* b = (RawBank*)dsc.data;
	    b->setMagic();
	    b->setType(RawBank::DAQ);
	    b->setSize(MDFHeader::sizeOf(3));
	    b->setVersion(DAQ_STATUS_BANK);
	    read_ptr = b->begin<char>();
	    ::memcpy(read_ptr,size_buf,sizeof(size_buf));
	    read_ptr += sizeof(size_buf);
	    read_len = evt_size-sizeof(size_buf);
	    dsc.len  = evt_size+b->hdrSize();
	    dsc.type = EVENT_TYPE_EVENT;
	  }
	  else {
	    static int id = -1;
	    MEPEVENT* e = (MEPEVENT*) dsc.data;
	    MEPEvent* me = (MEPEvent*) e->data;
	    me->setSize(evt_size);
	    e->refCount = m_refCount;
	    e->evID = ++id;
	    e->begin = long(e) - long(m_producer->bufferAddress());
	    e->packing = -1;
	    e->valid = 1;
	    e->magic = mep_magic_pattern();
	    for (size_t j = 0; j < MEP_MAX_PACKING; ++j)   {
	      e->events[j].begin = 0;
	      e->events[j].evID = 0;
	      e->events[j].status = EVENT_TYPE_OK;
	      e->events[j].signal = 0;
	    }
	    read_ptr = (char*)me->start();
	    ::memcpy(read_ptr,size_buf+1,2*sizeof(size_buf[0]));
	    read_ptr += 2*sizeof(size_buf[0]);
	    read_len = me->size() - 2*sizeof(int);
	    dsc.len  = sizeof(MEPEVENT) + me->sizeOf() + me->size();
	    dsc.type = EVENT_TYPE_MEP;
	  }
	  status = ::file_read(file_handle,read_ptr, read_len);
	  if (status <= 0)   {
	    ::close(file_handle);
	    file_handle = 0;
	    m_current = "";
	    continue;
	  }
	  if (status == MBM_NORMAL)  {
	    // Check if there are consumers pending before declaring the event.
	    // This should be a rare case, since there ARE (were?) consumers.
	    // Though: In this case the event is really lost!
	    // But what can I do...
	    for(cons_wait = m_maxConsWait; !check_consumers(mbmInfo,partid,dsc.type) && --cons_wait>=0; )
	      ::lib_rtl_sleep(1000);

	    if ( cons_wait <= 0 )  {
	      error("No consumers present to consume event. Safe rest of file and finish."
		    " Skipping rest of file: "+m_current);
	      m_receiveEvts = false;
	      ::lseek(file_handle, file_position, SEEK_SET);
	      safeRestOfFile(file_handle);
	      /// Go to state PAUSED, all the work is done
	      incidentSvc()->fireIncident(Incident(name(),"DAQ_ERROR"));
	      return StatusCode::FAILURE;
	    }
	    //cout << "Event length:" << dsc.len << endl;
	    dsc.mask[0] = partid;
	    dsc.mask[1] = ~0x0;
	    dsc.mask[2] = ~0x0;
	    dsc.mask[3] = ~0x0;
	    m_producer->declareEvent();
	    status = m_producer->sendSpace();
	    if (status == MBM_NORMAL)    {
	      m_evtCount++;
	    }
	  }
	}
        else if (file_handle)   {
          // undo reading of the first integer before saving rest of file
          ::lseek(file_handle, -3*sizeof(int), SEEK_CUR);
        }
      }
    }
    safeRestOfFile(file_handle);
    // Bad file: Cannot read input (m_evtCount==0)
    m_evtCount = 0;
  }
  return StatusCode::SUCCESS;
}

/// IRunable implementation : Run the class implementation
StatusCode HltBufferedIOReader::run()   {
  try  {
    StatusCode sc = i_run();
    return sc;
  }
  catch (const exception& e)  {
    error("Exception while reading MEP files:" + string(e.what()));
    throw;
  }
  catch (...)  {
    error("UNKNWON Exception while reading MEP files.");
    throw;
  }
}
