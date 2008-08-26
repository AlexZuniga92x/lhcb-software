// $Id: PatSeedingTool.h,v 1.13 2008-08-26 09:57:02 mschille Exp $
#ifndef PATSEEDINGTOOL_H
#define PATSEEDINGTOOL_H 1

// Include files
#include <cmath>
#include <string>
#include <iostream>
#include <iomanip>
#include <algorithm>

// from Gaudi
#include "GaudiAlg/GaudiTool.h"
#include "GaudiAlg/ISequencerTimerTool.h"

#include "TrackInterfaces/IPatSeedingTool.h"
#include "TrackInterfaces/ITracksFromTrack.h"
#include "TrackInterfaces/ITrackMomentumEstimate.h"
#include "Kernel/ILHCbMagnetSvc.h"

#include "Event/Track.h"

#include "TfKernel/RegionID.h"
#include "TfKernel/RecoFuncs.h"
#include "TfKernel/TStationHitManager.h"

#include "PatKernel/PatForwardHit.h"
#include "PatSeedTool.h"
#include "PatSeedTrack.h"
#include "PatRange.h"

namespace LHCb {
  class State;
}

static const InterfaceID IID_PatSeedingTool ( "PatSeedingTool", 1, 0 );


/** @class PatSeedingTool PatSeedingTool.h
 *
 *  Pat Seeding tool. Used internally by Pat Seeding and for L0
 *  confirmation in the HLT.
 *
 *  @author Olivier Callot
 *  @date   2006-10-13 Initial version
 *  @date   2007-08-20 Update for a-team framework 
 *  @date   2008-04-15 Major code refactoring, new pass over low quality
 *          tracks, bug fixes
 *  @date   2008-05-03 added methods to cache region z at y = 0, dz/dy
 *  @date   2008-05-04 added convenience methods to access hit manager
 *  @date   2008-06-04 fixes to avoid dividing by zero in initialize,
 *          new passes to find more tracks in IT and OT (ideas due to Matt
 *          Needham and Olivier Callot, respectively - thanks a lot!)
 */

