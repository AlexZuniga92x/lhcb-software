// $Id$
// ============================================================================
// Include files 
// ============================================================================
// STD & STL 
// ============================================================================
#include <vector>
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/StatusCode.h"
#include "GaudiKernel/StringKey.h"
#include "GaudiKernel/AlgFactory.h"
// ============================================================================
// Event 
// ============================================================================
#include "Event/L0MuonCandidate.h"
#include "Event/L0DUReport.h"
// ============================================================================
// LoKi
// ============================================================================
#include "LoKi/HltBase.h"
// ============================================================================
// Local 
// ============================================================================
#include "L0Muon2TrackBase.h"
// ============================================================================
/* @file
 *
 *  This file is a part of LoKi project - 
 *    "C++ ToolKit  for Smart and Friendly Physics Analysis"
 *
 *  The package has been designed with the kind help from
 *  Galina PAKHLOVA and Sergey BARSUK.  Many bright ideas, 
 *  contributions and advices from G.Raven, J.van Tilburg, 
 *  A.Golutvin, P.Koppenburg have been used in the design.
 *
 *  By usage of this code one clearly states the disagreement 
 *  with the campain of Dr.O.Callot et al.: 
 *  ``No Vanya's lines are allowed in LHCb/Gaudi software.''
 *
 *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
 *  @date 2000-03-19
 *  
 *                   $Revision$
 *  Last moficiation $Date$
 *                by $Author$
 */
