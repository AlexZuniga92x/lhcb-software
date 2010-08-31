#define DATATRANSFER_INTERNALS

#include <cstdio>
#include <cerrno>
#include <memory>
#include <vector>
#include <stdexcept>
#include "RTL/que.h"
#include "RTL/Lock.h"
#include "NET/defs.h"
#include "NET/IOPortManager.h"
#include "NET/DataTransfer.h"
#include "TAN/TanInterface.h"

/// one-at-time hash function
static unsigned int hash32(const char* key) {
  unsigned int hash;
  const char* k;
  for (hash = 0, k = key; *k; k++) {
    hash += *k; hash += (hash << 10); hash ^= (hash >> 6); 
  }
  hash += (hash << 3); hash ^= (hash >> 11); hash += (hash << 15);
  return hash;
}

static const unsigned int COPY_LIMIT   = (8192*2);
static const int LINGER_VALUE          =  0;
static const unsigned int CHOP_SIZE    = (8192*4);
//static const unsigned int CHOP_SIZE    = (8192*8);
static const unsigned int LOWER_CHOP   = 4096;
static const int MAX_TCP_ERRORS        = 20;
static const int SNDBUF_VALUE          = 8192;
//static const int SNDBUF_VALUE          = CHOP_SIZE;
static const int RCVBUF_VALUE        = CHOP_SIZE;
//static const int RCVBUF_VALUE          = 100*CHOP_SIZE;

namespace DataTransfer  {
  struct netentry_t {
    int           chan;
    unsigned int  sndBuffSize;
    std::string   name;
    unsigned int  hash;
    sockaddr_in   addr;
    NET*          sys;
    netheader_t   header;
    NetErrorCode  status;
    void*         data;
    netentry_t() : chan(-1), sndBuffSize(0), sys(0), status(NET_SUCCESS)    {}
    ~netentry_t() {}
    void terminate();
    NetErrorCode setSockopts();
    NetErrorCode close();
    NetErrorCode setSendBuff(unsigned int siz);
    NetErrorCode send(const void *buff, size_t siz, unsigned int flag);
    NetErrorCode recv(void *buffer, size_t siz, unsigned int flag);
  };

  struct  NET  {
    //typedef DataTransfer::net_handler_t net_handler_t;
    struct Client {
      net_handler_t data;
      net_handler_t death;
      void*         param;
      unsigned int  fac;
    };
    typedef std::vector<Client> Clients;
    explicit NET(const std::string& proc);
    ~NET();
    unsigned int             m_refCount;
    IOPortManager            m_mgr;
    NetConnectionType        m_type;
    netentry_t               m_me;
    std::map<unsigned int,netentry_t*> m_db;
    lib_rtl_lock_t           m_lockid;
    Clients                  m_clients;
    NetConnectionType type() const { return m_type; }
    netentry_t* connect(const std::string& dest);
    NetErrorCode remove(netentry_t *e);
    void sendShutdown(netentry_t *e);
    NetErrorCode disconnect(netentry_t *e);
    NetErrorCode accept();
    NetErrorCode init(NetConnectionType type);
    NetErrorCode handleMessage(netentry_t* e);
    NetErrorCode getData(netentry_t* e, void* data);
    NetErrorCode setSockopts(int chan);
    NetErrorCode send(const void* buff, size_t size, netentry_t* dest, int fac);
    NetErrorCode send(const void* buff, size_t size, const std::string& dest, int fac);
    NetErrorCode subscribe(void* param, unsigned int fac, net_handler_t data, net_handler_t death);
    NetErrorCode unsubscribe(void* param, unsigned int fac);
    static int acceptAction (void* param);
    static int recvAction (void* param);
    unsigned int release();
    static NET* instance(const std::string& proc, NetConnectionType type);
  };
}
using namespace DataTransfer;

static NET*& inst()  {
  static NET* s_net_instance = 0;
  return s_net_instance;
}

//#define _net_printf printf
static inline void _net_printf(const char*, ...) {}

