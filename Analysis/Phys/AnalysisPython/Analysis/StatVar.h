// $Id:$ 
// ============================================================================
#ifndef ANALYSIS_STATVAR_H
#define ANALYSIS_STATVAR_H 1
// ============================================================================
// Include files
// ============================================================================
// STD & STL
// ============================================================================
#include <limits>
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/Kernel.h"
#include "GaudiKernel/StatEntity.h"
#include "GaudiKernel/SymmetricMatrixTypes.h"
// ============================================================================
// LHcbMath 
// ============================================================================
#include "LHCbMath/WStatEntity.h"
// ============================================================================
// Forward declarations 
// =============================================================================
class TTree  ;     // ROOT 
class TChain ;     // ROOT 
// =============================================================================
namespace Analysis 
{
  // ==========================================================================
  /** @class StatVar Analysis/StatVar.h
   *  Helper class to get statistical 
   *  infomation  about the variable/expression 
   *
   *  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
   *  @date   2013-10-13
   * 
   *                    $Revision:$
   *  Last modification $Date:$
   *                 by $Author:$
   */
  class GAUDI_API StatVar 
  {
  public:
    // ========================================================================
    /// the actual type for styatistic 
    typedef Gaudi::Math::WStatEntity  Statistic ;
    // ========================================================================
  public: 
    // ========================================================================
    /** build statistic for the <code>expression</code>
     *  @param tree (INPUT) the tree 
     *  @param expression (INPUT) the expression
     *
     *  @code
     *  tree = ... 
     *  stat = tree.statVar( 'S_sw' ) 
     *  @endcode 
     *
     *  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
     *  @date   2013-10-13
     */
    static Statistic 
    statVar ( TTree*              tree                                                   , 
              const std::string&  expression                                             ,
              const unsigned long first      = 0                                         ,
              const unsigned long entries    = std::numeric_limits<unsigned long>::max() ) ;
    // ========================================================================
    /** build statistic for the <code>expression</code>
     *  @param tree       (INPUT) the tree 
     *  @param expression (INPUT) the expression
     *  @param cuts       (INPUT) the selection criteria 
     *
     *  @code
     *  tree = ... 
     *  stat = tree.statVar( 'S_sw' ,'pt>1000') 
     *  @endcode 
     *
     *  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
     *  @date   2013-10-13
     */
    static Statistic
    statVar ( TTree*              tree        , 
              const std::string&  expression  , 
              const std::string&  cuts        ,
              const unsigned long first      = 0                                         ,
              const unsigned long entries    = std::numeric_limits<unsigned long>::max() ) ;
    // ========================================================================
    /** calculate the covariance of two expressions 
     *  @param tree  (INPUT)  the inpout tree 
     *  @param exp1  (INPUT)  the first  expresiion
     *  @param exp2  (INPUT)  the second expresiion
     *  @param stat1 (UPDATE) the statistic for the first  expression
     *  @param stat2 (UPDATE) the statistic for the second expression
     *  @param cov2  (UPDATE) the covariance matrix 
     *  @return number of processed events 
     *  
     *  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
     *  @date   2014-03-27
     */
    static unsigned long
    statCov ( TTree*               tree   , 
              const std::string&   exp1   , 
              const std::string&   exp2   , 
              Statistic&           stat1  ,  
              Statistic&           stat2  ,  
              Gaudi::SymMatrix2x2& cov2   , 
              const unsigned long  first   = 0 ,
              const unsigned long entries = std::numeric_limits<unsigned long>::max() ) ;
    // ========================================================================
    /** calculate the covariance of two expressions 
     *  @param tree  (INPUT)  the inpout tree 
     *  @param exp1  (INPUT)  the first  expresiion
     *  @param exp2  (INPUT)  the second expresiion
     *  @param cuts  (INPUT)  the selection criteria 
     *  @param stat1 (UPDATE) the statistic for the first  expression
     *  @param stat2 (UPDATE) the statistic for the second expression
     *  @param cov2  (UPDATE) the covariance matrix 
     *  @return number of processed events 
     *  
     *  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
     *  @date   2014-03-27
     */
    static unsigned long 
    statCov ( TTree*               tree   ,
              const std::string&   exp1   , 
              const std::string&   exp2   , 
              const std::string&   cuts   ,
              Statistic&           stat1  ,  
              Statistic&           stat2  ,  
              Gaudi::SymMatrix2x2& cov2   , 
              const unsigned long  first   = 0                                         ,
              const unsigned long  entries = std::numeric_limits<unsigned long>::max() ) ;
    // ========================================================================
  };
  // ==========================================================================
} //                                                  end of namespace Analysis
// ============================================================================
//                                                                      The END 
// ============================================================================
#endif // ANALYSIS_STATVAR_H
// ============================================================================
