// $Id: StompSensor.h,v 1.3 2010/09/22 13:06:48 frankb Exp $
//====================================================================
//  Comet
//--------------------------------------------------------------------
//
//  Package    : Stomp
//
//  Description: DIM enabled Stomp
//
//  Author     : M.Frank
//  Created    : 29/1/2008
//
//====================================================================
// $Header: /local/reps/lhcb/Online/Stomp/Stomp/StompSensor.h,v 1.3 2010/09/22 13:06:48 frankb Exp $
#ifndef STOMP_STOMSERVICE_H
#define STOMP_STOMSERVICE_H

#include "Config.h"
#include "CPP/Sensor.h"
#include "RTL/rtl.h"

// C++ include files
#include <string>
#include <vector>
#include <map>

// Forward declarations
struct apr_pool_t;
struct stomp_connection;

/*
 *   Stomp namespace declaration
 */
namespace Stomp  {

  // Forward declarations
  class StompSensor;

  /**@class DeliveryHandler DeliveryHandler.h Stomp/DeliveryHandler.h
   *
   * @author M.Frank
   */
  class StompSensor : public Sensor   {
  public:
    struct Frame {
      const char *command;
      void *headers;
      const char *body;
      int body_length;
    };
    struct Message {
      apr_pool_t *pool;
      Frame *frame;
      StompSensor* sensor;
      Message(StompSensor* s) : pool(0), frame(0), sensor(s) {}
      ~Message();
      int read();
      const char* channel() const;
    };
    typedef std::vector<Interactor*> Clients;
    typedef std::map<std::string, Clients >  InteractorTable;
  protected:
    /// Service name
    std::string m_name;
    /// Host name of the stomp data sink connection
    std::string m_host;
    /// Stomp port number for data sink connection
    int         m_port;
    /// Flag to be set when stopping the service
    bool        m_stop;
    /// APR memory pool
    apr_pool_t *m_pool;
    /// Reference to the stomp data sink connection
    stomp_connection *m_con;
    /// Client table
    InteractorTable   m_clients;
    /// Thread handle
    lib_rtl_thread_t  m_thread;

    /// Send command to stomp server
    int _command(const std::string& cmd, const std::string& channel, const void* body, int len);
    /// Poll thread start routine
    static int _exec(void* arg);

  public:
    /// Initializing constructor
    StompSensor(const std::string& name, const std::string& host, int port);
    /// Standad destructor
    virtual ~StompSensor();

    /// Set flag to stop polling thread and allow shutdown
    void stop()  {   m_stop = true; };

    /// Destry pool e.g. after reading messages
    void destroyPool(apr_pool_t*& pool);

    /// Connect to stomp server
    int connectServer();
    /// Disconnect from stomp server
    int disconnectServer();

    /// Connect to stomp server
    int connect();
    /// Disconnect from stomp service
    int disconnect();
    /// Read pending data from channel
    int read(Frame*& frame, apr_pool_t*& pool);
    /// Send data to the stomp service (push)
    int send(const std::string& destination, const void* data, size_t len);
    /// Subscribe to data channel
    int subscribe(const std::string& channel);
    /// Unsubscribe from data channel
    int unsubscribe(const std::string& channel);

    /// Start polling for messages....
    int start();
    /// Poll for messages....
    int execute();

    /// Sensor overload: Add interactor to sensor. Dispatch criteria is topic name
    virtual void add( Interactor* client, void* topic);
    /// Sensor overload: Remove interactor from sensor. Dispatch criteria is topic name
    virtual void remove( Interactor* client, void* topic);
    /// Sensor overload: Dispatch Stomp message to clients
    virtual void dispatch( void* arg );
  };
}      // End namespace Stomp
#endif // STOMP_STOMSERVICE_H
