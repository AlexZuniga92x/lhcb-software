// $Id$
// ============================================================================
// Include files
// ============================================================================
// STD & STL
// ============================================================================
#include <climits>
#include <sstream>
#include <algorithm>
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/Incident.h"
// ============================================================================
// GaudiAlg
// ============================================================================
#include "GaudiAlg/GaudiAlgorithm.h"
// ============================================================================
#ifdef __INTEL_COMPILER
#pragma warning(disable:1572) // non-pointer conversion ... may lose significant bits
#pragma warning(push)
#endif
// ============================================================================
// HltBase
// ============================================================================
#include "HltBase/HltUtils.h"
// ============================================================================
// LoKi
// ============================================================================
#include "LoKi/Algs.h"
#include "LoKi/UpgradeTool.h"
#include "LoKi/Combiner.h"
#include "LoKi/HelperTool.h"
// ============================================================================
// Local
// ============================================================================
#include  "LTTools.h"
#ifdef __INTEL_COMPILER
#pragma warning(pop)
#endif
// ============================================================================
/** @file
 *  Implementation file for class LoKi::Hlt1::UpgradeTool
 *  @date 2008-11-13
 *  @author Vanya BELYAEV
 */
// ============================================================================
// create the tool from info
// ============================================================================
LoKi::Hlt1::UpgradeTool::UpgradeTool
( const LoKi::Hlt1::UpgradeConf& info )
  : LoKi::Hlt1::HelperTool ( 1 )
  , m_config  ( info )
  , m_recoID  ( -1   )
  , m_upgrade (      )
