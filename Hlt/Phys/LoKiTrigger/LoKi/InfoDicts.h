// $Id: InfoDicts.h,v 1.2 2007-11-28 14:56:23 ibelyaev Exp $
// ============================================================================
#ifndef LOKI_INFODICTS_H 
#define LOKI_INFODICTS_H 1
// ============================================================================
// Include files
// ============================================================================
// LoKi
// ============================================================================
#include "LoKi/TriggerTypes.h"
// ============================================================================
namespace LoKi 
{
  namespace Dicts 
  {
    namespace TrInfo
    {
      // ======================================================================
      LoKi::BasicFunctors<const LHCb::Track*>::FunctionFromFunction
      info 
      ( const int                                                index          , 
        const LoKi::BasicFunctors<const LHCb::Track*>::Function& fun            ,
        const bool                                               update = false ) ;
      // ======================================================================
      LoKi::BasicFunctors<LHCb::RecVertex>::FunctionFromFunction
      info 
      ( const int                                                index          , 
        const LoKi::BasicFunctors<LHCb::RecVertex>::Function&    fun            ,
        const bool                                               update = false ) ;
    }
  }  
}
// ============================================================================
// The END 
// ============================================================================
#endif // LOKI_INFODICTS_H
// ============================================================================
