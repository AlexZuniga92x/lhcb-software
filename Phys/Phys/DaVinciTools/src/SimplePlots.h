// $Id: SimplePlots.h,v 1.1 2005-01-06 10:37:47 pkoppenb Exp $
#ifndef SIMPLEPLOTS_H 
#define SIMPLEPLOTS_H 1

// Include files
// from DaVinci, this is a specialized GaudiAlgorithm
#include "Kernel/DVAlgorithm.h"
#include "Kernel/IPlotTool.h"            // Interface


/** @class SimplePlots SimplePlots.h
 *  
 *  Simple algorithm that plots most common variables
 *  for all particles in the inputlocation depending
 *  on it's PID. One can define the histogram boundaries
 *  for all these variables or take the default ones.
 *  For the mass a value of -1 for both min and max
 *  sets the boundaries to a PID-dependent default.
 *
 *  @author Patrick KOPPENBURG
 *  @date   2004-10-22
 */
class SimplePlots : public DVAlgorithm {
public: 
  /// Standard constructor
  SimplePlots( const std::string& name, ISvcLocator* pSvcLocator );

  virtual ~SimplePlots( ); ///< Destructor

  virtual StatusCode initialize();    ///< Algorithm initialization
  virtual StatusCode execute   ();    ///< Algorithm execution
  virtual StatusCode finalize  ();    ///< Algorithm finalization

protected:

private:
  IPlotTool* m_plots;
  
};
#endif // SIMPLEPLOTS_H
