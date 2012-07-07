/*
 * qaul.net is free software
 * licensed under GPL (version 3)
 */

#include "qaullib_private.h"

#ifdef WIN32
#define close(x) closesocket(x)
#undef errno
#define errno WSAGetLastError()
#undef strerror
#define strerror(x) StrError(x)
#define perror(x) WinSockPError(x)
#endif

#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0
#endif


// ------------------------------------------------------------
int Qaullib_IpcConnect(void)
{

#ifdef WIN32
  int On = 1;
  unsigned long Len;
#else
  int flags;
#endif

  ipc_connected = 0;

  // fill in the socket structure with host information
  struct sockaddr_in saddr;
  saddr.sin_family = AF_INET;
  saddr.sin_port = htons(IPC_PORT);
  inet_aton("127.0.0.1", &saddr.sin_addr); // numeric IP only

  if ((ipc_socket = socket(PF_INET, SOCK_STREAM, 0)) == -1)
  {
	  perror("socket");
#ifdef WIN32
      int err = WSAGetLastError();
#else
      int err = -1;
#endif
      return err;
  }

  printf("Attempting connect...");

  // connect to PORT on HOST
  if ((ipc_conn = connect(ipc_socket, (struct sockaddr *)&saddr, sizeof(saddr))) < 0)
  {
	  fprintf(stderr, "Error connecting %d - %s\n", errno, strerror(errno));
	  // close socket if there was an error
	  Qaullib_IpcClose();
	  //return (int)errno;
  }
  else
  {
    printf("Connected!! socket: %i connection: %i\n",ipc_socket,ipc_conn);

    // Setting socket non-blocking
#ifdef WIN32
    if (WSAIoctl(ipc_socket, FIONBIO, &On, sizeof(On), NULL, 0, &Len, NULL, NULL) < 0)
    {
      fprintf(stderr, "Error while making socket non-blocking!\n");
    }
#else
    if ((flags = fcntl(ipc_socket, F_GETFL, 0)) < 0)
    {
      fprintf(stderr, "Error getting socket flags!\n");
    }

    if (fcntl(ipc_socket, F_SETFL, flags | O_NONBLOCK) < 0)
    {
      fprintf(stderr, "Error setting socket flags!\n");
    }
#endif
    ipc_connected = 1;
    return 1;
  }
  return 0;
}


// ------------------------------------------------------------
int Qaullib_IpcClose(void)
{
  if (close(ipc_socket))
    return 1;

  return 0;
}

// ------------------------------------------------------------
void Qaullib_IpcReceive(void)
{
	int bytes, tmp_len;
  char *tmp;
  union
  {
    char buf[BUFFSIZE + 1];
    union olsr_message msg;
  } inbuf;

  //printf("[qaullib] Qaullib_IpcReceive()\n");

  if (!ipc_connected)
  {
		printf("Connection closed, try to reconnect ...\n");
		// connect to the application
		Qaullib_IpcConnect();
  }
  else
  {
	  memset(&inbuf, 0, BUFFSIZE + 1);

    bytes = recv(ipc_socket, (char *)&inbuf, BUFFSIZE, 0);
    if (bytes == 0)
    {
      shutdown(ipc_socket, SHUT_RDWR);
      printf("bytes == 0: close connection\n");
      //set_net_info("Disconnected from server...", 1);
      ipc_connected = 0;
      close(ipc_socket);
    }

    if (bytes > 0)
    {
		printf("bytes: %i msg-size: %i type: %i\n", bytes, (int) ntohs(inbuf.msg.v4.olsr_msgsize), (int) inbuf.msg.v4.olsr_msgtype);

      tmp = (char *)&inbuf.msg;
      qaul_in_msg = &inbuf.msg;

		// do it as often as needed until all messages are out of the buffer.
		if (bytes > 0 && ntohs(inbuf.msg.v4.olsr_msgsize) <= bytes)
		{
			printf("[qaullib] IPC: message received \n");

			while (bytes > 0 && ntohs(qaul_in_msg->v4.olsr_msgsize) <= bytes)
			{
				//printf("read out bytes: %i %i\n", bytes, ntohs(qaul_in_msg->v4.olsr_msgsize));

				// proceed
				Qaullib_IpcEvaluateMessage(qaul_in_msg);

				// copy buffer to new location
				tmp_len = ntohs(qaul_in_msg->v4.olsr_msgsize);
				qaul_in_msg = (union olsr_message *)&tmp[tmp_len];
				tmp = &tmp[tmp_len];
				if (tmp_len == 0)
					break;
				bytes = bytes - tmp_len;
				tmp_len = ntohs(qaul_in_msg->v4.olsr_msgsize);

				// Copy to start of buffer
				if (tmp_len > bytes) {
					// Copy the buffer
					memcpy(&inbuf, tmp, bytes);
					bytes = recv(ipc_socket, (char *)&inbuf.buf[bytes], tmp_len - bytes, 0);
					tmp = (char *)&inbuf.msg;
					qaul_in_msg = (union olsr_message *)tmp;
				}
			}
		}
    }
  }
}


