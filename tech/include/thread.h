///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_THREAD_H
#define INCLUDED_THREAD_H

#ifdef _MSC_VER
#pragma once
#endif

#ifdef _WIN32
F_DECLARE_HANDLE(HTHREAD);
#endif

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cThread
//

class cThread
{
public:
   cThread();
   virtual ~cThread();

   bool Begin();
   uint End();

   uint GetId() const;

   uint Suspend();
   uint Resume();
   bool Terminate();

   virtual int Run();

private:
   static uint STDCALL ThreadEntry(void * param);

#ifdef _WIN32
   HTHREAD m_hThread;
#else
#error ("Need platform-specific thread members")
#endif
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_THREAD_H
