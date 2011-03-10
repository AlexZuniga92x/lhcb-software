// $Id$
// ============================================================================
#ifndef LOKI_IHYBRIDFACTORY_H 
#define LOKI_IHYBRIDFACTORY_H 1
// ============================================================================
// Include files 
// ============================================================================
// STD & STL
// ============================================================================
#include <string>
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/IAlgTool.h"
#include "GaudiKernel/StatusCode.h"
// ============================================================================
// LoKi 
// ============================================================================
#include "LoKi/PhysTypes.h"
#include "LoKi/ATypes.h"
#include "LoKi/PPTypes.h"
// ============================================================================
/** @file
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
 */
// ============================================================================
namespace LoKi
{
  // ==========================================================================
  /** @class IHybridFactory IHybridFactory.h LoKi/IHybridFactory.h
   *  The abstract interface to "hybrid factory"  
   *  @author Vanya BELYAEV ibelayev@physics.syr.edu
   *  @date   2007-06-10
   */
  class GAUDI_API IHybridFactory : public virtual IAlgTool
  {
  public: 
    // ========================================================================
    /// interface machinery 
    DeclareInterfaceID(LoKi::IHybridFactory, 5, 0);
    // ========================================================================
  public:
    // ========================================================================
    // predicates 
    // ========================================================================
    /** "Factory": get the the object form python code 
     *  @param pycode the python pseudo-code of the function
     *  @param cuts the placeholder for the result 
     *  @param context the context lines to be executed 
     *  @return StatusCode 
     */
    virtual StatusCode get
    ( const std::string& pycode       , 
      LoKi::Types::Cut&  cuts         , 
      const std::string& context = "" ) = 0 ;
    // ========================================================================
    /** "Factory": get the the object form python code 
     *  @param pycode the python pseudo-code of the function
     *  @param cuts the placeholder for the result 
     *  @param context the context lines to be executed 
     *  @return StatusCode 
     */
    virtual StatusCode get
    ( const std::string& pycode       , 
      LoKi::Types::VCut& cuts         ,
      const std::string& context = "" ) = 0 ;
    // ========================================================================
    /** "Factory": get the the object form python code 
     *  @param pycode the python pseudo-code of the function
     *  @param cuts the placeholder for the result 
     *  @param context the context lines to be executed 
     *  @return StatusCode 
     */
    virtual StatusCode get
    ( const std::string& pycode       , 
      LoKi::Types::ACut& cuts         , 
      const std::string& context = "" ) = 0 ;
    // ========================================================================
    /** "Factory": get the the object form python code 
     *  @param pycode the python pseudo-code of the function
     *  @param cuts the placeholder for the result 
     *  @param context the context lines to be executed 
     *  @return StatusCode 
     */
    virtual StatusCode get
    ( const std::string&  pycode       , 
      LoKi::Types::PPCut& cuts         , 
      const std::string&  context = "" ) = 0 ;
    // ========================================================================
    // functions 
    // ========================================================================
    /** "Factory": get the the object form python code 
     *  @param pycode the python pseudo-code of the function
     *  @param func the placeholder for the result 
     *  @param context the context lines to be executed 
     *  @return StatusCode 
     */
    virtual StatusCode get
    ( const std::string& pycode       , 
      LoKi::Types::Fun&  func         ,
      const std::string& context = "" ) = 0 ;
    // ========================================================================
    /** "Factory": get the the object form python code 
     *  @param pycode the python pseudo-code of the function
     *  @param func the placeholder for the result 
     *  @param context the context lines to be executed 
     *  @return StatusCode 
     */
    virtual StatusCode get
    ( const std::string& pycode       , 
      LoKi::Types::VFun& func         , 
      const std::string& context = "" ) = 0 ;
    // ========================================================================
    /** "Factory": get the the object form python code 
     *  @param pycode the python pseudo-code of the function
     *  @param func the placeholder for the result 
     *  @param context the context lines to be executed 
     *  @return StatusCode 
     */
    virtual StatusCode get
    ( const std::string& pycode       ,
      LoKi::Types::AFun& func         ,
      const std::string& context = "" ) = 0 ;
    // ========================================================================
    /** "Factory": get the the object form python code 
     *  @param pycode the python pseudo-code of the function
     *  @param func the placeholder for the result 
     *  @param context the context lines to be executed 
     *  @return StatusCode 
     */
    virtual StatusCode get
    ( const std::string&  pycode       ,
      LoKi::Types::PPFun& func         ,
      const std::string&  context = "" ) = 0 ;
    // ========================================================================
    // "maps"
    // ========================================================================
    /** "Factory": get the the object form python code 
     *  @param pycode the python pseudo-code of the function
     *  @param func the placeholder for the result 
     *  @param context the context lines to be executed 
     *  @return StatusCode 
     */
    virtual StatusCode get
    ( const std::string& pycode       ,
      LoKi::Types::Map&  func         ,
      const std::string& context = "" ) = 0 ;    
    // ========================================================================
    /** "Factory": get the the object form python code 
     *  @param pycode the python pseudo-code of the function
     *  @param func the placeholder for the result 
     *  @param context the context lines to be executed 
     *  @return StatusCode 
     */
    virtual StatusCode get
    ( const std::string& pycode       ,
      LoKi::Types::VMap& func         ,
      const std::string& context = "" ) = 0 ;    
    // ========================================================================
    /** "Factory": get the the object form python code 
     *  @param pycode the python pseudo-code of the function
     *  @param func the placeholder for the result 
     *  @param context the context lines to be executed 
     *  @return StatusCode 
     */
    virtual StatusCode get
    ( const std::string&  pycode       ,
      LoKi::Types::PPMap& func         ,
      const std::string&  context = "" ) = 0 ;    
    // ========================================================================
    // "pipes"
    // ========================================================================
    /** "Factory": get the the object form python code 
     *  @param pycode the python pseudo-code of the function
     *  @param func the placeholder for the result 
     *  @param context the context lines to be executed 
     *  @return StatusCode 
     */
    virtual StatusCode get
    ( const std::string& pycode       ,
      LoKi::Types::Pipe& func         ,
      const std::string& context = "" ) = 0 ;    
    // ========================================================================
    /** "Factory": get the the object form python code 
     *  @param pycode the python pseudo-code of the function
     *  @param func the placeholder for the result 
     *  @param context the context lines to be executed 
     *  @return StatusCode 
     */
    virtual StatusCode get
    ( const std::string&  pycode       ,
      LoKi::Types::VPipe& func         ,
      const std::string&  context = "" ) = 0 ;    
    // ========================================================================
    /** "Factory": get the the object form python code 
     *  @param pycode the python pseudo-code of the function
     *  @param func the placeholder for the result 
     *  @param context the context lines to be executed 
     *  @return StatusCode 
     */
    virtual StatusCode get
    ( const std::string&   pycode       ,
      LoKi::Types::PPPipe& func         ,
      const std::string&   context = "" ) = 0 ;    
    // ========================================================================
    // "fun-vals"
    // ========================================================================
    /** "Factory": get the the object form python code 
     *  @param pycode the python pseudo-code of the function
     *  @param func the placeholder for the result 
     *  @param context the context lines to be executed 
     *  @return StatusCode 
     */
    virtual StatusCode get
    ( const std::string&   pycode       ,
      LoKi::Types::FunVal& func         ,
      const std::string&   context = "" ) = 0 ;    
    // ========================================================================
    /** "Factory": get the the object form python code 
     *  @param pycode the python pseudo-code of the function
     *  @param func the placeholder for the result 
     *  @param context the context lines to be executed 
     *  @return StatusCode 
     */
    virtual StatusCode get
    ( const std::string&    pycode       ,
      LoKi::Types::VFunVal& func         ,
      const std::string&    context = "" ) = 0 ;    
    // ========================================================================
    /** "Factory": get the the object form python code 
     *  @param pycode the python pseudo-code of the function
     *  @param func the placeholder for the result 
     *  @param context the context lines to be executed 
     *  @return StatusCode 
     */
    virtual StatusCode get
    ( const std::string&     pycode       ,
      LoKi::Types::PPFunVal& func         ,
      const std::string&     context = "" ) = 0 ;    
    // ========================================================================
    // "cut-vals"
    // ========================================================================
    /** "Factory": get the the object form python code 
     *  @param pycode the python pseudo-code of the function
     *  @param func the placeholder for the result 
     *  @param context the context lines to be executed 
     *  @return StatusCode 
     */
    virtual StatusCode get
    ( const std::string&   pycode       ,
      LoKi::Types::CutVal& func         ,
      const std::string&   context = "" ) = 0 ;    
    // ========================================================================
    /** "Factory": get the the object form python code 
     *  @param pycode the python pseudo-code of the function
     *  @param func the placeholder for the result 
     *  @param context the context lines to be executed 
     *  @return StatusCode 
     */
    virtual StatusCode get
    ( const std::string&    pycode       ,
      LoKi::Types::VCutVal& func         ,
      const std::string&    context = "" ) = 0 ;    
    // ========================================================================
    /** "Factory": get the the object form python code 
     *  @param pycode the python pseudo-code of the function
     *  @param func the placeholder for the result 
     *  @param context the context lines to be executed 
     *  @return StatusCode 
     */
    virtual StatusCode get
    ( const std::string&     pycode       ,
      LoKi::Types::PPCutVal& func         ,
      const std::string&     context = "" ) = 0 ;    
    // ========================================================================
    // "sources"
    // ========================================================================
    /** "Factory": get the the object form python code 
     *  @param pycode the python pseudo-code of the function
     *  @param func the placeholder for the result 
     *  @param context the context lines to be executed 
     *  @return StatusCode 
     */
    virtual StatusCode get
    ( const std::string&    pycode       ,
      LoKi::Types::Source&  func         ,
      const std::string&    context = "" ) = 0 ;    
    // ========================================================================
    /** "Factory": get the the object form python code 
     *  @param pycode the python pseudo-code of the function
     *  @param func the placeholder for the result 
     *  @param context the context lines to be executed 
     *  @return StatusCode 
     */
    virtual StatusCode get
    ( const std::string&     pycode       ,
      LoKi::Types::VSource&  func         ,
      const std::string&     context = "" ) = 0 ;    
    // ========================================================================
    /** "Factory": get the the object form python code 
     *  @param pycode the python pseudo-code of the function
     *  @param func the placeholder for the result 
     *  @param context the context lines to be executed 
     *  @return StatusCode 
     */
    virtual StatusCode get
    ( const std::string&      pycode       ,
      LoKi::Types::PPSource&  func         ,
      const std::string&      context = "" ) = 0 ;    
    // ========================================================================
  protected:
    // ========================================================================
    /// virtual & protected desctructor 
    virtual ~IHybridFactory () ;                                  // Destructor
    // ========================================================================
  } ;
  // ==========================================================================
} //                                                      end of namespace LoKi
// ============================================================================
// The END 
// ============================================================================
#endif // LOKI_IHYBRIDFACTORY_H
// ============================================================================