//
{
  // retrive the tool
  m_upgrade = alg()->tool<ITracksFromTrack> ( trTool() ) ;
  //
  /// get the service
  SmartIF<IANNSvc> ann = LoKi::Hlt1::Utils::annSvc( *this ) ;
  //
  auto _info = ann->value(Gaudi::StringKey(std::string("InfoID")) , trTool()  );
  Assert( _info , " request for unknown Info ID : " + trTool() );
  //
  m_recoID = _info->second ;
  //
}
// ============================================================================
// anonymos namespace to hide some local tchnical details
namespace
{
  // ==========================================================================
#ifdef __INTEL_COMPILER
#pragma warning(disable:177) //  function .. was declared but never referenced
#endif
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-function"
#endif
  // ==========================================================================
  /// move LHCbIDs
  inline void moveIDs ( const LHCb::Track* tr1 , LHCb::Track* tr2 )
  {
    tr2->addSortedToLhcbIDs( tr1->lhcbIDs() );
  }
#ifdef __clang__
#pragma clang diagnostic pop
#endif
  // ==========================================================================
  /// move LHCbIDs
  inline void moveIDs ( const LHCb::Track* tr1 , std::vector<LHCb::Track*>& tracks )
  {
    for ( auto &track : tracks) moveIDs ( tr1 , track ) ;
  }
  // ==========================================================================
  /** is ancestor  ?
   *  simple structure to check the ancestors
   *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
   *  @date 2008-11-13
   */
  struct IsAncestor : public std::unary_function<const LHCb::Track*,bool>
  {
    // ========================================================================
    /// the default constructor is disabled
    IsAncestor() = delete;
    // ========================================================================
    /// constructor form the seed
    IsAncestor ( const LHCb::Track* seed ) : m_seed ( seed ) {}
    // ========================================================================
    bool operator() ( const LHCb::Track* track )
    {
      if ( !track ) { return false ; }
      auto& ancestors = track->ancestors() ;
      return std::any_of ( std::begin(ancestors) , std::end(ancestors)
                         , [&](const LHCb::Track* t)
                           { return t==m_seed; } );
    }
    // ========================================================================
  private:
    // ========================================================================
    /// the seed
    const LHCb::Track* m_seed ;
    // ========================================================================
  } ;
  // ==========================================================================
	/** is direct ancestor  ?
	*  simple structure to check the direct ancestor
	*/
	struct IsDirectAncestor : public std::unary_function<const LHCb::Track*,bool>
	{
		// ========================================================================
		/// the default constructor is disabled
		IsDirectAncestor() = delete;
		// ========================================================================
		/// constructor form the seed
		IsDirectAncestor ( const LHCb::Track* seed ) : m_seed ( seed ) {}
		// ========================================================================
		bool operator() ( const LHCb::Track* track )
		{
			if ( !track ) { return false ; }
			auto& ancestors = track->ancestors() ;
			return !ancestors.empty() && ancestors.back().target() == m_seed;
		}
		// ========================================================================
		private:
		// ========================================================================
		/// the seed
		const LHCb::Track* m_seed ;
		// ========================================================================
	} ;
}
// ==========================================================================
//                                                end of anonynmous namespace
// ============================================================================
// find the tracks within the recontructed
// ============================================================================
size_t LoKi::Hlt1::UpgradeTool::find
( const LHCb::Track*         seed    ,
  LHCb::Track::ConstVector&  tracks  ,
  LHCb::Track::Container*    otracks ) const
{
  //
  if ( !otracks ) {
    Error ("find(): LHCb::Track::Container* points to NULL!") ;
    return 0 ;
  }
  const size_t ntracks = tracks.size() ;
  std::copy_if( std::begin(*otracks), std::end(*otracks),
                std::back_inserter( tracks ) , IsAncestor{ seed }   ) ;
  return tracks.size() - ntracks ;
}
// ============================================================================
// find the tracks within the recontructed using direct ancestor
// (only called when normal find does not work)
// ============================================================================
size_t LoKi::Hlt1::UpgradeTool::findDirect
( const LHCb::Track*         seed    ,
  LHCb::Track::ConstVector&  tracks  ,
  LHCb::Track::Container*    otracks ) const
{
  //
  if ( !otracks ) {
    Error ("find(): LHCb::Track::Container* points to NULL!") ;
    return 0 ;
  }
  const size_t ntracks = tracks.size() ;
  std::copy_if( std::begin(*otracks), std::end(*otracks),
                std::back_inserter( tracks ) , IsDirectAncestor{ seed }   ) ;
  return tracks.size() - ntracks ;
}
// ============================================================================
// upgrade/reconstruct one track
// ============================================================================
StatusCode LoKi::Hlt1::UpgradeTool::reco
( const LHCb::Track*         seed    ,
  LHCb::Track::ConstVector&  tracks  ,
  LHCb::Track::Container*    otracks ) const
{
  typedef std::vector<LHCb::Track*> OUTPUT ;
  //
  OUTPUT out ;
  StatusCode sc = m_upgrade->tracksFromTrack ( *seed , out ) ;
  if ( sc.isFailure() )
  { return Error ( "Error from ITrackFromTrack tool '" + m_upgrade->name()+  "'" , sc, 0 ) ; }
  // post action:
  for ( LHCb::Track *trk : out )
  {
    if ( owner    () ) { trk->setType ( m_config.trType () )     ; }
    if ( moveIDs  () ) { ::moveIDs ( seed , trk )                ; }
    if ( moveAncs () ) { trk->addToAncestors ( seed )            ; }
    if ( moveInfo () ) { trk->setExtraInfo ( seed->extraInfo() ) ; }
  }
  //
  // This is nasty... use the size to indicate success or failure
  const_cast< LHCb::Track* >( seed )->addInfo ( recoID() , out.size() );
  // insert only "good" tracks into the stream
  //
  std::copy_if( std::begin(out) , std::end(out) ,
                std::back_inserter ( tracks ) , m_config ) ;
  //
  for ( const auto& track : out )
  {
    /// invalid or already registered track
    if ( !track || track->parent() != 0 ) { continue ; }
    /// force TES-registration if not done yet
    if ( !owner() ) { Warning ( "reco(): misconfiguration of 'Owner'-property! ignore" ); }
    if ( !otracks )
    { return Error ("reco(): LHCb::Track::Container* points to NULL!") ; }
     // finally: register
    otracks->insert ( track ) ;
  }
  //
  return StatusCode::SUCCESS ;                                       // RETURN
}
// ============================================================================
// upgrade set of tracks
// ============================================================================
StatusCode LoKi::Hlt1::UpgradeTool::upgrade
( const LHCb::Track::ConstVector&  itracks ,
  LHCb::Track::ConstVector&        output  ) const
{
  //
  Assert ( !(!m_upgrade) , "ITracksFromTrack* points to NULL!" ) ;
  Assert ( alg()         , "GaudiAlgorithm*   points to NULL!" ) ;

  // get or create the output
  LHCb::Track::Container* otracks = storedTracks( address() ) ;

  // ==========================================================================
  // loop over input tracks, upgrade track-by-track
  // ==========================================================================
  for (const LHCb::Track* seed : itracks )
  {
    if ( !seed ) { continue ; }                                  // CONTINUE
    StatusCode sc = iupgrade ( seed , output , otracks ) ;
    if ( sc.isFailure () )
    { Error ( "Error from iupgrade, skip track", sc, 0 ) ; continue ; }
    // ========================================================================
  } // end of the loop over input tracks
  // ==========================================================================
  // sort tracks ?
  if ( ptOrder() )
  { std::stable_sort ( std::begin(output) , std::end(output) , Hlt::SortTrackByPt() ) ; }
  //
  return StatusCode::SUCCESS ;                                        // RETURN
}
// ============================================================================
// upgrade one seed
// ============================================================================
StatusCode LoKi::Hlt1::UpgradeTool::upgrade
( const LHCb::Track*         itrack ,
  LHCb::Track::ConstVector&  output ) const
{
  //
  if ( !itrack )
  { return Error ( "Upgrade: LHCb::Track* points to NULL" ) ; }
  //
  Assert ( !(!m_upgrade) , "ITracksFromTrack* points to NULL!" ) ;
  Assert ( alg()         , "GaudiAlgorithm*   points to NULL!" ) ;
  //
  // get or create the output
  LHCb::Track::Container* otracks = storedTracks ( address() ) ;
  //
  StatusCode sc = iupgrade ( itrack , output , otracks ) ;
  if ( sc.isFailure() ) { return Error ( "upgrade: error from iupgrade" , sc, 0 ) ; }
  //
  // sort tracks ?
  if ( ptOrder() )
  { std::stable_sort ( std::begin(output) , std::end(output) , Hlt::SortTrackByPt() ) ; }
  //
  return StatusCode::SUCCESS ;
}
// ============================================================================
// upgrade one seed
// ============================================================================
StatusCode LoKi::Hlt1::UpgradeTool::iupgrade
( const LHCb::Track*         seed    ,
  LHCb::Track::ConstVector&  output  ,
  LHCb::Track::Container*    otracks ) const
{
  // not reconstructed yet ?
  double info = seed->info ( recoID () , -1 );
  if ( -1 == info )
  {
    // recontruct it!!!
    StatusCode sc = reco ( seed , output , otracks ) ;
    if ( sc.isFailure () )
    { return Error ( "Failure from ITrackFromTrack tool, skip track", sc, 0 ) ; }
  }
  // Previous reco failed, we're done
  else if ( 0 == info ) { return StatusCode::SUCCESS; }
  // We don't own the track, so if it's already flagged, just return the seed
  else if ( !owner() ) { output.push_back( seed ); }
  // find in the list of recontructed
  else
  {
     // Find previously upgraded tracks and check if the number is correct.
     size_t n = find ( seed , output , otracks ) ;
     if ( size_t(info + 0.5) != n )
     {
	output.erase( std::prev( std::end(output), n ), std::end(output) );
	// If unexpected number found check again using direct ancestors
	size_t n2 = findDirect ( seed , output , otracks ) ;
	if ( size_t(info + 0.5) != n2 )
	{
        	std::stringstream msg;
        	msg << trTool() << ": Number of previously upgraded tracks: " << size_t(info + 0.5)
        	    << " does not match number of found tracks: " << n << ".";
		//Assert( false, msg.str() ) ;
	}
     }
  }
  return StatusCode::SUCCESS ;
}
// ======================================================================
/*  upgrade the 'track'candidates
 *  @param itrack  input track/seeds
 *  @param otracks output container of upgraded tracks (cumulative)
 *  @return status code
 */
