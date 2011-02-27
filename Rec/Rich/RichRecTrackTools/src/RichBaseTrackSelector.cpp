
//-----------------------------------------------------------------------------
/** @file RichBaseTrackSelector.cpp
 *
 *  Implementation file for RICH reconstruction tool : Rich::Rec::BaseTrackSelector
 *
 *  @author Chris Jones   Christopher.Rob.Jones@cern.ch
 *  @date   12/08/2006
 */
//-----------------------------------------------------------------------------

// from Gaudi
#include "GaudiKernel/ToolFactory.h"

// local
#include "RichBaseTrackSelector.h"

// All code is in general Rich reconstruction namespace
using namespace Rich::Rec;

//-----------------------------------------------------------------------------

// Declaration of the Tool Factory
DECLARE_TOOL_FACTORY( BaseTrackSelector )

//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
BaseTrackSelector::BaseTrackSelector( const std::string& type,
                                      const std::string& name,
                                      const IInterface* parent )
  : ToolBase   ( type, name , parent ),
    m_isoTrack ( NULL )
{
  // interface
  declareInterface<IBaseTrackSelector>(this);

  // JOs

  // By default all cuts are off ....

  declareProperty( "MinPCut",    m_minPCut     = boost::numeric::bounds<double>::lowest()  ); // in GeV
  declareProperty( "MaxPCut",    m_maxPCut     = boost::numeric::bounds<double>::highest() ); // in GeV

  declareProperty( "MinPtCut",   m_minPtCut    = boost::numeric::bounds<double>::lowest()  ); // in GeV
  declareProperty( "MaxPtCut",   m_maxPtCut    = boost::numeric::bounds<double>::highest() ); // in GeV

  declareProperty( "MinChi2Cut", m_minChi2Cut  = boost::numeric::bounds<double>::lowest()  );
  declareProperty( "MaxChi2Cut", m_maxChi2Cut  = boost::numeric::bounds<double>::highest() );

  declareProperty( "MinLikelihood",  m_minLL = boost::numeric::bounds<double>::lowest()  );
  declareProperty( "MaxLikelihood",  m_maxLL = boost::numeric::bounds<double>::highest() );

  declareProperty( "MinCloneDistCut", m_minCloneCut    = boost::numeric::bounds<double>::lowest()  );
  declareProperty( "MaxCloneDistCut", m_maxCloneCut    = boost::numeric::bounds<double>::highest() );

  declareProperty( "MinGhostProbCut", m_minGhostProb   = boost::numeric::bounds<double>::lowest()  );
  declareProperty( "MaxGhostProbCut", m_maxGhostProb   = boost::numeric::bounds<double>::highest() );

  declareProperty( "Charge",       m_chargeSel    = 0 );

  declareProperty( "AcceptClones", m_acceptClones = false );

  declareProperty( "AcceptFitFailures", m_acceptFitFailures = false );

  declareProperty( "RejectNonIsolated", m_rejectNonIsolated = false );

}

//=============================================================================
// Destructor
//=============================================================================
BaseTrackSelector::~BaseTrackSelector() {}

//=============================================================================
// initialisation
//=============================================================================
StatusCode BaseTrackSelector::initialize()
{
  // Sets up various tools and services
  const StatusCode sc = ToolBase::initialize();
  if ( sc.isFailure() ) { return sc; }

  // print track selection
  std::ostringstream sel;
  printSel(sel);
  if ( !sel.str().empty() )
  {
    // get track type from name
    const int slash          = name().find_last_of(".");
    const std::string tkName = ( slash>0 ? name().substr(slash+1) : "UnknownTrackType" );
    info() << boost::format( " %|.12s| %|12t|" ) % tkName;
    info() << sel.str() << endmsg;
  }

  return sc;
}

