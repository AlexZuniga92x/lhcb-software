// $Id: $
// ============================================================================
#ifndef ANALYSIS_PYSELECTOR_H 
#define ANALYSIS_PYSELECTOR_H 1
// ============================================================================
// Include files
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/Kernel.h"
// ============================================================================
// ROOT
// ============================================================================
#include "TPySelector.h"
// ============================================================================
// Forward declaratios
// ============================================================================
class TTree  ;            // ROOT 
class TChain ;            // ROOT 
// ============================================================================
namespace Analysis 
{
  // ==========================================================================
  /** @class Selector PySelector.h Analysis/PySelector.h
   *
   *  Helper class for implementation of "python TSelector"
   *
   * The fix has been kindly provided by Wim Lavrijsen
   *
   *  @author Vanya Belyaev Ivan.Belyaev@cern.ch
   *  @date   2011-01-21
   * 
   *                    $Revision$
   *  Last modification $Date$
   *                 by $Author$
   */
  class GAUDI_API Selector : public  TPySelector
  {
    // ========================================================================
  public:
    // ========================================================================
    ClassDef(Analysis::Selector, 1) ;
    // ========================================================================
  public:
    // ========================================================================
    /// constructor 
    Selector
    ( TTree*    tree = 0 , 
      PyObject* self = 0 ) ;
    // ========================================================================
    /// destructor
    virtual ~Selector() ;
    // ========================================================================
  } ;
  // ==========================================================================
} //                                              the end of namespace Analysis 
// ============================================================================
namespace Analysis 
{
  // ==========================================================================
  /** @class Process
   *  Helper class to fix strange "feature" of (Py)ROOT:
   *     - ROOT.TTree  does have method <c>TTree.Process</c>  with TSelector as argument 
   *     - ROOT.TChain has *NO*  method <c>TChain.Process</c> with TSelector as argument 
   *  This trick allows to access these methods indirectly.
   *  @author Vanya BELYAEV Ivan.Belyaev Ivan.Belyaev@cern.ch
   *  @date 2010-11-21
   */
  class GAUDI_API Process 
  {
  public:
    // ========================================================================
    /** helper function to use TTree::Process in python 
     * 
     *  @param tree      root-tree 
     *  @param selector  the selector 
     *  
     *  @see TTree 
     *  @see TTree::Process 
     *  @see TSelector 
     *
     *  @author Vanya Belyaev Ivan.Belyaev@cern.ch
     *  @date   2011-01-21
     */
    static
    long process
    ( TTree*             tree      ,
      TSelector*         selector  ) ;
    // ========================================================================
    /** helper function to use TChain::Process in python 
     * 
     *  @param tree      root-tree 
     *  @param selector  the selector 
     *  
     *  @see TTree 
     *  @see TTree::Process 
     *  @see TSelector 
     *
     *  @author Vanya Belyaev Ivan.Belyaev@cern.ch
     *  @date   2011-01-21
     */
    static 
    long process 
    ( TChain*    chain     ,
      TSelector* selector  ) ;
    // ========================================================================
  };  
  // ==========================================================================
} //                                              the end of namespace Analysis 
// ============================================================================
//                                                                      The END 
// ============================================================================
#endif // ANALYSIS_PYSELECTOR_H
// ============================================================================