class PatSeedingTool : public GaudiTool,  virtual public IPatSeedingTool,
  virtual public ITracksFromTrack
{
  public:
    typedef Tf::TStationHitManager<PatForwardHit>::HitRange HitRange;

    // Return the interface ID
    static const InterfaceID& interfaceID() { return IID_PatSeedingTool; }

    /// Standard constructor
    PatSeedingTool( const std::string& type,
	const std::string& name,
	const IInterface* parent);

    /// destructor
    virtual ~PatSeedingTool( );

    /// initialize the tool for subsequent use
    virtual StatusCode initialize();
    /// finalize the tool
    virtual StatusCode finalize();

    /** search for tracks in T stations using state from given tracks as seed,
     * thus restricting track search to tiny region of interest
     * @param seed track from which state closest to T will be taken as seed
     * @param out container for output
     * @return StatusCode 
     */
    virtual StatusCode tracksFromTrack(const LHCb::Track& seed,
	std::vector<LHCb::Track*, std::allocator<LHCb::Track*> >& out);

    /** search for tracks in T stations
     * @param output container for tracks found
     * @param state optional parameter: state to restrict region of interest
     * @return StatusCode
     */
    virtual StatusCode performTracking (LHCb::Tracks* output,
	const LHCb::State* state = 0);

    /** search for tracks in T stations
     * @param output container for tracks found
     * @param state optional parameter: state to restrict region of interest
     * @return StatusCode
     */
    virtual StatusCode performTracking (std::vector<LHCb::Track*>& outputTracks,
	const LHCb::State* state = 0);

    /** prepare hits in IT/OT for use with the tool
     * @return number of hits in IT and OT
     */
    virtual unsigned prepareHits();

  protected:
    /// helper for debugging printouts
    void debugFwdHit ( const PatFwdHit* coord, MsgStream& msg ) const;

    /// add a track candidate to the pool if it is better than others in there
    void addIfBetter( PatSeedTrack& track,
	std::vector<PatSeedTrack>& pool) const;

    /// find candidates in xz projection in region reg, layer lay
    void findXCandidates ( unsigned lay, unsigned reg,
	std::vector<PatSeedTrack>& pool,
	const LHCb::State* state );

    /// collect stereo hits compatible with track candidate in xz projection
    void collectStereoHits ( PatSeedTrack& track, unsigned typ,
	PatFwdHits& stereo, const LHCb::State* state );

    /// search for cluster of stereo hits in projection plane
    bool findBestRange ( unsigned reg, int minNbPlanes, PatFwdHits& stereo ) const;
    /// search for cluster of stereo hits in projection plane (for cosmics only)
    bool findBestRangeCosmics ( unsigned reg, int minNbPlanes, PatFwdHits& stereo ) const;

    /// fit a line to the hits found by findBestRange
    bool fitLineInY ( PatFwdHits& stereo, double& y0, double& sl ) const;

    /// convert a seed track to an LHCb track, estimating momentum
    void storeTrack( const PatSeedTrack& track,
	std::vector<LHCb::Track*>& outputTracks ) const;

    void processForwardTracks( const std::string& location,
	std::vector<LHCb::Track*>& outputTracks ) const;

    /// check if the hit pointed to by it is isolated in the range range
    bool isIsolated(HitRange::const_iterator it, const HitRange& range) const;

    /// first stage: collect tracks per region
    void collectPerRegion(
	std::vector<PatSeedTrack>& pool,
	std::vector<PatSeedTrack>& lowQualTracks,
	std::vector<PatSeedTrack*>& finalSelection,
	std::vector<LHCb::Track*>& outputTracks,
	const LHCb::State* state,
	bool OTonly = false);

    /// second stage: make tracks in one IT station, collect more OT/IT hits
    void collectITOT(
	std::vector<PatSeedTrack>& pool,
	std::vector<PatSeedTrack*>& finalSelection,
	std::vector<LHCb::Track*>& outputTracks,
	const LHCb::State* state,
	bool doOverlaps = false);

    /// third stage: pick good ones among low quality tracks from first stage
    void collectLowQualTracks(
	std::vector<PatSeedTrack>& lowQualTracks,
	std::vector<PatSeedTrack*>& finalSelection,
	std::vector<LHCb::Track*>& outputTracks,
	const LHCb::State* state);

    /** sorts tracks by quality (many hits first, low chi^2 preferred) and
     *  kills tracks with too many common hits from input container
     *  surviving tracks are added to output container
     */
    void killClonesAndStore(
	std::vector<PatSeedTrack*>& finalSelection,
	std::vector<LHCb::Track*>& outputTracks,
	double maxUsedFraction) const;

  private:
    ILHCbMagnetSvc*  m_magFieldSvc;
    std::string      m_inputTracksName;

    double m_zMagnet;
    double m_xMagTol;
    double m_tolCollectOT;
    double m_tolCollectIT;
    double m_initialArrow;
    double m_zReference;
    std::vector<double> m_zOutputs;
    double m_dRatio;
    double m_tolExtrapolate;

    int    m_minXPlanes;
    double m_curveTol;
    double m_commonXFraction;
    double m_maxRangeOT;
    double m_maxRangeIT;
    double m_maxChi2HitOT;
    double m_maxChi2HitIT;
    int m_minTotalPlanes;
    double m_maxTrackChi2;
    double m_maxFinalChi2;
    double m_maxFinalTrackChi2;
    double m_maxYAtOrigin;
    double m_maxYAtOriginLowQual;
    double m_yCorrection;

    double m_stateErrorX2;
    double m_stateErrorY2;
    double m_stateErrorTX2;
    double m_stateErrorTY2;
    double m_momentumScale;
    double m_minMomentum;

    double m_tolCollect;
    double m_maxChi2Hit;
    double m_zForYMatch;

    Tf::TStationHitManager <PatForwardHit> *  m_tHitManager;    ///< Tool to provide hits

    PatSeedTool*       m_seedTool;

    double m_wireVelocity;
    double m_driftVelocity;
    double m_maxImpact;

    bool m_measureTime;
    ISequencerTimerTool* m_timer;
    int m_timeInit;
    int m_timeReuseTracks;
    int m_timePerRegion;
    int m_timeX;
    int m_timeStereo;
    int m_timeItOt;
    int m_timeLowQual;
    bool m_useForward;
    bool m_useForwardTracks;

    bool m_printing;

    // turns on some tuning for B field off
    bool m_fieldOff;

    // maximum distance of two tracks to be considered for pattern reco
    // clone killing
    double m_cloneMaxXDist;

    // chi^2 cutoff for tracks with few OT hits
    double m_maxTrackChi2LowMult;

    // weighting for quality variable:
    // first element is for number of coordinates
    // second element is for chi^2
    std::vector<double> m_qualityWeights;

    // maximum number of missed layers during x search
    int m_maxMisses;

    // shall we enforce isolation criteria?
    bool m_enforceIsolation;
    // hits/clusters must be isolated in a window around their mean position
    // (window extends for m_[IO]TIsolation in both directions)
    double m_OTIsolation;
    double m_ITIsolation;
    // true (default) if tracks found by a previous instance of PatSeedingTool
    // should be reused by this instance
    bool m_reusePatSeeding;

    // use tool for momentum parametrisation
    std::string m_momentumToolName;
    ITrackMomentumEstimate *m_momentumTool;

    // cuts on fraction of used hits during final selection (per stage)
    double m_maxUsedFractPerRegion;
    double m_maxUsedFractITOT;
    double m_maxUsedFractLowQual;

    // cut on drift radius
    std::vector<double> m_driftRadiusRange;
    // threshold below which an OT track is considered to have few hits, thus
    // having to satisfy stricter criteria
    unsigned m_otNHitsLowThresh;

    // clone killing among forward tracks
    double m_forwardCloneMaxXDist;
    double m_forwardCloneMaxYDist;
    double m_forwardCloneMaxTXDist;
    double m_forwardCloneMaxShared;
    bool m_forwardCloneMergeSeg;

    bool m_cosmics;

    static const unsigned int m_nSta = Tf::RegionID::OTIndex::kNStations;
    static const unsigned int m_nLay = Tf::RegionID::OTIndex::kNLayers;
    static const unsigned int m_nReg = Tf::RegionID::OTIndex::kNRegions +
      Tf::RegionID::ITIndex::kNRegions;
    static const unsigned int m_nOTReg = Tf::RegionID::OTIndex::kNRegions;
    static const unsigned int m_nITReg = Tf::RegionID::ITIndex::kNRegions;

    // constants roughly defining the extension of the inefficient area in
    // IT and OT due to gap between ladders/division of OT modules into
    // lower and upper half
    static const int m_centralYOT = 50; // mm
    static const int m_centralYIT = 4; // mm

    //----------------------------------------------------------------------
    // convenience functions
    // (think of these as the vocabulary needed to talk about the algorithm
    // in a convenient way)
    //----------------------------------------------------------------------
    /// true if region reg is OT region
    inline bool isRegionOT(const unsigned reg) const
    { return reg < Tf::RegionID::OTIndex::kNRegions; }
    /// true if region reg is IT region
    inline bool isRegionIT(const unsigned reg) const
    { return reg >= Tf::RegionID::OTIndex::kNRegions; }

    /// provide convenient access to regions via hit manager
    inline const Tf::EnvelopeBase* region(const int sta, const int lay,
	const int reg) const
    { return m_tHitManager->region(sta, lay, reg); }

    /// provide convenient access to hits via hit manager
    inline HitRange hits() const { return m_tHitManager->hits(); }

    /// provide convenient access to hits via hit manager
    inline HitRange hits(const int sta, const int lay, const int reg) const
    { return m_tHitManager->hits(sta, lay, reg); }

    /// provide convenient access to hits with minimum x via hit manager
    inline HitRange hitsWithMinX(const double xmin, const int sta,
	const int lay, const int reg) const
    { return m_tHitManager->hitsWithMinX(xmin, sta, lay, reg); }

    /// provide convenient access to hits in range
    inline HitRange hitsInRange( const PatRange& xrange, const unsigned sta,
	const unsigned lay, const unsigned reg) const;

    // cache some values for the different regions
    double m_RCdzdy[m_nSta * m_nLay * m_nReg]; ///< cache for regionDzDy
    double m_RCz0[m_nSta * m_nLay * m_nReg]; ///< cache for regionZ0
    double m_RCtanT[m_nSta * m_nLay * m_nReg]; ///< cache for regionTanT

    /// estimate dz/dy of a region (only OT, returns 0 for IT)
    inline double regionDzDy(const int sta, const int lay,
	const int reg) const
    { return m_RCdzdy[(sta * m_nLay + lay) * m_nReg + reg]; }

    /// estimate z at y = 0 of a region (only OT, returns ymid for IT)
    inline double regionZ0(const int sta, const int lay,
	const int reg) const
    { return m_RCz0[(sta * m_nLay + lay) * m_nReg + reg]; }

    /// shortcut for a region's sinT/cosT
    inline double regionTanT(const int sta, const int lay,
	const int reg) const
    { return m_RCtanT[(sta * m_nLay + lay) * m_nReg + reg]; }

    /// restore coordinates and drift time of a hit at y=0
    inline void restoreCoordinate(PatFwdHit* hit) const;

    /// helper to add neighbouring hit to track
    inline void addNeighbour(PatSeedTrack& track, const PatFwdHit* hit,
	HitRange::const_iterator& itH, const HitRange& range) const;

    /// combine two hits into a cluster by calculating the cluster position
    inline void combineCluster(const PatFwdHit* h1, const PatFwdHit* h2,
	double& x, double& z) const;

    struct isTStation : std::unary_function<const LHCb::LHCbID, bool> {
      bool operator()(const LHCb::LHCbID id) const
      { return id.isIT() || id.isOT(); }
    };

    /// give x range at y = 0 predicted by state extrapolated to z
    inline PatRange predictXAtYEq0(const LHCb::State* state,
	double z, double dxdy, double dzdy) const;

    /// give x range at y = 0 predicted by state
    inline PatRange predictXinRegion(const LHCb::State* state,
	unsigned sta, unsigned lay, unsigned reg) const
    {
      return predictXAtYEq0(state, regionZ0(sta,lay,reg),
	  -regionTanT(sta,lay,reg), regionDzDy(sta,lay,reg));
    }

    /// check if hit is inside x range predicted by state at z of hit
    inline bool isGoodHit(const PatFwdHit* hit, const LHCb::State* state) const
    {
      return predictXAtYEq0(state, hit->hit()->zAtYEq0(), hit->hit()->dxDy(),
	  hit->hit()->dzDy()).isInside(hit->hit()->xAtYEq0());
    }

    /// check that slopes are good
    inline bool isGoodSlope(const LHCb::State* state,
	bool isTx, double slope) const;

    /// return range satisfying center of magnet constraint
    inline PatRange magnetRange(const double x0, const double zScaling) const
    { return symmetricRange(x0 * (1. + zScaling), m_xMagTol * zScaling); }

    /// return range satisfying impact condition at z=0
    inline PatRange impactRange(const double x0, const double zScaling) const
    { return symmetricRange(x0 * (1. + zScaling), m_maxImpact * zScaling); }

    /// return extension of a region in x
    inline PatRange regionX(const unsigned sta, const unsigned lay,
	const unsigned reg, const double tol = 0.) const
    {
      return PatRange(region(sta,lay,reg)->xmin() - tol,
	  region(sta,lay,reg)->xmax() + tol);
    }

    /// return extension of a region in y
    inline PatRange regionY(const unsigned sta, const unsigned lay,
	const unsigned reg, const double tol = 0.) const
    {
      return PatRange(region(sta,lay,reg)->ymin() - tol,
	  region(sta,lay,reg)->ymax() + tol);
    }

    /// return extension of a region in z
    inline PatRange regionZ(const unsigned sta, const unsigned lay,
	const unsigned reg, const double tol = 0.) const
    {
      return PatRange(region(sta,lay,reg)->zmin() - tol,
	  region(sta,lay,reg)->zmax() + tol);
    }

    /// return range of valid slopes ty per region
    inline PatRange tyRange(const unsigned reg) const
    {
      static const PatRange tyrange[m_nReg] = {
	PatRange(-0.33, 0.01), PatRange(-0.01, 0.33), PatRange(-0.01, 0.01),
	PatRange(-0.01, 0.01), PatRange(-0.04, -0.01), PatRange(0.01, 0.04) };
      return tyrange[reg];
    }

    /** return tilt corrected range for a given prediction
     * @param sta station in which to give tilt corrected range
     * @param lay layer in which to give tilt corrected range
     * @param reg region in which to give tilt corrected range
     * @param tyreg region from which to take the permissible ty range
     * @param slope prediction
     * @param xpred x position prediction
     * @param tol window around xpred before tilt correction
     */
    inline PatRange tiltCorrectedRange(
	const unsigned sta, const unsigned lay, const unsigned reg,
	const unsigned tyreg, const double dz,
	const double slope, const double xpred, const double tol) const;

    /** return tilt corrected range for a given prediction
     * @param sta station in which to give tilt corrected range
     * @param lay layer in which to give tilt corrected range
     * @param reg region in which to give tilt corrected range
     * @param tyreg region from which to take the permissible ty range
     * @param track track to be used to predict xpred in sta,lay,reg
     * @param tol window around xpred before tilt correction
     */
    inline PatRange tiltCorrectedRange(
	const unsigned sta, const unsigned lay, const unsigned reg,
	const unsigned tyreg, const PatSeedTrack& track, const double tol) const;

    /** return tilt corrected range for a given prediction
     * @param hit from which to take zAtYEq0 and dzdy
     * @param tyreg region from which to take the permissible ty range
     * @param track track to be used to predict xpred in sta,lay,reg
     * @param tol window around xpred before tilt correction
     */
    inline PatRange tiltCorrectedRange(
	const PatFwdHit* hit, const unsigned tyreg,
	const PatSeedTrack& track, const double tol) const;

    /// propagate the x component of a track vector by dz (neglecting curvature)
    inline double propagateX(const double dz, const Gaudi::TrackVector& v) const
    { return v(0) + dz * v(2); }
    /// propagate the y component of a track vector by dz
    inline double propagateY(const double dz, const Gaudi::TrackVector& v) const
    { return v(1) + dz * v(2); }
    /// propagate the error in x by dz (neglecting curvature terms)
    inline double propagateEX2(const double dz, const Gaudi::TrackSymMatrix& cov) const
    { return cov(0, 0) + dz * (2.0 * cov(0, 2) + dz * cov(2, 2)); }
    /// propagate the error in y by dz
    inline double propagateEY2(const double dz, const Gaudi::TrackSymMatrix& cov) const
    { return cov(1, 1) + dz * (2.0 * cov(1, 3) + dz * cov(3, 3)); }
};

