// $Id: CPUMon.h,v 1.10 2010-09-15 17:11:36 frankb Exp $
//====================================================================
//  ROMon
//--------------------------------------------------------------------
//
//  Package    : ROMon
//
//  Description: Structure of basic items published by the readout
//               monitoring.
//
//  Author     : M.Frank
//  Created    : 29/1/2008
//
//====================================================================
// $Header: /afs/cern.ch/project/cvs/reps/lhcb/Online/ROMon/ROMon/CPUMon.h,v 1.10 2010-09-15 17:11:36 frankb Exp $
#ifndef ROMON_CPUMON_H
#define ROMON_CPUMON_H 1

// C++ include files
#include <ctime>
#include <vector>
#include <string>
#include "ROMon/Collections.h"
#include "RTL/Pack.h"

/*
 *   ROMon namespace declaration
 */
namespace ROMon {

  void ro_get_node_name(char* name, size_t len);

  /**@class Memory CPUInfo.h ROMon/CPUInfo.h
   *
   * Class which represents the memory information of a single node.
   * Note: All numbers are in KB!
   *
   * @author M.Frank
   */
  PACK_DATA(class) Memory {
  public:
    int memTotal;     // 1
    int memFree;      // 2
    int buffers;      // 3
    int cached;       // 4
    int swapCached;   // 5
    int active;       // 6
    int inactive;     // 7
    int highTotal;    // 8
    int highFree;     // 9
    int lowTotal;     // 10
    int lowFree;      // 11
    int swapTotal;    // 12
    int swapFree;     // 13
    int dirty;        // 14
    int writeback;    // 15
    int mapped;       // 16
    int slab;         // 17
    int commitLimit;  // 18
    int committed_AS; // 19
    int pageTables;   // 20
    int vmallocTotal; // 21
    int vmallocUsed;  // 22
    int vmallocChunk; // 23
    int _last;        // 24 ... padding
    /// Empty constructor
    Memory();
    /// Reset data content
    Memory*          reset();
    /// Size of the object
    long   sizeOf() const {  return sizeof(Memory); }
  };

  /**@class CPU CPUInfo.h ROMon/CPUInfo.h
   *
   * Class which represents a CPU core in a single node
   *
   * @author M.Frank
   */
  PACK_DATA(class) CPU {
  public:
    /// Clock speed [from cpu/info]
    float          clock;
    /// Cache size [from cpu/info]
    int            cache;
    /// Bogomips estimator
    float          bogomips;
    struct Stat {
      /// Usage of CPU in user mode in [%] [from cpu/stat]
      float          user;
      /// Usage of CPU in system mode in [%] [from cpu/stat]
      float          system;
      /// CPU nice value [from cpu/stat]
      float          nice;
      /// Usage of CPU in idle mode in [%] [from cpu/stat]
      float          idle;
      /// CPU in IO wait in [%] [from cpu/stat]
      float          iowait;
      /// Interrupt rate in [%] [from cpu/stat]
      float          IRQ;
      /// Soft interrupt rate in [%] [from cpu/stat]
      float          softIRQ;
    } stats;
    /// Empty constructor
    CPU();
    /// Reset data content
    CPU*          reset();
    /// Size of the object
    long   sizeOf() const {  return sizeof(CPU); }
  };

  /**@class CPUset ROMon.h ROMon/ROMon.h
   *
   * Class which represents all CPU cores in a single node
   *
   * @author M.Frank
   */
  PACK_DATA(class) CPUset {
  public:
    typedef FixItems<CPU> Cores;
    /// Node name
    char         name[32];
    /// CPU Familiy name
    char         family[32];
    /// Context switch rate in [Hz] [from cpu/stat]
    float        ctxtRate;
    /// Boot time in seconds since epoch
    int          boot;
    /// Total memory in kB
    int          memory;
    /// Free memory in kB
    int          memfree;
    /// Time stamp of last information update
    int          time;
    /// Time stamp of the monitor snapshot [milli seconds]
    unsigned int millitm;
    /// Average readings
    CPU::Stat    averages;
    /// Variable size array of node items
    Cores        cores;
    /// Default constructor
    CPUset();
    /// Reset object structure
    CPUset* reset();
    /// Length of the object in bytes
    int length()  const {  return sizeof(CPUset)+cores.data_length(); }
  };

