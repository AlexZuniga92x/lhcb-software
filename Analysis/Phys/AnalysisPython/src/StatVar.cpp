// $Id:$ 
// ============================================================================
// Include files
// ============================================================================
// Local: 
// ============================================================================
#include "Analysis/StatVar.h"
#include "Analysis/Formula.h"
// ============================================================================
// LHCbMath
// ============================================================================
#include "LHCbMath/MatrixUtils.h"
// ============================================================================
// ROOT 
// ============================================================================
#include "TTree.h"
#include "TCut.h"
// ============================================================================
// Boost 
// ============================================================================
#include "boost/static_assert.hpp"
// ============================================================================
/** @file 
 *  Implementation file for class Analysis::StatVar
 *  @date 2013-10-13 
 *  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
 * 
 *                    $Revision:$
 *  Last modification $Date:$
 *                 by $Author:$
 */
// ============================================================================
namespace 
{
  // ==========================================================================
  BOOST_STATIC_ASSERT(std::numeric_limits<unsigned long>::is_specialized );
  // ==========================================================================
  /** @class Notifier
   *  Local helper class to keep the proper notifications for TTree
   *  @date 2013-10-13 
   *  @author Vanya BELYAEV Ivan.Brlyaev@itep.ru
   */
  class Notifier : public TObject 
  {
    //
  public:
    //
    Notifier 
    ( TTree*   tree     , 
      TObject* obj0     , 
      TObject* obj1 = 0 , 
      TObject* obj2 = 0 , 
      TObject* obj3 = 0 , 
      TObject* obj4 = 0 ,
      TObject* obj5 = 0 , 
      TObject* obj6 = 0 , 
      TObject* obj7 = 0 , 
      TObject* obj8 = 0 , 
      TObject* obj9 = 0 ) ;
    /// virtual destructor 
    virtual        ~Notifier () ;  // virtual destructor 
    virtual Bool_t  Notify   () ;
    //
  private:
    Notifier () ;
    Notifier ( const Notifier & ) ;
  private:
    TTree*   m_tree ;
    TObject* m_old  ; // old notofier  
    // list of fobject to be notified 
    std::vector<TObject*> m_objects ;
  } ;
  // ==========================================================================
  Notifier::Notifier 
  ( TTree*   tree , 
    TObject* obj0 , 
    TObject* obj1 , 
    TObject* obj2 , 
    TObject* obj3 , 
    TObject* obj4 ,
    TObject* obj5 , 
    TObject* obj6 , 
    TObject* obj7 , 
    TObject* obj8 , 
    TObject* obj9 ) 
    : TObject   () 
    , m_tree    ( tree ) 
    , m_old     ( 0    )
    , m_objects () 
  {
    //
    if ( 0 != m_tree ) { m_old = m_tree->GetNotify ()  ; }
    //
    if ( 0 != m_old  ) { m_objects.push_back ( m_old ) ; } // prepend it! 
    //
    if ( 0 != obj0   ) { m_objects.push_back ( obj0  ) ; }
    if ( 0 != obj1   ) { m_objects.push_back ( obj1  ) ; }
    if ( 0 != obj2   ) { m_objects.push_back ( obj2  ) ; }
    if ( 0 != obj3   ) { m_objects.push_back ( obj3  ) ; }
    if ( 0 != obj4   ) { m_objects.push_back ( obj4  ) ; }
    if ( 0 != obj5   ) { m_objects.push_back ( obj5  ) ; }
    if ( 0 != obj6   ) { m_objects.push_back ( obj6  ) ; }
    if ( 0 != obj7   ) { m_objects.push_back ( obj7  ) ; }
    if ( 0 != obj8   ) { m_objects.push_back ( obj8  ) ; }
    if ( 0 != obj9   ) { m_objects.push_back ( obj9  ) ; }
    //
    if ( 0 != m_tree ) { m_tree->SetNotify   ( this  ) ; }
    //
  }
  // ==========================================================================
  Notifier::~Notifier() { if ( 0 != m_tree ) { m_tree->SetNotify ( m_old ) ; } }
  // ==========================================================================
  Bool_t Notifier::Notify() 
  {
    //
    for (  std::vector<TObject*>::iterator it = m_objects.begin() ;
           m_objects.end() != it ; ++it ) 
    {
      TObject* o = *it ;
      if ( 0 != o ) { o->Notify() ; }
    }
    //
    return kTRUE ;
  }
  // ========================================================================== 
} //                                                 end of anonymous namespace 
// ============================================================================
/*  build statistic for the <code>expression</code>
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
// ============================================================================
Analysis::StatVar::Statistic
Analysis::StatVar::statVar 
( TTree*             tree       , 
  const std::string& expression , 
  const unsigned long first     ,
  const unsigned long entries   )
{
  Statistic result ;
  if ( 0 == tree          ) { return result ; }  // RETURN 
  Analysis::Formula formula ( "" , expression , tree ) ;
  if ( !formula.GetNdim() ) { return result ; }  // RETURN 
  //
  Notifier notify ( tree , &formula ) ;
  //
  const unsigned long nEntries = 
    std::min ( entries , (unsigned long) tree->GetEntries() ) ;
  //
  for ( unsigned long entry = first ; entry < nEntries ; ++entry )   
  {
    //
    long ievent = tree->GetEntryNumber ( entry ) ;
    if ( 0 > ievent ) { return result ; }                // RETURN
    //
    ievent      = tree->LoadTree ( ievent ) ;
    if ( 0 > ievent ) { return result ; }                // RETURN 
    //
    result += formula.evaluate() ;
  }
  //
  return result ;
}
// ============================================================================
/*  build statistic for the <code>expression</code>
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
// ============================================================================
Analysis::StatVar::Statistic
Analysis::StatVar::statVar 
( TTree*              tree       , 
  const std::string&  expression ,
  const std::string&  cuts       , 
  const unsigned long first      ,
  const unsigned long entries    )
{
  //
  Gaudi::Math::WStatEntity result ;
  if ( 0 == tree        ) { return result ; }            // RETURN 
  Analysis::Formula selection ( "" , cuts      , tree ) ;
  if ( !selection.ok () ) { return result ; }            // RETURN 
  Analysis::Formula formula   ( "" , expression , tree ) ;
  if ( !formula  .ok () ) { return result ; }            // RETURN 
  //
  Notifier notify ( tree , &selection,  &formula ) ;
  //
  const unsigned long nEntries = 
    std::min ( entries , (unsigned long) tree->GetEntries() ) ;
  //
  for ( unsigned long entry = first ; entry < nEntries ; ++entry )   
  {
    //
    long ievent = tree->GetEntryNumber ( entry ) ;
    if ( 0 > ievent ) { return result ; }                // RETURN
    //
    ievent      = tree->LoadTree ( ievent ) ;
    if ( 0 > ievent ) { return result ; }                // RETURN 
    //
    const double w = selection.evaluate() ;
    const double v = !w ? 0.0 : formula.evaluate() ;
    //
    result.add ( v , w ) ;
    //
  }
  //
  return result ;
}
// ============================================================================
/*  build statistic for the <code>expression</code>
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
// ============================================================================
Analysis::StatVar::Statistic
Analysis::StatVar::statVar 
( TTree*              tree       , 
  const std::string&  expression ,
  const TCut&         cuts       , 
  const unsigned long first      ,
  const unsigned long entries    )
{
  //
  const std::string _cuts = cuts.GetTitle() ;
  //
  return statVar ( tree , expression , _cuts , first , entries) ;
}
// ============================================================================
/*  calculate the covariance of two expressions 
 *  @param tree  (INPUT)  the input tree 
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
// ============================================================================
unsigned long
Analysis::StatVar::statCov
( TTree*               tree    , 
  const std::string&   exp1    , 
  const std::string&   exp2    , 
  Analysis::StatVar::Statistic& stat1 ,  
  Analysis::StatVar::Statistic& stat2 ,  
  Gaudi::SymMatrix2x2& cov2    , 
  const unsigned long  first   ,
  const unsigned long  entries )
{
  //
  stat1.reset () ;
  stat2.reset () ;
  Gaudi::Math::setToScalar ( cov2 , 0.0 ) ;
  //
  if ( 0 == tree        ) { return 0 ; } // RETURN 
  Analysis::Formula formula1 ( "" , exp1 , tree ) ;
  if ( !formula1 .ok () ) { return 0 ; } // RETURN 
  Analysis::Formula formula2 ( "" , exp2 , tree ) ;
  if ( !formula2 .ok () ) { return 0 ; } // RETURN 
  //
  Notifier notify ( tree , &formula1 , &formula2 ) ;
  //
  const unsigned long nEntries = 
    std::min ( entries , (unsigned long) tree->GetEntries() ) ;
  //
  for ( unsigned long entry = first ; entry < nEntries ; ++entry )   
  {
    //
    long ievent = tree->GetEntryNumber ( entry ) ;
    if ( 0 > ievent ) { return 0 ; } // RETURN
    //
    ievent      = tree->LoadTree ( ievent ) ;
    if ( 0 > ievent ) { return 0 ; } // RETURN 
    //
    const double v1 = formula1.evaluate() ;
    const double v2 = formula2.evaluate() ;
    //
    stat1 += v1 ;
    stat2 += v2 ;
    //
    cov2 ( 0 , 0 ) += v1*v1 ;
    cov2 ( 0 , 1 ) += v1*v2 ;
    cov2 ( 1 , 1 ) += v2*v2 ;
    //
  }
  //
  if ( 0 == stat1.nEntries() ) { return 0 ; }
  //
  cov2 /= stat1.nEntries () ;
  //
  const double v1_mean = stat1.mean() ;
  const double v2_mean = stat2.mean() ;
  //
  cov2 ( 0 , 0 ) -= v1_mean * v1_mean ;
  cov2 ( 0 , 1 ) -= v1_mean * v2_mean ;
  cov2 ( 1 , 1 ) -= v2_mean * v2_mean ;  
  //
  return stat1.nEntries () ;
}
// ============================================================================
/*  calculate the covariance of two expressions 
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
// ============================================================================
unsigned long 
Analysis::StatVar::statCov
( TTree*               tree    , 
  const std::string&   exp1    , 
  const std::string&   exp2    , 
  const std::string&   cuts    , 
  Analysis::StatVar::Statistic& stat1 ,  
  Analysis::StatVar::Statistic& stat2 ,  
  Gaudi::SymMatrix2x2& cov2    , 
  const unsigned long  first   ,
  const unsigned long  entries )
{
  //
  stat1.reset () ;
  stat2.reset () ;
  Gaudi::Math::setToScalar ( cov2 , 0.0 ) ;
  //
  if ( 0 == tree        ) { return 0 ; } // RETURN 
  Analysis::Formula formula1 ( "" , exp1 , tree ) ;
  if ( !formula1 .ok () ) { return 0 ; } // RETURN 
  Analysis::Formula formula2 ( "" , exp2 , tree ) ;
  if ( !formula2 .ok () ) { return 0 ; } // RETURN 
  Analysis::Formula selection ( "" , cuts      , tree ) ;
  if ( !selection.ok () ) { return 0 ; } // RETURN 
  //
  Notifier notify ( tree , &formula1 , &formula2 ) ;
  //
  const unsigned long nEntries = 
    std::min ( entries , (unsigned long) tree->GetEntries() ) ;
  //
  for ( unsigned long entry = first ; entry < nEntries ; ++entry )   
  {
    //
    long ievent = tree->GetEntryNumber ( entry ) ;
    if ( 0 > ievent ) { return 0 ; } // RETURN
    //
    ievent      = tree->LoadTree ( ievent ) ;
    if ( 0 > ievent ) { return 0 ; } // RETURN 
    //
    const double w = selection.evaluate() ;
    //
    const double v1 = !w ? 0.0 : formula1.evaluate() ;
    const double v2 = !w ? 0.0 : formula2.evaluate() ;
    //
    stat1.add ( v1 , w ) ;
    stat2.add ( v2 , w ) ;
    //
    cov2 ( 0 , 0 ) += w*v1*v1 ;
    cov2 ( 0 , 1 ) += w*v1*v2 ;
    cov2 ( 1 , 1 ) += w*v2*v2 ;
    //
  }
  //
  if ( 0 == stat1.nEntries() || 0 == stat1.nEff () ) { return 0 ; }
  //
  cov2 /= stat1.weights().sum()  ;
  //
  const double v1_mean = stat1.mean() ;
  const double v2_mean = stat2.mean() ;
  //
  cov2 ( 0 , 0 ) -= v1_mean * v1_mean ;
  cov2 ( 0 , 1 ) -= v1_mean * v2_mean ;
  cov2 ( 1 , 1 ) -= v2_mean * v2_mean ;  
  //
  return stat1.nEntries() ;
}
// ============================================================================
/*  calculate the covariance of two expressions 
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
// ============================================================================
unsigned long 
Analysis::StatVar::statCov
( TTree*               tree    , 
  const std::string&   exp1    , 
  const std::string&   exp2    , 
  const TCut&          cuts    , 
  Analysis::StatVar::Statistic& stat1 ,  
  Analysis::StatVar::Statistic& stat2 ,  
  Gaudi::SymMatrix2x2& cov2    , 
  const unsigned long  first   ,
  const unsigned long  entries )
{
  const std::string _cuts = cuts.GetTitle() ;
  //
  return statCov ( tree  , 
                   exp1  , exp2    , _cuts , 
                   stat1 , stat2   , cov2  , 
                   first , entries ) ; 
}
// ============================================================================
// The END 
// ============================================================================
