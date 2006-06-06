// ============================================================================
#ifndef LHCBMATH_POWER_H 
#define LHCBMATH_POWER_H 1
// ============================================================================
// Include files
// ============================================================================

// ============================================================================
/** @file
 *
 *  This file has been taken from the LoKi project - 
 *    "C++ ToolKit  for Smart and Friendly Physics Analysis"
 *
 *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
 *  @date 2001-01-23 
 */
// ============================================================================

namespace Gaudi 
{  
  namespace Math 
  {
    
    /** @fn pow 
     *  simple utility for efficient "pow"
     *
     *  @code 
     *   
     *   const double result = Gaudi::Math::pow ( value , 10 ) ;
     *
     *  @code 
     *
     *  The actual code is copied from 
     *     std::__cmath_power bits/cmath.tcc 
     *
     *  @author Vanya BELYAEV Ivan.Belyaev@lapp.in2p3.fr
     *  @date 2005-04-09 
     */
    template<typename TYPE>
    inline TYPE pow ( TYPE __x , unsigned long __n )
    {
      TYPE __y = __n % 2 ? __x : 1;
    
      while ( __n >>= 1 )
      {
        __x = __x * __x;
        if ( __n % 2) { __y = __y * __x; }
      } ;
    
      return __y ;
    };

  }; // end of namespace Math
  
}; // end of namespace Gaudi

// ============================================================================
// The END 
// ============================================================================
#endif // LHCBMATH_POWER_H
// ============================================================================
