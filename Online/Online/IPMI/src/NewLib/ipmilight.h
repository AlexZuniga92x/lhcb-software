#include "ipmi_intf.h"
#include "ipmi_chassis.h"
 ipmi_intf * ipmi_light_init_intf(    char *hname,
    char *uname,
    char *pword);
// ,
//struct ipmi_cmd * cmdlist);
//int ipmi_light_do_it( ipmi_intf *ipmi_main_intf,int argc, char ** argv);
int ipmi_light_Cleanup( ipmi_intf *ipmi_main_intf);

