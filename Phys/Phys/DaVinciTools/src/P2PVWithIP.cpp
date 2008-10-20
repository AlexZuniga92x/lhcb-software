// $Id: P2PVWithIP.cpp,v 1.1 2008-10-20 15:15:40 jpalac Exp $
// Include files

// from Gaudi
#include "GaudiKernel/DeclareFactoryEntries.h" 

// Local
#include "GenericParticle2PVRelator.h"
#include "DistanceCalculatorNames.h"
#include "P2PVLogic.h"
//DaVinci
#include "Kernel/IDistanceCalculator.h"

/** @class P2PVWithIP P2PVWithIP.cpp
 *  
 *
 *  @author Juan PALACIOS
 *  @date   2008-10-15
 */
typedef GenericParticle2PVRelator<_p2PVWithIP, OfflineDistanceCalculatorName>  P2PVWithIP;
// Declaration of the Tool Factory
DECLARE_TOOL_FACTORY( P2PVWithIP )
