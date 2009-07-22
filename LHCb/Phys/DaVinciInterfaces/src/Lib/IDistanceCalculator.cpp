// $Id: IDistanceCalculator.cpp,v 1.1.1.1 2009-07-22 20:54:51 jpalac Exp $
// ============================================================================
// Include files 
// ============================================================================
// DaVinciKernel
// ============================================================================
#include "Kernel/IDistanceCalculator.h"
// ============================================================================
/** @file 
 *  The Implementation file for the abstarct interface IDistanceCalculator
 *  @see IDistanceCalculator
 *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
 *  @date 2008-03-08 
 */
// ============================================================================
namespace 
{
  // the unique interface identifier 
  const InterfaceID s_IID ( "IDistanceCalculator" , 1 , 0 ) ;
}
// ============================================================================
// Retrieve the uniqie interface ID
// ============================================================================
const InterfaceID& IDistanceCalculator::interfaceID() { return s_IID ; }
// ============================================================================
// virtual & protected desctructor
// ============================================================================
IDistanceCalculator::~IDistanceCalculator(){}
// ============================================================================



// ============================================================================
// The END 
// ============================================================================
