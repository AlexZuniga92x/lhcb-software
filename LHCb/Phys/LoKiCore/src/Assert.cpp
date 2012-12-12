// $Id$
// ============================================================================
// Include files 
// ============================================================================
// STD &STL
// ============================================================================
#include <string>
// ============================================================================
// LoKi
// ============================================================================
#include "LoKi/Exception.h"
#include "LoKi/Assert.h"
// ============================================================================
/** @file
 *  Implementation file for function LoKi::throwException 
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
 *  with the smear campaign of Dr.O.Callot et al.: 
 *  ``No Vanya's lines are allowed in LHCb/Gaudi software.''
 *
 *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
 *  @date 2007-08-11
 */
// ============================================================================
void LoKi::throwException ( const std::string& message ) 
{ throw LoKi::Exception ( message ) ; }
// ============================================================================

// ============================================================================
// The END 
// ============================================================================
