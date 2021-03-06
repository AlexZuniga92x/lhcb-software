//	============================================================
//
//	FileWriteMgr.h
//	------------------------------------------------------------
//
//	Package   : GaudiOnline
//
//	Author    : Markus Frank
//
//	===========================================================
#ifndef GAUDIONLINE_FILEWRITEMGR_H
#define GAUDIONLINE_FILEWRITEMGR_H 1

// Framework include files
#include "GaudiKernel/IRunable.h"
#include "GaudiKernel/IIncidentListener.h"
#include "GaudiOnline/OnlineService.h"
#include "MDF/RawEventHelpers.h"
#include "MDF/RawEventDescriptor.h"
#include <map>
#include <list>
#include <vector>
#include "stdio.h"
#include "time.h"
#include "pthread.h"

// Forward declarations
class IIncidentSvc;
namespace MBM
{
  class Consumer;
  class EventDesc;
}

/*
 *  LHCb namespace declaration
 */
namespace LHCb
{
  class MEPManager;
  class FileWriterSvc;
  class FileWriteMgr: public OnlineService,
      virtual public IRunable,
      virtual public IIncidentListener
  {
  public:
    typedef std::vector<std::string>        Requirements;
  protected:
    /// Reference to MEP manager service
    pthread_mutex_t m_listlock;
    MEPManager* m_mepMgr;
    MBM::Consumer* m_consumer;
    /// Flag indicating that MBM event retrieval is active
    bool m_receiveEvts;
    /// Property: printout frequence
    float m_freq;
    /// Property: Flag to register callback for MEP error event handling. 0=No handling, 1=save as MEP, 2=save as events
    int m_handleErrs;
    /// Property: 4rth. Word of trigger mask for sending ERROR MEPs.
    int m_routingBits;
    /// Property: Request specification
    Requirements m_req;
    /// Property: Name of output buffer for error events (requires handleErrs to be set)
    std::string m_errBuffer;
    /// Property: MEP manager service name/type
    std::string m_mepMgrName;
    /// Name of the data input buffer
    std::string m_input;
    /// Cached minimal allocation size for multi burst mode
    int m_minAlloc;
    /// Monitoring quantity: Number of MEPs processed
    long m_mepIn;
    /// Monitoring quantity: Number of MEPs processed
    long m_mepOut;
    /// Bytes Written to file
    long m_BytesOut;
    /// Number of Event in
    long m_EvIn;
    /// Number of Event out
    long m_EvOut;
    /// Number of files
    long m_NumFiles;
    /// Current Run Number (initialized to 0)
    unsigned int m_RunNumber;
    ///Limit on the filesize in MB=1024*1024 Bytes
    long m_SizeLimit;
//    FileDescr *m_FileDesc;
    ///List of runs and descriptors;
//    std::map<unsigned int,RunDesc*> m_RunList;
    std::string m_node;
    int m_FileCloseDelay;
  public:
    ISvcLocator* m_SvcLoc;
    typedef std::list<FileWriterSvc*> SvcList;
    typedef std::list<IService*> ISvcList;
    typedef std::vector<std::string> FileWriterList;
    SvcList m_ServiceList;
    FileWriterList m_FileWriters;
    /// Standard Constructor
    FileWriteMgr(const std::string& name, ISvcLocator* svc);
    /// Standard Destructor
    virtual ~FileWriteMgr();
    /// IInterface implementation: Query interface
    virtual StatusCode queryInterface(const InterfaceID& riid,
        void** ppvInterface);
    /// Incident handler implemenentation: Inform that a new incident has occured
    void handle(const Incident& inc);
    /// Service overload: initialize()
    virtual StatusCode initialize();
    /// Service overload: finalize()
    virtual StatusCode finalize();
    /// Process events
    virtual StatusCode run();
    /// get the Run Number from the current MEP
    virtual unsigned int getRunNumber(MEPEvent *me);
    /// Open a new file
//    virtual FileDescr *openFile(unsigned int runnumber,FTYPE t);
    /// Declare Monitoring information on a run
//    virtual void CreateMonitoringInfo(unsigned int runn);
    /// get the file time string
    std::string FileTime();
//    virtual void handleFileWriteError();
//    virtual ssize_t Write(int fd, const void *buf, size_t n);
//    virtual void Markclose(FileDescr* d);
    void LockList(void)
    {
      pthread_mutex_lock(&m_listlock);
    }
    void UnlockList(void)
    {
      pthread_mutex_unlock(&m_listlock);
    }
//    std::list<FileDescr*> m_FileCloseList;
    bool m_texit;
    pthread_t m_tid;
  };
} // End namespace LHCb
#endif //  GAUDIONLINE_FILEWRITEMGR_H
