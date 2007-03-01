// $Header: /afs/cern.ch/project/cvs/reps/lhcb/Online/PVSSInterface/src/CfgManager.cpp,v 1.1 2007-03-01 10:39:50 frankb Exp $
//  ====================================================================
//  CfgManager.cpp
//  --------------------------------------------------------------------
//
//  Author    : Markus Frank
//
//  ====================================================================
// $ID: $

// Framework include files
#include "PVSS/CfgManager.h"
#include "PVSS/CfgType.h"
#include <algorithm>

using namespace PVSS;

/// Initializing constructor
CfgManager::CfgManager(ControlsManager* mgr, int i, const std::string& nam)
: NamedIdentified(i,nam), m_manager(mgr)
{
}

/// Standard destructor
CfgManager::~CfgManager()   {
  std::for_each(m_configs.begin(),m_configs.end(),CfgManip<CfgType>());
}

/// Add new configuration
void CfgManager::addConfig(CfgType* cfg)  {
  if ( cfg ) m_configs.push_back(cfg);
}
