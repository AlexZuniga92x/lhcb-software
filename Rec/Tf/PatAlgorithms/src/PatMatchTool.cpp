// Include files
#include <limits>
#include <vector>
#include <map>
#include <algorithm>

#include <boost/foreach.hpp>

// from Gaudi
#include "GaudiKernel/ToolFactory.h"

#include "Event/StateParameters.h"

#include "TrackInterfaces/IAddTTClusterTool.h"
#include "TrackInterfaces/ITrackMomentumEstimate.h"

// local
#include "PatMatchTool.h"

//-----------------------------------------------------------------------------
// Implementation file for class : PatMatchTool
//
// 2008-04-25 : Johannes Albrecht
//-----------------------------------------------------------------------------

// Declaration of the Tool Factory
DECLARE_TOOL_FACTORY(PatMatchTool)

//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
PatMatchTool::PatMatchTool(const std::string& type,
	const std::string& name, const IInterface* parent) :
    GaudiTool(type, name , parent),
    m_fastMomentumTool(0), m_addTTClusterTool(0)
{
    declareInterface<IMatchTool>(this);
    declareInterface<ITrackMatch>(this);

    declareProperty("zMagnet"         , m_zMagnet       = 5200. * Gaudi::Units::mm);
    declareProperty("zMagnetTx2"      , m_zMagnetTx2    =-1700. * Gaudi::Units::mm);
    declareProperty("zMagnetDsl2"     , m_zMagnetDsl2   =  500. * Gaudi::Units::mm);
    declareProperty("zMatchY"         , m_zMatchY       = 8420. * Gaudi::Units::mm);
    declareProperty("dxTol"           , m_dxTol         =    8. * Gaudi::Units::mm);
    declareProperty("dxTolSlope"      , m_dxTolSlope    =   80. * Gaudi::Units::mm);
    declareProperty("dyTol"           , m_dyTol         =    6. * Gaudi::Units::mm);
    declareProperty("dyTolSlope"      , m_dyTolSlope    =  300. * Gaudi::Units::mm);
    declareProperty("MagnetBend"      , m_magnetBend    =-1000. * Gaudi::Units::mm);
    declareProperty("maxMatchChi2"    , m_maxChi2       = 4);
    declareProperty("FastMomentumToolName",
					m_fastMomentumToolName = "FastMomentumEstimate");
    declareProperty("AddTTClusters"   , m_addTT            = true);
    declareProperty("writeNNVariables", m_writeNNVariables = true);

}
//=============================================================================
// Destructor
//=============================================================================
PatMatchTool::~PatMatchTool() {}

//=============================================================================

//=============================================================================
// Initialization
//=============================================================================
StatusCode PatMatchTool::initialize()
{
    StatusCode sc = GaudiTool::initialize();
    if (sc.isFailure()) return sc;    // error printed already by GaudiAlgorithm

    if (UNLIKELY(msgLevel(MSG::DEBUG)))
	debug() << "==> Initialize" << endmsg;

    m_addTTClusterTool = tool<IAddTTClusterTool>("PatAddTTCoord");

    m_fastMomentumTool = tool<ITrackMomentumEstimate>(m_fastMomentumToolName);

    return StatusCode::SUCCESS;
}

StatusCode PatMatchTool::matchSingle(const LHCb::Track& velo,
	const LHCb::Track& seed , LHCb::Track& output, double& chi2)
{
    // calculate quality of match between tracks
    chi2 = getChi2Match(velo, seed);

    if (m_maxChi2 > chi2) {
	// set states and flags of output track
	makeTrack(velo, seed, output, chi2);
	if (m_addTT) m_addTTClusterTool->addTTClusters(output);
	return StatusCode::SUCCESS;
    }

    return StatusCode::FAILURE;
}

