// $Id: TrackExpectedHitsXYZTool.h,v 1.2 2010/02/01 15:55:31 mschille Exp $
#ifndef INCLUDE_TRACKEXPECTEDHITSXYZTOOL_H
#define INCLUDE_TRACKEXPECTEDHITSXYZTOOL_H 1

#include <vector>
#include <utility>

#include <boost/array.hpp>
#include <algorithm>

#include "GaudiAlg/GaudiTool.h"
#include "GaudiKernel/Vector3DTypes.h"

namespace LHCb {
    class Track;
    class TrackTraj;
    class Trajectory;
    class State;
}

namespace Tf {
    template<class Hit> class TStationHitManager;
    template<class Hit> class TTStationHitManager;
    class DefaultVeloRHitManager;
    class DefaultVeloPhiHitManager;
    template<class A, class B, int C> class DefaultVeloHitManager;
    class LineHit;
    class VeloRHit;
    class VeloPhiHit;
}

class PatForwardHit;
class PatTTHit;
class DeVeloRType;
class DeVeloPhiType;
class ITrajPoca;

static const InterfaceID IID_TrackExpectedHitsXYZTool ( "TrackExpectedHitsXYZTool", 1, 0 );

/** @class TrackExpectedHitsXYZTool TrackExpectedHitsXYZTool.h
 *
 * obtain track-layer intersection points 
 *
 * @author Manuel Tobias Schiller <schiller@physi.uni-heidelberg.de>
 * @date   2010-06-10
 *
 * @author Georg Alexander Krocker <krocker@physi.uni-heidelberg.de>
 * @date   2010-07-02
 *
 * current limitation: does not work for Muon
 */
class TrackExpectedHitsXYZTool : public GaudiTool
{
    public:
	typedef Tf::TStationHitManager<PatForwardHit> HitManT;
	typedef Tf::TTStationHitManager<PatTTHit> HitManTT;
	typedef Tf::DefaultVeloPhiHitManager HitManVeloPhi;
	typedef Tf::DefaultVeloRHitManager HitManVeloR;

    public:
	// Return the interface ID
	static const InterfaceID& interfaceID() { return IID_TrackExpectedHitsXYZTool; }

	/// Standard Constructor
	TrackExpectedHitsXYZTool(const std::string& type,
		const std::string& name,
		const IInterface* parent);

	virtual ~TrackExpectedHitsXYZTool(); ///< Destructor

	virtual StatusCode initialize(); ///< Tool initialization
	virtual StatusCode   finalize(); ///< Tool finalize

	/// represent points in each layer where we expect a hit (approx. positions)
	struct HitPatternXYZ {
	    boost::array<Gaudi::XYZVector, 23> VeloRA;	///< Velo R sensor A side
	    boost::array<Gaudi::XYZVector, 23> VeloRC;	///< Velo R sensor C side
	    boost::array<Gaudi::XYZVector, 23> VeloPhiA;///< Velo Phi sensor A side
	    boost::array<Gaudi::XYZVector, 23> VeloPhiC;///< Velo Phi sensor C side
	    boost::array<Gaudi::XYZVector, 4> TT;	///< TT
	    boost::array<Gaudi::XYZVector, 12> ITAC;	///< IT A/C side boxes
	    boost::array<Gaudi::XYZVector, 12> ITTopBot;///< IT top/bottom boxes
	    boost::array<Gaudi::XYZVector, 12> OT;	///< OT

	    /// default constructor to fill members with invalid values (quiet NaNs)
	    HitPatternXYZ();
	};

	/// get pattern of track-layer intersection points
	/** get pattern of track-layer intersection points
	 *
	 * note: this is only an approximate position; for example, we
	 * completely ignore OT monolayers in the code...
	 *
	 * @param tr	track around which hits are to be collected
	 * @param pat	"HitPattern" in terms of track-layer intersection
	 * 		points
	 * @param collectVelo collect hits in Velo
	 * @param collectTT collect hits in TT
	 * @param collectIT collect hits in IT
	 * @param collectOT collect hits in OT
	 * @param collectMuon collect hits in Muon
	 * @return StatusCode::SUCCESS or StatusCode::FAILURE
	 */
	virtual StatusCode execute(
		const LHCb::Track& tr, HitPatternXYZ& pat,
		bool collectVelo = true, bool collectTT = true,
		bool collectIT = true, bool collectOT = true,
		bool collectMuon = true);

    protected:


    private:
	// hit managers
	HitManT* m_hitManagerT;
	HitManTT* m_hitManagerTT;
	HitManVeloR* m_hitManagerVeloR;
	HitManVeloPhi* m_hitManagerVeloPhi;
	// poca tool for Velo hits
	ITrajPoca* m_trajPoca;

	double m_tolX; ///< initial tolerance when getting rough hit list
	double m_tolY; ///< initial tolerance when getting rough hit list
	double m_tolR; ///< initial tolerance when getting rough hit list
	double m_tolPhi; ///< initial tolerance when getting rough hit list
	double m_nSigma; ///< initial track pos. uncertainty scale for rough hit list
	double m_maxDistOT; ///< max. final dist. hit-track in OT
	double m_maxDistIT; ///< max. final dist. hit-track in IT
	double m_maxDistTT; ///< max. final dist. hit-track in TT
	double m_maxDistVelo; ///< max. final dist. hit-track in Velo
	
	/// collect line hits as used in IT, TT and OT
	template<typename HITMANAGER, typename IndexHelper,
	    unsigned regMin, unsigned regMax,
	    unsigned staMin, unsigned staMax,
	    unsigned layMin, unsigned layMax>
	void collectLineHits(HITMANAGER* hitman,
		const LHCb::TrackTraj& ttraj,
		HitPatternXYZ& pat) const;

	/// collect Velo hits (R/Phi depending on HITMANAGER)
	template<typename HITMANAGER, typename IndexHelper>
	void collectVeloHits(HITMANAGER* hitman,
		const LHCb::TrackTraj& ttraj,
		HitPatternXYZ& pat) const;

};
#endif // INCLUDE_TRACKEXPECTEDHITSXYZTOOL_H

// vim: sw=4:tw=78:ft=cpp
