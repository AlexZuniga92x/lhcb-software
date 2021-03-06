// $Id: $
#include "GaudiKernel/AlgFactory.h"

#include "RefitParticleTracks.h"

#include "TrackInterfaces/ITrackFitter.h"
#include "TrackInterfaces/IGhostProbability.h"
#include "Event/Track.h"
#include "LoKi/PhysExtract.h"
#include <boost/foreach.hpp>


//-----------------------------------------------------------------------------
// Implementation file for class : RefitParticleTracks
//
// 2012-11-23 : Paul Seyfert <pseyfert@cern.ch>
//-----------------------------------------------------------------------------

DECLARE_ALGORITHM_FACTORY( RefitParticleTracks )


//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
RefitParticleTracks::RefitParticleTracks(const std::string& name, ISvcLocator* pSvcLocator)
  : DaVinciAlgorithm(name, pSvcLocator)
{
  declareProperty("DoFit"              ,m_fit = true);
  declareProperty("DoProbability"      ,m_manipulate = false);
  declareProperty("WriteToTES"         ,m_writeOutput = false);
  declareProperty("UpdateProbability"  ,m_update = false);
  declareProperty("ReplaceTracks"      ,m_overwrite = true);
  declareProperty("Fitter"             ,m_fittername = "TrackInitFit");
  declareProperty("Manipulator"        ,m_manipulatorname = "TrackNNGhostId");
  declareProperty("ValidationMode"     ,m_validationmode = false);
}

//=============================================================================
// Destructor
//=============================================================================
RefitParticleTracks::~RefitParticleTracks() {;}

//=============================================================================
// Initialization
//=============================================================================
StatusCode RefitParticleTracks::initialize()
{
  StatusCode sc = DaVinciAlgorithm::initialize(); // must be executed first
  if (sc.isFailure()) return sc;  // error printed already by DaVinciAlgorithm

  if (!m_overwrite && !m_update && !m_writeOutput) {
    Warning("Neither output nor overwriting/updating specified. Don't know what to do.").ignore();
  }
  if (m_overwrite || m_update) {
    Warning("Don't run this option in the Stripping! (UpdateProbability or ReplaceTracks are evil there.) Please copy your tracks somewhere.").ignore();
  }
  if (!m_manipulate && m_update) {
    return Warning("Not implemented combination (UpdateProbability w/o DoProbability) [1]",StatusCode::FAILURE);
  }
  if (m_overwrite && m_manipulate && !m_update) {
    return Warning("Sorry, I thought about this combination but didn't want to implement it (ReplaceTracks and DoProbability w/o UpdateProbability[2]",StatusCode::FAILURE);
  }
  if (m_overwrite && m_manipulate && m_update) {
    m_update = false; // sorry for this hack. I here attempt to keep the code more readable.[3]
  }

  if ( UNLIKELY(msgLevel(MSG::DEBUG)) ) {
    debug() << "==> Initialize" << endmsg;
    debug() <<  "DoFit = "              << m_fit << endmsg;
    debug() <<  "DoProbability = "      << m_manipulate << endmsg;
    debug() <<  "WriteToTES = "         << m_writeOutput << endmsg;
    debug() <<  "UpdateProbability = "  << m_update << endmsg;
    debug() <<  "ReplaceTracks = "      << m_overwrite << endmsg;
 
  }

  m_trackFitter = tool<ITrackFitter>(m_fittername,this);
  m_ghostprobability = tool<IGhostProbability>(m_manipulatorname,this);


  return StatusCode::SUCCESS;
}

