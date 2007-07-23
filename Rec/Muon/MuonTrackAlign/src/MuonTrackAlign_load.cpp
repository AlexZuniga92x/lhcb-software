// $Id: MuonTrackAlign_load.cpp,v 1.1.1.1 2007-07-23 07:14:36 asatta Exp $
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

DECLARE_FACTORY_ENTRIES(MuonTrackAlign) {
   DECLARE_ALGORITHM( AlignMuonRec )
   DECLARE_ALGORITHM( MuonRead )
}
