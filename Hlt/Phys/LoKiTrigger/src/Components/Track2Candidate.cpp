// $Id: $
// ============================================================================
// Include files 
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/AlgFactory.h"
// ============================================================================
// Event 
// ============================================================================
#include "Event/Track.h"
// ============================================================================
// HltBase 
// ============================================================================
#include "Event/HltCandidate.h"
// ============================================================================
// LoKi
// ============================================================================
#include "LoKi/HltBase.h"
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
 * @author Vanya BELYAEV Ivan.Belyaev@cern.ch
 * @date 2010-11-18
 *  
 *                   $Revision$
 *  Last moficiation $Date$
 *                by $Author$
 */
// ============================================================================
namespace Hlt 
{
  // ==========================================================================
  /** @class Track2Candidate
   *  Simple class which converts tarcks into Hlt::Candidate
   *  @see Hlt::Candidate
   *  @author Vanya BELYAEV Ivan.Belyaev@cern.ch
   *  @date 2010-11-18
   */
  class Track2Candidate : public Hlt::Base 
  {
    // ========================================================================
    /// friend factory for instantiation 
    friend class AlgFactory<Hlt::Track2Candidate> ;
    // ========================================================================    
  public:
    // ========================================================================
    /// initialize the algorithm 
    virtual StatusCode initialize () ;
    /// execute the algorithm 
    virtual StatusCode execute    () ;
    /// finalize the algorithm 
    virtual StatusCode finalize   () ;
    /// =======================================================================
  protected:
    // ========================================================================
    /** standard constructor  
     *  @param name algorithm instance name 
     *  @param pSvc pointer to Service Locator 
     */
    Track2Candidate
    ( const std::string& name ,                  //     algorithm instance name 
      ISvcLocator*       pSvc ) ;                 //  pointer to Service Locator 
    /// virtual and protected destructor 
    virtual ~Track2Candidate () ;
    // ========================================================================
  private:
    // ========================================================================
    /// the default constructor is disabled 
    Track2Candidate () ;                              // no default constructor
    /// the copy constructor is disabled 
    Track2Candidate ( const Track2Candidate& ) ;         // no copy constructor 
    /// the assignement operator is disabled 
    Track2Candidate & operator=( const Track2Candidate& ) ;   // no assignement
    // ========================================================================
  private:
    // ========================================================================
    /// the selection 
    Hlt::TSelection<Hlt::Candidate>* m_selection ;             // the selection 
    /// the output selection 
    Location m_output         ;                                //    the output
    /// the input tarck location in TES 
    Location m_input ;                       // the input tarck location in TES 
    // ========================================================================
  } ;
  // ==========================================================================
} //                                                       end of namespace Hlt 
// ============================================================================
/*  standard constructor  
 *  @param name algorithm instance name 
 *  @param pSvc pointer to Service Locator 
 */
// ============================================================================
Hlt::Track2Candidate::Track2Candidate
( const std::string& name ,                  //     algorithm instance name 
  ISvcLocator*       pSvc )                  //  pointer to Service Locator 
  : Hlt::Base   ( name , pSvc ) 
  , m_selection ( 0    ) 
  , m_output    ( name ) 
  , m_input     ( "<UNDEFINED>") 
