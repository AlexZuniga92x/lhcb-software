// $Id: PatSeedTrack.h,v 1.5 2009-04-22 13:09:00 smenzeme Exp $
#ifndef PATSEEDTRACK_H
#define PATSEEDTRACK_H 1

// Include files

#include <cmath>
#include <algorithm>
#include <functional>

#include <array>

#include "PatKernel/PatForwardHit.h"
#include "TfKernel/RecoFuncs.h"
#include "TfKernel/RegionID.h"
#include "TfKernel/STHit.h"

#include "LHCbMath/BloomFilter.h"

/** @class PatSeedTrack PatSeedTrack.h
 *  This is the working track class during the seeding
 *
 *  @author Olivier Callot
 *  @date   2006-10-17 Initial version
 *  @date   2007-08-20 Updated for A-Team framework 
 *  @date   2008-04-13 Added assignment operator
 *  @date   2008-04-15 Removed unused members
 *  @date   2010-09-12 added convenience functions
 */

class PatSeedTrack {
  private:
    enum {
      kNPlanes = Tf::RegionID::OTIndex::kNLayers * Tf::RegionID::OTIndex::kNStations,
      kNLayers = Tf::RegionID::OTIndex::kNLayers,
      kNStations = Tf::RegionID::OTIndex::kNStations
    };
    typedef std::array<unsigned char, kNPlanes> PlaneArray;

  public:
    /// Standard constructor
    PatSeedTrack( double x0, double x1, double x2, 
	double z0, double z1, double z2, double zRef, double dRatio);

    PatSeedTrack( PatFwdHit* c0, PatFwdHit* c1, PatFwdHit* c2, PatFwdHit* c3,
	double zRef, double dRatio, double arrow );

    PatSeedTrack( const PatSeedTrack& other) { *this = other; }

    virtual ~PatSeedTrack( ); ///< Destructor

    /// get track parameters
    void getParameters( double& z0, double& bx, double &ax, double &cx, double& dx,
			double& ay, double& by) const {
      z0 = m_z0;
      bx = m_bx;
      ax = m_ax;
      cx = m_cx;
      dx = m_dx;
      ay = m_ay;
      by = m_by;
    }

    /// set track parameters
    void setParameters (double z0, double bx, double ax, double cx, double dx,
			double ay, double by) {
      m_z0 = z0;
      m_bx = bx;
      m_ax = ax;
      m_cx = cx;
      m_dx = dx;
      m_ay = ay;
      m_by = by;
      m_cosine =  1. / std::sqrt( 1. +  m_bx * m_bx  );
    }

    double z0() const { return m_z0; } ///< return reference z
    double xAtZEqZ0() const { return m_ax; } ///< return x at reference z

    double xAtZ( double z ) const ///< return x at given z
    {
      const double dz = z - m_z0;
      return m_ax + dz * ( m_bx + dz * ( m_cx + dz * m_dx ));
    }

    double xSlope( double z ) const ///< return slope in x at given z
    {
      const double dz = z - m_z0;
      return m_bx + dz * ( 2. * m_cx + 3. * dz * m_dx );
    }

    double yAtZ( double z ) const ///< return y at given z
    { return m_ay + m_by * z; }

    double ySlope( double ) const ///< return slope in y at given z
    { return m_by; }

    double curvature() const ///< return curvature
    { return m_cx; }

    /// const iterator to first coordinate(hit)
    PatFwdHits::const_iterator coordBegin() const { return m_coords.begin(); }
    /// const iterator to end of coordinates(hits)
    PatFwdHits::const_iterator coordEnd()   const { return m_coords.end(); }
    /// iterator to first coordinate(hit)
    PatFwdHits::iterator coordBegin()             { return m_coords.begin(); }
    /// iterator to end of coordinates(hits)
    PatFwdHits::iterator coordEnd()               { return m_coords.end(); }
    /// const reference to coordinate(hit) container
    const PatFwdHits& coords() const		  { return m_coords; }

    unsigned nCoords() const ///< return number of hits on the track
    { return m_coords.size(); }

    unsigned nPlanes() const ///< return number of planes on the track
    { return m_nbPlanes; }

    unsigned nXPlanes() const; ///< return number of X planes on the track
    unsigned nStPlanes() const; ///< return number of stereo planes on the track

    double cosine() const ///< return cosine of track angle in xz projection
    { return m_cosine; }

    /// return number of holes (planes without hit)
    inline unsigned nHoles() const;

    /// return minimum number of planes per station of all stations
    inline unsigned minPlanesPerStation(unsigned* minSta = 0) const;

    ///< weighted number of hits (hits in clusters count twice)
    inline unsigned nbOnSide() const;

    /// return number of hits in monolayer 1 minus monolayer2
    inline int otMonoAsym() const;

    ///< number of hits above ST high threshold
    inline unsigned nbHighThreshold() const;

    double chi2() const ///< track chi^2/ndf
    { return m_chi2; }