//----------------------------------------------------------------------------------
void netheader_t::fill(const char* nam,unsigned int hash_val, size_t siz, int fac, int mtype, const void *buf)    {
  magic    = NET_MAGIC;
  size     = htonl(siz);
  facility = htonl(fac);
  msg_type = htonl(mtype);
  hash     = htonl(hash_val);
  strncpy(name,nam,sizeof(name));
  name[sizeof(name)-1] = 0;
  if (buf != 0 && siz>0)  {
    ::memcpy(this+1,buf,siz);
  }
}
//----------------------------------------------------------------------------------
void netheader_t::net_to_host ()  {
  hash     = ntohl(hash);
  size     = ntohl(size);
  msg_type = ntohl(msg_type);
  facility = ntohl(facility);
}
//----------------------------------------------------------------------------------
int NET::recvAction (void* param)    {
  netentry_t* e = (netentry_t*)param;
  e->sys->handleMessage(e);
  return NET_SUCCESS;
}
//----------------------------------------------------------------------------------
NetErrorCode netentry_t::setSockopts()  {
  int on = 1;
  struct linger Linger;
  Linger.l_onoff = 1;
  Linger.l_linger = LINGER_VALUE;
  ::setsockopt(chan, SOL_SOCKET, SO_LINGER,   (const char*)&Linger, sizeof(Linger));
  ::setsockopt(chan, SOL_SOCKET, SO_SNDBUF,   (const char*)&SNDBUF_VALUE, sizeof(int)) ;
  ::setsockopt(chan, SOL_SOCKET, SO_RCVBUF,   (const char*)&RCVBUF_VALUE, sizeof(int)) ;
  ::setsockopt(chan, SOL_SOCKET, SO_REUSEADDR,(const char*)&on, sizeof(on));
  ::setsockopt(chan, SOL_SOCKET, SO_BROADCAST,(const char*)&on, sizeof(on));
  ::setsockopt(chan, SOL_SOCKET, SO_OOBINLINE,(const char*)&on, sizeof(on));
  return NET_SUCCESS;
}
//----------------------------------------------------------------------------------
NetErrorCode netentry_t::setSendBuff(unsigned int siz) {
  unsigned int sndbuf = (siz>CHOP_SIZE) ? CHOP_SIZE : (siz<LOWER_CHOP) ? LOWER_CHOP : siz;
  sndbuf *= 2;
  //sndbuf = siz;
  if ( sndbuf != sndBuffSize )     {
    ::setsockopt(chan, SOL_SOCKET, SO_SNDBUF, (const char*)&sndbuf, sizeof(int));
    sndBuffSize = sndbuf;
  }
  return NET_SUCCESS;
}
//----------------------------------------------------------------------------------
void netentry_t::terminate () {
  if ( addr.sin_port )  {
    ::tan_deallocate_port_number(name.c_str());
    addr.sin_addr.s_addr = 0;
    addr.sin_port = 0;
  }
  if ( chan )  {
    close();
  }
}
//----------------------------------------------------------------------------------
NetErrorCode netentry_t::close()  {
  setSockopts();
  ::shutdown(chan,2);
  ::socket_close(chan);
  chan = 0;
  return NET_SUCCESS;
}
//----------------------------------------------------------------------------------
NetErrorCode netentry_t::send(const void *buffer,size_t siz,u_int flag) {
  const char* buff = (const char*)buffer;
  u_int Npack = siz / CHOP_SIZE;
  u_int Nrest = siz % CHOP_SIZE;
  u_int already_sent = 0;
  for (size_t i = 0; i <= Npack; ++i )  {
    u_int tosend = i<Npack ? CHOP_SIZE : Nrest;
    u_int sent   = 0;
    while (tosend>0)  {
      int sent_now = ::send (chan, buff + already_sent, tosend, flag);
      if (sent_now == -1)  {
        errno = ::lib_rtl_socket_error();
        ::lib_rtl_signal_message(LIB_RTL_OS,"NET: send error size:%d/%d/%d/%d errno=%d",
                                 siz,already_sent,tosend,sent,errno);
        // lib_rtl_start_debugger();
        return NET_ERROR;
      }
      already_sent += sent_now;
      sent         += sent_now;
      tosend       -= sent_now;
    }
  }
  return NET_SUCCESS;
}
//----------------------------------------------------------------------------------
NetErrorCode netentry_t::recv(void *buffer, size_t siz, unsigned int flag)  {
  u_int got = 0;
  int count = 0;
  size_t toget = siz;
  char* buff = (char*)buffer;
  while (got != siz)  {
    int got_now = ::recv (chan, buff + got, toget, flag);
    if (got_now <= 0)    {
      errno = lib_rtl_socket_error();
      _net_printf("NET: receive error errno=%d\n",errno);
      switch(errno)  {
        case ESOCK_CONNREFUSED:
        case ESOCK_NOTCONN:
        case ESOCK_CONNABORTED:
        case ESOCK_CONNRESET:
        case ESOCK_NETRESET:
        case ESOCK_NETDOWN:
        case ESOCK_SHUTDOWN:
          return NET_CONNCLOSED;
        case ESOCK_NOTSOCK:
        case ESOCK_FAULT:
        case ESOCK_INVAL:
        case ESOCK_BADF:
          return NET_TERRIBLE;
        case ESOCK_AGAIN:
          continue;
        default:
          if (++count == MAX_TCP_ERRORS)
            return NET_NODATA;
          else {
            _net_printf("NET: receive error (ignored) errno=%d\n",errno);
            got_now = 0;
          }
      }
    }
    got += got_now;
    toget -= got_now;
  }
  return NET_SUCCESS;
}
//----------------------------------------------------------------------------------
NET::NET(const std::string& proc) 
: m_refCount(0), m_mgr(0), m_type(NET_SERVER), m_lockid(0)  
{
  m_lockid = 0;
  m_me.sys = this;
  m_me.name = proc;
  m_me.hash = hash32(m_me.name.c_str());
  m_me.addr.sin_port = 0;
  m_me.addr.sin_family = AF_INET;
  m_me.addr.sin_addr.s_addr = INADDR_ANY; //IN_CLASSA_HOST; // 
}
//----------------------------------------------------------------------------------
NET::~NET() {
  if ( m_me.chan )  {
    m_mgr.remove(m_me.chan, false);
  }
  m_me.terminate();
  for (std::map<unsigned int,netentry_t*>::iterator i=m_db.begin(); i!=m_db.end();++i)  {
    netentry_t* e = (*i).second;
    if ( e )    {
      netheader_t h;
      h.fill(m_me.name.c_str(),m_me.hash,0,0,NET_CONNCLOSED,0);
      m_mgr.remove(e->chan, false);
      e->setSendBuff(sizeof(h));
      e->send(&h, sizeof(h), 0);
      e->close();
      delete e;
    }
  }
  m_db.clear();
  if (m_lockid)  {
    lib_rtl_unlock(m_lockid);
    lib_rtl_delete_lock(m_lockid);
  }
  m_lockid = 0;
}
//----------------------------------------------------------------------------------
int NET::acceptAction (void* param)    {
  NET* sys = (NET*)param;
  return sys->accept();
}
//----------------------------------------------------------------------------------
NetErrorCode NET::remove(netentry_t *e)    {
  std::map<unsigned int,netentry_t*>::iterator i=m_db.find(e->hash);
  if ( i != m_db.end() )  {
    m_db.erase(i);
    return NET_SUCCESS;
  }
  return NET_TASKNOTFOUND;
}
//----------------------------------------------------------------------------------
netentry_t *NET::connect(const std::string& dest)  {
  std::auto_ptr<netentry_t> e(new netentry_t);
  e->name = dest;
  e->hash = hash32(e->name.c_str());
  int sc = ::tan_get_address_by_name(e->name.c_str(),&e->addr);
  if ( sc != TAN_SS_SUCCESS )  {
    return 0;
  }
  e->sys  = this;
  e->chan = ::socket(AF_INET,SOCK_STREAM,0); 
  if (e->chan == -1)  {
    return 0;
  }
  e->setSockopts();
  sc = ::connect(e->chan,(sockaddr*)&e->addr,sizeof(sockaddr_in));
  if (sc == -1)  {
    ::socket_close(e->chan);
    return 0;
  }
  sc = e->send(m_me.name.c_str(),sizeof(e->header.name),0);
  if ( sc != NET_SUCCESS )  {
    e->close();
    return 0;
  }
  m_mgr.add(1,e->chan,recvAction,e.get());
  return e.release();
}
//----------------------------------------------------------------------------------
NetErrorCode NET::accept()   {
  std::auto_ptr<netentry_t> e(new netentry_t());
  socklen_t n = sizeof (e->addr);
  e->sys  = this;
  e->chan = ::accept(m_me.chan, (sockaddr*)&e->addr, &n);
  if ( e->chan > 0 )  {
    netheader_t h;
    int status = e->recv(h.name,sizeof(h.name),0);
    h.name[sizeof(h.name)-1] = 0;
    e->name = h.name;
    e->hash = hash32(e->name.c_str());
    if ( status != NET_SUCCESS )  {
      e->close();
      return NET_ERROR;
    }
    RTL::Lock lock(m_lockid);
    e->setSockopts();
    m_db[e->hash] = e.get();
    //::printf("New connection: %s chan=%d addr=%s\n",
    //  e->name.c_str(),e->chan,inet_ntoa(e->addr.sin_addr));
    m_mgr.add(0,m_me.chan,acceptAction, this);
    m_mgr.add(1,e->chan,recvAction,e.get());
    e.release();
    return NET_SUCCESS;
  }
  errno = ::lib_rtl_socket_error();
  return NET_ERROR;
}
//----------------------------------------------------------------------------------
NetErrorCode NET::disconnect(netentry_t *e)   {
  //::printf("Close connection: chan=%d addr=%s\n",e->chan,inet_ntoa(e->addr.sin_addr));
  m_mgr.remove(e->chan, false);
  e->close();
  remove(e);
  delete e;
  return NET_SUCCESS;
}
//----------------------------------------------------------------------------------
void NET::sendShutdown(netentry_t *e)  {
  try {
    e->header.fill(e->name.c_str(),e->hash,0,0,NET_TASKDIED,0);
    e->header.net_to_host();
    for(Clients::iterator i=m_clients.begin(); i != m_clients.end(); ++i)  {
      if ( (*i).death ) (*((*i).death))(e,e->header,(*i).param);
    }
  }
  catch(...)  {
    ::lib_rtl_output(LIB_RTL_ERROR,"Exception during disconnect handling.\n");
  }
  disconnect(e);
}
//----------------------------------------------------------------------------------
NetErrorCode NET::handleMessage(netentry_t* e) {
  RTL::Lock lock(m_lockid);
  e->status = e->recv(&e->header,sizeof(netheader_t),0);
  e->header.net_to_host();
  switch (e->status)  {
  case NET_SUCCESS:        // allocate memory for message
    switch(e->header.msg_type)  {
    case NET_CONNCLOSED:
      sendShutdown(e);
      break;
    default:
      e->data = 0;
      for(Clients::iterator i=m_clients.begin(); i != m_clients.end(); ++i)  {
        if ( (*i).data && (*i).fac == e->header.facility ) {
          (*((*i).data))(e,e->header,(*i).param);
        }
      }
      if ( !e->data )  {
        char* buff = new char[e->header.size];
        e->status = getData(e,buff);
        delete [] buff;
      }
      switch(e->status)  {
      case NET_SUCCESS:
        m_mgr.add(1,e->chan,recvAction,e);
        return NET_SUCCESS;
      case NET_CONNCLOSED:
      default:
        sendShutdown(e);
        return NET_SUCCESS;
      }
      break;
    }
    return NET_SUCCESS;
  case NET_NODATA:
    return NET_SUCCESS;
  case NET_CONNCLOSED:
  default:
    sendShutdown(e);
    return NET_SUCCESS;
  }
}
//----------------------------------------------------------------------------------
NetErrorCode NET::send(const void* buff, size_t size, netentry_t* e, int facility)  {
  NetErrorCode status = NET_SUCCESS;
  // Build full source and destination names 
  if (size <= 0)  {
    return NET_ILLEGAL_LENGTH;
  }
  if (e == 0) {
    return NET_TASKNOTFOUND;
  }
  size_t len = size+sizeof(netheader_t);
  e->setSendBuff(len);
  std::auto_ptr<netheader_t> h(new(::operator new(len)) netheader_t);
  if (size <= COPY_LIMIT)    {
    h->fill(m_me.name.c_str(),m_me.hash,size,facility,NET_MSG_DATA,buff);
    status = e->send(h.get(),len,0);
  }
  else    {
    h->fill(m_me.name.c_str(),m_me.hash,size,facility,NET_MSG_DATA,0);
    status = e->send(h.get(),sizeof(netheader_t),0);
    if (status == NET_SUCCESS)  {
      status = e->send(buff,size,0);
    }
  }
  return status;
}
//----------------------------------------------------------------------------------
NetErrorCode NET::send(const void* buff, size_t size, const std::string& dest, int facility)  {
  //
  // This logic here is a bit complicated:
  // The IO port manager MUST be locked before sending, because concurrent messages
  // may arrive (if things go fast!). Taking the lock of the IO port manager
  // prevents the NET from receiving messages in case the sender died and the 
  // netentry_t must be removed. Otherwise a dead-lock situation cannot be
  // avoided.
  //
  // M.Frank  10/08/2007
  //
  NetErrorCode status = NET_CONNCLOSED;
  unsigned int hash = hash32(dest.c_str());
  void* io_lock = m_mgr.lock();
  if ( io_lock ) {
    RTL::Lock lock(m_lockid);
    std::map<unsigned int,netentry_t*>::iterator i=m_db.find(hash);
    netentry_t *e = i != m_db.end() ? (*i).second : 0;
    if ( !e ) {
      m_mgr.unlock(io_lock);
      io_lock = 0;
      if ( !(e=connect(dest)) ) {
        return NET_TASKNOTFOUND;
      }
      m_db[e->hash] = e;                           // Connection ok 
    }
    status = send(buff,size,e,facility);
    if (status != NET_SUCCESS)    {
      disconnect(e);
    }
  }
  if ( io_lock ) m_mgr.unlock(io_lock);
  return status;
}
//----------------------------------------------------------------------------------
NetErrorCode NET::init(NetConnectionType type)  {
  // Only initialize if needed!
  if ( m_me.addr.sin_port == 0 )  {
    int status = 1;
    if ( 0 == m_lockid ) {
      status = ::lib_rtl_create_lock(0, &m_lockid);
      if ( !lib_rtl_is_success(status) )  {
        ::lib_rtl_signal_message(LIB_RTL_OS,"Error creating NET lock. Status %d",status);
        return NET_ERROR;
      }
    }
    m_type = type;
    if ( type == NET_SERVER )  {
      status = ::tan_allocate_port_number(m_me.name.c_str(),&m_me.addr.sin_port);
      if ( status != TAN_SS_SUCCESS )  {
        ::lib_rtl_signal_message(LIB_RTL_OS,"Allocating port number. Status %d",status);
        return NET_ERROR;
      }
      m_mgr.setPort(m_me.addr.sin_port);
      m_me.chan = ::socket(AF_INET, SOCK_STREAM, 0);
      if (m_me.chan == -1)   {
        errno = ::lib_rtl_get_error();
        m_me.chan = 0;
        m_me.terminate();
        return NET_ERROR;
      }
      m_me.setSockopts();
      status = ::bind(m_me.chan,(sockaddr*)&m_me.addr,sizeof(sockaddr_in));
      if (status == -1)  {
        errno = ::lib_rtl_get_error();
        m_me.terminate();
        return NET_ERROR;
      }
      status = ::listen (m_me.chan, 5);
      if (status == -1)  {
        errno = ::lib_rtl_get_error();
        ::socket_close (m_me.chan);
        m_me.terminate();
        return NET_ERROR;
      }
      m_mgr.add(0,m_me.chan,acceptAction,this);
    }
    else   {
      m_mgr.setPort(0xFEED);
      m_mgr.add(0,0,acceptAction,this);
    }
  }
  return NET_SUCCESS;
}
//----------------------------------------------------------------------------------
NetErrorCode NET::getData(netentry_t* e, void* data)  {
  e->status = e->recv(data,e->header.size,0);
  e->data =  data;
  return e->status;
}
//----------------------------------------------------------------------------------
NET* NET::instance(const std::string& proc, NetConnectionType type)  {
  NET*& n = inst();
  if ( 0 == n )  {
    std::auto_ptr<NET> conn(new NET(proc));
    NetErrorCode status = conn->init(type);
    if ( status != NET_SUCCESS )  {
      throw std::runtime_error("Cannot initialize network worker.");
    }
    n = conn.release();
  }
  else if ( n->type() == NET_CLIENT && type == NET_SERVER )  {
    NetErrorCode status = n->init(type);
    if ( status != NET_SUCCESS )  {
      throw std::runtime_error("Cannot initialize network server.");
    }
  }
  RTL::Lock lock(n->m_lockid);
  n->m_refCount++;
  return n;
}
//----------------------------------------------------------------------------------
unsigned int NET::release()  {
  NET*& n = inst();
  if ( n )  {
    void* lock = n->m_mgr.lock();
    ::lib_rtl_lock(n->m_lockid);
    unsigned int cnt = --n->m_refCount;
    if ( cnt == 0 )  {
      __NetworkPort__  port = n->m_mgr.port();
      IOPortManager mgr(port);
      delete n;
      mgr.stop(lock);
      mgr.unlock(lock);
      mgr.join(lock);
      n = 0;
      return cnt;
    }
    ::lib_rtl_unlock(n->m_lockid);
    n->m_mgr.unlock(lock);
    return cnt;
  }
  return 0;
}
//----------------------------------------------------------------------------------
NetErrorCode NET::subscribe(void* param, unsigned int fac, net_handler_t data, net_handler_t death) {
  RTL::Lock lock(m_lockid);
  for(Clients::iterator i=m_clients.begin(); i != m_clients.end(); ++i)  {
    if ( (*i).fac == fac && (*i).param == param ) {
      (*i).data = data;
      (*i).death = death;
      return NET_SUCCESS;
    }
  }
  m_clients.push_back(Client());
  m_clients.back().death = death;
  m_clients.back().data = data;
  m_clients.back().param = param;
  m_clients.back().fac = fac;
  return NET_SUCCESS;
}
//----------------------------------------------------------------------------------
NetErrorCode NET::unsubscribe(void* param, unsigned int fac) {
  RTL::Lock lock(m_lockid);
  for(Clients::iterator i=m_clients.begin(); i != m_clients.end(); ++i)  {
    if ( (*i).fac == fac && (*i).param == param ) {
      m_clients.erase(i);
      return NET_SUCCESS;
    }
  }
  return NET_SUCCESS;
}
//----------------------------------------------------------------------------------
NET* DataTransfer::net_init(const std::string& proc, NetConnectionType type)
{ return NET::instance(proc, type);                 }
void DataTransfer::net_close(NET* net)
{ net->release();                                   }
int DataTransfer::net_receive(NET* net, netentry_t* e, void* buff) 
{ return net->getData(e,buff);                      }
int DataTransfer::net_send(NET* net, const void* buff, size_t size, const std::string& dest, unsigned int fac)
{ return net->send(buff,size,dest,fac);             }
int DataTransfer::net_send(NET* net, const void* buff, size_t size, netentry_t* dest, unsigned int fac)
{ return net->send(buff,size,dest,fac);             }
int DataTransfer::net_subscribe(NET* net, void* param, unsigned int fac, net_handler_t data, net_handler_t death)
{ return net->subscribe(param,fac,data,death);      }
int DataTransfer::net_unsubscribe(NET* net, void* param, unsigned int fac)
{ return net->unsubscribe(param,fac);               }
void* DataTransfer::net_lock(NET* net)
{ return net ? net->m_mgr.lock() : 0;               }
void DataTransfer::net_unlock(NET* net, void* lock)
{ if ( net && lock ) net->m_mgr.unlock(lock);       }