inline void PatSeedingTool::restoreCoordinate(PatFwdHit* hit) const
{
  // reset the coordinates of a hit to their values at y = 0
  // (once the true y is known, hit coordinates are shifted to their
  // correct position)
  hit->setX(hit->hit()->xAtYEq0());
  hit->setZ(hit->hit()->zAtYEq0());
  const Tf::OTHit* otHit = hit->hit()->othit();
  if (otHit)
    hit->setDriftDistance(otHit->untruncatedDriftDistance(0));
}

inline void PatSeedingTool::addNeighbour(PatSeedTrack& track,
    const PatFwdHit* hit, HitRange::const_iterator& itH,
    const HitRange& range) const
{
  if (range.end() == (itH + 1)) return;
  PatFwdHit* hit2 = *(itH + 1);
  if (hit2->isUsed()) return;
  if (hit2->x() - hit->x() > m_tolCollect) return;
  ++itH;			// consume hit
  restoreCoordinate(hit2);
  track.addCoord(hit2);	// and add to track
  if ( m_printing ) info() << "   + hit sta " << hit2->hit()->station() <<
    " lay " << hit2->hit()->layer() << " x = " << hit2->x() << endreq;
}

inline void PatSeedingTool::combineCluster(const PatFwdHit* h1, const PatFwdHit* h2,
    double& x, double& z) const
{
  const Tf::OTHit* otHit1 = h1->hit()->othit();
  const Tf::OTHit* otHit2 = h2->hit()->othit();
  if (0 != otHit1 && 0 != otHit2) {
    // form a weighted sum of wire positions to determine the cluster
    // position (this degenerates to the arithmetic mean if we run without
    // drift time information)
    // to make sure that the weighting procedure does not diverge by accident,
    // we add the measurement variance to the squared drift radius, and take
    // the square of this expression to obtain the weights
    const double rr1 = otHit1->untruncatedDriftDistance(0.);
    const double r1 = std::sqrt(otHit1->variance() + rr1 * rr1);
    const double rr2 = otHit2->untruncatedDriftDistance(0.);
    const double r2 = std::sqrt(otHit2->variance() + rr2 * rr2);
    // the cluster position should be closer to the hit with the smaller
    // drift radius
    x = (h1->hit()->xAtYEq0() * r2 + h2->hit()->xAtYEq0() * r1) / (r1 + r2);
    z = (h1->hit()->zAtYEq0() * r2 + h2->hit()->zAtYEq0() * r1) / (r1 + r2);
  } else {
    x = 0.5 * (h1->hit()->xAtYEq0() + h2->hit()->xAtYEq0());
    z = 0.5 * (h1->hit()->zAtYEq0() + h2->hit()->zAtYEq0());
  }
}