// ======================================================================
StatusCode LoKi::Hlt1::UpgradeTool::upgradeTracks
( const Hlt::Candidate::ConstVector& input  ,
  Hlt::Candidate::ConstVector&       output ) const
{
  //
  Assert ( !(!m_upgrade) , "ITracksFromTrack* points to NULL!" ) ;
  Assert ( 0 != alg()    , "GaudiAlgorithm*   points to NULL!" ) ;
  //
  LHCb::Track::Container*     otracks = storedTracks ( address () ) ;
  //
  // loop over input candidates, upgrade one-by-one
  for ( const Hlt::Candidate* candidate : input ) {
    if ( !candidate )
    { Error ( "Invalid Hlt::Candidate, skip it!") ; continue ; } // CONTINUE
    const Hlt::Stage*     stage  = candidate->currentStage() ;
    if ( !stage     )
    { Error ( "Invalid Hlt::Stage,     skip it!") ; continue ; } // CONTINUE
    //
    if ( !stage->is<LHCb::Track> () )
    { Error ( "No      LHCb::Track,    skip it!") ; continue ; } // CONTINUE
    //
    // upgrade single track
    StatusCode sc = _i_upgrade_1track
      ( candidate   ,
        output      ,
        otracks     ) ;
    //
    if ( sc.isFailure() ) { Error ( "Error from i_upgrade_1track", sc, 0 ) ; }
    //
  } //
  //
  return StatusCode::SUCCESS ;
}
// ======================================================================
/*  upgrade the candidates
 *  @param itrack  input track/seeds
 *  @param otracks output container of upgraded tracks (cumulative)
 *  @return status code
 */
