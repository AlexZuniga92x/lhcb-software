#ifndef IRADDAMAGETOOL_H 
#define IRADDAMAGETOOL_H 1

// Include files

// from Gaudi
#include "GaudiKernel/IAlgTool.h"

namespace LHCb{
  class MCHit;
}

/** @class IRadDamageTool IRadDamageTool.h
 *  Interface class for reduction in effective charge with position
 *  initially for VELO radiation damage simulation
 *
 *  @author David Hutchcroft
 *  @date   2011-3-31
 */

// Declaration of  the interface ID
static const InterfaceID IID_IRadDamageTool( "IRadDamageTool", 1, 0 );

class IRadDamageTool : virtual public IAlgTool {
public:
  /// Static access to interface id
  static const InterfaceID& interfaceID() { return IID_IRadDamageTool; }

  /** fraction of charge seen due to radiation damage
  * @param point : position (global frame) of hit 
  * @return fraction of charge after rad damage
  */
  virtual double chargeFrac(const LHCb::MCHit &hit) const = 0;

};
#endif // IRADDAMAGETOOL_H 