    void setChi2( double chi2 ) ///< set track chi^2/ndf
    { m_chi2 = chi2; }

    void addCoord( PatFwdHit* hit ) ///< add a hit
    {
      m_coords.push_back( hit );
      if ( 0 == m_planeList[hit->planeCode()]++ ) ++m_nbPlanes;
    }

    void removeCoord( PatFwdHits::iterator& worst ) ///< remove a hit
    {
      if ( 0 == --m_planeList[(*worst)->planeCode()] ) --m_nbPlanes;
      worst = m_coords.erase( worst ) - 1;
    }

    /// perpendicular distance for fit
    double distanceForFit( const PatFwdHit* hit ) const
    {
      double dist = hit->x() - xAtZ( hit->z() );
      if ( hit->hit()->type() == Tf::RegionID::IT ) return dist;
      const double dx = hit->driftDistance() / m_cosine;
      if ( fabs( dist - dx ) < fabs( dist + dx ) )
	return dist - dx;
      else
	return dist + dx;
    }

    /// distance track to hit with ambiguity fixed
    double distanceWithRL( const PatFwdHit* hit ) const
    {
      if ( 0 == hit->rlAmb() || hit->hit()->type() == Tf::RegionID::IT )
	return distanceForFit( hit );
      const double dist = ( hit->x() - xAtZ( hit->z() ) );
      const double dx = hit->driftDistance() / m_cosine;
      if ( 0 < hit->rlAmb() )
	return dist + dx;
      else
	return dist - dx;
    }

    /// distance track to hit
    double distance( const PatFwdHit* hit ) const
    {
      double dist = hit->x() - xAtZ( hit->z() );
      if ( hit->hit()->type() == Tf::RegionID::IT ) return dist;
      dist *= m_cosine;
      const double dx = hit->driftDistance();
      if ( fabs( dist - dx ) < fabs( dist + dx ) )
	return dist - dx;
      else
	return dist + dx;
    }

    /// chi^2 contribution of a hit
    double chi2Hit( const PatFwdHit* hit) const
    {
      const double dist = distance( hit );
      return dist * dist * hit->hit()->weight();
    }

    void setYParams( double y0, double sl ) ///< set track parameters in y
    { m_ay = y0; m_by = sl; updateHits( ); }

    /// update hit positions
    inline void updateHits();

    /// sort hits on track by increasing z
    inline void sort();

    void setValid( bool flag ) ///< set if a track is valid
    { m_valid = flag; }
    
    bool valid() const ///< return if a track is valid
    { return m_valid; }

    /// update track parameters in y
    void updateYParameters( double day, double dby  )
    {
      m_ay += day;
      m_by += dby;
      if ( 0 < m_coords.size() )
	m_z0 += day * (*m_coords.begin())->hit()->dzDy();
      updateHits();
    }

    /// update track parameters in x
    void updateParameters( double dax, double dbx, double dcx, double ddx = 0. )
    {
      m_ax += dax;
      m_bx += dbx;
      m_cx += dcx;
      m_dx += ddx;
      m_cosine =  1. / std::sqrt( 1. +  m_bx * m_bx  );
    }

    /// return length of track (start layer - end layer + 1)
    unsigned length() const
    {
      unsigned start = 0, end = kNPlanes;
      while (end > start && !m_planeList[end - 1]) --end;
      while (end > start && !m_planeList[start]) ++start;
      return end - start;
    }

    /// return number of stations hit
    unsigned nStations() const
    {
      return
	((m_planeList[0] || m_planeList[1] || m_planeList[2] || m_planeList[3]) ? 1 : 0) +
	((m_planeList[4] || m_planeList[5] || m_planeList[6] || m_planeList[7]) ? 1 : 0) +
	((m_planeList[8] || m_planeList[9] || m_planeList[10] || m_planeList[11]) ? 1 : 0);
    }

    /// type to represent IT only/OT only/ITOT overlap track
    typedef enum { OT = 1, IT = 2, ITOT = 3 } TrackRegion;
    /// return track region (IT only, OT only, ITOT overlap)
    TrackRegion trackRegion() const
    {
      unsigned trreg = 0;
      for(const PatFwdHit* hit: m_coords) {
	LHCb::LHCbID id(hit->hit()->lhcbID());
	if (id.isOT()) trreg |= OT;
	else trreg |= IT;
      }
      return static_cast<TrackRegion>(trreg);
    }

    /** predicate to sort tracks by decreasing quality
     * longer tracks (i.e. more hits) are better that shorter ones
     * for tracks with same number of hits, lower chi^2 wins
     */
    struct decreasingQuality {
      public:
	bool operator() (const PatSeedTrack* first, const PatSeedTrack* second ) const {
	  if ( first->nCoords() > second->nCoords() ) return true;
	  if ( first->nCoords() < second->nCoords() ) return false;
	  return first->chi2() < second->chi2();
	};
	bool operator() (const PatSeedTrack& first, const PatSeedTrack& second ) const {
	  if ( first.nCoords() > second.nCoords() ) return true;
	  if ( first.nCoords() < second.nCoords() ) return false;
	  return first.chi2() < second.chi2();
	};
    };

