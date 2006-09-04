#ifndef GAUDIONLINE_MEPRXSYS_H
#define GAUDIONLINE_MEPRXSYS_H
//	============================================================
//
//	MEPRxSys.h
//	------------------------------------------------------------
//
//	Package   : GaudiOnline
//
//	Author    : Niko Neufeld 
//                 
//  OS dependent part of the event-builder
//	===========================================================
#include <string>
#include "RTL/types.h"

#define MEPRX_PEEK 1
#define MEPRX_WRONG_FD -2
#define MEP_REQ_TTL 10
#define MEP_REQ_TOS 0xFD  /* the D is historic from the MDPs :-) */

/*
 *  namespace MEPRxSys
 */
namespace MEPRxSys {
  /// Print error message
	std::string sys_err_msg(void);
	int open_sock(int ipproto, int rxbufsiz, int netdev, std::string ifname, bool mepreq, std::string &errmsg);
  int rx_select(int sec);
	int recv_msg(void *buf, int len,  int flags);
	int rx_would_block(void);
	std::string dotted_addr(u_int32_t addr);
	int parse_addr(const std::string &straddr, u_int32_t &addr);
	int addr_from_name(const std::string &hname, u_int32_t &addr, std::string &msg);
	int name_from_addr(u_int32_t addr, std::string &hname, std::string &msg);
	int send_msg(u_int32_t addr, u_int8_t, void *buf, int len, int flags);
	void usleep(int us);
} // namespace MEPRxSys

#endif // GAUDIONLINE_MEPRXSYS_H