// ======================================================================
StatusCode LoKi::Hlt1::UpgradeTool::upgradeMultiTracks
( const Hlt::Candidate::ConstVector& input  ,
  Hlt::Candidate::ConstVector&       output ) const
{
  //
  Assert ( !(!m_upgrade) , "ITracksFromTrack* points to NULL!" ) ;
  Assert ( 0 != alg()    , "GaudiAlgorithm*   points to NULL!" ) ;
  //
  LHCb::Track::Container* otracks = storedTracks ( address () ) ;
  //
  // loop over input candidates, upgrade one-by-one
  for ( const Hlt::Candidate* candidate : input ) {
    if ( !candidate )
    { Error ( "Invalid Hlt::Candidate, skip it!") ; continue ; } // CONTINUE
    const Hlt::Stage*     stage  = candidate->currentStage() ;
    if ( !stage     )
    { Error ( "Invalid Hlt::Stage,     skip it!") ; continue ; } // CONTINUE
    //
    // upgrade single track
    if ( !stage->is<Hlt::MultiTrack>() )
    { Error ( "No Hlt::MultiTrack,     skip it!") ; continue ; } // CONTINUE
    //
    StatusCode sc = _i_upgrade_multi_track
      ( candidate     ,
        output        ,
        otracks       ) ;
    //
    if ( sc.isFailure() ) { Warning( "Error from i_uprgade_multi_track", sc, 0 ) ; }
    //
  } //                                      end of the loop over new candidates
  //
  return StatusCode::SUCCESS ;
}
// ======================================================================
/*  upgrade the candidates
 *  @param itrack  input track/seeds
 *  @param otracks output container of upgraded tracks (cumulative)
 *  @return status code
 */
// ======================================================================
StatusCode LoKi::Hlt1::UpgradeTool::upgradeMultiTracks
( const Hlt::Candidate::ConstVector& input  ,
  const unsigned int                 index  ,
  Hlt::Candidate::ConstVector&       output ) const
{
  //
  Assert ( !(!m_upgrade) , "ITracksFromTrack* points to NULL!" ) ;
  Assert ( 0 != alg()    , "GaudiAlgorithm*   points to NULL!" ) ;
  //
  LHCb::Track::Container* otracks = storedTracks ( address () ) ;
  //
  // loop over input candidates, upgrade one-by-one
  for ( const Hlt::Candidate* candidate : input ) {
    if ( ! candidate )
    { Error ( "Invalid Hlt::Candidate, skip it!") ; continue ; } // CONTINUE
    const Hlt::Stage*     stage  = candidate->currentStage() ;
    if ( !stage     )
    { Error ( "Invalid Hlt::Stage,     skip it!") ; continue ; } // CONTINUE
    //
    // upgrade single track
    if ( !stage->is<Hlt::MultiTrack>() )
    { Error ( "No Hlt::MultiTrack,     skip it!") ; continue ; } // CONTINUE
    //
    StatusCode sc = _i_upgrade_multi_track_j
      ( candidate     ,
        index         ,
        output        ,
        otracks       ) ;
    //
    if ( sc.isFailure() ) { Warning( "Error from i_uprgade_multi_track", sc, 0 ) ; }
    //
  } //                                      end of the loop over new candidates
  //
  return StatusCode::SUCCESS ;
}
// ======================================================================
/*  upgrade the candidates
 *  @param itrack  input track/seeds
 *  @param otracks output container of upgraded tracks (cumulative)
 *  @return status code
 */
