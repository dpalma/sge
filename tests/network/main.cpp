///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "socket.h"
#include "thread.h"

#include "globalobj.h"

#ifdef _WIN32
#include <winsock.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#endif
#include <cstdio>
#include <cstring>

#include "dbgalloc.h" // must be last header

#define SERVER_PORT 1500

///////////////////////////////////////////////////////////////////////////////

class cServerThread : public cThread
{
protected:
   virtual int Run();
};

int cServerThread::Run()
{
   int cliLen;
   struct sockaddr_in cliAddr;
   char msg[100];

   cSocket serverSocket;

   if (!serverSocket.Create(SERVER_PORT, SOCK_DGRAM, NULL))
   {
      printf("cannot open socket \n");
      return -1;
   }

   printf("waiting for data on port UDP %u\n", SERVER_PORT);

   for (;;)
   {
      memset(msg, 0, sizeof(msg));

      cliLen = sizeof(cliAddr);

      int result = serverSocket.ReceiveFrom(msg, sizeof(msg), (struct sockaddr *)&cliAddr, &cliLen);

      if (result == SOCKET_ERROR)
      {
         printf("cannot receive data \n");
         continue;
      }

      printf("from %s:UDP%u : %s \n", inet_ntoa(cliAddr.sin_addr), ntohs(cliAddr.sin_port), msg);
   }

   return 0;
}

///////////////////////////////////////////////////////////////////////////////

class cClientThread : public cThread
{
protected:
   virtual int Run();
};

int cClientThread::Run()
{
   cSocket clientSocket;

   if (!clientSocket.Create(0, SOCK_DGRAM, NULL))
   {
      printf("cannot open socket \n");
      return -1;
   }

   struct hostent * h = gethostbyname("127.0.0.1");
   if (h == NULL)
   {
      printf("unknown host\n");
      return -1;
   }

   struct sockaddr_in addr;
   addr.sin_family = h->h_addrtype;
   memcpy(&addr.sin_addr.s_addr, h->h_addr_list[0], h->h_length);
   addr.sin_port = htons(SERVER_PORT);

   char buffer[100];
   int len = 0;

   int result = 0;

   for (;;)
   {
      char c = getc(stdin);

      if (c == '\n')
      {
         if (stricmp(buffer, "quit") == 0)
         {
            break;
         }

         if (clientSocket.SendTo(buffer, strlen(buffer) + 1, (struct sockaddr *)&addr, sizeof(addr)) == SOCKET_ERROR)
         {
            result = -1;
            break;
         }

         memset(buffer, 0, sizeof(buffer));
         len = 0;
      }
      else if ((len + 2) < sizeof(buffer))
      {
         buffer[len++] = c;
         buffer[len] = 0;
      }
   }

   return result;
}

///////////////////////////////////////////////////////////////////////////////

int main(int argc, char * argv[])
{
   StartGlobalObjects();

   /* Testing anonymous CVS access */

   if (!cSocket::Init())
   {
      printf("%s: could not initialize socket library\n", argv[0]);
      exit(1);
   }

   cServerThread serverThread;
   cClientThread clientThread;

   serverThread.Create();
   clientThread.Create();
   
   clientThread.Join();

   serverThread.Terminate();

   cSocket::Term();

   StopGlobalObjects();

   return 0;
}

///////////////////////////////////////////////////////////////////////////////