//=============================================================================
// Main execution
//=============================================================================
StatusCode RefitParticleTracks::execute()
{
  setFilterPassed(false);
  if ( UNLIKELY(msgLevel(MSG::DEBUG)) ) debug() << "==> Execute" << endmsg;

  const LHCb::Particle::Range inputParticles = particles();

  if (0==inputParticles.size()) return StatusCode::SUCCESS;
  if ( UNLIKELY(msgLevel(MSG::DEBUG)) ) verbose() << inputParticles.size() << "input particles" << endmsg;
  if (m_ghostprobability->beginEvent().isFailure()) {
    return Error("GhostIdTool could not begin event.",StatusCode::SUCCESS,10);
  }

  std::vector<const LHCb::Track*> alltracks;
  std::vector<LHCb::Track*>::const_iterator iterT;
  LHCb::Tracks::const_iterator iterTs;
  std::vector<const LHCb::Track*>::const_iterator itercT;
  LoKi::Extract::getTracks(inputParticles.begin(), inputParticles.end(), std::back_inserter(alltracks));


  LHCb::Tracks uniqueTracks;
  if ( UNLIKELY(msgLevel(MSG::DEBUG)) && inputParticles.size()>0 && 0==alltracks.size())
    verbose() << "no tracks from " << inputParticles.size() << " particles " << endmsg;

  itercT = alltracks.begin();
  uniqueTracks.reserve(alltracks.size());
  for ( ; alltracks.end() != itercT ; ++itercT ) {
    if (!uniqueTracks.object((*itercT)->key())) {
      uniqueTracks.add(const_cast<LHCb::Track*>(*itercT));  ///@todo: this is evil
    }
  }

  std::vector<LHCb::Track*> incasts;
  std::vector<LHCb::Track*> copies;
  incasts.clear();
  copies.clear();

  std::vector<LHCb::Track*>* toworkwith;


  incasts.reserve(uniqueTracks.size());
  //if (m_overwrite || m_update) 
  iterTs = uniqueTracks.begin();
  for ( ; uniqueTracks.end() != iterTs ; ++iterTs ) {
    incasts.push_back( const_cast<LHCb::Track*>(*iterTs) );
    if (!m_overwrite) {
      copies.push_back( (*iterTs)->cloneWithKey() );
      if (incasts.back()->key() != copies.back()->key()) {
        fatal() << "SEVERE PROBLEM. DATA WILL BE CORRUPTED!" << endmsg;
      }
    }
  }

  if (m_overwrite) {
    toworkwith = &incasts;
  } else {
    if (copies.size() != incasts.size()) {
      fatal() << "FAILED SANITY CHECK! WILL LIKELY SEGFAULT SOON!" << endmsg;
    }
    toworkwith = &copies;
  }

  if (m_fit) fit(toworkwith);
  if (m_manipulate) manipulate(toworkwith);
  if ( UNLIKELY (msgLevel(MSG::DEBUG)) ) debug() << " manipulations done " << endmsg;

  if (m_update) { /// if we anyhow overwrite, the property has been switched off in initialize() [3]
                  /// for better text editor support I add the following line as a comment:
                  /// if (m_overwrite = false)
    std::vector<LHCb::Track*>::const_iterator iterFrom = copies.begin();
    std::vector<LHCb::Track*>::const_iterator iterTo   = incasts.begin();
    for ( ; copies.end() != iterFrom ; ++iterFrom) {
      (*iterTo)->setGhostProbability((*iterFrom)->ghostProbability());
      ///@todo: what about more general updates?
      ++iterTo; // do this only at the very end of the loop
    }
  }
  if ( UNLIKELY (msgLevel(MSG::DEBUG)) ) debug() << " updates done " << endmsg;

  if (m_writeOutput) {
    LHCb::Tracks* outtracks = new LHCb::Tracks();
    iterT = toworkwith->begin();
      for ( ; toworkwith->end() != iterT ; ++iterT) {
      outtracks->add(*iterT);
    }
    put(outtracks,outputLocation());
    if (UNLIKELY(msgLevel(MSG::DEBUG))) debug() << "put to TES" << endmsg;
  } else {
    while (copies.size()) {
      delete copies.back();
      copies.pop_back();
    }
  }

  setFilterPassed(true);
  return StatusCode::SUCCESS;
}

