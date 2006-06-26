// $Id: GaudiOnline.cpp,v 1.12 2006-06-26 08:45:15 frankb Exp $
//====================================================================
//  GaudiOnline_dll.cpp
//--------------------------------------------------------------------
//
//  Package    : Gaudi
//
//  Description: Implementation of DllMain routine.
//               The DLL initialisation must be done seperately for 
//               each DLL. 
//
//  Author     : M.Frank
//  Created    : 13/1/99
//  Changes    : 
//
//====================================================================
// Include Files
#include "GaudiKernel/LoadFactoryEntries.h"

LOAD_FACTORY_ENTRIES(GaudiOnline)

#include "GaudiKernel/DeclareFactoryEntries.h"

DECLARE_FACTORY_ENTRIES(GaudiOnline)  {
  DECLARE_NAMESPACE_SERVICE(   LHCb,DimMessageSvc);
  DECLARE_NAMESPACE_SERVICE(   LHCb,DimErrorLogger);
  DECLARE_NAMESPACE_SERVICE(   LHCb,MBMCnvSvc);
  DECLARE_NAMESPACE_SERVICE(   LHCb,MEPManager);
#ifndef _WIN32
  DECLARE_NAMESPACE_SERVICE(   LHCb,MEPRxSvc);
#endif // _WIN32
  DECLARE_NAMESPACE_SERVICE(   LHCb,MEPHolderSvc);
  DECLARE_NAMESPACE_SERVICE(   LHCb,MEPConverterSvc);
  DECLARE_NAMESPACE_SERVICE(   LHCb,OnlineRunable);
  DECLARE_NAMESPACE_SERVICE(   LHCb,OnlineMessageSvc);
  DECLARE_NAMESPACE_SERVICE(   LHCb,OnlineEvtSelector);
  DECLARE_NAMESPACE_ALGORITHM( LHCb,DecisionSetterAlg);
  DECLARE_NAMESPACE_ALGORITHM( LHCb,MDFSender);
  DECLARE_NAMESPACE_ALGORITHM( LHCb,MDFReceiver);
}
