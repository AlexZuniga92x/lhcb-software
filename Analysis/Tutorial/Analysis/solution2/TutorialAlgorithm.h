// $Id: TutorialAlgorithm.h,v 1.4 2005-11-07 15:30:45 pkoppenb Exp $
#ifndef TutorialAlgorithm_H 
#define TutorialAlgorithm_H 1

// Include files
// from DaVinci, this is a specialized GaudiAlgorithm
#include "DaVinciTools/DVAlgorithm.h"


/** @class TutorialAlgorithm TutorialAlgorithm.h
 *
 *  Solution for exercise 2 
 *
 *  @author Patrick KOPPENBURG
 *  @date   2004-11-04
 */
class TutorialAlgorithm : public DVAlgorithm {
public: 
  /// Standard constructor
  TutorialAlgorithm( const std::string& name, ISvcLocator* pSvcLocator );

  virtual ~TutorialAlgorithm( ); ///< Destructor

  virtual StatusCode initialize();    ///< Algorithm initialization
  virtual StatusCode execute   ();    ///< Algorithm execution
  virtual StatusCode finalize  ();    ///< Algorithm finalization

protected:

private:
  double m_massWin ; ///< Mass window
  double m_chi2 ;    ///< Max chi^2
  std::string m_name ; ///< Name of particle
  int m_iD ;         ///< ID
  double m_mass ;    ///< mass
  int m_nFound ;     ///< number of selected particles
  int m_nEvents ;    ///< number of events
};
#endif // TutorialAlgorithm_H