//
{
  declareProperty 
    ( "OutputSelection"                 , 
      m_output                          ,
      "The name of output selection of Hlt::Candidates"    ) ;
  declareProperty
    ( "InputSelection"                  , 
      m_input                           , 
      "TES Location of inptut tracks"   ) ;
}
// ============================================================================
// virtual and protected destructor 
// ============================================================================
Hlt::Track2Candidate::~Track2Candidate(){}
// ============================================================================
// initialize the algorithm 
// ============================================================================
StatusCode Hlt::Track2Candidate::initialize () 
{
  /// initialize the base 
  StatusCode sc = Hlt::Base::initialize () ;
  if ( sc.isFailure() ) { return sc ; }                          // REUTRN
  /// Lock the service to enable the output selection registration 
  Hlt::IRegister::Lock lock ( regSvc() , this ) ;
  /// register TES input selection
  sc = lock -> registerTESInput ( m_input     , this ) ;
  Assert ( sc.isSuccess () , "Unable to register INPUT  selection" , sc ) ;
  /// register the output selection
  m_selection = new Hlt::TSelection<Hlt::Candidate>( m_output ) ;
  sc = lock -> registerOutput ( m_selection , this ) ;
  Assert ( sc.isSuccess () , "Unable to register OUTPUT selection" , sc );
  //
  declareInfo ( "#accept" , "" , &counter("#accept") , 
                0, std::string ("Events accepted by "    ) + name () ) ;
  declareInfo ( "#input"  , "" , &counter("#input" ) ,
                0, std::string ("Candidates seen by "    ) + name () ) ;
  declareInfo ( "#output" , "" , &counter("#output") ,
                0, std::string ("Candidates accepted by ") + name () ) ;
  //
  return StatusCode::SUCCESS ;
}
// ============================================================================
// execute the algorithm 
// ============================================================================
StatusCode Hlt::Track2Candidate::execute  () 
{
  // some sanity checks:
  Assert ( 0 != m_selection , "Invalid Local pointer to selection" ) ;
  if ( !m_selection->empty() ) { Warning("Local selection is not empty!") ; }
  
  // get all L0 Muons from TES  
  typedef LHCb::Track::Container Tracks;
  const Tracks* tracks = tesData<Tracks> ( m_input ) ;
  
  // get(or create) containter of Hlt-Candidates from TES
  Hlt::Candidate::Container*  candidates = hltCandidates () ;
  // get(or create) containter of Hlt-Stages     from TES
  Hlt::Stage::Container*      stages     =  hltStages    () ;
  
  // loop over tracks
  for ( Tracks::const_iterator itrack = tracks->begin() ; 
        tracks->end() != itrack ; ++itrack) 
  {
    const LHCb::Track* track = *itrack ;
    if ( 0 == track ) { continue ; }                                  // CONTINUE 
    //                                          
    // create the new candidate:
    Hlt::Candidate* candidate = new Hlt::Candidate() ;
    //
    candidate  -> addToWorkers ( m_input ) ;
    candidate  -> addToWorkers ( this    ) ;
    candidates -> push_back ( candidate )  ; // add candidate into TES constainer 
    //
    // create the stage
    Hlt::Stage* stage = new Hlt::Stage() ;
    // add it into candidate 
    stages     -> push_back ( stage    )   ;  // add stage  into TES container 
    // add it into Candidate! 
    candidate  -> addToStages ( stage ) ;  // add stage to candidate 
    
    // lock the stage!
    Hlt::Stage::Lock lock ( stage , this ) ;
    lock.addToHistory ( m_input ) ;
    stage->set ( track ) ;
    
    // insert the candidate into output selection 
    m_selection->push_back( candidate ) ;
  }
  //
  // a bit of monitoring 
  counter ( "#input"  ) +=  tracks      -> size  () ;
  counter ( "#output" ) +=  m_selection -> size  () ;
  counter ( "#accept" ) += !m_selection -> empty () ;
  m_selection -> setDecision( !m_selection->empty() );
  
  setFilterPassed ( ! m_selection -> empty() ) ;
  
  return StatusCode::SUCCESS ;
}
// ============================================================================
// finalize the algorithm 
// ============================================================================
StatusCode Hlt::Track2Candidate::finalize () 
{
  m_selection = 0 ;
  return Hlt::Base::finalize () ;
}
// ============================================================================
// the algorithm factory
// ============================================================================
DECLARE_NAMESPACE_ALGORITHM_FACTORY(Hlt,Track2Candidate)
// ============================================================================
// The END 
// ============================================================================  
