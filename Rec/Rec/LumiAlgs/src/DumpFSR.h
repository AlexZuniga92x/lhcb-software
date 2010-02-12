// $Id: DumpFSR.h,v 1.2 2010-02-12 16:56:42 panmanj Exp $
#ifndef DUMPFSR_H 
#define DUMPFSR_H 1

// Include files
// from Gaudi
#include "GaudiAlg/GaudiAlgorithm.h"

#include "GaudiKernel/IRegistry.h"
#include "GaudiKernel/IDataManagerSvc.h"

// event model
#include "Event/LumiFSR.h"
#include "Event/EventCountFSR.h"
#include "Event/TimeSpanFSR.h"

/** @class DumpFSR DumpFSR.h
 *   
 *
 *  @author Jaap Panman
 *  @date   2009-02-27
 */
class DumpFSR : public GaudiAlgorithm {
public: 
  /// Standard constructor
  DumpFSR( const std::string& name, ISvcLocator* pSvcLocator );

  virtual ~DumpFSR( ); ///< Destructor

  virtual StatusCode initialize();    ///< Algorithm initialization
  virtual StatusCode execute   ();    ///< Algorithm execution
  virtual StatusCode finalize  ();    ///< Algorithm finalization

protected:
  virtual void dump_file();            ///< print the FSRs of one input file
  virtual std::string fileID();       ///< get the fileID
  virtual std::vector< std::string > navigate(std::string rootname, std::string tag); 
  virtual void explore(IRegistry* pObj, std::string tag, std::vector< std::string >& a);

protected:
  /// Reference to run records data service
  IDataProviderSvc* m_fileRecordSvc;

  std::string m_rawEventLocation;               // Location where we get the RawEvent
  std::string m_FileRecordName;                 // location of FileRecords
  std::string m_FSRName;                        // specific tag of summary data in FSR
  std::string m_LowFSRName;                     // specific tag of low lumi summary data in FSR
  std::string m_EventCountFSRName;              // specific tag of event summary data in FSR
  std::string m_TimeSpanFSRName;                // specific tag of event summary data in FSR
  std::string m_current_fname;                  // current file ID string 
  int         m_count_files;                    // number of files read
  int         m_count_events;                   // number of events read

private:

};
#endif // DUMPFSR_H
