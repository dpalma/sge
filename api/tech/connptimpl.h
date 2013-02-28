///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_CONNPTIMPL_H
#define INCLUDED_CONNPTIMPL_H

#include <algorithm>
#include <functional>
#include <vector>

#define BOOST_MEM_FN_ENABLE_STDCALL
#include <boost/mem_fn.hpp>

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////
// functions should work with STL vectors, lists, and deques

template <typename CONTAINER, class I>
bool has_interface(CONTAINER & c, I * pI)
{
   typename CONTAINER::iterator f = std::find_if(c.begin(), c.end(),
      std::bind1st(std::ptr_fun(CTIsSameObject), pI));
   if (f != c.end())
   {
      return true;
   }
   return false;
}

template <typename CONTAINER, class I>
bool add_interface(CONTAINER & c, I * pI)
{
   if (has_interface(c, pI))
      return false;
   c.push_back(pI);
   pI->AddRef();
   return true;
}

template <typename CONTAINER, class I>
bool remove_interface(CONTAINER & c, I * pI)
{
   typename CONTAINER::iterator iter;
   for (iter = c.begin(); iter != c.end(); iter++)
   {
      if (CTIsSameObject(*iter, pI))
      {
         (*iter)->Release();
         c.erase(iter);
         return true;
      }
   }
   return false;
}

///////////////////////////////////////////////////////////////////////////////
//
// TEMPLATE: cConnectionPoint
//

template <class INTRFC, class SINKINTRFC>
class cConnectionPoint : public INTRFC
{
public:
   tResult Connect(SINKINTRFC * pSink)
   {
      return add_interface(m_sinks, pSink) ? S_OK : E_FAIL;
   }

   tResult Disconnect(SINKINTRFC * pSink)
   {
      return remove_interface(m_sinks, pSink) ? S_OK : E_FAIL;
   }

protected:
   typedef std::vector<SINKINTRFC *> tSinks;
   typedef typename tSinks::iterator tSinksIterator;

   tSinksIterator Disconnect(tSinksIterator iter)
   {
      return m_sinks.erase(iter);
   }

   void DisconnectAll()
   {
      std::for_each(m_sinks.begin(), m_sinks.end(), boost::mem_fn(&SINKINTRFC::Release));
      m_sinks.clear();
   }

   inline tSinksIterator BeginSinks() { return m_sinks.begin(); }
   inline tSinksIterator EndSinks() { return m_sinks.end(); }

   ~cConnectionPoint()
   {
      DisconnectAll();
   }

   template <class F>
   F ForEachConnection(F f)
   {
      return std::for_each(m_sinks.begin(), m_sinks.end(), f);
   }

private:
   tSinks m_sinks;
};

///////////////////////////////////////

#define IMPLEMENTSCP(Interface, SinkInterface) \
   cConnectionPoint<Interface, SinkInterface>, &IID_##Interface

///////////////////////////////////////////////////////////////////////////////

#undef BOOST_MEM_FN_ENABLE_STDCALL

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_CONNPTIMPL_H
