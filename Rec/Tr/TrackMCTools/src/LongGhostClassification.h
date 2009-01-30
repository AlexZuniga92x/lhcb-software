#ifndef _LongGhostClassification_H
#define _LongGhostClassification_H

#include "TrackGhostClassificationBase.h"


namespace LHCb{
  class Track;
}

class LongGhostClassification: public TrackGhostClassificationBase {

public:

  /// constructer
  LongGhostClassification(const std::string& type,
                               const std::string& name,
                               const IInterface* parent);

  /** destructer */
  virtual ~LongGhostClassification();

  /**
   *  Check this is a ghost .
   *  specialize for long tracks [check velo and T separately]
   *  @param aTrack to link
   *  @return bool true if a ghost
  */
  virtual bool isGhost(LHCbIDs::const_iterator& start, 
                       LHCbIDs::const_iterator& stop) const;

 private:

 virtual void specific(LHCbIDs::const_iterator& start, 
                       LHCbIDs::const_iterator& stop, LHCb::GhostTrackInfo& tinfo) const;


};

#endif
