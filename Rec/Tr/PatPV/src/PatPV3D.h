#ifndef PATPV_PATPV3D_H 
#define PATPV_PATPV3D_H 1

// Include files
// -------------
// From Gaudi
#include "GaudiAlg/GaudiAlgorithm.h"
// Interfaces
#include "TrackInterfaces/IPVOfflineTool.h"

// From Event
#include "Event/Track.h"
#include "Event/RecVertex.h"

/** @class PatPV3D PatPV3D.h
 *  Algorithm to find the primary vertices at the HLT.
 *
 *  @author Eduardo Rodrigues
 *  @date   2004-02-17
 */

//-----------------------------------------------------------------------------

class PatPV3D : public GaudiAlgorithm {
public:
  /// Standard constructor
  PatPV3D( const std::string& name, ISvcLocator* pSvcLocator );

  virtual ~PatPV3D( );    ///< Destructor

  virtual StatusCode initialize();    ///< Algorithm initialization
  virtual StatusCode execute   ();    ///< Algorithm execution
  virtual StatusCode finalize  ();    ///< Algorithm finalization

protected:

protected:
 
private: 
    
  LHCb::RecVertices* m_outputVertices; ///< Pointer to the 3D vertices

  std::string m_outputVerticesName;  

  IPVOfflineTool* m_pvsfit;    // PV fitting tool

};
#endif // PATPV_PATPV3D_H
