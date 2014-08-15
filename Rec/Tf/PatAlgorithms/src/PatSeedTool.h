// $Id: PatSeedTool.h,v 1.6 2009-11-26 18:00:48 mschille Exp $
#ifndef PATSEEDTOOL_H
#define PATSEEDTOOL_H 1

// Include files
// from Gaudi
#include "GaudiAlg/GaudiTool.h"

#include "PatSeedTrack.h"

#include "TfKernel/HitExtension.h"


static const InterfaceID IID_PatSeedTool ( "PatSeedTool", 1, 0 );


/** @class PatSeedTool PatSeedTool.h
 *  Tool to fit the PatSeeding tracks
 *
 *  @author Olivier Callot
 *  @date   2006-10-23 Initial version
 *  @date   2007-08-20 Update for a-team framework 
 *  @date   2008-06-04 protect against tracks with too few hits to solve
 *          for fit parameters (i.e. protect against solving a linear system
 *          represented by a singular matrix)
 *  @date   2008-07-11 protect against singular matrix, 2nd part
 *  @date   2010-09-02 simultaneous fit to x and stereo hits, stub refit for
 *          PatSeeding, OT ambiguities from pitch residuals
 */

class PatSeedTool : public GaudiTool {
  public:

    // Return the interface ID
    static const InterfaceID& interfaceID() { return IID_PatSeedTool; }

    /// Standard constructor
    PatSeedTool( const std::string& type,
	const std::string& name,
	const IInterface* parent);

    virtual ~PatSeedTool( ); ///< Destructor

    /** fit a track
     * @param track PatSeedTrack to fit
     * @param maxChi2 outlier removal starts at maxChi2
     * @param minPlanes track must have at least minPlanes layers
     * @param xOnly ignore stereo part if true
     * @param forceDebug force printing debugging information
     * @return false if fit failed to satisfy criteria given by arguments, true otherwise
     */
    bool fitTrack( PatSeedTrack& track, double maxChi2, unsigned minPlanes,
	bool xOnly, bool forceDebug ) const;

    /** refit a stub (four IT hits in same station)
     *
     * simulaneous fit for ty, x0, tx at the track's referenze z0
     * the track is kept pointing to y = 0 at z = 0, its curvature is inferred
     * from the x intercept of a straight line extrapolation to z = 0
     *
     * @param track PatSeedTrack to refit
     * @param dRatio ratio of cubic to parabolic term to be used
     * @param arrow conversion constant from x intercept at z=0 to curvature
     * @return false on failure, true otherwise
     */
    bool refitStub(PatSeedTrack& track, double dRatio, double arrow) const;

  protected:

    /// do an initial fit in xz projection, fixing OT ambiguities if possible
    bool fitInitialXProjection( PatSeedTrack& track, bool forceDebug ) const;

    /// do a fit in xz projection, return false if fewer than 3 x hits on track
    template<bool forceRLAmb = false, unsigned maxIter = 10>
    bool fitXProjection( PatSeedTrack& track, bool forceDebug ) const;

    /// do a fit in yz projection, return false if fewer than 2 stereo hits on track
    bool fitInitialStereoProjection( PatSeedTrack& track, bool forceDebug ) const;

    /// helper for debugging output
    inline void printTCoord( MsgStream& msg,
	const PatSeedTrack& track, const PatFwdHit* hit ) const;
  private:
    bool m_ambigFromPitchResiduals; ///< OT ambiguities from pitch residuals
    bool m_ambigFromLargestDrift; ///< OT ambig. from largest drift time in each station

    bool isNeighbour(const PatFwdHit* h1, const PatFwdHit* h2) const;
    void resAmbFromPitchRes(PatSeedTrack& tr) const;
    bool fitSimultaneousXY(PatSeedTrack& track, bool forceDebug) const;
};


void PatSeedTool::printTCoord( MsgStream& msg,
    const PatSeedTrack& track, const PatFwdHit* hit ) const
{
  double dist = track.distance( hit );
  double chi2 = dist*dist* hit->hit()->weight();
  msg << "  Hit st " << hit->hit()->station() << " lay " << hit->hit()->layer()
    << " region " << hit->hit()->region()
    << format( " code%3d z %7.1f distWire%7.2f drift%5.2f dist%8.3f rl%2d Chi2 %8.3f",
	hit->planeCode(), hit->z(), hit->x() - track.xAtZ( hit->z() ),
	hit->driftDistance(), dist, hit->rlAmb(), chi2 );
}

#endif // PATSEEDTOOL_H
// vim:shiftwidth=2:tw=78
