// $ID: $

// Include Files
#include "GaudiKernel/SmartIF.h"
#include "GaudiKernel/System.h"
#include "GaudiKernel/IService.h"
#include "GaudiKernel/IProperty.h"
#include "GaudiKernel/ISvcLocator.h"
#include "Gaudi/ServiceMenu.h"
#include "Gaudi/PropertyMenu.h"
#include "CPP/IocSensor.h"
#include "Gaudi/DialogMenu.h"
#include "CPP/Event.h"
#include "RTL/rtl.h"

#include <list>

namespace {
  static const char* serviceState(const IService* is)  {
    switch(is->state())
    {
    case IService::OFFLINE:     return "OFFLINE";
    case IService::CONFIGURED:  return "CONFIGURED";
    case IService::FINALIZED:   return "FINALIZED";
    case IService::INITIALIZED: return "INITIALIZED";
    default:                    return "UNKNOWN";
    }
  }
}

/// Initializing constructor
Gaudi::ServiceMenu::ServiceMenu(ISvcLocator* svc, Interactor* par) 
: SubMenu("Hit return on service for properties",par), m_svcLoc(svc)  {
  const std::list<IService*>& svcs = m_svcLoc->getServices();
  if ( !svcs.empty() )  {
    int cmd = CMD_SHOW;
    const char* fmt = "%-32s %-32s  %s";
    m_window->addCOM(cmd, fmt, "Name", "Type", "State");
    for(std::list<IService*>::const_iterator i=svcs.begin(); i != svcs.end(); ++i )  {
      const IService* is = *i;
      std::string typ = System::typeinfoName(typeid(*is));
      m_lines.insert(std::make_pair(++cmd,const_cast<IService*>(is)));
      m_window->addCMD(cmd, fmt, is->name().c_str(),typ.c_str(),serviceState(is));
    }
  }
  else  {
    m_window->addCOM(CMD_SHOW,"No services found!");
  }
  mapWindow();
}

/// Standard destructor
Gaudi::ServiceMenu::~ServiceMenu()  {
}

/// Interactor overload: handle menu interupts
void Gaudi::ServiceMenu::handle (const Event& event)   {
  if ( event.eventtype == UpiEvent )  {
    int cmd = event.command_id;
    if ( cmd > CMD_SHOW )  {
      Lines::iterator i=m_lines.find(cmd);
      if ( i != m_lines.end() )  {
        m_cursor = cmd;
        m_cursor = cmd;
        if ( m_child ) delete m_child;
        m_child = new PropertyMenu((*i).second->name(),(*i).second,this);
      }
      return;
    }
  }
  SubMenu::handle(event);
}