#ifndef ONLINEKERNEL_NO_TESTS

#include "RTL/rtl.h"
namespace {
  void help() {
    ::printf("net_send -opt [-opt]\n");
    ::printf("    -t<urns>=<number>          Number of receive/send turns\n");
    ::printf("    -l<ength>=<number>         Message length (Client only)\n");
    ::printf("    -b<ounce>                  Run in message bounce mode) \n");
  }
  void help_recv() {
    ::printf("net_recv -opt [-opt]\n");
    ::printf("    -b<ounce>                  Run in message bounce mode) \n");
  }
  struct NetSensor  {
    bool m_bounce;
    NET* m_net;
    static void handle_data(netentry_t* e, const netheader_t& hdr, void* param)   {
      NetSensor* s = (NetSensor*)param;
      static time_t start = time(0);
      static char buff[1024*128];
      static int cnt = 0;
      net_receive(s->m_net,e, buff);
      if ( (++cnt)%1000 == 0 )  {
        ::printf("%3ld %s %d messages [%s]. chan:%d port:%d addr:%s\n",
          time(0)-start, s->m_bounce ? "Bounced" : "Received",cnt,
          hdr.name,e->chan,e->addr.sin_port,inet_ntoa(e->addr.sin_addr));
      }
      if ( s->m_bounce )  {
        int sc = net_send(s->m_net,buff,e->header.size,e,e->header.facility);
        if ( sc != NET_SUCCESS )  {
          ::printf("Failed to send message. ret=%d\n",sc);
        }
      }
    }
    static void handle_death(netentry_t* e, const netheader_t& hdr, void* /* param */ )
    { ::printf("Task died: %s chan=%d addr=%s\n",hdr.name,e->chan,inet_ntoa(e->addr.sin_addr));  }
    explicit NetSensor(const std::string& proc,bool bounce=false) : m_bounce(bounce) 
    { net_subscribe(m_net=net_init(proc),this,1,handle_data,handle_death);  }
    virtual ~NetSensor() 
    { net_unsubscribe(m_net,this,1);                         }
  };
}
static std::string host_name()  {
  char host[64];
  ::gethostname(host,sizeof(host));
  hostent* h = gethostbyname(host);
  const char* add = inet_ntoa(*(in_addr*)h->h_addr_list[0]);
  return add;
}
extern "C" int net_send(int argc, char **argv)  {
  char *wmessage;
  int count=1, length=256, loop=100000;
  std::string target = "RCV_0", name="SND_0";

  RTL::CLI cli(argc, argv, help);
  bool bounce = cli.getopt("bounce",1) != 0;
  cli.getopt("count",1,count);
  cli.getopt("length",1,length);
  cli.getopt("turns",2,loop);
  cli.getopt("target",2,target);
  cli.getopt("name",1,name);

  count = bounce ? 1 : count;
  std::string proc = host_name()+"::"+name;
  std::string to   = host_name()+"::"+target;
  ::printf (" Starting net sender:%d turns:%d\n",length,loop);
  while(count-- > 0)  {
    NetSensor c1(proc,bounce);
    if ( length<=0 ) length=10;
    wmessage = new char[length];

    // receive some messages and bounce them
    for (int i=0, mx=loop; mx > 0; --mx, ++i)  {
      for (int k = 0; k < length; k++) wmessage[k] = char((length + k) & 0xFF);
      int sc = net_send(c1.m_net,wmessage, length, to, 1);
      if (sc != NET_SUCCESS)
        printf("Client::send Failed: Error=%d\n",sc);
      if (i % 1000 == 0) printf ("%ld Sent %d messages\n",time(0),i);
    }
    if ( bounce )  {
      while(1)  {
        ::lib_rtl_sleep(100);
      }
    }
  }
  printf("Hit key+Enter to exit ...");
  getchar();
  return 0x1;
}

extern "C" int net_recv(int argc, char **argv)  {
  RTL::CLI cli(argc, argv, help_recv);
  std::string proc = host_name()+"::RCV_0";
  bool run = true, bounce = cli.getopt("bounce",1) != 0;
  printf (" Starting receiver:%s. Bounce:%s\n",proc.c_str(),bounce ? "true" : "false");
  NetSensor cl(proc, bounce);
  while(run)  {
    ::lib_rtl_sleep(100);
  }
  return 0x1;
}
#endif
