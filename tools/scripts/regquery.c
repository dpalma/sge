/* $Id$
 */

#define WIN32_LEAN_AND_MEAN
#define NOGDI
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
   char * p;
   HKEY hBaseKey = NULL, hKey = NULL;
   DWORD dwType, dwSize;
   int result = EXIT_FAILURE;
   DWORD dwValue;
   char * pszValue;

   if (argc == 3)
   {
      p = strchr(argv[1], '\\');
      if (p != NULL)
      {
         if (strncmp(argv[1], "HKEY_LOCAL_MACHINE", p - argv[1]) == 0)
         {
            hBaseKey = HKEY_LOCAL_MACHINE;
         }
         else if (strncmp(argv[1], "HKEY_CURRENT_USER", p - argv[1]) == 0)
         {
            hBaseKey = HKEY_CURRENT_USER;
         }

         if (hBaseKey != NULL)
         {
            if (RegOpenKeyEx(hBaseKey, p + 1, 0, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS)
            {
               if (RegQueryValueEx(hKey, argv[2], NULL, &dwType, NULL, &dwSize) == ERROR_SUCCESS)
               {
                  if (dwType == REG_SZ)
                  {
                     pszValue = (char *)_alloca(dwSize + 1);
                     if (pszValue != NULL)
                     {
                        if (RegQueryValueEx(hKey, argv[2], NULL, &dwType, pszValue, &dwSize) == ERROR_SUCCESS)
                        {
                           printf(pszValue);
                           result = EXIT_SUCCESS;
                        }
                     }
                  }
                  else if (dwType == REG_DWORD)
                  {
                     if (RegQueryValueEx(hKey, argv[2], NULL, &dwType, (LPBYTE)&dwValue, &dwSize) == ERROR_SUCCESS)
                     {
                        printf("%d", dwValue);
                        result = EXIT_SUCCESS;
                     }
                  }
               }
            }
         }
      }
   }

   return result;
}