// ======================================================================
StatusCode LoKi::Hlt1::UpgradeTool::upgradeAll
( const Hlt::Candidate::ConstVector& input  ,
  Hlt::Candidate::ConstVector&       output ) const
{
  //
  Assert ( !(!m_upgrade) , "ITracksFromTrack* points to NULL!" ) ;
  Assert ( 0 != alg()    , "GaudiAlgorithm*   points to NULL!" ) ;
  //
  LHCb::Track::Container* otracks = storedTracks ( address() ) ;
  //
  // loop over input candidates, upgrade one-by-one
  for ( const Hlt::Candidate* candidate : input ) {
    if ( ! candidate )
    { Error ( "Invalid Hlt::Candidate, skip it!") ; continue ; } // CONTINUE
    const Hlt::Stage*     stage  = candidate->currentStage() ;
    if ( ! stage     )
    { Error ( "Invalid Hlt::Stage,     skip it!") ; continue ; } // CONTINUE
    //
    // upgrade single track
    if     ( stage->is<LHCb::Track>     () )
    {
      StatusCode sc = _i_upgrade_1track
        ( candidate   ,
          output      ,
          otracks     ) ;
      if ( sc.isFailure() ) { Warning( "Error from i_uprgade_track", sc, 0 ) ; }
    }
    else if ( stage->is<Hlt::MultiTrack>() )
    {
      StatusCode sc = _i_upgrade_multi_track
        ( candidate     ,
          output        ,
          otracks       ) ;
      if ( sc.isFailure() ) { Warning( "Error from i_uprgade_multi_track", sc, 0 ) ; }
    }
    else { Warning("Invalid type for track-upgrade!") ; }
    // ========================================================================
  } //                                      end of the loop over new candidates
  // ==========================================================================
  return StatusCode::SUCCESS ;
}
// ============================================================================
/*  upgrade the candidate
 *  @param itrack  input track/seeds
 *  @param otracks output container of upgraded tracks (cumulative)
 *  @return status code
 */
// ============================================================================
StatusCode LoKi::Hlt1::UpgradeTool::_i_upgrade_1track
( const Hlt::Candidate*              input      ,
  Hlt::Candidate::ConstVector&       output     ,
  LHCb::Track::Container*            otracks    ) const
{
  //
  if ( ! input ) { return Error ( "Hlt::Candidate* points to NULL") ; }
  //
  const Hlt::Stage* stage = input->currentStage() ;
  if ( ! stage || !stage->is<LHCb::Track> () )
  { return Error ( "Invalid Hlt::Stage*" ) ; }
  //
  const LHCb::Track*    seed = stage->get<LHCb::Track> () ;
  //
  LHCb::Track::ConstVector out ;
  StatusCode sc = iupgrade ( seed , out , otracks ) ;              // USE TOOL
  if ( sc.isFailure () )
  { return Error ( "Error from iupgrade, skip track", sc, 0 ) ; }    // RETURN
  //
  // Process output tracks, create a new candidate for tracks beyond the first
  for ( LHCb::Track::ConstVector::const_iterator iout = std::begin(out) ;
        std::end(out) != iout ; ++iout ) {
     const LHCb::Track* track = *iout ;
     if ( !track ) continue ;
     //
     Hlt::Candidate* _input = 0;
     if ( iout == out.begin() ) {
        _input = const_cast<Hlt::Candidate*>  ( input ) ;
     } else {
        //
        // start new Candidate:
        _input = newCandidate() ;
        _input -> addToWorkers ( alg() ) ;
        //
        // the initiator of new candidate is the stage of the initial candidate:
        Hlt::Stage* stage1 = newStage() ;
        _input -> addToStages ( stage1 ) ;
        //
        Hlt::Stage::Lock lock1 ( stage1, upgradeTool() ) ;
        lock1.addToHistory ( input->workers() ) ;
        // lock1.addToHistory ( this ) ;
        stage1 -> set ( stage ) ; // add stage into stage as initiator
     }
     if ( track != seed ) {
        // Add the new stage to the candidate
        Hlt::Stage* newstage = newStage() ;
        //
        Hlt::Stage::Lock lock ( newstage , upgradeTool() ) ;
        newstage -> set( track ) ; // add track to the stage
        _input   -> addToStages ( newstage ) ;
     }
     _input   -> addToWorkers ( alg() ) ;
     output.push_back ( _input ) ;                                 // OUTPUT++
  }  //                                     end of the loop over output tracks
  return StatusCode::SUCCESS ;
}
// ============================================================================
/*  upgrade the candidate
 *  @param itrack  input track/seeds
 *  @param otracks output container of upgraded tracks (cumulative)
 *  @return status code
 */
