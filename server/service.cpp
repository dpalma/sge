////////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "tech/configapi.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "tech/dbgalloc.h" // must be last header

////////////////////////////////////////////////////////////////////////////////

const tChar             g_serviceName[] = _T("ServerService");
SERVICE_STATUS          g_serviceStatus = {0};
SERVICE_STATUS_HANDLE   g_serviceStatusHandle = NULL;
volatile long           g_bStopService = FALSE;

////////////////////////////////////////////////////////////////////////////////

void ServerLoop()
{
   while (!g_bStopService)
   {
      if (ServerFrame() != S_OK)
      {
         break;
      }
   }
}

////////////////////////////////////////////////////////////////////////////////

void WINAPI ServiceControlHandler(DWORD controlCode)
{
   if (controlCode == SERVICE_CONTROL_SHUTDOWN || controlCode == SERVICE_CONTROL_STOP)
   {
      g_serviceStatus.dwCurrentState = SERVICE_STOP_PENDING;
      SetServiceStatus(g_serviceStatusHandle, &g_serviceStatus);
      InterlockedIncrement(&g_bStopService);
   }
}

////////////////////////////////////////////////////////////////////////////////

void WINAPI ServiceMain(DWORD argc, tChar *argv[])
{
   g_serviceStatus.dwServiceType = SERVICE_WIN32;
   g_serviceStatus.dwCurrentState = SERVICE_STOPPED;
   g_serviceStatus.dwControlsAccepted = 0;
   g_serviceStatus.dwWin32ExitCode = NO_ERROR;
   g_serviceStatus.dwServiceSpecificExitCode = NO_ERROR;
   g_serviceStatus.dwCheckPoint = 0;
   g_serviceStatus.dwWaitHint = 0;

   g_serviceStatusHandle = RegisterServiceCtrlHandler(g_serviceName, ServiceControlHandler);

   if (g_serviceStatusHandle != NULL)
   {
      g_serviceStatus.dwCurrentState = SERVICE_START_PENDING;
      SetServiceStatus(g_serviceStatusHandle, &g_serviceStatus);

      if (ServerInit(argc, argv) == S_OK)
      {
         g_serviceStatus.dwControlsAccepted |= (SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN);
         g_serviceStatus.dwCurrentState = SERVICE_RUNNING;
         SetServiceStatus(g_serviceStatusHandle, &g_serviceStatus);

         ServerLoop();

         g_serviceStatus.dwCurrentState = SERVICE_STOP_PENDING;
         SetServiceStatus(g_serviceStatusHandle, &g_serviceStatus);
      }

      ServerTerm();

      g_serviceStatus.dwControlsAccepted &= ~(SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN);
      g_serviceStatus.dwCurrentState = SERVICE_STOPPED;
      SetServiceStatus(g_serviceStatusHandle, &g_serviceStatus);
   }
}

////////////////////////////////////////////////////////////////////////////////

tResult InstallService(const tChar * pszServiceName,
                       const tChar * pszDisplayName,
                       const tChar * pszExecutable)
{
   if (pszServiceName == NULL || pszDisplayName == NULL || pszExecutable == NULL)
   {
      return E_POINTER;
   }

   tResult result = E_FAIL;

   SC_HANDLE serviceControlManager = OpenSCManager(0, 0, SC_MANAGER_CREATE_SERVICE);

   if (serviceControlManager != NULL)
   {
      SC_HANDLE service = CreateService(serviceControlManager, pszServiceName, pszDisplayName,
         SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS, SERVICE_AUTO_START, SERVICE_ERROR_IGNORE,
         pszExecutable, NULL, NULL, NULL, NULL, NULL);

      if (service != NULL)
      {
         result = S_OK;
         CloseServiceHandle(service);
      }
      else
      {
         DWORD error = GetLastError();
         if (error == ERROR_SERVICE_EXISTS)
         {
            result = S_FALSE;
         }
      }

      CloseServiceHandle(serviceControlManager);
   }

   return result;
}

////////////////////////////////////////////////////////////////////////////////

tResult UninstallService(const tChar * pszServiceName)
{
   if (pszServiceName == NULL)
   {
      return E_POINTER;
   }

   tResult result = E_FAIL;

   SC_HANDLE serviceControlManager = OpenSCManager(0, 0, SC_MANAGER_CONNECT);

   if (serviceControlManager != NULL)
   {
      SC_HANDLE service = OpenService(serviceControlManager, pszServiceName, SERVICE_QUERY_STATUS | DELETE);
      if (service != NULL)
      {
         SERVICE_STATUS serviceStatus;
         if (QueryServiceStatus(service, &serviceStatus))
         {
            if (serviceStatus.dwCurrentState == SERVICE_STOPPED)
            {
               result = DeleteService(service) ? S_OK : E_FAIL;
            }
            else if (ControlService(service, SERVICE_CONTROL_STOP, &serviceStatus))
            {
               // TODO: Wait for service to actually stop?
               result = DeleteService(service) ? S_OK : E_FAIL;
            }
         }

         CloseServiceHandle(service);
      }

      CloseServiceHandle(serviceControlManager);
   }

   return result;
}

////////////////////////////////////////////////////////////////////////////////

int _tmain(int argc, tChar *argv[])
{
   ParseCommandLine(argc, argv, g_pConfig);

   if (ConfigIsTrue(_T("install")))
   {
      InstallService(g_serviceName, _T("<TODO> Game Server Service Name"), argv[0]);
   }
   else if (ConfigIsTrue(_T("uninstall")))
   {
      UninstallService(g_serviceName);
   }
   else if (ConfigIsTrue(_T("run")))
   {
      // Run as regular executable
      if (ServerInit(argc, argv) == S_OK)
      {
         ServerLoop();
      }
      ServerTerm();
   }
   else
   {
      // Run as service
      SERVICE_TABLE_ENTRY serviceTable[] =
      {
         { const_cast<tChar*>(g_serviceName), ServiceMain },
         { 0, 0 }
      };
      StartServiceCtrlDispatcher(serviceTable);
   }

   return 0;
}

////////////////////////////////////////////////////////////////////////////////
