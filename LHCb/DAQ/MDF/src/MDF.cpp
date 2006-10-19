// $Id: MDF.cpp,v 1.14 2006-10-19 14:12:02 frankb Exp $
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

LOAD_FACTORY_ENTRIES(MDF)

#include "GaudiKernel/DeclareFactoryEntries.h"

DECLARE_FACTORY_ENTRIES(MDF)  {
  DECLARE_NAMESPACE_SERVICE(  LHCb,MDFSelector);
  DECLARE_NAMESPACE_SERVICE(  LHCb,MIFSelector);
  DECLARE_NAMESPACE_SERVICE(  LHCb,RawDataCnvSvc);
  DECLARE_NAMESPACE_ALGORITHM(LHCb,MDFWriter);
#ifndef _WIN32
  DECLARE_NAMESPACE_ALGORITHM(LHCb,MDFWriterLite);
#endif
  DECLARE_NAMESPACE_ALGORITHM(LHCb,MIFWriter);
  DECLARE_NAMESPACE_ALGORITHM(LHCb,MEPWriter);
  DECLARE_NAMESPACE_ALGORITHM(LHCb,RawDataWriter);
  DECLARE_NAMESPACE_ALGORITHM(LHCb,MEPDump);
  DECLARE_NAMESPACE_ALGORITHM(LHCb,MEPTester);
  DECLARE_NAMESPACE_ALGORITHM(LHCb,RawEventCopy);
  DECLARE_NAMESPACE_ALGORITHM(LHCb,RawEventTestDump);
  DECLARE_NAMESPACE_ALGORITHM(LHCb,RawEventTestCreator);
} 
