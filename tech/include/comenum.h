////////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_COMENUM_H
#define INCLUDED_COMENUM_H

#include "techdll.h"

#ifdef _MSC_VER
#pragma once
#endif

////////////////////////////////////////////////////////////////////////////////
//
// TEMPLATE: cComEnum
//

////////////////////////////////////////

template <class T>
class CopyAssign
{
public:
   static void Copy(T * p1, T * p2)
   {
      *p1 = *p2;
   }
   static void Destroy(T *)
   {
   }
};

////////////////////////////////////////

template <class T>
class CopyMemcpy
{
public:
   static void Copy(T * p1, T * p2)
   {
      memcpy(p1, p2, sizeof(typename T));
   }
   static void Destroy(T *)
   {
   }
};

////////////////////////////////////////

template <class T>
class CopyInterface
{
public:
   static void Copy(T * * p1, T * * p2)
   {
      *p1 = CTAddRef(*p2);
   }
   static void Copy(T * * p1, T * const * p2)
   {
      *p1 = CTAddRef(*p2);
   }
   static void Destroy(T * * p)
   {
      SafeRelease(*p);
   }
};

////////////////////////////////////////

template <class TENUM, const IID * pTENUMIID, class T, class TCOPY, class TCONTAINER>
class cComEnum : public TENUM
{
public:
   virtual ~cComEnum()
   {
      Destroy();
   }

   virtual tResult Next(ulong count, typename T * ppElements, ulong * pnElements)
   {
      if (ppElements == NULL || pnElements == NULL)
      {
         return E_POINTER;
      }

      ulong nReturned = 0;
      for (ulong i = 0; i < count; i++)
      {
         if (m_iterator == m_elements.end())
         {
            break;
         }
         else
         {
            TCOPY::Copy(ppElements, &(*m_iterator));
            nReturned++;
            m_iterator++;
            ppElements++;
         }
      }

      if (pnElements != NULL)
      {
         *pnElements = nReturned;
      }

      return (nReturned == count) ? S_OK : S_FALSE;
   }

   virtual tResult Skip(ulong count)
   {
      uint nSkipped = 0;
      while (count-- && (m_iterator != m_elements.end()))
      {
         nSkipped++;
         m_iterator++;
      }
      return (nSkipped == count) ? S_OK : S_FALSE;
   }

   virtual tResult Reset()
   {
      m_iterator = m_elements.begin();
      return S_OK;
   }

   virtual tResult Clone(typename TENUM * * ppEnum)
   {
      return Create(m_elements, ppEnum);
   }

   static tResult Create(const typename TCONTAINER & container, typename TENUM * * ppEnum)
   {
      if (ppEnum == NULL)
      {
         return E_POINTER;
      }
      typedef cComObject<cComEnum<TENUM, pTENUMIID, T, TCOPY, TCONTAINER>, pTENUMIID> Class;
      Class *pClass = new Class;
      if (pClass == NULL)
      {
         return E_OUTOFMEMORY;
      }
      pClass->Initialize(container.begin(), container.end());
      *ppEnum = static_cast<typename TENUM *>(pClass);
      return S_OK;
   }

protected:
   void Initialize(typename TCONTAINER::const_iterator first,
                   typename TCONTAINER::const_iterator last)
   {
      typename TCONTAINER::const_iterator iter = first;
      for (; iter != last; iter++)
      {
         T t;
         TCOPY::Copy(&t, &(*iter));
         m_elements.push_back(t);
      }
      m_iterator = m_elements.begin();
   }

   void Destroy()
   {
      typename TCONTAINER::iterator iter = m_elements.begin();
      for (; iter != m_elements.end(); iter++)
      {
         TCOPY::Destroy(&(*iter));
      }
      m_elements.clear();
      m_iterator = m_elements.end();
   }

private:
   typename TCONTAINER m_elements;
   typename TCONTAINER::iterator m_iterator;
};

////////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_COMENUM_H
