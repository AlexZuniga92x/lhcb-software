/*
 * ipmi_chassis.h
 *
 *  Created on: Feb 1, 2016
 *      Author: beat
 */

#ifndef ONLINE_IPMI_SRC_NEWLIB_IPMI_CHASSIS_H_
#define ONLINE_IPMI_SRC_NEWLIB_IPMI_CHASSIS_H_

/*
 * Copyright (c) 2003 Sun Microsystems, Inc.  All Rights Reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * Redistribution of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 * Redistribution in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * Neither the name of Sun Microsystems, Inc. or the names of
 * contributors may be used to endorse or promote products derived
 * from this software without specific prior written permission.
 *
 * This software is provided "AS IS," without a warranty of any kind.
 * ALL EXPRESS OR IMPLIED CONDITIONS, REPRESENTATIONS AND WARRANTIES,
 * INCLUDING ANY IMPLIED WARRANTY OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE OR NON-INFRINGEMENT, ARE HEREBY EXCLUDED.
 * SUN MICROSYSTEMS, INC. ("SUN") AND ITS LICENSORS SHALL NOT BE LIABLE
 * FOR ANY DAMAGES SUFFERED BY LICENSEE AS A RESULT OF USING, MODIFYING
 * OR DISTRIBUTING THIS SOFTWARE OR ITS DERIVATIVES.  IN NO EVENT WILL
 * SUN OR ITS LICENSORS BE LIABLE FOR ANY LOST REVENUE, PROFIT OR DATA,
 * OR FOR DIRECT, INDIRECT, SPECIAL, CONSEQUENTIAL, INCIDENTAL OR
 * PUNITIVE DAMAGES, HOWEVER CAUSED AND REGARDLESS OF THE THEORY OF
 * LIABILITY, ARISING OUT OF THE USE OF OR INABILITY TO USE THIS SOFTWARE,
 * EVEN IF SUN HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
 */


#include "ipmi.h"

#define IPMI_CHASSIS_CTL_POWER_DOWN 0x0
#define IPMI_CHASSIS_CTL_POWER_UP   0x1
#define IPMI_CHASSIS_CTL_POWER_CYCLE    0x2
#define IPMI_CHASSIS_CTL_HARD_RESET 0x3
#define IPMI_CHASSIS_CTL_PULSE_DIAG 0x4
#define IPMI_CHASSIS_CTL_ACPI_SOFT  0x5

#define IPMI_CHASSIS_POLICY_NO_CHANGE   0x3
#define IPMI_CHASSIS_POLICY_ALWAYS_ON   0x2
#define IPMI_CHASSIS_POLICY_PREVIOUS    0x1
#define IPMI_CHASSIS_POLICY_ALWAYS_OFF  0x0

//int ipmi_chassis_power_status(struct ipmi_intf * intf);
//int ipmi_chassis_power_control(struct ipmi_intf * intf, uint8_t ctl);
//int ipmi_chassis_main(struct ipmi_intf * intf, int argc, char ** argv);
//int ipmi_power_main(struct ipmi_intf * intf, int argc, char ** argv);

class ipmi_intf;
class ipmi_chassis
{
  public:
    ipmi_intf *intf;
    ipmi_chassis (ipmi_intf *);
    int ipmi_chassis_power_status();
    int ipmi_chassis_print_power_status( );
    int ipmi_chassis_power_control( uint8_t ctl);
    int ipmi_chassis_identify( char * arg);

    int ipmi_chassis_poh( );

    int ipmi_chassis_restart_cause( );
    int ipmi_chassis_status( );
    int ipmi_chassis_selftest( );
    int ipmi_chassis_set_bootparam( uint8_t param, uint8_t * data, int len);
    int ipmi_chassis_get_bootparam( char * arg);
    int get_bootparam_options(char *optstring,unsigned char *set_flag, unsigned char *clr_flag);
    int ipmi_chassis_get_bootvalid( );
    int ipmi_chassis_set_bootvalid( uint8_t set_flag, uint8_t clr_flag);
    int ipmi_chassis_set_bootdev( char * arg, uint8_t *iflags);
    int ipmi_chassis_power_policy( uint8_t policy);
    int ipmi_power_main(int argc, char ** argv);
    void ipmi_chassis_set_bootflag_help();
    int ipmi_chassis_main(int argc, char ** argv);
};



#endif /* ONLINE_IPMI_SRC_NEWLIB_IPMI_CHASSIS_H_ */
