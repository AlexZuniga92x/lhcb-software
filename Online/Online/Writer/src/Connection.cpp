#ifdef BUILD_WRITER

#include <string>
#include <cerrno>
#include <stdexcept>
#include <iostream>

#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>
#include <netdb.h>
#include <pthread.h>
#include <unistd.h>
extern "C" {
#include "Writer/chunk_headers.h"
}

#include "Writer/FailoverMonitor.h"
#include "Writer/Connection.h"
#include "Writer/MM.h"
#include "Writer/Utils.h"
#include "Writer/SendThread.h"

#include "GaudiKernel/MsgStream.h"

#define POLL_INTERVAL         5000   /*<< The interval for poll() calls in millis.*/
#define FAILOVER_RETRY_SLEEP  20     /*<< Number of seconds between  retries.*/

/**
 * A thread local variable to keep track of which thread we're in.
 */
//__thread int currThread;

using namespace LHCb;

/**
 * Constructor.
 * Does nothing except initialise member variables.
 */
Connection::Connection(std::string serverAddr, int serverPort, int sndRcvSizes,
    MsgStream * log, INotifyClient *nClient) {

  m_seqCounter = 0;
  m_state = Connection::STATE_CONN_CLOSED;
  m_serverAddr = serverAddr;
  m_serverPort = serverPort;
  m_log = log;
  m_notifyClient = nClient;
  m_sndRcvSizes = sndRcvSizes;
  m_failoverMonitor = NULL;

  pthread_mutex_init(&m_failoverLock, NULL);
  //currThread = MDFWRITER_THREAD;
}

/**
 * Connects to a storage cluster node.
 */
void Connection::initialize(void) {
	m_failoverMonitor = new FailoverMonitor(m_serverAddr, m_serverPort + 1, this, m_log);
	*m_log << MSG::INFO << "Instantiated failover monitor" << endmsg;

	connect();
	*m_log << MSG::INFO << "Connected stream." << endmsg;

	m_failoverMonitor->start();
	*m_log << MSG::INFO << "Started failover monitor." << endmsg;


	m_ackThread = new AckThread(this, m_sockfd, &m_mmObj, m_log);
	m_ackThread->setNotifyClient(m_notifyClient);
	m_ackThread->start();
	*m_log << MSG::INFO << "Started ack thread." << endmsg;

	m_sendThread = new SendThread(this, m_sockfd, &m_mmObj, m_log);
	m_sendThread->start();
	*m_log << MSG::INFO << "Started send thread." << endmsg;

}

/** Connects to a storage cluster node.
 */
void Connection::connect() {
  struct sockaddr_in destAddr;
  m_sockfd = -1;
  m_state = Connection::STATE_CONN_CLOSED;
  m_failoverMonitor->getAddress(&destAddr);
	/*Need to change port to the storage service port.*/
	destAddr.sin_port = htons(m_serverPort);
	m_sockfd = Utils::connectToAddress(&destAddr, m_sndRcvSizes, m_sndRcvSizes, m_log);
	if(m_sockfd < 0)
		throw std::runtime_error("Could not init stream to server (found fo).");
 	m_state = Connection::STATE_CONN_OPEN;
}

/** Closes the TCP connection.
 * This function blocks till all messages in the queue
 * are confirmed to have been sent.
 */
void Connection::closeConnection()
{
	*m_log << MSG::INFO << "Stopping everything ..." << endmsg;

  m_sendThread->stop(STOP_AFTER_PURGE);	/*Stop after all messages are sent.*/
  m_ackThread->stop(STOP_AFTER_PURGE);	/*Stop after all acks have been received.*/
  m_failoverMonitor->stop();
  delete m_sendThread;
  delete m_ackThread;
  delete m_failoverMonitor;

  Utils::closeSocket((int*)&m_sockfd, m_log);
}

/** Fails over onto an alternative storage cluster server.
 * This entails connecting to a different server and
 * re-sending all the queued requests.
 * @param stopAckThread If 1, then the ackThread needs
 * to be stopped. If 0, then it means that the function
 * is being called by the ack thread, and the thread
 * will stop on its own after function returns.
 */
