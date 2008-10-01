// $Id: FilterDesktop.cpp,v 1.6 2008-10-01 13:23:38 ibelyaev Exp $
// ============================================================================
// Include files 
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/AlgFactory.h"
#include "GaudiKernel/StatEntity.h"
// ============================================================================
// DaVinci Kernel
// ============================================================================
#include "Kernel/DVAlgorithm.h"
#include "Kernel/IPlotTool.h"
// ============================================================================
// LoKi
// ============================================================================
#include "LoKi/PhysTypes.h"
#include "LoKi/IHybridFactory.h"
// ============================================================================
// Boost 
// ============================================================================
#include "boost/algorithm/string.hpp"
// ============================================================================
namespace 
{
  /// make the plots ? 
  inline bool validPlots ( std::string name ) 
  {
    if ( name.empty() ) { return false ; }
    boost::to_lower ( name ) ;
    return "none" != name ;
  }
}
// ============================================================================
using namespace LoKi ;
// ============================================================================
/** @class FilterDesktop 
 *  LoKi/Bender "Hybrid" (re)implementation of simple algorithm with 
 *  filters the input particles ("FilterDesktop")
 *
 *  The important properties (in addtion to the base class' properties)
 *    - "Factory"   : the type/name of LoKi/Bender 'hybrid' factory
 *    - "Preambulo" : the preambulo, to eb used for Bender/Python script
 *    - "Code"      : the actual Bender/Python code 
 *    - "InputPlotsTool"   : the type/name of PlotTool for 'input' particles 
 *    - "InputPlotsPath"   : THS path for 'input' plots 
 *    - "OutputPlotsTool"  : the type/name of PlotTool for 'output' particles 
 *    - "OutputPlotsPath"  : THS path for 'output' plots 
 *    - "CloneFinalStates" : ? 
 *    - "CloneDaughetrs"   : ? 
 *    - "CloneTree"        : ? 
 *
 *  The important counters (in addition to counters form DVAlgorithm)
 *    - "#inputs"    : number of inptu particles 
 *    - "#passed"    : number of particles, which passed criteria
 *    - "efficiency" : the selection efficiency (per particle) 
 *  
 *  @see DVAlgorithm 
 *  @see CombineParticles 
 *  @see IHybridFactory
 * 
 *  @author Vanya BELYAEV Ivan.Belyaev@nikhef,nl
 *  @date 2008-09-22
 */
