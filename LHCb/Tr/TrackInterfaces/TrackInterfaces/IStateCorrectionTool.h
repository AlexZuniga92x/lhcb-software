// $Id: IStateCorrectionTool.h,v 1.1 2006-08-22 10:40:21 erodrigu Exp $
#ifndef TRACKINTERFACES_ISTATECORRECTIONTOOL_H 
#define TRACKINTERFACES_ISTATECORRECTIONTOOL_H 1

// Include files
// -------------
// from Gaudi
#include "GaudiKernel/IAlgTool.h"

// Forward declarations
namespace LHCb {
  class State;
};

static const InterfaceID IID_IStateCorrectionTool ( "IStateCorrectionTool", 1, 0 );

/** @class IStateCorrectionTool TrackExtrapolators/IStateCorrectionTool.h
 *  
 *  Interface for state correction tools
 *
 *  @author Eduardo Rodrigues
 *  @date   2006-08-18
 */
class IStateCorrectionTool : virtual public IAlgTool {
public: 
  
  /// Correct a State
  virtual void correctState( LHCb::State& state,
                             const Material* material,
                             double wallThickness = 0,
                             bool upstream = true ) = 0;
  
  // Return the interface ID
  static const InterfaceID& interfaceID()
  { return IID_IStateCorrectionTool; }
  
};
#endif // TRACKINTERFACES_ISTATECORRECTIONTOOL_H