// ============================================================================
StatusCode LoKi::Hlt1::UpgradeTool::_i_upgrade_multi_track
( const Hlt::Candidate*              input        ,
  Hlt::Candidate::ConstVector&       output       ,
  LHCb::Track::Container*            otracks      ) const
{
  //
  // FIXME, check/update this code to the latest "model"
  // return StatusCode::FAILURE;
  return Error  ( "FIXME, check/update this code to the latest" ) ;
  //
  //
  if ( !input ) { return Error("Hlt::Candidate points to NULL") ; }
  const Hlt::Stage* stage = input->currentStage() ;
  //
  if ( !stage || !stage->is<Hlt::MultiTrack> () )
  { return Error("Invalid Hlt::Stage*" ) ; }
  //
  const Hlt::MultiTrack* multi_track = stage->get<Hlt::MultiTrack> () ;
  //
  typedef std::vector<LHCb::Track::ConstVector> OUTs ;

  bool OK    = true  ;
  bool split = false ;
  OUTs outs ;

  for ( const auto& track : multi_track->tracks() )
  {
    LHCb::Track::ConstVector out ;
    StatusCode sc = iupgrade( track , out , otracks ) ;
    if ( sc.isFailure() )
    { Error    ( "Error from iupgrade", sc, 0 ); OK = false; break; } // BREAK
    if ( out.empty()    )
    { Warning  ( "No tracks", sc, 0)           ; OK = false; break; } // BREAK
    //
    if ( 1 != out.size() ) { split = true ; }
    //
    outs.push_back ( out ) ;
  }
  //
  if ( !OK || multi_track->tracks().size() != outs.size() )
  { return Error ( "Upgrade failure", StatusCode::FAILURE, 0 ) ; }   // RETURN
  //
  if ( !split )
  {
    // new multi-track
    Hlt::MultiTrack* mtrack = newMultiTrack() ;
    //
    for ( const auto& out : outs )  mtrack->addToTracks( out.front() ) ;
    //
    // new stage
    Hlt::Stage* newstage = newStage() ;
    //
    Hlt::Stage::Lock lock ( newstage , upgradeTool () ) ;
    // lock.addToHistory ( this ) ;
    newstage -> set( mtrack ) ; // add multitrack to the stage
    Hlt::Candidate* _input = const_cast<Hlt::Candidate*>  ( input ) ;
    _input -> addToWorkers ( alg()    ) ;
    _input -> addToStages  ( newstage ) ;
    output.push_back ( input ) ;                               // OUTPUT++
  } else {
    //
    Warning ( "Reconstruction causes split of candidate" ) ;
    //
    LoKi::Combiner_<LHCb::Track::ConstVector> combiner ;
    //
    // fill combiner with data:
    for ( const auto& out : outs )  combiner.add( LHCb::Track::Range ( out ) ) ;
    //
    // start looping over all combinations
    for ( ; combiner.valid() ; ++combiner )
    {
      //
      Hlt::Candidate*  newcand   = newCandidate  () ;
      newcand  -> addToWorkers ( alg () ) ;
      //
      Hlt::Stage*      newstage1 = newStage      () ;
      Hlt::Stage*      newstage2 = newStage      () ;

      Hlt::MultiTrack* newtrack  = newMultiTrack () ;
      //
      Hlt::Stage::Lock lock1 ( newstage1 , upgradeTool () ) ;
      lock1.addToHistory ( input->workers() ) ;
      // lock1.addToHistory ( this ) ;
      newstage1 -> set<Hlt::Stage>      ( stage    ) ;
      //
      Hlt::Stage::Lock lock2 ( newstage1 , upgradeTool () ) ;
      // lock2.addToHistory ( this ) ;
      newstage2 -> set<Hlt::MultiTrack> ( newtrack ) ;
      //
      // fill multi-track:
      for ( auto ia : combiner.current() )  newtrack->addToTracks ( *ia ) ;
      //
      newcand -> addToStages ( newstage1 ) ;
      newcand -> addToStages ( newstage2 ) ;
      //
      output.push_back ( newcand ) ;
    }
  }
  return StatusCode::SUCCESS ;
}
// ============================================================================
/*  upgrade the candidate
 *  @param itrack  input track/seeds
 *  @param otracks output container of upgraded tracks (cumulative)
 *  @return status code
 */