// ============================================================================
namespace Hlt
{
  // ==========================================================================
  /** @class L0Muon2Track
   *  Simple class which converts L0Muon candidates into "tracks" using 
   *  the special tool by Johannes albrecht 
   *  @see IMuonSeedTrack
   *  The actual lines are stolen from 
   *     Gerhard Raven & Jose Angel Hernando  Morata
   *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
   *  @date 2000-03-19
   */
  class L0Muon2Track : public Hlt::L0Muon2TrackBase
  {
    // ========================================================================
    /// the friend factory for instantiation
    friend class AlgFactory<Hlt::L0Muon2Track> ;
    // ========================================================================
  public:
    // ========================================================================
    /// initialize the algorithm 
    virtual StatusCode initialize () 
    {
      /// initialize the base 
      StatusCode sc = Hlt::L0Muon2TrackBase::initialize() ;      
      if ( sc.isFailure() ) { return sc ; }          // REUTRN
      //
      return StatusCode::SUCCESS ;
    }
    /// execute the algorithm 
    virtual StatusCode execute  () ;
    /// finalize the algorithm 
    virtual StatusCode finalize ()
    { return Hlt::L0Muon2TrackBase::finalize () ; }
    /// =======================================================================
  protected:
    // ========================================================================
    /** standard constructor  
     *  @param name algorithm instance name 
     *  @param pSvc pointer to Service Locator 
     */
    L0Muon2Track
    ( const std::string& name ,                  //     algorithm instance name 
      ISvcLocator*       pSvc )                  //  pointer to Service Locator 
      : Hlt::L0Muon2TrackBase( name , pSvc ) 
    {}    
    /// virtual and protected destructor 
    virtual ~L0Muon2Track() {}
    // ========================================================================
  private:
    // ========================================================================
    const Hlt::Candidate* checkMuon 
    ( const LHCb::Track*                     muon       ,
      const Hlt::TSelection<Hlt::Candidate>* candidates ) const 
    {
      if ( 0 == muon       ) { return 0 ; }
      if ( 0 == candidates ) { return 0 ; }
      for ( Hlt::TSelection<Hlt::Candidate>::const_iterator icand = 
              candidates->begin() ; candidates->end() != icand ; ++icand ) 
      {
        const Hlt::Candidate* candidate = *icand ;
        if ( 0 == candidate ) { continue ; }
        typedef  Hlt::Candidate::Stages Stages ;
        const Stages& stages = candidate->stages() ;
        for ( Stages::const_iterator istage = stages.begin() ; 
              stages.end() != istage ; ++istage ) 
        {
          const Hlt::Stage* stage = *istage ;
          if ( 0 == stage ) { continue ; }
          //
          if ( muon == stage->get<LHCb::Track>() ) { return candidate ; }
        } 
      }
      //
      return 0 ;
    }
    // ========================================================================
  private:
    // ========================================================================
    /// the default constructor is disabled 
    L0Muon2Track () ;                    // the default constructor is disabled 
    /// the copy constructor is disabled 
    L0Muon2Track ( const L0Muon2Track& ) ;  // the copy constructor is disabled 
    /// the assignement operator is disabled 
    L0Muon2Track& operator=( const L0Muon2Track& ) ; // assignement is disabled
    // ========================================================================
  protected:
    // ========================================================================
    bool checkClone ( const LHCb::L0MuonCandidate&  muon) const ;
    // ========================================================================
  } ;
  // ==========================================================================
} //                                                       end of namespace Hlt 
// ============================================================================
// execute the algorithm 
// ============================================================================
StatusCode Hlt::L0Muon2Track::execute  () 
{ 
  typedef  Hlt::TSelection<Hlt::Candidate> Input ;
  
  /// get input selection from Hlt-service 
  const Input* input = hltSvc()->get<Hlt::Candidate>  ( m_input , this ) ;
  Assert ( 0 != input           , "Input  selection is invalid!" ) ;
  
  /// check the output selectiom
  Assert ( m_selection->empty() , "Output selection is not empty!" ) ;
  
  /// create the container of muons/tracks and register it in TES 
  LHCb::Track::Container* muons = getMuons  () ;
  
  /// constaiener of stages 
  Hlt::Stage::Container* stages = hltStages () ;
  
  
  /// why we shodul decode it for each event ????
  std::pair<LoKi::L0::L0MuonCut,bool> _cuts = muonCut() ;
  
  const LoKi::L0::L0MuonCut cut     = _cuts.first  ;
  const bool                noMuon  = _cuts.second ;
  
  if ( noMuon ) {}
  else 
  {
    // loop over input data
    for ( Input::const_iterator item = input->begin() ; 
          input->end() != item ; ++item  )
    {
      const Hlt::Candidate*   candidate = *item ;
      if ( 0 == candidate ) { continue ; }
      //
      const Hlt::Stage* stage     = candidate->currentStage() ;
      if ( 0 == stage     ) { continue ; }
      //  
      const LHCb::L0MuonCandidate* l0muon = stage->get<LHCb::L0MuonCandidate>() ;
      //
      debug() << "l0pt " << l0muon->pt() << " l0encodedPt " << l0muon->encodedPt()<< endmsg;
      // check the cut:
      if ( 0 == l0muon || !cut ( l0muon) ) { continue ; }     // CONTINUE 
      debug() << "l0pt " << l0muon->pt() << " l0encodedPt " << l0muon->encodedPt()<< " accept " << endmsg;
      
      
      // Has been this L0MuonCandidate already converted into track?
      const LHCb::Track* track = seekMuon ( *l0muon , muons  ) ;
      
      // it is already selected and it is clone ? 
      if      ( 0 != track && 0 != checkMuon ( track , m_selection ) )
      {
        debug() << "is clone " << endmsg;
        continue ;
      }
      
      // there is no track 
      if ( 0 == track )
      {
        track = makeMuon ( *l0muon , muons ) ;
        if ( 0 == track ) { continue ; }
      }
      
      //
      // keep it: create new stage 
      //
      
      Hlt::Stage* newstage = new Hlt::Stage() ;
      stages      -> push_back   ( newstage ) ;
      const_cast<Hlt::Candidate*>(candidate)   -> addToStages ( newstage ) ;
      //
      Hlt::Stage::Lock lock    ( newstage , this  ) ;
      newstage    -> set       ( track            ) ;
      //
      m_selection -> push_back ( candidate ) ; 
    }
  }
  
  if (msgLevel(MSG::DEBUG)) 
  {
    debug() << "# Input: "      << input       -> size() 
            << " -> # Output: " << m_selection -> size() << endreq;
  }
  
  counter ( "#input"  ) +=  input       -> size  () ;
  counter ( "#output" ) +=  m_selection -> size  () ;
  counter ( "#accept" ) += !m_selection -> empty () ;
  
  //
  m_selection -> setDecision ( !m_selection->empty() );
  
  setFilterPassed ( !m_selection->empty() ) ;
  
  return StatusCode::SUCCESS ;
}
// ============================================================================
// the algorithm factory
// ============================================================================
DECLARE_NAMESPACE_ALGORITHM_FACTORY(Hlt,L0Muon2Track)
// ============================================================================
// The END 
// ============================================================================
