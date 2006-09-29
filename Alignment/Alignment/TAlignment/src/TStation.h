//-----------------------------------------------------------------------------
#ifndef Alignment_TALIGNMENT_H
#define Alignment_TALIGNMENT_H
/** @file TAlignment.h
 *
 *  Header file for Tstation alignment algorithm: TAlignment
 *
 *  CVS Log :-
 *  $Id: TStation.h,v 1.1.1.1 2006-09-29 10:09:56 jblouw Exp $
 *
 *  @author J. Blouw  Johan.Blouw@cern.ch
 *  @author M.Needham Matt.Needham@cern.ch
 *  @author Chris Jones   Christopher.Rob.Jones@cern.ch
 *  @date   30/12/2005
 */
//-----------------------------------------------------------------------------



#include "AlignmentInterfaces/IATrackSelectorTool.h"
#include <string>

// GaudiKernel
#include "GaudiAlg/GaudiTupleAlg.h"
#include "GaudiKernel/HashMap.h"

#include "Event/Track.h"

class TStation : public GaudiTupleAlg {

public:

  /// constructer
  TStation( const std::string& name,
                 ISvcLocator* pSvcLocator);

  virtual ~TStation();

  // Tool initialization
  virtual StatusCode initialize();

  //  Algorithm execution
  virtual StatusCode execute();

  // Algorithm finalization
  virtual StatusCode finalize();

  /** Returns if the given track is selected or not
   *
   *  @param aTrack Reference to the Track to test
   *
   *  @return boolean indicating if the track is selected or not
   *  @retval true  Track is selected
   *  @retval false Track is rejected
   */


 private:

  std::string m_inputcontainer;
  std::string m_OTTrackLocation;
  std::string m_ITTrackLocation;
  std::string m_XTrackLocation; 
  
  IATrackSelectorTool* m_trackselection;

  LHCb::Tracks* ITTracks;
  LHCb::Tracks* OTTracks;
  LHCb::Tracks* XTracks;
};

#endif // TRACKTOOLS_JBSelector_H
