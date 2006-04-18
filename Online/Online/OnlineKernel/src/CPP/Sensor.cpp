/*
**++
**  FACILITY:  DEXPERT (sensor.cc)
**
**  MODULE DESCRIPTION:
**
**	Sensor class. Object Sensor is used to interface any external estimulus
**      to the program with the corresponding interactor. There exists one
**      Sensor object for each different input type to the program. For example,
**      AMS messages, alarms, keyword , etc.
**
**  AUTHORS:
**
**      P. Mato
**
**  CREATION DATE:  14-Nov-1990
**
**  DESIGN ISSUES:
**
**
**  MODIFICATION HISTORY:
**
**      {@tbs@}...
**--
*/

#include "CPP/Sensor.h"
#include "WT/wtdef.h"
#include "RTL/rtl.h"

static bool    Initdone  = false;
static Sensor *Head      = 0;

//------------------------------------------------------------------------------
int s_rearm(unsigned int /* fac */, void* /* param */ )    {
  for(Sensor* sn = Head; sn; sn = sn->next())  {
    if( sn->m_rearmPending )    { 
      sn->rearm();
      sn->setRearm(false);
    }
  }
  return WT_SUCCESS;
}

//------------------------------------------------------------------------------
int s_action(unsigned int facility, void* param) {
  for(Sensor* sn = Head; sn; sn = sn->next())  {
    if ( sn->m_facility == facility )  {
      sn->setRearm(true);
      sn->dispatch(param);
      break;
    }
  }   
  return WT_SUCCESS;
}
//------------------------------------------------------------------------------
Sensor::Sensor( unsigned int facility, const char* name, bool wtsubs) {
  int status;
  if ( !Initdone )  {
    status = wtc_init();
    if ( status != WT_SUCCESS ) lib_rtl_signal_message(LIB_RTL_ERRNO,"Failed wtc_init");
  } 
  Name = name;
  m_next = Head;
  Head = this;
  m_facility = facility;

  if( wtsubs )  {
    status = wtc_subscribe( m_facility, s_rearm, s_action );
    if ( status != WT_SUCCESS ) lib_rtl_signal_message(LIB_RTL_ERRNO,"Failed wtc_subscribe");
  }
  m_rearmPending = true;
}
//------------------------------------------------------------------------------
Sensor::~Sensor() {
  int status = wtc_remove( m_facility );
  if ( status != WT_SUCCESS ) lib_rtl_signal_message(LIB_RTL_ERRNO, "Failed wtc_remove");
}
//------------------------------------------------------------------------------
int Sensor::run()  {
  int status, substatus;
  unsigned int facility;
  void* param;
  while(1)  {
    status = wtc_wait( &facility, &param, &substatus );  
    if ( status != WT_SUCCESS && status != WT_BADACTIONSTAT && status != WT_NOSUBSCRIBED) {
      lib_rtl_printf (" exiting wt_wait status = %d param %p substat %d, fac %d \n",
              status, param, substatus, facility);
    }
  } 
  return 1;
}
