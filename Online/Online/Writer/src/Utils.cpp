#ifdef BUILD_WRITER

#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/poll.h>
#include <errno.h>

#include <stdexcept>

#include "GaudiKernel/MsgStream.h"
#include "Writer/Utils.h"

using namespace LHCb;

int Utils::nameLookup(const char *serverAddr,
  struct sockaddr_in *destAddr, MsgStream * /*log*/) {

	struct hostent* hostname = NULL;
  hostname = gethostbyname(serverAddr);
  if(!hostname || hostname->h_length == 0 || hostname->h_addr_list[0] == NULL) {
  	return -1;
  }
  memcpy(&destAddr->sin_addr.s_addr, hostname->h_addr_list[0], hostname->h_length);
  return 0;
}

int Utils::nameLookup(std::string &serverAddr,
	struct sockaddr_in *destAddr, MsgStream * log) {
  return Utils::nameLookup(serverAddr.c_str(), destAddr, log);
}

int Utils::setupSocket(int sndBufSize, int rcvBufSize, MsgStream *log) {

  socklen_t optlen;
  int retSocket;
  int ret;

	retSocket = socket(AF_INET, SOCK_STREAM, 0);
  if(retSocket < 0) {
    throw std::runtime_error("Could not create socket.");
  }

  //Set options.
  optlen = sizeof(sndBufSize);
  ret = setsockopt(retSocket, SOL_SOCKET, SO_SNDBUF, &sndBufSize, optlen);
  if(ret < 0 && log) {
    *log << MSG::WARNING << "Could not set SO_SNDBUF size." << endmsg;
  }
  ret = setsockopt(retSocket, SOL_SOCKET, SO_RCVBUF, &rcvBufSize, optlen);
  if(ret < 0 && log) {
    *log << MSG::WARNING << "Could not set SO_RCVBUF size." << endmsg;
  }

  return retSocket;
}

int Utils::connectToAddress(struct sockaddr_in *destAddr,
	int sndSize, int rcvSize, MsgStream *log)
{
	int ret;
	int sock;

	sock = setupSocket(sndSize, rcvSize, log);
	if(sock < 0)
		return -1;

  if(log)
	  *log << MSG::INFO << "Connecting. . .Addr: port = " <<
  		((destAddr->sin_addr.s_addr & 0xff)) << "." <<
		  ((destAddr->sin_addr.s_addr & 0xff00) >> 8) << "." <<
		  ((destAddr->sin_addr.s_addr & 0xff0000) >> 16) << "." <<
		  ((destAddr->sin_addr.s_addr & 0xff000000) >> 24) << ":" <<
  ntohs(destAddr->sin_port) << endmsg;

	ret = connect(sock, (struct sockaddr*)destAddr,
      (socklen_t)sizeof(struct sockaddr_in));

  if(log)
    *log << MSG::INFO << "Connected" << endmsg;


	if(ret != 0) {
	   if(log)
	     *log << MSG::ERROR << "Could not connect, errno = " << errno << endmsg;
		return -1;
	}
  return sock;
}

int Utils::closeSocket(int *sock, MsgStream * /*log*/) {
	shutdown(*sock, SHUT_RDWR);
  close(*sock);
  *sock = -1;
  return 0;
}

/**
 * Receives data into the buffer and sleeps till pol
 */
int BIF::nbRecv()
{
	int ret;

	struct pollfd fds[1];
	fds[0].fd = m_sockFd;
	fds[0].events = POLLIN|POLLERR;
	fds[0].revents = 0;

	ret = poll(fds, 1, RECV_TIMEOUT);
	if(ret == 0)
		return AGAIN;
	else if(ret == -1)
		return DISCONNECTED;
	else if(fds[0].revents & POLLERR)
		return DISCONNECTED;
	else if(fds[0].revents & POLLIN) {
		ret = ::recv(m_sockFd, m_data+m_bytesRead, m_bufLen-m_bytesRead, MSG_DONTWAIT);
		if(ret < 0 && (errno == EAGAIN || errno == EINTR))
			return AGAIN;
		else if(ret == 0 || ret < 0)
			return DISCONNECTED;
		else
			m_bytesRead += ret;
	}
	if(m_bytesRead == m_bufLen)
		return m_bytesRead;
	return AGAIN;
}

/**
 * Sends data from the buffer and returns only when the data is completely
 * sent, except in case the thread is stopped.
 */
int BIF::nbSend()
{
	while(1) {
		int ret = ::send(m_sockFd, m_data+m_bytesRead, m_bufLen-m_bytesRead, MSG_DONTWAIT);
		if(ret < 0 && (errno == EAGAIN || errno == EINTR))
			continue;
		else if(ret == 0 || ret < 0)
			return DISCONNECTED;
		else
			m_bytesRead += ret;

		if(m_bytesRead == m_bufLen)
			return m_bytesRead;
	}
	return DISCONNECTED;
}

/**
 * Sends data from the buffer and returns only when the data is completely
 * sent, except in case the thread is stopped.
 */
int BIF::nbSendTimeout()
{
	int ret;
	struct pollfd fds[1];
	time_t endTime;
	fds[0].fd = m_sockFd;
	fds[0].events = POLLOUT|POLLERR;
	fds[0].revents = 0;

	endTime = time(NULL) + SEND_TIMEOUT/1000;

	while(time(NULL) < endTime) {
		ret = ::poll(fds, 1, SEND_TIMEOUT);
		if((ret < 0 && (errno == EAGAIN || errno == EINTR)) || ret == 0)
			continue;
		else if(ret < 0)
			return DISCONNECTED;

		int ret = ::send(m_sockFd, m_data+m_bytesRead, m_bufLen-m_bytesRead, MSG_DONTWAIT);
		if(ret < 0 && (errno == EAGAIN || errno == EINTR))
			continue;
		else if(ret <= 0)
			return DISCONNECTED;
		else
			m_bytesRead += ret;

		if(m_bytesRead == m_bufLen)
			return m_bytesRead;
	}
	return TIMEDOUT;
}

/**
 * Receives data from the buffer and returns only when the data is completely
 * sent, except in case the thread is stopped.
 */
int BIF::nbRecvTimeout()
{
	int ret;
	struct pollfd fds[1];
	time_t endTime;
	fds[0].fd = m_sockFd;
	fds[0].events = POLLIN|POLLERR;
	fds[0].revents = 0;

	endTime = time(NULL) + RECV_TIMEOUT/1000;

	while(time(NULL) < endTime) {
		ret = ::poll(fds, 1, SEND_TIMEOUT);
		if((ret < 0 && (errno == EAGAIN || errno == EINTR)) || ret == 0)
			continue;
		else if(ret < 0)
			return DISCONNECTED;

		int ret = ::recv(m_sockFd, m_data+m_bytesRead, m_bufLen-m_bytesRead, MSG_DONTWAIT);
		if(ret < 0 && (errno == EAGAIN || errno == EINTR))
			continue;
		else if(ret <= 0)
			return DISCONNECTED;
		else
			m_bytesRead += ret;

		if(m_bytesRead == m_bufLen)
			return m_bytesRead;
	}
	return TIMEDOUT;
}



#endif