inline PatRange PatSeedingTool::predictXAtYEq0(
    const LHCb::State* state, double z, double dxdy, double dzdy) const
{
  // how far to propagate
  const double dz = z - state->z();
  const Gaudi::TrackVector& v = state->stateVector();
  const Gaudi::TrackSymMatrix& cov = state->covariance();
  // work out predicted uncertainties
  const double xtol2 = propagateEX2(dz, cov);
  const double ytol2 = propagateEY2(dz, cov);
  const double txtol2 = cov(2, 2);
  const double x = propagateX(dz, v);
  const double y = propagateY(dz, v);
  const double tx = state->tx();
  // bring to frame specified by dxdy and dzdy
  const double xpred = x - y * (dxdy - tx * dzdy);
  // work out uncertainty of xpred
  const double epred = std::sqrt(xtol2 +
      (dxdy * dxdy + tx * tx * dzdy * dzdy) * ytol2 +
      y * y * dzdy * dzdy * txtol2);

  return PatRange(xpred - epred, xpred + epred);
}

inline bool PatSeedingTool::isGoodSlope(
    const LHCb::State* state, bool isTx, double slope) const
{
  const double delta = slope - (isTx ? state->tx() : state->ty());
  const Gaudi::TrackSymMatrix& cov = state->covariance();
  return delta * delta < (isTx ? cov(2, 2) : cov(3, 3));
}

