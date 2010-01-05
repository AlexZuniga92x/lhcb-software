// $Id: TrackContainerCopy.h,v 1.5 2010-01-05 11:30:13 wouter Exp $
#ifndef _TrackContainerCopy_H_
#define _TrackContainerCopy_H_

/** @class TrackContainerCopy TrackContainerCopy.h
 *
 *  Copy a container of tracks. By default do not copy tracks that failed the fit
 *
 *  @author M.Needham
 *  @date   30/05/2006
 */

#include "GaudiAlg/GaudiAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "TrackInterfaces/ITrackSelector.h"
#include <string>

class TrackContainerCopy: public GaudiAlgorithm {

public:

  // Constructors and destructor
  TrackContainerCopy(const std::string& name,
              ISvcLocator* pSvcLocator);
  virtual ~TrackContainerCopy();

  virtual StatusCode initialize();
  virtual StatusCode finalize();

  virtual StatusCode execute();

private:
  std::string m_inputLocation;
  std::string m_outputLocation;
  bool        m_copyFailures;   ///< If true, copy also tracks that failed fit 
  ToolHandle<ITrackSelector> m_selector;
};

#endif
