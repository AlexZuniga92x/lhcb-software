// $Id: RichGlobalPIDFinalize.h,v 1.2 2003-07-02 09:02:59 jonrob Exp $
#ifndef RICHRECALGS_RICHGLOBALPIDFINALIZE_H
#define RICHRECALGS_RICHGLOBALPIDFINALIZE_H 1

// Base class
#include "RichGlobalPIDAlgBase.h"

// from Gaudi
#include "GaudiKernel/AlgFactory.h"

// Event
#include "Event/ProcStatus.h"
#include "Event/RichRecStatus.h"

/** @class RichGlobalPIDFinalize RichGlobalPIDFinalize.h
 *
 *  Finalises the PID results for the Global PID algorithm
 *
 *  @author Chris Jones   Christopher.Rob.Jones@cern.ch
 *  @date   12/12/2002
 */

class RichGlobalPIDFinalize : public RichGlobalPIDAlgBase {

public:

  /// Standard constructor
  RichGlobalPIDFinalize( const std::string& name,
                         ISvcLocator* pSvcLocator );

  virtual ~RichGlobalPIDFinalize();   ///< Destructor

  virtual StatusCode initialize();    ///< Algorithm initialization
  virtual StatusCode execute   ();    ///< Algorithm execution
  virtual StatusCode finalize  ();    ///< Algorithm finalization

private:
  
  /// Location of processing status object in TES
  std::string m_procStatLocation;
  
};

#endif // RICHRECALGS_RICHGLOBALPIDFINALIZE_H
