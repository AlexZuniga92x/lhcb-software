// $Id: PrintDecayTree.cpp,v 1.1 2008-03-30 18:05:28 ibelyaev Exp $
// ============================================================================
// Include files 
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/AlgFactory.h"
// ============================================================================
// DaVinciKernel
// ============================================================================
#include "Kernel/DVAlgorithm.h"
#include "Kernel/IPrintDecay.h"
// ============================================================================
/** @class PrintDecayTree 
 *  The simplified version of the algorithm PrintTree, 
 *  which deals only with the reconstructed particles 
 *  @see PrintTree 
 *  @see IPRintDecay
 *  @author Vanya BELYAEV Ivan.Belayev@nikhef.nl
 *  @date 2008-03-30
 */
class PrintDecayTree : public DVAlgorithm
{
  // ==========================================================================
  // the friend factory for instantiation 
  friend class AlgFactory<PrintDecayTree> ;
  // ==========================================================================
public:
  // ==========================================================================
  /// the standard execution of the algorithm
  virtual StatusCode execute() 
  {
    // get the tool 
    if ( 0 == m_printDecay ) 
    { m_printDecay = tool<IPrintDecay>( m_printDecayName , this ) ; }
    // get the particles 
    LHCb::Particle::ConstVector parts = desktop()->particles();
    //
    m_printDecay->printTree ( parts.begin() , parts.end () , m_maxDepth ) ;
    //
    setFilterPassed ( !parts.empty() ) ;
    //
    return StatusCode::SUCCESS ;
  }
  /// the standard finalization of the algorithm
  virtual StatusCode finalize () 
  {
    m_printDecay = 0 ;
    return DVAlgorithm::finalize () ;
  }
  // ==========================================================================
protected:
  // ==========================================================================
  /** the standard constructor 
   *  @param name algorithm instance name 
   *  @param pSvc service locator 
   */
  PrintDecayTree 
  ( const std::string& name , 
    ISvcLocator*       pSvc ) 
    : DVAlgorithm ( name , pSvc ) 
    , m_printDecayName ( "PrintDecayTreeTool/PrintDecay" )
    , m_printDecay ( 0 ) 
    , m_maxDepth   ( 3 )  
  {
    declareProperty 
      ( "PrintDecayTool" , m_printDecayName , 
        "The type/name of the IPrintDecay tool" ) ;
    declareProperty 
      ( "MaxDepth"       , m_maxDepth       , 
        "The maximal depth (numbe rof levels)"  ) ;
  }
  /// virtual & protected destructor 
  virtual ~PrintDecayTree () {}
  // ==========================================================================
private:
  // ==========================================================================
  /// the default constructor is disabled 
  PrintDecayTree () ; // the default constructor is disabled 
  /// the copy constructor is disabled 
  PrintDecayTree ( const PrintDecayTree& ) ; // no copy constructor 
  /// the assignement operator is disabled 
  PrintDecayTree& operator=( const PrintDecayTree& ) ; // no assignement
  // ==========================================================================
private:
  // ==========================================================================
  /// the type/name of the IPrintDecay tool 
  std::string  m_printDecayName ; // the type/name of the IPrintDecay tool 
  /// the IPrintDecay tool itself
  IPrintDecay* m_printDecay     ; // the IPrintDecay tool itself
  /// the maximal printout depth 
  int          m_maxDepth       ; // the maximal printout depth 
  // ==========================================================================
};
// ============================================================================
/// declare the factory (needed for instantiation)
DECLARE_ALGORITHM_FACTORY(PrintDecayTree) ;
// ============================================================================
// The END 
// ============================================================================
