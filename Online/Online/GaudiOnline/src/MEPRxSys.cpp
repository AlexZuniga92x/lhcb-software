//	============================================================
//
//	MEPRxSys.cpp
//	------------------------------------------------------------
//
//	Package   : GaudiOnline
//
//	Author    : Niko Neufeld 
//                 
//  System dependent parts of the eventbuilder MEPRxSvc
//	===========================================================
#include "GaudiOnline/MEPRxSys.h"
#ifdef _WIN32
#include <WinSock2.h>
#include <Ws2tcpip.h>
#include <Windows.h>
#define herrno 0
#define hstrerror(x) sys_err_msg()
#define SOL_IP IPPROTO_IP
#define SHUT_RD SD_RECEIVE
#else // UNIX 
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <netdb.h>
#include <ctime>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <cerrno>
#endif // ifndef _WIN32

namespace MEPRxSys {
#ifndef _WIN32
// wrapper classes for Unix System structs
struct MsgHdr: public msghdr {
  MsgHdr(struct iovec *v, int n) {
    msg_iov = v; msg_iovlen = n; msg_control = NULL; msg_controllen = 0; msg_name = NULL; msg_namelen = 0; }
};
struct InAddr: public in_addr {
  InAddr(u_int32_t a) {
    s_addr = a;
  }
};
struct IOVec: public iovec {
  IOVec(void *base, size_t len) {
    iov_base = base; iov_len = len;
  }
};
#endif // _WIN32

static int s = -1;

std::string sys_err_msg(void) {
#ifdef _WIN32	
	char msg[512];
	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), 0, msg, 512, NULL);
	std::string errstr(msg);
#else
	std::string errstr(strerror(errno));
#endif
	return errstr;
}

std::string dotted_addr(u_int32_t addr)
{
	struct in_addr in;

#ifdef _WIN32
	in.S_un.S_addr = addr;
#else
	in.s_addr = addr;
#endif
	return inet_ntoa(in);
}

int open_sock(int ipproto, int rxbufsiz, int netdev, std::string ifname, 
					bool mepreq, std::string &errmsg) 
{
#ifndef _WIN32
  char netdev_name[10];
  ifname = "MickeyMouse";
  int fd;
  if ((fd = open("/proc/raw_cap_hack", O_RDONLY)) != -1) {
    ioctl(fd, 0, 0);	
    close(fd);
  } // if we can't open the raw_cap_hack we have to be root 
#else
  u_int32_t myaddr = inet_addr(ifname.c_str());
	struct in_addr addr;
	addr.S_un.S_addr = myaddr; 
	struct sockaddr_in saddr = {AF_INET, 0, addr }; 
#endif
  if ((s = socket(AF_INET, SOCK_RAW, ipproto)) < 0) {
    errmsg = "socket";
    goto drop_out;
  }
  if (setsockopt(s, SOL_SOCKET, SO_RCVBUF, (const char *)
		 &rxbufsiz, sizeof(rxbufsiz))) {
    errmsg = "setsockopt SO_RCVBUF";
    goto shut_out;
  }
#ifdef _WIN32
	if (myaddr == INADDR_NONE) { 
		errmsg = "inet_addr(" + ifname + ")";
		goto shut_out;
	}
	if (bind(s, (const struct sockaddr *) &saddr, sizeof(saddr))) {
		errmsg = "bind";
		goto shut_out;
	}
#else
  sprintf(netdev_name, netdev < 0 ? "lo" : "eth%d", netdev);           
  if (setsockopt(s, SOL_SOCKET, SO_BINDTODEVICE, (void *) netdev_name,
		 1 + strlen(netdev_name))) {
    errmsg = "setsockopt SO_BINDTODEVICE";
    goto shut_out;
  }
#endif
  if (mepreq) {
    int val;
    val = MEP_REQ_TTL;
    if (setsockopt(s, SOL_IP, IP_TTL, (const char *) &val, sizeof(int))) {
      errmsg = "setsockopt SOL_IP TTL";
      goto shut_out;
    }
    val = MEP_REQ_TOS;
    if (setsockopt(s, SOL_IP, IP_TOS, (const char *) &val, sizeof(int))) {
      errmsg = "setsockopt SOL";
      goto shut_out;
    }
  } 
  return 0;
shut_out:
  shutdown(s, SHUT_RD);
drop_out:
  return 1;
}

