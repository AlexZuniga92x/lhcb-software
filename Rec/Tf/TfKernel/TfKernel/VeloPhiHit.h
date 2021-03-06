// $Id: VeloPhiHit.h,v 1.9 2010-04-07 05:17:03 wouter Exp $
#ifndef INCLUDE_VELOPHIHIT_H
#define INCLUDE_VELOPHIHIT_H 1

#include "GaudiKernel/Range.h"
#include "GaudiKernel/PhysicalConstants.h"
#include "TfKernel/VeloHit.h"

#include "VeloDet/DeVeloPhiType.h"

namespace LHCb {
  class VeloLiteCluster;
}

namespace Tf {

  /** @class VeloPhiHit VeloPhiHit.h
   *  Representation of VELO phi measurement
   *
   * @authors S. Hansmann-Menzemer, W. Hulsbergen, C. Jones, K. Rinnert
   * @date   2007-06-15
   */

  class VeloPhiHit : public VeloHit {

  public:

    //----------------------------------------------------------------------
    // typedefs
    //----------------------------------------------------------------------

    typedef std::vector<VeloPhiHit* > container_type;
    typedef Gaudi::Range_<container_type> range_type;

    typedef velo_phihit_tag hit_type_tag; ///< the hit type tag

  public:

    /// Standard Constructor
    inline VeloPhiHit(const DeVeloPhiType* sensor, const LHCb::VeloLiteCluster& clu, double signal);

    //== simple accessors
    double phi()                           const noexcept { return m_coord; }
    double phiHalfBox()                    const noexcept { return m_coordHalfBox; }
    double phiIdeal()                      const noexcept { return m_coordIdeal; }
    double z()                             const noexcept { return m_sensor->z(); }
    const DeVeloPhiType* sensor()          const noexcept { return m_sensor; }
    //== access to coords used for sorting and searching (mapped to [pi/2,3/2pi] on the right side)
    double sortCoord()                     const noexcept { return m_sortCoord; }
    double sortCoordHalfBox()              const noexcept { return m_sortCoordHalfBox; }
    double sortCoordIdeal()                const noexcept { return m_sortCoordIdeal; }

  private:

    const DeVeloPhiType* m_sensor;        ///< link to detector element

    double m_sortCoord;        ///< phi mapping for sorting in global frame, [pi/2,3/2pi] on the right side 
    double m_sortCoordHalfBox; ///< phi mapping for sorting in half box frame, [pi/2,3/2pi] on the right side           
    double m_sortCoordIdeal;   ///< phi mapping for sorting in ideal frame, [pi/2,3/2pi] on the right side 
  };
  

  //----------------------------------------------------------------------
  // shortcut typedefs
  //----------------------------------------------------------------------

  typedef VeloPhiHit::container_type VeloPhiHits;
  typedef VeloPhiHit::range_type VeloPhiHitRange;

  //----------------------------------------------------------------------
  // inline implementations
  //----------------------------------------------------------------------

  // constructor
  inline VeloPhiHit::VeloPhiHit(const DeVeloPhiType* s, const LHCb::VeloLiteCluster& c, double signal)
    : VeloHit(c
              , RegionID(c.channelID(),s)
              , s->globalPhi(c.channelID().strip(),c.interStripFraction())
              , s->halfboxPhi(c.channelID().strip(),c.interStripFraction())
              , s->idealPhi(c.channelID().strip(),c.interStripFraction())
              , s->phiPitch( c.channelID().strip() ) / std::sqrt(12.0)
              , signal
              )
    , m_sensor(s)
    , m_sortCoord(s->isRight() && m_coord<0 ? m_coord+2.0*Gaudi::Units::pi : m_coord)
    , m_sortCoordHalfBox(s->isRight() && m_coordHalfBox<0 ? m_coordHalfBox+2.0*Gaudi::Units::pi : m_coordHalfBox)
    , m_sortCoordIdeal(s->isRight() && m_coordIdeal<0 ? m_coordIdeal+2.0*Gaudi::Units::pi : m_coordIdeal)
  {
    ;
  }

  // our dynamic casts
  inline const VeloPhiHit* HitBase::veloPhiHit() const noexcept
  {
    return type()==RegionID::VeloPhi ? static_cast<const VeloPhiHit*>(this) : nullptr;
  }
}

#endif // INCLUDE_VELOPHIHIT_H

