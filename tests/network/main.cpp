///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "socket.h"
#include "thread.h"

#include "globalobj.h"

#include <winsock.h>
#include <cstdio>
#include <cstring>

#include "dbgalloc.h" // must be last header

#define LOCAL_SERVER_PORT 1500

int main(int argc, char * argv[])
{
   int cliLen;
   struct sockaddr_in cliAddr;
   char msg[100];

   StartGlobalObjects();

   cSocket serverSocket;

   if (!cSocket::Init())
   {
      printf("%s: could not initialize socket library\n", argv[0]);
      exit(1);
   }

   if (!serverSocket.Create(LOCAL_SERVER_PORT, SOCK_DGRAM, NULL))
   {
      printf("%s: cannot open socket \n",argv[0]);
      exit(1);
   }

   printf("%s: waiting for data on port UDP %u\n", 
      argv[0],LOCAL_SERVER_PORT);

   for (;;)
   {
      memset(msg, 0, sizeof(msg));

      cliLen = sizeof(cliAddr);

      int result = serverSocket.ReceiveFrom(msg, sizeof(msg), (struct sockaddr *)&cliAddr, &cliLen);

      if (result == SOCKET_ERROR)
      {
         printf("%s: cannot receive data \n",argv[0]);
         continue;
      }

      printf("%s: from %s:UDP%u : %s \n", 
         argv[0],inet_ntoa(cliAddr.sin_addr),
         ntohs(cliAddr.sin_port),msg);
   }

   cSocket::Term();

   StopGlobalObjects();

   return 0;
}