  /**@class CPUfarm ROMon.h ROMon/ROMon.h
   *
   * Class which represents all CPUs in a DIM_DNS_NODE
   *
   * @author M.Frank
   */
  PACK_DATA(class) CPUfarm {
  public:
    typedef VarItems<CPUset> Nodes;
    typedef std::pair<int,unsigned int> TimeStamp;
    enum { TYPE = 4 };
    /// First word: Data type descriptor (MUST always be 4)
    int   type;
    /// Total size of the node information
    int  totalSize;
    /// Date of last update
    int time;
    /// Farm name
    char  name[32];
    /// Variable size array of node items
    Nodes  nodes;
    /// Reset object structure
    CPUfarm* reset();
    /// Fix-up object sizes
    void fixup();
    /// Length of the object in bytes
    int length()  const {  return sizeof(CPUfarm)+nodes.data_length(); }
    /// Retrieve timestamp of earliest updated node
    TimeStamp firstUpdate() const;
    /// Retrieve timestamp of most recent updated node
    TimeStamp lastUpdate() const;
  };

  /**@class Process CPUInfo.h ROMon/CPUInfo.h
   *
   * Class which represents a single Process (task)
   *
   * @author M.Frank
   */
  PACK_DATA(class) Process {
  public:
    /// Processes UTGID
    char           utgid[64];
    /// Processes command
    char           cmd[64];
    /// Processes owner
    char           owner[32];
    /// CPU usage in percent
    double         cpu;
    /// Memory usage in percent
    float          mem;
    /// Size of virtual memory
    float          vsize;
    /// Size of resident memory
    float          rss;
    /// Stack size of the process
    float          stack;
    /// Startup time in seconds since epoch
    int            start;
    /// Process ID
    unsigned short pid;
    /// Parent ID
    unsigned short ppid;
    /// Number of threads
    unsigned short threads;
    /// Process state
    char           state;
    /// Padding
    unsigned char pad;
    /// Empty constructor
    Process();
    /// Reset data content
    Process*      reset();
    /// Size of the object
    long   sizeOf() const {  return sizeof(Process); }
  };

  /**@class Procset ROMon.h ROMon/ROMon.h
   *
   * Class which represents all Processes in a single node
   *
   * @author M.Frank
   */
  PACK_DATA(class) Procset {
  public:
    typedef FixItems<Process> Processes;
    /// Node name
    char   name[32];
    /// Time stamp of last information update
    int time;
    /// Time stamp of the monitor snapshot [milli seconds]
    unsigned int millitm;
    /// Variable size array of node items
    Processes processes;
    /// Default constructor
    Procset();
    /// Reset object structure
    Procset* reset();
    /// Length of the object in bytes
    int length()  const {  return sizeof(Procset)+processes.data_length(); }
  };

  /**@class ProcFarm ROMon.h ROMon/ROMon.h
   *
   * Class which represents all CPUs in a DIM_DNS_NODE
   *
   * @author M.Frank
   */
  PACK_DATA(class) ProcFarm {
  public:
    typedef VarItems<Procset> Nodes;
    typedef std::pair<int,unsigned int> TimeStamp;
    enum { TYPE = 5 };
    /// First word: Data type descriptor (MUST always be 5)
    int   type;
    /// Total size of the node information
    int  totalSize;
    /// Date of last update
    int time;
    /// Farm name
    char  name[32];
    /// Variable size array of node items
    Nodes  nodes;
    /// Reset object structure
    ProcFarm* reset();
    /// Fix-up object sizes
    void fixup();
    /// Length of the object in bytes
    int length()  const {  return sizeof(ProcFarm)+nodes.data_length(); }
    /// Retrieve timestamp of earliest updated node
    TimeStamp firstUpdate() const;
    /// Retrieve timestamp of most recent updated node
    TimeStamp lastUpdate() const;
  };