// ------------------------------------------------------------
void Qaullib_IpcEvaluateMessage(union olsr_message *msg)
{
	//printf("message arrived: %i\n", msg->v4.olsr_msgtype);
	switch(msg->v4.olsr_msgtype)
	{
		case 222:
			Qaullib_IpcEvaluateChat(msg);
			break;
		case 223:
			Qaullib_IpcEvaluateCom(msg);
			break;
		case 224:
			Qaullib_IpcEvaluateTopo(msg);
			break;
		default:
			printf("not a known message type\n");
			break;
	}
}

// ------------------------------------------------------------
void Qaullib_IpcEvaluateChat(union olsr_message *msg)
{
	char buffer[10240];
	char* stmt = buffer;
	char *error_exec = NULL;
	char ipbuf[MAX(INET6_ADDRSTRLEN, INET_ADDRSTRLEN)];
	char chat_msg[MAX_MESSAGE_LEN +1];
	char chat_user[MAX_USER_LEN +1];

	//printf("IpcEvaluateChat\n");
	//printf("type: %i, name: %s\n", msg->v4.olsr_msgtype, msg->v4.message.chat.name);

	// get chat & username
	memcpy(&chat_user, msg->v4.message.chat.name, MAX_USER_LEN);
	memcpy(&chat_user[MAX_USER_LEN], "\0", 1);
	memcpy(&chat_msg, msg->v4.message.chat.msg, MAX_MESSAGE_LEN);
	memcpy(&chat_msg[MAX_MESSAGE_LEN], "\0", 1);

	// TODO: ipv6
	sprintf(stmt, sql_msg_set_received,
			1,
			chat_user,
			chat_msg,
			inet_ntop(AF_INET, &msg->v4.originator, &ipbuf, sizeof(ipbuf)),
			4,
			(int)msg->v4.hopcnt,
			(int)msg->v4.ttl,
			(int)ntohs(msg->v4.seqno),
			me_to_reltime(msg->v4.olsr_vtime)
			);
	//printf("statement: %s\n", stmt);
	if(sqlite3_exec(db, stmt, NULL, NULL, &error_exec) != SQLITE_OK)
	{
		printf("SQLite error: %s\n",error_exec);
		sqlite3_free(error_exec);
		error_exec=NULL;
	}

	// remember username
	union olsr_ip_addr ip;
	memcpy(&ip.v4, &msg->v4.originator, sizeof(msg->v4.originator));
	Qaullib_UserCheckUser(&ip, chat_user);

	qaul_new_msg = 1;
}

// ------------------------------------------------------------
void Qaullib_IpcEvaluateCom(union olsr_message *msg)
{
	switch(msg->v4.message.ipc.type)
	{
		case 1:
			break;
		default:
			printf("not a known message type\n");
			break;
	}
}

// ------------------------------------------------------------
void Qaullib_IpcEvaluateTopo(union olsr_message *msg)
{
	// check if user has our ip
	//inet_ntop(AF_INET, &msg->v4.message.node.ip.v4, &ipbuf, sizeof(ipbuf));
	//if(strcmp(qaul_ip_str, ipbuf)==0) return;
	if(memcmp(&qaul_ip_addr.v4, &msg->v4.message.node.ip.v4, 4) == 0) return;

	// check if user exists, create it if not
	Qaullib_UserTouchIp(&msg->v4.message.node.ip);
}

// ------------------------------------------------------------
void Qaullib_IpcSendCom(int commandId)
{
	char buffer[1024];
	union olsr_message *msg = (union olsr_message *)buffer;
	int size;

	// pack chat into olsr message
	// ipv4 only at the moment
    msg->v4.olsr_msgtype = 223;
    msg->v4.message.ipc.type = commandId;
    size = sizeof( struct qaulipcmsg);
	size = size + sizeof(struct olsrmsg);
    msg->v4.olsr_msgsize = htons(size);

	// send package
	Qaullib_IpcSend(msg);
}


// ------------------------------------------------------------
void Qaullib_IpcSend(union olsr_message *msg)
{
	int size;
	size = (int) ntohs(msg->v4.olsr_msgsize);

	if (send(ipc_socket,(const char *)msg, size, MSG_NOSIGNAL) < 0) {
		printf("[qaullib] IPC connection lost!\n");
		CLOSE(ipc_socket);
		ipc_connected = 0;
	}
}