//=============================================================================
// Main execution
//=============================================================================
StatusCode PatMatchTool::match(const LHCb::Tracks& velos,
	const LHCb::Tracks& seeds, LHCb::Tracks& matchs)
{
    // build a match-chi^2 sorted table of velo-seed matches
    std::vector<MatchCandidate> cands;
    cands.reserve(4 * std::max(velos.size(), seeds.size()));
    BOOST_FOREACH(const LHCb::Track* vTr, velos) {
	if (vTr->checkFlag(LHCb::Track::Backward)) continue;
	if (vTr->checkFlag(LHCb::Track::Invalid)) continue;
	BOOST_FOREACH(const LHCb::Track* sTr, seeds) {
	    if (sTr->checkFlag(LHCb::Track::Invalid)) continue;
	    const double dist = getChi2Match(*vTr, *sTr);
	    if (m_maxChi2 > dist) {
		cands.push_back(MatchCandidate(vTr, sTr, dist));
	    }
	}
    }
    std::stable_sort(cands.begin(), cands.end(), MatchCandidate::lowerByChi2());

    // for each track, tag if used or not.
    std::map<const LHCb::Track*,bool> used;
    // convert unused match candidates to tracks
    BOOST_FOREACH(const MatchCandidate& cand, cands) {
	const LHCb::Track* vTr = cand.vTr();
	const LHCb::Track* sTr = cand.sTr();

	if (UNLIKELY(msgLevel(MSG::DEBUG))) {
	    debug() << "Candidate Velo " << vTr->key() << " used " << used[ vTr ]
		<< " Seed " << sTr->key() << " used " << used[ sTr ]
		<< " dist " << cand.dist() << endmsg;
	}

	bool& vUsed = used[vTr];
	if (vUsed) continue;
	bool& sUsed = used[sTr];
	if (sUsed) continue;

	vUsed = sUsed = true;

	LHCb::Track* match = new LHCb::Track();
	matchs.insert(match);

	makeTrack(*vTr, *sTr, *match, cand.dist());

	if (m_addTT) {
	    StatusCode sc = m_addTTClusterTool->addTTClusters(*match);
	    if (sc.isFailure())
		Warning("adding TT clusters failed!",sc).ignore();
	}

	// added for NNTools -- check how many tracks have common hits
	// This is always 1.0 as we require that velo and seed tracks are used only once.
	if (m_writeNNVariables)
	    match->addInfo(LHCb::Track::NCandCommonHits, 1.0);
    }//end loop match cands

    return StatusCode::SUCCESS;
}

//=============================================================================
//  Finalize
//=============================================================================
StatusCode PatMatchTool::finalize()
{
    if (UNLIKELY(msgLevel(MSG::DEBUG))) debug() << "==> Finalize" << endmsg;

    return GaudiTool::finalize();
}

//=============================================================================

double PatMatchTool::getChi2Match(const LHCb::Track& velo,
	const LHCb::Track& seed) const
{

    double chi2 = std::numeric_limits<double>::max();
    if (velo.checkFlag(LHCb::Track::Invalid)) return chi2;
    if (velo.checkFlag(LHCb::Track::Backward)) return chi2;
    if (seed.checkFlag(LHCb::Track::Invalid)) return chi2;

    const LHCb::State& vState = velo.closestState(0.);
    const LHCb::State& sState = seed.closestState(m_zMatchY);

    const double dSlope = vState.tx() - sState.tx();
    const double zForX = m_zMagnet
	+ m_zMagnetTx2 * vState.tx() * vState.tx()
	+ m_zMagnetDsl2 * dSlope * dSlope;
    const double dxTol2 = m_dxTol * m_dxTol;
    const double dxTolSlope2 = m_dxTolSlope * m_dxTolSlope;

    const double xV = vState.x() + (zForX     - vState.z()) * vState.tx();
    const double yV = vState.y() + (m_zMatchY - vState.z()) * vState.ty() +
	vState.ty() * dSlope * dSlope * m_magnetBend;

    const double xS = sState.x() + (zForX     - sState.z()) * sState.tx();
    const double yS = sState.y() + (m_zMatchY - sState.z()) * sState.ty();

    const double distX = xS - xV;
    const double distY = yS - yV;
    const double dslx  = vState.tx() - sState.tx();
    const double tolX = dxTol2 + dslx * dslx *  dxTolSlope2;
    const double teta2 = vState.tx() * vState.tx() + vState.ty() * vState.ty();
    const double tolY  = m_dyTol * m_dyTol + teta2 * m_dyTolSlope * m_dyTolSlope;

    chi2 = distX * distX / tolX + distY * distY /tolY;

    const double dSlY = sState.ty() - vState.ty();
    chi2 += dSlY * dSlY / sState.errTy2() / 16.;

    return chi2;
}