inline PatSeedingTool::HitRange PatSeedingTool::hitsInRange(
    const PatRange& xrange,
    const unsigned sta, const unsigned lay, const unsigned reg) const
{
  HitRange hitRange = hits(sta,lay,reg);
  if (hitRange.end() == hitRange.begin()) return hitRange;
  HitRange::iterator beg =
    std::lower_bound(hitRange.begin(), hitRange.end(), xrange.min(),
	Tf::compByX_LB<PatFwdHit>());
  if (hitRange.end() == beg) return HitRange(beg, beg);
  // range is typically small, so scan up to n elements to find the end
  // if range is not small, we wasted time to look at n elements,
  // and use std::lower_bound instead (which is O(N log(N)) where N is
  // the size of hitRange)
  unsigned n = 8;
  HitRange::iterator end = beg;
  while (hitRange.end() != end &&
      (*end)->hit()->xAtYEq0() < xrange.max() && n)
    ++end, --n;
  if (0 == n && hitRange.end() != end &&
      (*end)->hit()->xAtYEq0() < xrange.max())
    end = std::lower_bound(end, hitRange.end(), xrange.max(),
	Tf::compByX_LB<PatFwdHit>());

  return HitRange(beg, end);
}

inline PatRange PatSeedingTool::tiltCorrectedRange(
    const unsigned sta, const unsigned lay, const unsigned reg,
    const unsigned tyreg, const double dz,
    const double slope, const double xpred, const double tol) const
{
  const double dzdy = regionDzDy(sta,lay,reg);
  // convert permissible slopes ty in region to x range
  PatRange range = tyRange(tyreg) *= dz * dzdy * slope;
  // shift to xpred and widen to at least tol
  range.widen(tol);
  return range += xpred;
}