  /**@class NodeStats ROMon.h ROMon/ROMon.h
   *
   * Class which represents all stat counters of a given node
   * -- Memory information
   * -- CPU information
   * -- Process information
   *
   * @author M.Frank
   */
  PACK_DATA(class) NodeStats {
  public:
    enum { TYPE = 6 };
    /// First word: Data type descriptor (MUST always be 6)
    int  type;
    /// Total size of the node information
    int  totalSize;
    /// Size of the MBM buffer information
    int  cpuSize;
    /// Size of the FSM task list
    int  procSize;
    /// Time stamp of the monitor snapshot [seconds since epoche]
    int  time;
    /// Time stamp of the monitor snapshot [milli seconds]
    unsigned int millitm;
    /// Memory information
    Memory memory;
    /// Object name
    char name[32];

  public:    // Public data accessors
    /// Reset object structure
    NodeStats* reset();
    /// Access to the variable length array of CPU information
    CPUset*  cpu() const;
    /// Access to the variable length array of process descriptors
    Procset* procs() const;
    /// Fix-up object sizes
    void fixup();
    /// Length of the object in bytes
    int length()  const {  return totalSize; }
  };

  /**@class Connectionset ROMon.h ROMon/ROMon.h
   *
   * Class which represents a single outgoing Connection from node
   *
   * @author M.Frank
   */
  PACK_DATA(class) Connection {
  public:
    char node[32];
    int  last;
    int  status;
    /// Standard constructor
    Connection();
    /// Reset data structure content
    Connection* reset();
  };

  /**@class Connectionset ROMon.h ROMon/ROMon.h
   *
   * Class which represents all outgoing ping-Connections of a single node
   *
   * @author M.Frank
   */
  PACK_DATA(class) Connectionset {
  public:
    enum { TYPE = 7 };
    typedef FixItems<Connection> Connections;
    /// First word: Data type descriptor (MUST always be 6)
    int  type;
    /// Node name
    char   name[32];
    /// Time stamp of last information update
    int time;
    /// Time stamp of the monitor snapshot [milli seconds]
    unsigned int millitm;
    /// Variable size array of node items
    Connections connections;
    /// Default constructor
    Connectionset();
    /// Reset object structure
    Connectionset* reset();
    /// Length of the object in bytes
    int length()  const {  return sizeof(Connectionset)+connections.data_length(); }
  };


  std::vector<std::string> psItems(const char*& ptr);
  std::vector<std::string> psLabels();
  enum PsDataItems {
    UTGID=0, 
    TID, 
    TGID, 
    PPID, 
    PGID, 
    NLWP, 
    USER_NAME,
    GROUP_NAME,
    TTY,
    CMD,
    SCHED,
    PRIO,
    RTPRIO,
    NICE,
    PSR,
    STAT,
    PERCENT_CPU,
    PERCENT_MEMORY,
    VIRTUAL_SIZE,
    LOCK,
    RESIDENT_MEMORY,
    DATA_MEMORY,
    STACK_MEMORY,
    EXECUTABLE,
    LIB,
    SHARE,
    IGNORED_SIGNALS,
    PENDING_SIGNALS,
    CATCHED_SIGNALS,
    BLOCKED_SIGNALS,
    START_TIME,
    ELAPSED_TIME,
    CPU_TIME,
    CMDLINE
  };
  union CPUMonData {
    char*            str;
    void*            ptr;
    CPU*             cpu;
    CPUset*          cpuSet;
    CPUfarm*         farm;
    Process*         proc;
    Procset*         processes;
    Connectionset*   connections;
    ProcFarm*        procFarm;
    NodeStats*       node;
    CPUMonData(void* buffer) { ptr=buffer; }
    int type() { return this->ptr ? *(int*)this->ptr : -1; }
  };
}
#include "RTL/Unpack.h"

#endif /* ROMON_CPUMON_H */

