// $Id: CaloDAQ_load.cpp,v 1.1.1.1 2005-01-11 07:51:47 ocallot Exp $
// Include files 


#include "GaudiKernel/DeclareFactoryEntries.h"

// Declare  OBJECT / CONVERTER / ALGORITHM / TOOL using the macros DECLARE_xxx
// The statements are like that:
//
// DECLARE_ALGORITHM( MyAlgorithm );
// DECLARE_TOOL( MyTool );
// DECLARE_OBJECT( DataObject );
//
// They should be inside the 'DECLARE_FACTORY_ENTRIES' body.

DECLARE_FACTORY_ENTRIES(CaloDAQ) {
  DECLARE_ALGORITHM( CaloFillRawBuffer );
  DECLARE_ALGORITHM( CaloFillPrsSpdRawBuffer );

  DECLARE_TOOL( CaloTriggerFromRaw );
  DECLARE_TOOL( CaloPrsBitFromRaw );
  DECLARE_TOOL( CaloSpdBitFromRaw );

  DECLARE_ALGORITHM( CaloDigitsFromRaw );
  DECLARE_ALGORITHM( CompareCaloDigits );
}
