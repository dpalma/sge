///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_CONNPTIMPL_H
#define INCLUDED_CONNPTIMPL_H

#include <vector>
#include <algorithm>

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
   typename CONTAINER::iterator iter;
   for (iter = c.begin(); iter != c.end(); iter++)
   {
      if (CTIsSameObject(*iter, pI))
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

template <class INTRFC, class SINKINTRFC, class CONTAINER = std::vector<SINKINTRFC *> >
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
   void DisconnectAll()
   {
      std::for_each(m_sinks.begin(), m_sinks.end(), boost::mem_fn(&SINKINTRFC::Release));
      m_sinks.clear();
   }

   typedef CONTAINER tSinks;
   typedef typename CONTAINER::iterator tSinksIterator;
   typedef typename CONTAINER::reverse_iterator tSinksReverseIterator;

   inline tSinksIterator BeginSinks() { return m_sinks.begin(); }
   inline tSinksIterator EndSinks() { return m_sinks.end(); }
   inline tSinksReverseIterator RBeginSinks() { return m_sinks.rbegin(); }
   inline tSinksReverseIterator REndSinks() { return m_sinks.rend(); }

   ~cConnectionPoint()
   {
      DisconnectAll();
   }

   template <typename RETURN>
   inline void ForEachConnection(RETURN (SINKINTRFC::*pfnMethod)())
   {
      typename CONTAINER::iterator iter;
      for (iter = m_sinks.begin(); iter != m_sinks.end(); iter++)
      {
         // Note that return values are always ignored, so it doesn't
         // make sense for a sink method to have a non-void return value
         // if this mechanism will be used to do the callbacks.
         ((*iter)->*pfnMethod)();
      }
   }

   template <typename RETURN, typename ARG0>
   inline void ForEachConnection(RETURN (SINKINTRFC::*pfnMethod)(ARG0), ARG0 arg0)
   {
      typename CONTAINER::iterator iter;
      for (iter = m_sinks.begin(); iter != m_sinks.end(); iter++)
      {
         // See comments above about return values.
         ((*iter)->*pfnMethod)(arg0);
      }
   }

   template <typename RETURN, typename ARG0, typename ARG1>
   inline void ForEachConnection(RETURN (SINKINTRFC::*pfnMethod)(ARG0, ARG1), ARG0 arg0, ARG1 arg1)
   {
      typename CONTAINER::iterator iter;
      for (iter = m_sinks.begin(); iter != m_sinks.end(); iter++)
      {
         // See comments above about return values.
         ((*iter)->*pfnMethod)(arg0, arg1);
      }
   }

   template <typename RETURN, typename ARG0, typename ARG1, typename ARG2>
   inline void ForEachConnection(RETURN (SINKINTRFC::*pfnMethod)(ARG0, ARG1, ARG2), ARG0 arg0, ARG1 arg1, ARG2 arg2)
   {
      typename CONTAINER::iterator iter;
      for (iter = m_sinks.begin(); iter != m_sinks.end(); iter++)
      {
         // See comments above about return values.
         ((*iter)->*pfnMethod)(arg0, arg1, arg2);
      }
   }

   template <typename RETURN, typename ARG0, typename ARG1, typename ARG2, typename ARG3>
   inline void ForEachConnection(RETURN (SINKINTRFC::*pfnMethod)(ARG0, ARG1, ARG2, ARG3), ARG0 arg0, ARG1 arg1, ARG2 arg2, ARG3 arg3)
   {
      typename CONTAINER::iterator iter;
      for (iter = m_sinks.begin(); iter != m_sinks.end(); iter++)
      {
         // See comments above about return values.
         ((*iter)->*pfnMethod)(arg0, arg1, arg2, arg3);
      }
   }

private:
   CONTAINER m_sinks;
};

///////////////////////////////////////

#define IMPLEMENTSCP(Interface, SinkInterface) \
   cConnectionPoint<Interface, SinkInterface>, &IID_##Interface


///////////////////////////////////////////////////////////////////////////////
//
// TEMPLATE: cConnectionPointEx
//
// The sink data goes first in the pair to make it easier to sort by it

template <class T, class SINKINTRFC, typename SINKDATA = int, class CONTAINER = std::vector< std::pair<SINKDATA, SINKINTRFC *> > >
class cConnectionPointEx
{
protected:
   ~cConnectionPointEx()
   {
      DisconnectAll();
   }

   void DisconnectAll()
   {
      typename CONTAINER::iterator iter;
      for (iter = m_sinks.begin(); iter != m_sinks.end(); iter++)
      {
         iter->second->Release();
      }
      m_sinks.clear();
   }

   tResult AddSink(SINKINTRFC * pSink, SINKDATA sinkData)
   {
      if (pSink == NULL)
      {
         return E_POINTER;
      }
      if (HasSink(pSink) == S_OK)
      {
         return S_FALSE;
      }
      m_sinks.push_back(std::make_pair(sinkData, CTAddRef(pSink)));
      T * pT = static_cast<T*>(this);
      pT->SortSinks(m_sinks.begin(), m_sinks.end());
      return S_OK;
   }

   tResult RemoveSink(SINKINTRFC * pSink)
   {
      if (pSink == NULL)
      {
         return E_POINTER;
      }
      typename CONTAINER::iterator iter;
      for (iter = m_sinks.begin(); iter != m_sinks.end(); iter++)
      {
         if (CTIsSameObject(pSink, iter->second))
         {
            iter->second->Release();
            m_sinks.erase(iter);
            return S_OK;
         }
      }
      return S_FALSE;
   }

   tResult GetSinkData(SINKINTRFC * pSink, SINKDATA * pSinkData) const
   {
      if (pSink == NULL || pSinkData == NULL)
      {
         return E_POINTER;
      }
      typename CONTAINER::const_iterator iter;
      for (iter = m_sinks.begin(); iter != m_sinks.end(); iter++)
      {
         if (CTIsSameObject(pSink, iter->second))
         {
            *pSinkData = iter->first;
            return S_OK;
         }
      }
      return S_FALSE;
   }

   tResult HasSink(SINKINTRFC * pSink)
   {
      SINKDATA sinkData;
      return GetSinkData(pSink, &sinkData);
   }

   typedef CONTAINER tSinks;
   typedef typename CONTAINER::iterator tSinksIterator;

   inline tSinksIterator BeginSinks() { return m_sinks.begin(); }
   inline tSinksIterator EndSinks() { return m_sinks.end(); }

   void SortSinks(tSinksIterator first, tSinksIterator last)
   {
      // Do nothing by default
   }

private:
   CONTAINER m_sinks;
};


///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_CONNPTIMPL_H