int recv_msg(void *buf, int len,  int flags)
{
	int ioflags = 0;
#ifndef _WIN32
	struct IOVec bufs(buf, len);
	struct MsgHdr msg(&bufs, 1);
#endif
	if (flags & MEPRX_PEEK) {
   ioflags |= MSG_PEEK;
	}
#ifdef _WIN32
  int rlen = recv(s, (char *) buf, len, ioflags);
  return ((rlen == -1 && WSAGetLastError() == WSAEMSGSIZE) ? len : rlen);  
#else 
  return (recvmsg(s, &msg, ioflags | MSG_DONTWAIT));
#endif
}

#ifdef _WIN32
int send_msg(u_int32_t addr, void *buf, int len, int flags) {
	int ioflags = 0;
	struct in_addr in;
	in.S_un.S_addr = addr;
	struct sockaddr_in sinaddr = {AF_INET, 0, in, 0,}; 
	return (sendto(s, (const char *) buf, len, ioflags, (const struct sockaddr *) &sinaddr, sizeof(sinaddr)));
#else
int send_msg(u_int32_t addr, void *buf, int len, int /* flags */) {
	struct IOVec bufs(buf, len);	
	struct MsgHdr msg(&bufs, 1);
	struct in_addr in;
	in.s_addr = addr;
	static struct sockaddr _addr = { AF_INET, {0, }};
	memcpy(_addr.sa_data, &addr, 4);
	msg.msg_name = &_addr;
	msg.msg_namelen = sizeof(_addr);
  return (sendmsg(s, &msg, MSG_DONTWAIT | MSG_CONFIRM));
#endif
}

int parse_addr(const std::string &straddr, u_int32_t &addr)
{
#ifdef _WIN32
	if ((addr = inet_addr(straddr.c_str())) == INADDR_NONE) return 1;
	else return 0;
#else
  struct in_addr a;
  int rc = inet_aton(straddr.c_str(), &a);
  addr = a.s_addr;
  return rc ? 0 : 1; // inet_aton returns 1 on success!
#endif
}

int 
name_from_addr(u_int32_t addr, std::string &hname, std::string &msg)
{
  struct hostent *h;

  if (!(h = gethostbyaddr((const char *) &addr, 4, AF_INET))) {
    msg = dotted_addr(addr) + " " + hstrerror(h_errno);
    return 1;
  } 
  hname = h->h_name;
	msg = "";
  return 0;
}

int 
addr_from_name(const std::string &hname, u_int32_t &addr, std::string &msg) 
{
  struct hostent *h;

  if (!(h = gethostbyname(hname.c_str()))) {
    msg = hname + " " + hstrerror(h_errno);
    return 1;
  }
  addr = *((u_int32_t *) h->h_addr_list[0]);    
  return 0;
}

void usleep(int us) 
{
#ifdef _WIN32
  long millisecs = us/1000;
  ::Sleep( millisecs<=0 ? 1 : millisecs);
#else
	struct timespec t = { 0, 1000 * us }; // 0 s,  ns
  nanosleep(&t, &t);
#endif
}

int rx_select(int sec)
{
  fd_set rfds;
  FD_ZERO(&rfds);
  FD_SET(s, &rfds);
  struct timeval timeout = {sec, 0}; /* seconds */
  int maxfd = s + 1;
  int n;
  
  n = select(maxfd, &rfds, NULL, NULL, &timeout);
#ifdef _WIN32
  if (n == SOCKET_ERROR) return -1;
#else
  if (n == -1) return -1;
#endif
  if (n == 1 && !FD_ISSET(s, &rfds)) return MEPRX_WRONG_FD;
  return n;
}

int rx_would_block() 
{
#ifdef _WIN32
	if (WSAGetLastError() == WSAEWOULDBLOCK) return 1;
#else
	if (errno == EAGAIN) return 1;
#endif
	return 0;
}


} // namespace MEPRxSys