StatusCode RefitParticleTracks::fit(std::vector<LHCb::Track*> *vec) {
  if ( UNLIKELY(msgLevel(MSG::DEBUG)) ) debug() << "==> FITTING " << vec->size() << " tracks" << endmsg;

  std::vector<LHCb::Track*>::iterator iterT = vec->begin();
  for ( ; vec->end() != iterT ; ++iterT ) {
    double vchi_old, tchi_old, mchi_old, chi_old, vdof_old, tdof_old, p_old;
    double vchi_new, tchi_new, mchi_new, chi_new, vdof_new, tdof_new, p_new;
    if (m_validationmode) {
      vchi_old = (*iterT)->info(LHCb::Track::FitVeloChi2,-99.);
      tchi_old = (*iterT)->info(LHCb::Track::FitTChi2,-99.);
      mchi_old = (*iterT)->info(LHCb::Track::FitMatchChi2,-99.);
       chi_old = (*iterT)->chi2();
      vdof_old = (*iterT)->info(LHCb::Track::FitVeloNDoF,-99.);
      tdof_old = (*iterT)->info(LHCb::Track::FitTNDoF,-99.);
         p_old = (*iterT)->p();
    }
    m_trackFitter->fit(*(*iterT)).ignore();
    if (m_validationmode) {
      vchi_new = (*iterT)->info(LHCb::Track::FitVeloChi2,-99.);
      tchi_new = (*iterT)->info(LHCb::Track::FitTChi2,-99.);
      mchi_new = (*iterT)->info(LHCb::Track::FitMatchChi2,-99.);
       chi_new = (*iterT)->chi2();
      vdof_new = (*iterT)->info(LHCb::Track::FitVeloNDoF,-99.);
      tdof_new = (*iterT)->info(LHCb::Track::FitTNDoF,-99.);
         p_new = (*iterT)->p();
/// http://stackoverflow.com/questions/3378560/how-to-disable-gcc-warnings-for-a-few-lines-of-code
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
         /// all these chi^2, p, ndof should be != 0 anyway, so no FPE (or something about which i actually want to get warned, so FPE, come at me!
      if (1e-4<fabs((vchi_old - vchi_new) / vchi_new)) fatal() << "DaVinci Track fit did not reproduce Brunel result in variable vchi:" << fabs((vchi_old - vchi_new) / vchi_new) << endmsg;
      if (1e-4<fabs((tchi_old - tchi_new) / tchi_new)) fatal() << "DaVinci Track fit did not reproduce Brunel result in variable tchi:" << fabs((tchi_old - tchi_new) / tchi_new) << endmsg;
      if (1e-4<fabs((mchi_old - mchi_new) / mchi_new)) fatal() << "DaVinci Track fit did not reproduce Brunel result in variable mchi:" << fabs((mchi_old - mchi_new) / mchi_new) << endmsg;
      if (1e-4<fabs(( chi_old -  chi_new) /  chi_new)) fatal() << "DaVinci Track fit did not reproduce Brunel result in variable  chi:" << fabs(( chi_old -  chi_new) /  chi_new) << endmsg;
      if (1e-4<fabs((vdof_old - vdof_new) / vdof_new)) fatal() << "DaVinci Track fit did not reproduce Brunel result in variable vdof:" << fabs((vdof_old - vdof_new) / vdof_new) << endmsg;
      if (1e-4<fabs((tdof_old - tdof_new) / tdof_new)) fatal() << "DaVinci Track fit did not reproduce Brunel result in variable tdof:" << fabs((tdof_old - tdof_new) / tdof_new) << endmsg;
      if (1e-4<fabs((   p_old -    p_new) /    p_new)) fatal() << "DaVinci Track fit did not reproduce Brunel result in variable    p:" << fabs((   p_old -    p_new) /    p_new) << endmsg;
#pragma GCC diagnostic pop
    }
  }
  return StatusCode::SUCCESS;
}

StatusCode RefitParticleTracks::manipulate(std::vector<LHCb::Track*> *vec) {
  if ( UNLIKELY(msgLevel(MSG::DEBUG)) ) debug() << "==> MANIPULATING" << endmsg;
  std::vector<LHCb::Track*>::iterator iterT = vec->begin();
  for ( ; vec->end() != iterT ; ++iterT ) {
    double gp_old, gp_new;
    if (m_validationmode) {
      gp_old = (*iterT)->ghostProbability();
    }
    m_ghostprobability->execute(*(*iterT)).ignore();
    if (m_validationmode) {
      gp_new = (*iterT)->ghostProbability();
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
      /// avoid division by zero here. gp is on a 0...1 scale anyway
      if (1e-8<fabs(gp_old - gp_new)) {
        fatal() << "DaVinci ghost probability did not reproduce Brunel result" << endmsg;
        fatal() << " old = " << gp_old << "\t\tnew = " << gp_new << endmsg;
      }
#pragma GCC diagnostic pop
    }
  }
  return StatusCode::SUCCESS;
}