int Connection::failover(int currThread)
{
	int ret = 0;

	ret = pthread_mutex_trylock(&m_failoverLock);
	if(ret == EBUSY) /* Someone else is already in the failover routine. Die.*/
		return KILL_THREAD;

	switch(currThread) {
		case ACK_THREAD:
			*m_log << MSG::INFO << "Failover initiated by ACK thread." <<
				errno << endmsg;
			break;
		case SEND_THREAD:
			*m_log << MSG::INFO << "Failover initiated by SEND thread." <<
				errno << endmsg;
			break;
		case FAILOVER_THREAD:
			*m_log << MSG::INFO << "Failover initiated by FAILOVER thread." <<
				errno << endmsg;
			break;
	}

	/*m_stopSending could be STOP_PURGE or 0, we must save and restore. */
	int oldSendStopLevel = m_sendThread->getState();
	int oldAckStopLevel = m_ackThread->getState();

	/*
	 * This is called by one of the three threads. Stop the other two.
	 */
	if(currThread == ACK_THREAD) {
		m_sendThread->stop(STOP_URGENT);
		m_failoverMonitor->stop();
	} else if(currThread == SEND_THREAD) {
		m_ackThread->stop(STOP_URGENT);
		m_failoverMonitor->stop();
	} else if(currThread == FAILOVER_THREAD) {
		m_sendThread->stop(STOP_URGENT);
		m_ackThread->stop(STOP_URGENT);
	}

	Utils::closeSocket((int*)&m_sockfd, m_log);

	while(1) {
  	*m_log << MSG::INFO << "Reconnecting socket now..." << endmsg;
	  try {
		  m_failoverMonitor->connectToAlternative();
  		connect();
  		break;
  	}catch(std::exception ex) {
  		*m_log << MSG::WARNING << "Could not connect:" << ex.what() << endmsg;
	  }
	  sleep(1);
	}

  m_ackThread->reInit(m_sockfd);	/*Reinit ack thread's data.*/
	m_sendThread->reInit(m_sockfd);	/*Reinit send thread's data.*/

	/*
	 * Start up the threads that we stopped.
	 */
  if(currThread == FAILOVER_THREAD) {
  	m_ackThread->start();
  	m_sendThread->start();
  } else if(currThread == ACK_THREAD) {
  	m_failoverMonitor->start();
  	m_sendThread->start();
	} else if(currThread == SEND_THREAD) {
		m_failoverMonitor->start();
  	m_ackThread->start();
  }

  /* It could be possible that the failover is taking place
   * during a finalize()/Connection::closeConnection(), where the
   * threads were in STOP_AFTER_PURGE state. So for consistency,
   * we just restore them to whatever state they were in.
   */
  m_ackThread->restoreState(oldAckStopLevel);
  m_sendThread->restoreState(oldSendStopLevel);

  *m_log << MSG::INFO << "Successfully failed over. " << endmsg;
  pthread_mutex_unlock(&m_failoverLock);

  return 0;
}


/**
 * Equivalent to sendCommand(header, NULL);
 */
void Connection::sendCommand(struct cmd_header *header)
{
  sendCommand(header, NULL);
}

/** Sends a command to the server.
 * This command can be a close request, an open request, or a
 * write request. The Connection class takes care of failing
 * over in case of a problem. In case the command could not be
 * sent even after a failover attempt, then the INotifyClient
 * interface is used to notify the calling class of the error.
 * @param header The command header which contains the command
 *               to send. The sequence number does not need to
 *               be set at this point. This is set automatically
 *               by this method.
 * @param data   In case of the WRITE_CHUNK command, this will
 *               be non-NULL. The sizeof this buffer will be
 *      determined from header.
 */
void Connection::sendCommand(struct cmd_header *header, void *data)
{
  struct cmd_header *newHeader;
  int totalSize = 0;
  switch(header->cmd) {
    case CMD_WRITE_CHUNK:
      totalSize += header->data.chunk_data.size;
    case CMD_CLOSE_FILE:
      m_state = STATE_CONN_OPEN;
    case CMD_OPEN_FILE:
      m_state = STATE_FILE_OPEN;
      totalSize += sizeof(struct cmd_header);
      break;
  }

  header->data.chunk_data.seq_num = m_seqCounter++;
  newHeader = m_mmObj.allocAndCopyCommand(header, data);	/* Will always succeed. */

  m_mmObj.enqueueCommand(newHeader);
}

#endif /* _WIN32 */