void BaseTrackSelector::printSel( std::ostringstream & os ) const
{
  // p cut
  m_pCutEnabled = false;
  if ( m_minPCut > boost::numeric::bounds<double>::lowest() ||
       m_maxPCut < boost::numeric::bounds<double>::highest() )
  {
    m_pCutEnabled = true;
    os << boost::format( " : P = %|-4.2e|->%|-4.2e| GeV" ) % m_minPCut % m_maxPCut ;
  }

  // pt cut
  m_ptCutEnabled = false;
  if ( m_minPtCut > boost::numeric::bounds<double>::lowest() ||
       m_maxPtCut < boost::numeric::bounds<double>::highest() )
  {
    m_ptCutEnabled = true;
    os << boost::format( " : Pt = %|-4.2e|->%|-4.2e| GeV" ) % m_minPtCut % m_maxPtCut ;
  }

  // Chi^2 cut
  m_chiSqCutEnabled = false;
  if ( m_minChi2Cut > boost::numeric::bounds<double>::lowest() ||
       m_maxChi2Cut < boost::numeric::bounds<double>::highest() )
  {
    m_chiSqCutEnabled = true;
    os << boost::format( " : fitchi2 = %|-4.2e|->%|-4.2e|" ) % m_minChi2Cut % m_maxChi2Cut ;
  }

  // Likelihood cut
  m_likelihoodCutEnabled = false;
  if ( m_minLL > boost::numeric::bounds<double>::lowest() ||
       m_maxLL < boost::numeric::bounds<double>::highest() )
  {
    m_likelihoodCutEnabled = true;
    os << boost::format( " : Likelihood = %|-4.2e|->%|-4.2e|" ) % m_minLL % m_maxLL ;
  }

  // Clone Dist Cut
  m_cloneDistCutEnabled = !m_acceptClones;
  if ( m_minCloneCut > boost::numeric::bounds<double>::lowest() ||
       m_maxCloneCut < boost::numeric::bounds<double>::highest() )
  {
    m_cloneDistCutEnabled = true;
    os << boost::format( " : CloneCut = %|-4.2e|->%|-4.2e|" ) % m_minCloneCut % m_maxCloneCut ;
  }

  // Ghost Prob Cut
  m_ghostProbCutEnabled = false;
  if ( m_minGhostProb > boost::numeric::bounds<double>::lowest() ||
       m_maxGhostProb < boost::numeric::bounds<double>::highest() )
  {
    m_ghostProbCutEnabled = true;
    os << boost::format( " : GhostProb = %|-4.2e|->%|-4.2e|" ) % m_minGhostProb % m_maxGhostProb ;
  }

  if ( m_acceptClones ) os << " clonesOK";

  if ( m_acceptFitFailures ) os << " fitFailuresOK";

  if ( m_chargeSel != 0 ) os << " chargeSel=" << m_chargeSel;

  if ( m_rejectNonIsolated ) os << " IsolatedTracks";

}

//=============================================================================
// finalisation
//=============================================================================
StatusCode BaseTrackSelector::finalize()
{
  // Execute base class method
  return ToolBase::finalize();
}

