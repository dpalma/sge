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

   void Join(uint timeoutMs);
   bool Terminate();

protected:
   virtual int Run() = 0;

   HANDLE GetHandle() const;

private:
   static ulong STDCALL ThreadEntry(void * param);

#ifdef _WIN32
   HTHREAD m_hThread;
#else
#error ("Need platform-specific thread members")
#endif
   ulong m_threadId;
};

///////////////////////////////////////

inline uint cThread::GetId() const
{
   return m_threadId;
}

///////////////////////////////////////

inline HANDLE cThread::GetHandle() const
{
   return m_hThread;
}

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_THREAD_H
