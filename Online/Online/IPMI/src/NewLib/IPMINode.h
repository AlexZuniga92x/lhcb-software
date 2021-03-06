#ifndef IPMISRV_H
#define IPMISRV_H
#include <string>
using namespace std;
//  #include "ipmilight.h"
enum IPMIState
{
  uninited,
  inited,
  on,
  off
};
enum PowerState
{
  Power_unknown = -1,
  Power_off,
  Power_on
};
class DimService;
class DimCommand;
class ipmi_intf;
class IPMINode
{
  public:
#include "newnew.h"
    string m_Name;
    string m_intName;
    string m_username;
    string m_password;
    ipmi_intf *m_ipmiInterface;
    ipmi_intf *m_retintf;
    IPMINode (string &nname,string &uname,string &passw);
    ~IPMINode();
    IPMIState m_state;
    PowerState m_pstate;
    DimService *m_service;
    DimService *m_lastUpdate;
    DimCommand *m_cmd;
    string m_servName;
    string m_cmdName;
    int m_svalue;
    int m_lastread;
    int turnOn();
    int turnOff();
    int cycle();
    int reset();
    int getPowerState();
    int setup(char *DimserverName);
    int shutdown();
};
#endif