// Test if the given Track is selected under the current criteria
bool
BaseTrackSelector::trackSelected( const LHCb::Track * track ) const
{
  if (!track) { Warning("Null Track pointer").ignore(); return false; }

  if ( msgLevel(MSG::VERBOSE) )
  {
    verbose() << "Trying Track key = " << track->key() << " type = " << track->type()
              << endmsg;
  }

  // Fit status
  if ( !m_acceptFitFailures && 
       track->fitStatus() == LHCb::Track::FitFailed )
  {
    if ( msgLevel(MSG::VERBOSE) )
      verbose() << " -> Fit Status FAILED -> Reject" << endmsg;
    return false;
  }

  // cut p
  if ( m_pCutEnabled )
  {
    const double p = track->p() / Gaudi::Units::GeV;
    if ( p < m_minPCut || p > m_maxPCut )
    {
      if ( msgLevel(MSG::VERBOSE) )
        verbose() << " -> P " << p << " failed cut "
                  << m_minPCut << " -> " << m_maxPCut
                  << endmsg;
      return false;
    }
  }

  // cut on pt
  if ( m_ptCutEnabled )
  {
    const double pt = track->pt() / Gaudi::Units::GeV;
    if ( pt < m_minPtCut || pt > m_maxPtCut )
    {
      if ( msgLevel(MSG::VERBOSE) )
        verbose() << " -> Pt " << pt << " failed cut "
                  << m_minPtCut << " -> " << m_maxPtCut
                  << endmsg;
      return false;
    }
  }

  // chi-squared
  if ( m_chiSqCutEnabled )
  {
    const double chi2 = track->chi2PerDoF();
    if ( chi2 < m_minChi2Cut || chi2 > m_maxChi2Cut )
    {
      if ( msgLevel(MSG::VERBOSE) )
        verbose() << " -> Chi^2 " << chi2 << " failed cut "
                  << m_minChi2Cut << " -> " << m_maxChi2Cut
                  << endmsg;
      return false;
    }
  }

  // track charge
  if ( m_chargeSel != 0 && m_chargeSel*track->charge() < 0 )
  {
    if ( msgLevel(MSG::VERBOSE) )
      verbose() << " -> Track charge " << track->charge() << " failed cut" << endmsg;
    return false;
  }

  // clones
  if ( m_cloneDistCutEnabled )
  {
    const double cloneDist = track->info(LHCb::Track::CloneDist,9e99);
    if ( !m_acceptClones && ( track->checkFlag(LHCb::Track::Clone) ||
                              cloneDist < m_minCloneCut || cloneDist > m_maxCloneCut ) )
    {
      if ( msgLevel(MSG::VERBOSE) )
        verbose() << " -> Track failed clone rejection" << endmsg;
      return false;
    }
  }

  // Likelihood
  if ( m_likelihoodCutEnabled )
  {
    if ( track->likelihood() < 99 && // check against default value in Track class. Accept in this case.
         ( track->likelihood() < m_minLL || track->likelihood() > m_maxLL ) )
    {
      if ( msgLevel(MSG::VERBOSE) )
        verbose() << " -> Track Likelihood " << track->likelihood() << " failed cut "
                  << m_minLL << " -> " << m_maxLL
                  << endmsg;
      return false;
    }
  }

  // GhostProbability
  if ( m_ghostProbCutEnabled )
  {
    if ( track->ghostProbability() < 99 && // check against default value in Track class. Accept in this case.
         ( track->ghostProbability() < m_minGhostProb || track->ghostProbability() > m_maxGhostProb ) )
    {
      if ( msgLevel(MSG::VERBOSE) )
        verbose() << " -> Track GhostProbability " << track->ghostProbability() << " failed cut "
                  << m_minGhostProb << " -> " << m_maxGhostProb
                  << endmsg;
      return false;
    }
  }

  // isolation
  if ( m_rejectNonIsolated )
  {
    if ( !isoTrackTool()->isIsolated(track) )
    {
      if ( msgLevel(MSG::VERBOSE) )
        verbose() << " -> Isolation Criteria failed" << endmsg;
      return false;
    }
  }

  if ( msgLevel(MSG::VERBOSE) ) verbose() << " -> Track selected" << endmsg;
  return true;
}

