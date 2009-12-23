// $Id: HltVertexToTracks.h,v 1.4 2009-12-23 16:14:31 graven Exp $
#ifndef HLTCOMMON_HLTVERTEXTOTRACKS_H 
#define HLTCOMMON_HLTVERTEXTOTRACKS_H 1

// Include files
// from Gaudi
#include "HltBase/HltSelectionContainer.h"
#include "HltBase/HltAlgorithm.h"

/** @class L0Vertex2Tracks L0Vertex2Tracks.h
 *  
 *
 *  @author Alessia Satta
 *  @date   2007-08-03
 */
class HltVertexToTracks : public HltAlgorithm {
public: 
  /// Standard constructor
  HltVertexToTracks( const std::string& name, ISvcLocator* pSvcLocator );

  virtual ~HltVertexToTracks( ); ///< Destructor

  virtual StatusCode initialize();    ///< Algorithm initialization
  virtual StatusCode execute   ();    ///< Algorithm execution

private:
  Hlt::SelectionContainer2<LHCb::Track,LHCb::RecVertex> m_selections;

};
#endif // L0VERTEX2TRACKS_H
