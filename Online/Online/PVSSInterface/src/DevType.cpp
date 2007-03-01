// $Header: /afs/cern.ch/project/cvs/reps/lhcb/Online/PVSSInterface/src/DevType.cpp,v 1.1 2007-03-01 10:39:50 frankb Exp $
//  ====================================================================
//  DevType.cpp
//  --------------------------------------------------------------------
//
//  Author    : Markus Frank
//
//  ====================================================================
// $ID: $

// Framework include files
#include "PVSS/DevType.h"
#include "PVSS/DevTypeElement.h"
#include <algorithm>

using namespace PVSS;


/// Initializing constructor
DevType::DevType(DevTypeManager* mgr, int i, const std::string& nam, Type typ)
: DevTypeElement(0,i,nam,typ), m_manager(mgr)
{
  m_type = this;
}

/// Standard destructor
DevType::~DevType()   {
}