inline PatRange PatSeedingTool::tiltCorrectedRange(
    const unsigned sta, const unsigned lay, const unsigned reg,
    const unsigned tyreg, const PatSeedTrack& track, const double tol) const
{
  const double z0 = regionZ0(sta,lay,reg);
  const double dz = z0 - m_zReference;
  const double dzdy = regionDzDy(sta,lay,reg);
  // convert permissible range in slopes ty to z range
  PatRange range = tyRange(tyreg) *= dz * dzdy;
  range += z0;
  // convert to range in x
  range.min() = track.xAtZ(range.min());
  range.max() = track.xAtZ(range.max());
  if (range.min() > range.max())
    std::swap(range.min(), range.max());
  range.widen(tol);
  return range;
}

inline PatRange PatSeedingTool::tiltCorrectedRange(
    const PatFwdHit* hit, const unsigned tyreg,
    const PatSeedTrack& track, const double tol) const
{
  const double z0 = hit->hit()->zAtYEq0();
  const double dz = z0 - m_zReference;
  const double dzdy = hit->hit()->dzDy();
  // convert permissible range in slopes ty to z range
  PatRange range = tyRange(tyreg) *= dz * dzdy;
  range += z0;
  // convert to range in x
  range.min() = track.xAtZ(range.min());
  range.max() = track.xAtZ(range.max());
  if (range.min() > range.max())
    std::swap(range.min(), range.max());
  range.widen(tol);
  return range;
}

#endif // PATSEEDINGTOOL_H

// vim:shiftwidth=2:tw=78