class FilterDesktop : public DVAlgorithm
{
  // ==========================================================================
  /// friend factory for instantiation 
  friend class AlgFactory<FilterDesktop> ;
  // ==========================================================================
public:
  // ==========================================================================
  /// the specific initialization 
  virtual StatusCode initialize ()             // the specific initialization 
  {
    // initialize the base class 
    StatusCode sc = DVAlgorithm::initialize () ;
    if ( sc.isFailure () ) { return sc ; }                          // RETURN 
    // ensure the proper initialization of LoKi service 
    svc<IService> ( "LoKiSvc" ) ;
    // decode the cut:  
    sc = updateMajor () ;
    if ( sc.isFailure () ) 
    { return Error ("The error from updateMajor" , sc ) ; }
    // take care about the histos
    sc = updateHistos () ;
    if ( sc.isFailure () ) 
    { return Error ("The error from updateHistos" , sc ) ; }
    //
    return StatusCode::SUCCESS ;
  }  
  /// the most interesting method 
  virtual StatusCode execute    () ;           // the most interesting method 
  // ==========================================================================
protected:
  // ==========================================================================
  /** standard constructor 
   *  @see DVAlgorithm
   *  @see GaudiTupleAlg
   *  @see GaudiHistoAlg
   *  @see GaudiAlgorithm
   *  @see Algorithm
   *  @see AlgFactory
   *  @see IAlgFactory
   *  @param name the algorithm instance name 
   *  @param pSvc pointer to Service Locator 
   */
  FilterDesktop                                        // standard contructor 
  ( const std::string& name ,                  // the algorithm instance name 
    ISvcLocator*       pSvc )                   // pointer to Service Locator
    : DVAlgorithm ( name , pSvc ) 
    // LoKi/Bender "hybrid" factory name 
    , m_factory ( "LoKi::Hybrid::Tool/HybridFactory:PUBLIC" )
    // the preambulo 
    , m_preambulo ()
    // the selection functor (predicate) itself 
    , m_code ( "PNONE" )
    , m_cut  ( LoKi::BasicFunctors<const LHCb::Particle*>::BooleanConstant ( false ) ) 
    // input plots 
    , m_inputPlotsTool   ( "LoKi::Hybrid::PlotTool/InputPlots"  )
    , m_inputPlots       (  0  )
    , m_inputPlotsPath   ( "I" + name  ) 
    // output plots 
    , m_outputPlotsTool  ( "LoKi::Hybrid::PlotTool/OutputPlots" )
    , m_outputPlots      (  0  )
    , m_outputPlotsPath  ( "O" + name  )
    // cloning rules:
    , m_cloneFinalStates ( false ) 
    , m_cloneDaughters   ( false ) 
    , m_cloneTree        ( false )
  {
    // the factory 
    declareProperty 
      ( "Factory" , 
        m_factory , 
        "The Type/Name for C++/Python Hybrid Factory" ) 
      -> declareUpdateHandler ( &FilterDesktop::updateHandler1 , this ) ;
    // the preambulo
    declareProperty 
      ( "Preambulo" , 
        m_preambulo , 
        "The preambulo to be used for Bender/Pthon script" ) 
      -> declareUpdateHandler ( &FilterDesktop::updateHandler1 , this ) ;
    // the functor (predicate) 
    declareProperty 
      ( "Code" , 
        m_code , 
        "The valid LoKi/Bender 'hybrid' functor name" ) 
      -> declareUpdateHandler ( &FilterDesktop::updateHandler1 , this ) ;
    // input plot tool 
    declareProperty 
      ( "InputPlotsTool"  , 
        m_inputPlotsTool  ,
        "The Type/Name for 'input' Plots tool" ) 
      -> declareUpdateHandler ( &FilterDesktop::updateHandler2 , this ) ;
    // output plot tool 
    declareProperty 
      ( "OutputPlotsTool" , 
        m_outputPlotsTool ,
        "The Type/Name for 'output' Plots tool" ) 
      -> declareUpdateHandler ( &FilterDesktop::updateHandler2 , this ) ;
    // the path for the input plots 
    declareProperty 
      ( "InputPlotsPath"  , 
        m_inputPlotsPath  ,
        "The THS path for the input plots" ) 
      -> declareUpdateHandler ( &FilterDesktop::updateHandler2 , this ) ;
    // the path for the output plots 
    declareProperty 
      ( "OutputPlotsPath" , 
        m_outputPlotsPath ,
        "The THS path for the output plots" ) 
      -> declareUpdateHandler ( &FilterDesktop::updateHandler2 , this ) ;
    //
    declareProperty ( "CloneFinalState" , m_cloneFinalStates ) ;
    declareProperty ( "CloneDaughters"  , m_cloneDaughters   ) ;
    declareProperty ( "CloneTree"       , m_cloneTree        ) ;
    // 
    StatusCode sc = setProperty ( "HistoProduce" , false ) ;
    Assert ( sc.isSuccess() , "Could not reset property HistoProduce" ) ;
    {
      Property* p = Gaudi::Utils::getProperty ( this , "HistoProduce" ) ;
      if ( 0 != p && 0 == p->updateCallBack() ) 
      { p -> declareUpdateHandler ( &FilterDesktop::updateHandler2 , this ) ; }
    }
  }
  // ==========================================================================
  /// virtual & protected destructor 
  virtual ~FilterDesktop() {}               // virtual & protected destructor 
  // ==========================================================================
public:
  // ==========================================================================
  /// update the structural property  
  void updateHandler1 ( Property& p ) 
  {
    // no action if not initialized yet:
    if ( Gaudi::StateMachine::INITIALIZED > FSMState() ) { return ; }
    /// mark as "to-be-updated" 
    m_to_be_updated1 = true ;
    Warning ( "The structural property '" + p.name() + 
              "' is updated. It will take affect at the next 'execute'" , 
              StatusCode( StatusCode::SUCCESS, true ) ) ;
    info () << "The updated property is: " << p << endreq ;
  }
  /// update the histogram property  
  void updateHandler2 ( Property& p ) 
  {
    // no action if not initialized yet:
    if ( Gaudi::StateMachine::INITIALIZED > FSMState() ) { return ; }
    /// mark as "to-be-updated" 
    m_to_be_updated2 = true ;
    debug () << "The histogramming property is updated: " << p << endreq ;
  }
  // ==========================================================================
private:
  // ==========================================================================
  /// construct the preambulo string 
  std::string preambulo() const 
  {
    std::string result ;
    for ( std::vector<std::string>::const_iterator iline = 
            m_preambulo.begin() ; m_preambulo.end() != iline ; ++iline ) 
    {
      if ( m_preambulo.begin() != iline ) { result += "\n" ; }
      result += (*iline) ;
    }
    return result ;
  }
  // ==========================================================================
  // update the major properties 
  StatusCode updateMajor () 
  {
    // locate the factory
    IHybridFactory* factory = tool<LoKi::IHybridFactory> ( m_factory , this ) ;
    // use the factory 
    StatusCode sc = factory-> get ( m_code , m_cut , preambulo() ) ;
    if ( sc.isFailure() ) 
    { return Error ( "Error from LoKi/Bender 'hybrid' factory for Code='" 
                     + m_code + "'" , sc )  ; }
    // release the factory (not needed anymore) 
    release ( factory ) ;
    //
    m_to_be_updated1 = false ;
    //
    return StatusCode::SUCCESS ;
  }
  // ==========================================================================
  // update histos 
  StatusCode updateHistos () 
  {
    // ========================================================================
    if ( 0 != m_inputPlots  ) 
    { releaseTool ( m_inputPlots  ) ; m_inputPlots  = 0 ; }
    if ( 0 != m_outputPlots ) 
    { releaseTool ( m_outputPlots ) ; m_outputPlots = 0 ; }
    // =========================================================================
    if ( produceHistos () ) 
    {
      // =======================================================================
      if ( validPlots ( m_inputPlotsTool ) )
      {
        m_inputPlots = tool<IPlotTool>( m_inputPlotsTool, this ) ;
        StatusCode sc = m_inputPlots -> setPath ( m_inputPlotsPath ) ;
        if ( sc.isFailure() ) 
        { return Error ( "Unable to set Input Plots path" , sc ) ; }
      }
      //
      if ( validPlots ( m_outputPlotsTool ) ) 
      {
        m_outputPlots = tool<IPlotTool>( m_outputPlotsTool, this ) ;
        StatusCode sc = m_outputPlots -> setPath ( m_outputPlotsPath ) ;
        if ( sc.isFailure() ) 
        { return Error ("Unable to set Output Plots path" , sc ) ; }
      }
    }
    //
    m_to_be_updated2 = false ;
    //
    return StatusCode::SUCCESS ;
  }
  // ==========================================================================
private:
  // ==========================================================================
  /// the default constructor is disabled 
  FilterDesktop () ;                    // the default consructor is disabled
  /// copy constructor is disabled 
  FilterDesktop ( const FilterDesktop& ) ;    // copy constructor is disabled 
  /// assignement operator is disabled 
  FilterDesktop& operator=( const FilterDesktop& ) ;        // no assignement
  // ==========================================================================
private:
  // ==========================================================================
  /// LoKi/Bender "hybrid" factory name 
  std::string m_factory ; // LoKi/Bender "hybrid" factory name 
  /// the preambulo 
  std::vector<std::string> m_preambulo ; // the preambulo 
  /// the code for the functor/predicate 
  std::string m_code    ; // the code for the functor/predicate
  /// the predicate itself 
  LoKi::BasicFunctors<const LHCb::Particle*>::PredicateFromPredicate m_cut ;
  //
  // input plots
  //
  /// type/name for the input plots tool 
  std::string m_inputPlotsTool  ;           // type-name for input plots tool 
  /// the inputplots tool itself 
  IPlotTool*  m_inputPlots      ;              // the input plots tool itself
  /// the THS path for the input plots 
  std::string m_inputPlotsPath  ;                     // the input plots path 
  //
  // output plots 
  //
  /// type/name for the output plots tool 
  std::string m_outputPlotsTool ;          // type-name for output plots tool 
  /// the inputplots tool itself 
  IPlotTool*  m_outputPlots     ;             // the output plots tool itself
  /// the THS path for the output plots 
  std::string m_outputPlotsPath ;                    // the output plots path 
  //
  // cloning rules:
  //
  bool m_cloneFinalStates ;
  bool m_cloneDaughters   ;
  bool m_cloneTree        ;
  // ==========================================================================
  /// the flag to indicate the nesessity of update 
  bool m_to_be_updated1 ; // the flag to indicate the nesessity of update 
  /// the flag to indicate the nesessity of update 
  bool m_to_be_updated2 ; // the flag to indicate the nesessity of update 
  // ==========================================================================
};
// ============================================================================
// the most interesting method 
// ============================================================================
StatusCode FilterDesktop::execute ()       // the most interesting method 
{
  //
  if  ( m_to_be_updated1 ) 
  {
    StatusCode sc = updateMajor () ;
    if ( sc.isFailure() ) 
    { return Error ( "The error from updateMajor"  , sc ) ; }    // RETURN 
  }
  if  ( m_to_be_updated2 ) 
  {
    StatusCode sc = updateHistos () ;
    if ( sc.isFailure() ) 
    { return Error ( "The error from updateHistos" , sc ) ; }    // RETURN 
  }
  
    
  // get the input particles 
  const LHCb::Particle::ConstVector particles = desktop() -> particles () ;
  // make plots 
  if ( produceHistos () && 0 != m_inputPlots ) 
  { 
    StatusCode sc = m_inputPlots -> fillPlots ( particles ) ; 
    if ( sc.isFailure () ) 
    { return Error ( "Error from Input Plots tool", sc ) ; }
  }
  //
  // copy the lines from the previous implementation 
  //
  LHCb::Particle::ConstVector accepted  ;
  accepted.reserve ( particles.size () ) ;
  LHCb::Particle::ConstVector daughters ;
  //
  unsigned int nPass = 0  ;
  StatEntity& cnt = counter ( "efficiency" ) ;
  //
  for ( LHCb::Particle::ConstVector::const_iterator ip = particles.begin() ; 
        particles.end() != ip ; ++ip )
  {
    const LHCb::Particle* p = *ip ;
    // satisfy the criteria ? 
    const bool decision = m_cut ( p ) ;
    // some statistics 
    cnt += decision ;
    if  ( !decision ) { continue ; }                       // CONTINUE
    // statistics 
    ++nPass ;                     // statistics 
    //
    if (!m_cloneFinalStates && !m_cloneDaughters) 
    { accepted.push_back ( p ) ; }
    // 
    if ( m_cloneTree || m_cloneFinalStates || m_cloneDaughters)
    {
      daughters.clear() ;
      if      ( m_cloneDaughters   ) { daughters = p->daughtersVector ()           ; }
      else if ( m_cloneTree        ) { daughters = descendants()->descendants( p ) ; }
      else if ( m_cloneFinalStates ) { daughters = descendants()->finalStates( p ) ; }
      // 
      accepted.insert ( accepted.end() , daughters.begin() , daughters.end() ) ;
    }
    //
  }  
  // clone the selected particles & trees 
  StatusCode sc = desktop()->cloneTrees(accepted);
  if ( sc.isFailure() ) { return Error ("Error from desktop()::clone", sc )  ; }
  // make the final plots 
  if ( produceHistos () && 0 != m_outputPlots ) 
  {
    sc = m_outputPlots -> fillPlots ( accepted ) ;
    if ( sc.isFailure () ) 
    { return Error ( "Error from Output Plots tool", sc ) ; }
  }
  /// make the filter decision
  setFilterPassed ( !accepted.empty() );
  // some statistics 
  counter ( "#input"  ) += particles.size() ;
  counter ( "#passed" ) += nPass            ;
  //
  return StatusCode::SUCCESS;
}
// ============================================================================
/// the factory 
DECLARE_ALGORITHM_FACTORY(FilterDesktop) 
// ============================================================================
// The END 
// ============================================================================
