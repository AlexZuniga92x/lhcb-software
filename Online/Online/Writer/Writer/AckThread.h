#ifndef ACKTHREAD_H_
#define ACKTHREAD_H_

#include "Writer/Connection.h"
#include "Writer/MM.h"
#include "Writer/chunk_headers.h"
#include "GaudiKernel/MsgStream.h"

#include <pthread.h>

namespace LHCb {

	class Connection;
	class INotifyClient;
	/**
	 * Takes care of purging the queue and acking chunks to the server.
	 */
	class AckThread {

		private:

			Connection *m_conn;
			MsgStream *m_log;
			MM *m_mmObj;
			volatile int m_stopAcking;
			volatile int m_sockFd;
			pthread_t m_ackThread;
			INotifyClient *m_notifyClient;

		public:
			AckThread(Connection *conn, int sockfd, MM *mmObj, MsgStream *log) {
				m_conn = conn;
				m_log = new MsgStream(*log);
				m_mmObj = mmObj;
				m_sockFd = sockfd;
				m_notifyClient = NULL;
			}
			virtual ~AckThread() { delete m_log; }
			int getState(void) { return m_stopAcking; }
			void restoreState(int state) { m_stopAcking = state; }
			void start(void);
			void stop(int stopLevel);
			void reInit(int sockfd);
			int processAcks(void);
			void notify(struct cmd_header *cmd);
			void setNotifyClient(INotifyClient *nl) {
				m_notifyClient = nl;
			}

	};


}

#endif /*ACKTHREAD_H_*/