//=============================================================================
void PatMatchTool::makeTrack(const LHCb::Track& velo,
	const LHCb::Track& seed, LHCb::Track& output, double chi2) const
{
    //== add ancestors
    output.addToAncestors(velo);
    output.addToAncestors(seed);
    //== Adjust flags
    output.setType(LHCb::Track::Long);
    output.setHistory(LHCb::Track::PatMatch);
    output.setPatRecStatus(LHCb::Track::PatRecIDs);
    output.addInfo(LHCb::Track::MatchChi2, chi2);
    //== copy LHCbIDs
    output.addSortedToLhcbIDs(velo.lhcbIDs());
    output.addSortedToLhcbIDs(seed.lhcbIDs());
    //== copy Velo and T states at the usual pattern reco positions
    std::vector<LHCb::State*> newstates;
    newstates.reserve(6);
    if (velo.hasStateAt(LHCb::State::ClosestToBeam))
	newstates.push_back(velo.stateAt(LHCb::State::ClosestToBeam)->clone());
    if (velo.hasStateAt(LHCb::State::FirstMeasurement))
	newstates.push_back(velo.stateAt(LHCb::State::FirstMeasurement)->clone());
    if (velo.hasStateAt(LHCb::State::EndVelo))
	newstates.push_back(velo.stateAt(LHCb::State::EndVelo)->clone());
    newstates.push_back(seed.closestState(StateParameters::ZBegT).clone());
    newstates.push_back(seed.closestState(StateParameters::ZMidT).clone());
    // make sure we don't include same state twice
    if (std::abs(newstates[newstates.size() - 2]->z() -
		newstates.back()->z()) < 300.) {
	delete newstates.back();
	newstates.pop_back();
    }
    newstates.push_back(seed.closestState(StateParameters::ZEndT).clone());
    // make sure we don't include same state twice
    if (std::abs(newstates[newstates.size() - 2]->z() -
		newstates.back()->z()) < 300.) {
	delete newstates.back();
	newstates.pop_back();
    }
    //== estimate q/p
    double qOverP, sigmaQOverP;
    bool const cubicFit = seed.checkHistory(LHCb::Track::PatSeeding);
    const LHCb::State& vState = velo.closestState(0.);
    const LHCb::State& sState =seed.closestState(m_zMatchY);
    StatusCode sc = m_fastMomentumTool->calculate(&vState,&sState,
	    qOverP, sigmaQOverP, cubicFit);
    if (sc.isFailure()) {
	Warning("momentum determination failed!",sc).ignore();
	// assume the Velo/T station standalone reco do something reasonable
    } else {
	// adjust q/p and its uncertainty
	sigmaQOverP = sigmaQOverP * sigmaQOverP;
	BOOST_FOREACH(LHCb::State* st, newstates) {
	    st->covariance()(4,4) = sigmaQOverP;
	    st->setQOverP(qOverP);
	}
    }
    //== add copied states to output track
    output.addToStates(newstates);
}

//=============================================================================
StatusCode PatMatchTool::match(const LHCb::Track& veloTrack,
	const LHCb::Track& tTrack,
	LHCb::Track& matchedTrack,
	double& quality, double& quality2)
{
    quality2=0;
    StatusCode sc = this->matchSingle(veloTrack, tTrack, matchedTrack, quality);
    if (sc.isFailure())
	if (UNLIKELY(msgLevel(MSG::DEBUG))) debug()<<"matching failed !"<<endmsg;

    return sc;
}

// vim: sw=4:tw=78:ft=cpp
