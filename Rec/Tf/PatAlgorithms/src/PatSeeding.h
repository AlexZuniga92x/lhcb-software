// $Id: PatSeeding.h,v 1.2 2008-05-14 17:22:18 mschille Exp $
#ifndef PATSEEDING_H
#define PATSEEDING_H 1

// Include files
// from Gaudi
#include "GaudiAlg/GaudiAlgorithm.h"

#include "TrackInterfaces/IPatSeedingTool.h"

  /** @class PatSeeding PatSeeding.h
   *
   *  Pat Seeding algorithm.
   *
   *  @author Olivier Callot
   *  @date   2006-10-13 Initial version
   *  @date   2007-08-20 Update for a-team framework 
   *  @author Manuel Schiller
   *  @date   2008-04-16 obtain a private version of PatSeedingTool
   */

  class PatSeeding : public GaudiAlgorithm
  {
  public:
    /// Standard constructor
    PatSeeding( const std::string& name, ISvcLocator* pSvcLocator );

    virtual ~PatSeeding( ); ///< Destructor

    virtual StatusCode initialize();    ///< Algorithm initialization
    virtual StatusCode execute   ();    ///< Algorithm execution
    virtual StatusCode finalize  ();    ///< Algorithm finalization

  private:
    std::string m_outputTracksName;
    IPatSeedingTool* m_seedingTool;

  };

#endif // PATSEEDING_H
