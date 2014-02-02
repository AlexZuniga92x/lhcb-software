// $Id$
/** @class OnlineP2PVWithIPChi2 OnlineP2PVWithIPChi2.cpp
 *  
 *
 *  @author Juan PALACIOS
 *  @date   2008-10-24
 */
// Include files 
// from Gaudi
#include "GaudiKernel/DeclareFactoryEntries.h" 

// Local
#include "GenericParticle2PVRelator.h"
#include "DistanceCalculatorNames.h"
#include "P2PVLogic.h"

/** @class P2PVWithIP P2PVWithIP.cpp
 *  
 *
 *  @author Juan PALACIOS
 *  @date   2008-10-16
 */
typedef GenericParticle2PVRelator<_p2PVWithIP, OnlineDistanceCalculatorName>  OnlineP2PVWithIP;
// Declaration of the Tool Factory
DECLARE_TOOL_FACTORY( OnlineP2PVWithIP )

#ifdef DECLARE_COMPONENT_WITH_ID
// backward compatibility with the old PluginService internal names
DECLARE_COMPONENT_WITH_ID(OnlineP2PVWithIP, "GenericParticle2PVRelator__p2PVWithIP_OnlineDistanceCalculatorName_")
#endif