// ============================================================================
StatusCode LoKi::Hlt1::UpgradeTool::_i_upgrade_multi_track_j
( const Hlt::Candidate*              input        ,
  const unsigned int                 index        , // the index
  Hlt::Candidate::ConstVector&       output       ,
  LHCb::Track::Container*            otracks      ) const
{
  // FIXME, check/update this code to the latest "model"
  // return StatusCode::FAILURE;
  return Error ( "FIXME, check/update this code to the latest ") ;
  //
  //
  if ( 0 == input ) { return Error("Hlt::Candidate points to NULL") ; }
  const Hlt::Stage* stage = input->currentStage() ;
  //
  if ( 0 == stage || !stage->is<Hlt::MultiTrack> () )
  { return Error("Invalid Hlt::Stage*" ) ; }
  //
  const Hlt::MultiTrack* multi_track = stage->get<Hlt::MultiTrack> () ;
  //

  typedef std::vector<LHCb::Track::ConstVector> OUTs ;


  const Hlt::MultiTrack::Tracks& tracks = multi_track->tracks() ;

  //
  if ( tracks.size() <= index )
  { return Error( "Invalid size of track-vector") ; }

  // get the track:
  const LHCb::Track* track = tracks[index] ;
  if ( 0 == track )
  { return Error ( "Invalid track by index" ) ; }

  // upgrade it!
  LHCb::Track::ConstVector out ;
  StatusCode sc = iupgrade ( track , out , otracks ) ;
  if ( sc.isFailure() ) { return Error    ( "Error from iupgrade", sc, 0 ) ; }
  if ( out.empty()    ) { return Warning  ( "No tracks", sc, 0 )           ; }
  //

  for ( LHCb::Track::ConstVector::const_iterator iout = out.begin() ;
        out.end() != iout ; ++iout )
  {
    const LHCb::Track* _track = *iout ;
    if ( 0 == _track ) { continue ; }
    //
    Hlt::Candidate* newcand = 0 ;
    if ( iout == out.begin() ) { newcand = const_cast<Hlt::Candidate*> (  input ) ; }
    //
    if ( 0 == newcand )
    {
      newcand              = newCandidate () ;
      newcand  -> addToWorkers ( alg()    ) ;
      //
      Hlt::Stage* newstage = newStage       () ;
      newcand  -> addToStages  ( newstage ) ;
      //
      Hlt::Stage::Lock lock ( newstage , upgradeTool () ) ;
      lock.addToHistory ( input -> workers() ) ;
      // lock.addToHistory ( myName() ) ;
      newstage -> set<Hlt::Stage>      ( stage ) ;
      //
    }
    //
    Hlt::Stage*      newstage   = newStage        () ;
    newcand  -> addToStages  ( newstage ) ;
    //
    Hlt::MultiTrack* newmtrack  = newMultiTrack   ()  ;
    Hlt::Stage::Lock lock ( newstage , upgradeTool () ) ;
    // lock.addToHistory ( myName() ) ;
    newstage -> set<Hlt::MultiTrack>  ( newmtrack ) ;
    //
    // fill multi-track:
    LHCb::Track::ConstVector _tracks ( std::begin(tracks) , std::end(tracks) ) ;
    _tracks [index] = _track ;
    //
    for ( const auto& i : _tracks ) newmtrack -> addToTracks ( i ) ;
    //
    output.push_back ( newcand ) ;
  }
  //
  return StatusCode::SUCCESS ;

}
// ============================================================================
std::string Gaudi::Utils::toCpp ( const LoKi::Hlt1::UpgradeTool& o )
{ return " LoKi::Hlt1::UpgradeTool( " + toCpp ( o.config () ) + ") "; }

// ============================================================================
// The END
// ============================================================================