    /// assignment operator
    const PatSeedTrack& operator=(const PatSeedTrack& other)
    {
      m_valid = other.m_valid;
      m_nbPlanes = other.m_nbPlanes;
      m_z0 = other.m_z0;
      m_bx = other.m_bx;
      m_ax = other.m_ax;
      m_cx = other.m_cx;
      m_dx = other.m_dx;
      m_ay = other.m_ay;
      m_by = other.m_by;
      m_cosine = other.m_cosine;
      m_chi2 = other.m_chi2;
      m_planeList = other.m_planeList;
      m_coords = other.m_coords;
      return *this;
    }

    void updateIDs()
    {
      m_ids.clear();
      for (const PatFwdHit* hit : m_coords) {
	m_ids.insert(hit->hit()->lhcbID());
      }
    }
    const BloomFilter<LHCb::LHCbID, 12, 81789, 1 << 20>& bloomfilter() const
    { return m_ids; }

  protected:


  private:
    bool   m_valid;
    unsigned    m_nbPlanes;

    double m_z0;
    double m_bx;
    double m_ax;
    double m_cx;
    double m_dx;

    double m_ay;
    double m_by;

    double m_cosine;
    double m_chi2;

    PlaneArray m_planeList;
    PatFwdHits m_coords;
    BloomFilter<LHCb::LHCbID, 12, 81789, 1 << 20> m_ids;

    struct countIfHighThreshold : /// helper predicate for nbHighThreshold
      public std::unary_function<const PatFwdHit*,bool>
  {
    bool operator() (const PatFwdHit* hit) const
    { const Tf::STHit* itHit = hit->hit()->sthit();
      return itHit && itHit->cluster().highThreshold(); }
  };

};

inline int PatSeedTrack::otMonoAsym() const
{ 
  int otMonoAsym = 0;
  for(const PatFwdHit* hit: m_coords) {
    LHCb::LHCbID id(hit->hit()->lhcbID());
    if (!id.isOT()) continue;
    LHCb::OTChannelID otid = id.otID();
    if (9 == otid.module() &&
	(0 == otid.quarter() || 2 == otid.quarter())) {
      if (otid.straw() > 32) ++otMonoAsym;
      else --otMonoAsym;
    } else {
      if (otid.straw() > 64) ++otMonoAsym;
      else --otMonoAsym;
    }
  }
  return otMonoAsym;
}

inline unsigned PatSeedTrack::nHoles() const
{
  unsigned nHoles = 0, i = 0, j = kNPlanes;
  // find first and last plane with hit
  while (!m_planeList[i] && i < kNPlanes) ++i;
  while (!m_planeList[--j] && j > 0);
  for ( ; i <= j; ++i)
    if (!m_planeList[i]) ++nHoles;
  return nHoles;
}

inline unsigned PatSeedTrack::nXPlanes() const
{
  unsigned retVal = 0;
  for (unsigned i = kNPlanes; i--; )
    if (0 == (i & 3) || 3 == (i & 3))
      if (m_planeList[i]) ++retVal;
  return retVal;
}

inline unsigned PatSeedTrack::nStPlanes() const
{
  unsigned retVal = 0;
  for (unsigned i = kNPlanes; i--; )
    if (1 == (i & 3) || 2 == (i & 3))
      if (m_planeList[i]) ++retVal;
  return retVal;
}

inline unsigned PatSeedTrack::minPlanesPerStation(unsigned* minSta) const
{
  std::array<unsigned, kNStations> pps = { { 0, 0, 0 } };
  for (unsigned i = kNPlanes; i--; )
    if (m_planeList[i]) ++pps[i / kNLayers];
  unsigned minPlanes = pps[0], minsta = 0;
  for (unsigned i = kNStations; --i; )
    if (0 == minPlanes || minPlanes > pps[i])
      minPlanes = pps[i], minsta = i;
  if (minSta) *minSta = minsta;
  return minPlanes;
}

inline unsigned PatSeedTrack::nbOnSide() const
{ 
  unsigned nb = 0;
  for(const PatFwdHit* hit: m_coords) {
    if (hit->hasNext()) ++nb;
    if (hit->hasPrevious()) ++nb;
  }
  return nb;
}

inline unsigned PatSeedTrack::nbHighThreshold() const
{
  return std::count_if( m_coords.begin(), m_coords.end(),
      countIfHighThreshold() );
}

inline void PatSeedTrack::updateHits()
{
  for( PatFwdHit* hit: m_coords )
    updateHitForTrack(hit, m_ay, m_by);
}

inline void PatSeedTrack::sort()
{
  std::sort( m_coords.begin(), m_coords.end(),
      Tf::increasingByZ<PatForwardHit>() );
}

#endif // PATSEEDTRACK_H
// vim:shiftwidth=2:tw=78
