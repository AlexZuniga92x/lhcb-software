// $Id: ConjugateNeutralPID.h,v 1.1 2006-12-18 11:17:00 jpalac Exp $
#ifndef CONJUGATENEUTRALPID_H 
#define CONJUGATENEUTRALPID_H 1

// Include files
// from DaVinci, this is a specialized GaudiAlgorithm
#include "Kernel/DVAlgorithm.h"
#include "Kernel/IChangePIDTool.h"


/** @class ConjugateNeutralPID ConjugateNeutralPID.h
 * 
 * Very simple algorithm that takes all particles on the desktop
 * and *CLONES* them with the conjugate PID.
 *
 * This algorithm should only be applied to Particles with PIDs that are
 * charge neutral and not self-conjugate.
 *
 * It is just an algorithm wrapper for the tool NeutralCCChangePIDTool that
 * performs the cloning and renaming.
 * 
 *  @author Patrick Spradlin
 *  @date   2006-12-15
 */
class ConjugateNeutralPID : public DVAlgorithm {
public: 
  /// Standard constructor
  ConjugateNeutralPID( const std::string& name, ISvcLocator* pSvcLocator );

  virtual ~ConjugateNeutralPID( ); ///< Destructor

  virtual StatusCode initialize();    ///< Algorithm initialization
  virtual StatusCode execute   ();    ///< Algorithm execution
  virtual StatusCode finalize  ();    ///< Algorithm finalization

protected:

  IChangePIDTool *m_changePIDTool;      ///< Tool that performs most of the work

private:

  int             m_nEvents;            ///< Number of events
  int             m_nAccepted;          ///< Number of events accepted
  int             m_nCandidates;        ///< Number of candidates

};
#endif // CONJUGATENEUTRALPID_H