bool
BaseTrackSelector::trackSelected( const LHCb::RichRecTrack * track ) const
{

  if (!track) { Warning("Null RichRecTrack pointer").ignore(); return false; }

  if ( msgLevel(MSG::VERBOSE) )
  {
    verbose() << "Trying RichRecTrack " << track->key() << " " << track->trackID().trackType()
              << endmsg;
  }

  /** @todo Add fit status cut for RichRecTracks */

  // cut p
  const double p = track->vertexMomentum() / Gaudi::Units::GeV;
  if ( p < m_minPCut || p > m_maxPCut )
  {
    if ( msgLevel(MSG::VERBOSE) )
      verbose() << " -> P " << p << " failed cut "
                << m_minPCut << " -> " << m_maxPCut
                << endmsg;
    return false;
  }

  // cut on pt
  const double pt = track->vertexPt() / Gaudi::Units::GeV;
  if ( pt < m_minPtCut || pt > m_maxPtCut )
  {
    if ( msgLevel(MSG::VERBOSE) )
      verbose() << " -> Pt " << pt << " failed cut "
                << m_minPtCut << " -> " << m_maxPtCut
                << endmsg;
    return false;
  }

  // chi-squared
  const double chi2 = track->chi2PerDoF();
  if ( chi2 < m_minChi2Cut || chi2 > m_maxChi2Cut )
  {
    if ( msgLevel(MSG::VERBOSE) )
      verbose() << " -> Chi^2 " << chi2 << " failed cut "
                << m_minChi2Cut << " -> " << m_maxChi2Cut
                << endmsg;
    return false;
  }

  // track charge
  if ( m_chargeSel != 0 && m_chargeSel*track->charge() < 0 )
  {
    if ( msgLevel(MSG::VERBOSE) )
      verbose() << " -> Track charge " << track->charge() << " failed cut" << endmsg;
    return false;
  }

  // clones
  if ( m_cloneDistCutEnabled )
  {
    if ( !m_acceptClones && ( !track->trackID().unique() ||
                              track->cloneDist() < m_minCloneCut ||
                              track->cloneDist() > m_maxCloneCut ) )
    {
      if ( msgLevel(MSG::VERBOSE) )
        verbose() << " -> Track failed clone rejection" << endmsg;
      return false;
    }
  }

  // Likelihood
  if ( m_likelihoodCutEnabled )
  {
    if ( track->likelihood() < 99 && // check against default value in Track class. Accept in this case.
         ( track->likelihood() < m_minLL || track->likelihood() > m_maxLL ) )
    {
      if ( msgLevel(MSG::VERBOSE) )
        verbose() << " -> Track Likelihood " << track->likelihood() << " failed cut "
                  << m_minLL << " -> " << m_maxLL
                  << endmsg;
      return false;
    }
  }

  // Ghost Prob
  if ( m_ghostProbCutEnabled )
  {
    if ( track->ghostProbability() < 99 && // check against default value in Track class. Accept in this case.
         ( track->ghostProbability() < m_minGhostProb ||
           track->ghostProbability() > m_maxGhostProb ) )
    {
      if ( msgLevel(MSG::VERBOSE) )
        verbose() << " -> Track GhostProbability " << track->ghostProbability() << " failed cut "
                  << m_minGhostProb << " -> " << m_maxGhostProb
                  << endmsg;
      return false;
    }
  }

  // isolation
  if ( m_rejectNonIsolated )
  {
    if ( !isoTrackTool()->isIsolated(track) )
    {
      if ( msgLevel(MSG::VERBOSE) )
        verbose() << " -> Isolation Criteria failed" << endmsg;
      return false;
    }
  }

  if ( msgLevel(MSG::VERBOSE) ) verbose() << " -> Track selected" << endmsg;
  return true;
}

//=============================================================================

double BaseTrackSelector::minPCut()          const { return m_minPCut; }
double BaseTrackSelector::maxPCut()          const { return m_maxPCut; }
double BaseTrackSelector::minPtCut()         const { return m_minPtCut; }
double BaseTrackSelector::maxPtCut()         const { return m_maxPtCut; }
double BaseTrackSelector::minChi2Cut()       const { return m_minChi2Cut; }
double BaseTrackSelector::maxChi2Cut()       const { return m_maxChi2Cut; }
int    BaseTrackSelector::chargeSel()        const { return m_chargeSel; }
double BaseTrackSelector::minLikelihoodCut() const { return m_minLL; }
double BaseTrackSelector::maxLikelihoodCut() const { return m_maxLL; }
double BaseTrackSelector::minCloneDistCut()  const { return m_minCloneCut; }
double BaseTrackSelector::maxCloneDistCut()  const { return m_maxCloneCut; }
double BaseTrackSelector::minGhostProbCut()  const { return m_minGhostProb; }
double BaseTrackSelector::maxGhostProbCut()  const { return m_maxGhostProb; }
