
#ifndef RICHMARKOV_HIT_H
#define RICHMARKOV_HIT_H

#include "CLHEP/Vector/TwoVector.h"
#include <list>
#include <vector>
#include <cassert>

// From RICH software
class RichRecPixel;

namespace RichMarkov {

  class Hit : public Hep2Vector {
    
  public:
    
    typedef std::list<double> CacheList;
    typedef std::vector<double> CircleProbabilityCache;
    
    /// Default constructor
    Hit ( ) : m_richPixel ( 0 ) {
      // Note that contents of Hep2Vector are undefined!
      // Note that contents of m_probabilityContributionCache are undefined!
    }
    
    /// Constructor from hit coordinates and pointer to parent RichRecPixel
    Hit ( const double x,
          const double y,
          RichRecPixel * pixel = 0 ) :
      Hep2Vector  ( x, y  ),
      m_richPixel ( pixel ) {
      // Note that contents of m_probabilityContributionCache are undefined!
    };
    
    
    /// Return corresponding RichRecPixel (non-const)
    RichRecPixel * richPixel() { return m_richPixel; }
    /// Return corresponding RichRecPixel (const)
    const RichRecPixel * richPixel() const { return m_richPixel; }


    //inline const double getCacheBack() const {
    //  assert(!(m_cacheList.empty()));
    //  return m_cacheList.back();
    //}

    inline const double getCacheFront() const {
      assert(!(m_cacheList.empty()));
      return m_cacheList.front();
    }

    inline void pushCacheBack(const double toBeStored) const {
      assert(m_cacheList.size()<2);
      m_cacheList.push_back(toBeStored);
    }

    inline void popCacheFront() const {
      assert(!(m_cacheList.empty()));
      m_cacheList.pop_front();
    }

    inline void popCacheBack() const {
      assert(!(m_cacheList.empty()));
      m_cacheList.pop_back();
    }

    inline unsigned int cacheSize() const {
      return m_cacheList.size();
    }

    inline CircleProbabilityCache & proposedCircleProbabilityCache() const {
      if (!currentIsA) {
        return m_circleProbabilityCacheA;
      } else {
        return m_circleProbabilityCacheB;
      }
    }
    inline CircleProbabilityCache & currentCircleProbabilityCache() const {
      if (currentIsA) {
        return m_circleProbabilityCacheA;
      } else {
        return m_circleProbabilityCacheB;
      }
    }
    inline void abandonProposedCircleProbabilityCache() const {
      // reset ready for next attempt
      //proposedCircleProbabilityCache() = currentCircleProbabilityCache();
    }
    inline void confirmProposedCircleProbabilityCache() const {
      m_circleProbabilityCacheA.swap(m_circleProbabilityCacheB);
    }

  private:
    /// RichRecPixel to which this hit corresponds
    RichRecPixel * m_richPixel; // was const

    mutable CacheList m_cacheList;
    mutable CircleProbabilityCache m_circleProbabilityCacheA;
    mutable CircleProbabilityCache m_circleProbabilityCacheB;
    mutable bool currentIsA; // doesn't matter how this is initialised!

  }; // end class

} // end namespace

#endif
