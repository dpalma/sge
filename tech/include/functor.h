///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_FUNCTOR_H
#define INCLUDED_FUNCTOR_H

#include "techdll.h"
#include "comtools.h"

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////
// Define to nothing by default. Can be used to force __stdcall or whatever
// on cFunctor0, cFunctor1, etc.

#ifndef FUNCTORCALLTYPE
#define FUNCTORCALLTYPE
#endif

#ifndef METHODFUNCTORCALLTYPE
#define METHODFUNCTORCALLTYPE
#endif

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cFunctor
//

class TECH_API cFunctor
{
public:
   virtual ~cFunctor() = 0;
   virtual void operator ()() = 0;
};

///////////////////////////////////////////////////////////////////////////////
//
// TEMPLATE: cFunctor0
//

template <typename RETURN>
class cFunctor0 : public cFunctor
{
protected:
   typedef RETURN (FUNCTORCALLTYPE * tFn)();

public:
   cFunctor0(tFn pfn);
   cFunctor0(const cFunctor0 & other);
   ~cFunctor0();

   const cFunctor0 & operator =(const cFunctor0 & other);

   void operator ()();

protected:
   tFn m_pfn;
};

////////////////////////////////////////

template <typename RETURN>
cFunctor0<RETURN>::cFunctor0(tFn pfn)
 : m_pfn(pfn)
{
}

////////////////////////////////////////

template <typename RETURN>
cFunctor0<RETURN>::cFunctor0(const cFunctor0 & other)
 : m_pfn(other.m_pfn)
{
}

////////////////////////////////////////

template <typename RETURN>
cFunctor0<RETURN>::~cFunctor0()
{
}

////////////////////////////////////////

template <typename RETURN>
const cFunctor0<RETURN> & cFunctor0<RETURN>::operator =(const cFunctor0 & other)
{
   m_pfn = other.m_pfn;
}

////////////////////////////////////////

template <typename RETURN>
void cFunctor0<RETURN>::operator ()()
{
   Assert(m_pfn != NULL);
   (*m_pfn)();
}

///////////////////////////////////////////////////////////////////////////////
//
// TEMPLATE: cFunctor1
//

template <typename RETURN, typename ARG1>
class cFunctor1 : public cFunctor
{
protected:
   typedef RETURN (FUNCTORCALLTYPE * tFn)(ARG1);

public:
   cFunctor1(tFn pfn, ARG1 arg1);
   cFunctor1(const cFunctor1 & other);
   ~cFunctor1();

   const cFunctor1 & operator =(const cFunctor1 & other);

   void operator ()();

protected:
   tFn m_pfn;
   ARG1 m_arg1;
};

////////////////////////////////////////

template <typename RETURN, typename ARG1>
cFunctor1<RETURN, ARG1>::cFunctor1(tFn pfn, ARG1 arg1)
 : m_pfn(pfn),
   m_arg1(arg1)
{
}

////////////////////////////////////////

template <typename RETURN, typename ARG1>
cFunctor1<RETURN, ARG1>::cFunctor1(const cFunctor1 & other)
 : m_pfn(other.m_pfn),
   m_arg1(other.m_arg1)
{
}

////////////////////////////////////////

template <typename RETURN, typename ARG1>
cFunctor1<RETURN, ARG1>::~cFunctor1()
{
}

////////////////////////////////////////

template <typename RETURN, typename ARG1>
const cFunctor1<RETURN, ARG1> & cFunctor1<RETURN, ARG1>::operator =(const cFunctor1 & other)
{
   m_pfn = other.m_pfn;
   m_arg1 = other.m_arg1;
}

////////////////////////////////////////

template <typename RETURN, typename ARG1>
void cFunctor1<RETURN, ARG1>::operator ()()
{
   Assert(m_pfn != NULL);
   (*m_pfn)(m_arg1);
}

///////////////////////////////////////////////////////////////////////////////
//
// TEMPLATE: cFunctor2
//

template <typename RETURN, typename ARG1, typename ARG2>
class cFunctor2 : public cFunctor
{
protected:
   typedef RETURN (FUNCTORCALLTYPE * tFn)(ARG1, ARG2);

public:
   cFunctor2(tFn pfn, ARG1 arg1, ARG2 arg2);
   cFunctor2(const cFunctor2 & other);
   ~cFunctor2();

   const cFunctor2 & operator =(const cFunctor2 & other);

   void operator ()();

protected:
   tFn m_pfn;
   ARG1 m_arg1;
   ARG2 m_arg2;
};

////////////////////////////////////////

template <typename RETURN, typename ARG1, typename ARG2>
cFunctor2<RETURN, ARG1, ARG2>::cFunctor2(tFn pfn, ARG1 arg1, ARG2 arg2)
 : m_pfn(pfn),
   m_arg1(arg1),
   m_arg2(arg2)
{
}

////////////////////////////////////////

template <typename RETURN, typename ARG1, typename ARG2>
cFunctor2<RETURN, ARG1, ARG2>::cFunctor2(const cFunctor2 & other)
 : m_pfn(other.m_pfn),
   m_arg1(other.m_arg1),
   m_arg2(other.m_arg2)
{
}

////////////////////////////////////////

template <typename RETURN, typename ARG1, typename ARG2>
cFunctor2<RETURN, ARG1, ARG2>::~cFunctor2()
{
}

////////////////////////////////////////

template <typename RETURN, typename ARG1, typename ARG2>
const cFunctor2<RETURN, ARG1, ARG2> &
cFunctor2<RETURN, ARG1, ARG2>::operator =(const cFunctor2 & other)
{
   m_pfn = other.m_pfn;
   m_arg1 = other.m_arg1;
   m_arg2 = other.m_arg2;
}

////////////////////////////////////////

template <typename RETURN, typename ARG1, typename ARG2>
void cFunctor2<RETURN, ARG1, ARG2>::operator ()()
{
   Assert(m_pfn != NULL);
   (*m_pfn)(m_arg1, m_arg2);
}

///////////////////////////////////////////////////////////////////////////////
//
// TEMPLATE: cFunctor3
//

template <typename RETURN, typename ARG1, typename ARG2, typename ARG3>
class cFunctor3 : public cFunctor
{
protected:
   typedef RETURN (FUNCTORCALLTYPE * tFn)(ARG1, ARG2, ARG3);

public:
   cFunctor3(tFn pfn, ARG1 arg1, ARG2 arg2, ARG3 arg3);
   cFunctor3(const cFunctor3 & other);
   ~cFunctor3();

   const cFunctor3 & operator =(const cFunctor3 & other);

   void operator ()();

protected:
   tFn m_pfn;
   ARG1 m_arg1;
   ARG2 m_arg2;
   ARG3 m_arg3;
};

////////////////////////////////////////

template <typename RETURN, typename ARG1, typename ARG2, typename ARG3>
cFunctor3<RETURN, ARG1, ARG2, ARG3>::cFunctor3(tFn pfn, ARG1 arg1, ARG2 arg2, ARG3 arg3)
 : m_pfn(pfn),
   m_arg1(arg1),
   m_arg2(arg2),
   m_arg3(arg3)
{
}

////////////////////////////////////////

template <typename RETURN, typename ARG1, typename ARG2, typename ARG3>
cFunctor3<RETURN, ARG1, ARG2, ARG3>::cFunctor3(const cFunctor3 & other)
 : m_pfn(other.m_pfn),
   m_arg1(other.m_arg1),
   m_arg2(other.m_arg2),
   m_arg3(other.m_arg3)
{
}

////////////////////////////////////////

template <typename RETURN, typename ARG1, typename ARG2, typename ARG3>
cFunctor3<RETURN, ARG1, ARG2, ARG3>::~cFunctor3()
{
}

////////////////////////////////////////

template <typename RETURN, typename ARG1, typename ARG2, typename ARG3>
const cFunctor3<RETURN, ARG1, ARG2, ARG3> &
cFunctor3<RETURN, ARG1, ARG2, ARG3>::operator =(const cFunctor3 & other)
{
   m_pfn = other.m_pfn;
   m_arg1 = other.m_arg1;
   m_arg2 = other.m_arg2;
   m_arg3 = other.m_arg3;
}

////////////////////////////////////////

template <typename RETURN, typename ARG1, typename ARG2, typename ARG3>
void cFunctor3<RETURN, ARG1, ARG2, ARG3>::operator ()()
{
   Assert(m_pfn != NULL);
   (*m_pfn)(m_arg1, m_arg2, m_arg3);
}

///////////////////////////////////////////////////////////////////////////////
//
// TEMPLATE: cMethodFunctor0
//
// Functor that encapsulates a call to a COM interface method

template <typename IFC, typename RETURN>
class cMethodFunctor0 : public cFunctor
{
protected:
   typedef RETURN (METHODFUNCTORCALLTYPE IFC::*tMethod)();

public:
   cMethodFunctor0(IFC * pIfc, tMethod pMethod);
   cMethodFunctor0(const cMethodFunctor0 & other);
   ~cMethodFunctor0();

   const cMethodFunctor0 & operator =(const cMethodFunctor0 & other);

   void operator ()();

protected:
   cAutoIPtr<IFC> m_pIfc;
   tMethod m_pMethod;
};

////////////////////////////////////////

template <typename IFC, typename RETURN>
cMethodFunctor0<IFC, RETURN>::cMethodFunctor0(IFC * pIfc, tMethod pMethod)
 : m_pIfc(CTAddRef(pIfc)),
   m_pMethod(pMethod)
{
}

////////////////////////////////////////

template <typename IFC, typename RETURN>
cMethodFunctor0<IFC, RETURN>::cMethodFunctor0(const cMethodFunctor0 & other)
 : m_pIfc(other.m_pIfc),
   m_pMethod(other.m_pMethod)
{
}

////////////////////////////////////////

template <typename IFC, typename RETURN>
cMethodFunctor0<IFC, RETURN>::~cMethodFunctor0()
{
}

////////////////////////////////////////

template <typename IFC, typename RETURN>
const cMethodFunctor0<IFC, RETURN> & cMethodFunctor0<IFC, RETURN>::operator =(const cMethodFunctor0 & other)
{
   m_pIfc = other.m_pIfc;
   m_pMethod = other.m_pMethod;
   return *this;
}

////////////////////////////////////////

template <typename IFC, typename RETURN>
void cMethodFunctor0<IFC, RETURN>::operator ()()
{
   Assert(!!m_pIfc);
   Assert(m_pMethod != NULL);
   (m_pIfc->*m_pMethod)();
}

///////////////////////////////////////////////////////////////////////////////
//
// TEMPLATE: cMethodFunctor1
//
// Functor that encapsulates a call to a COM interface method taking a
// single argument

template <typename IFC, typename RETURN, typename ARG1>
class cMethodFunctor1 : public cFunctor
{
protected:
   typedef RETURN (METHODFUNCTORCALLTYPE IFC::*tMethod)(ARG1);

public:
   cMethodFunctor1(IFC * pIfc, tMethod pMethod, ARG1 arg1);
   cMethodFunctor1(const cMethodFunctor1 & other);
   ~cMethodFunctor1();

   const cMethodFunctor1 & operator =(const cMethodFunctor1 & other);

   void operator ()();

protected:
   cAutoIPtr<IFC> m_pIfc;
   tMethod m_pMethod;
   ARG1 m_arg1;
};

////////////////////////////////////////

template <typename IFC, typename RETURN, typename ARG1>
cMethodFunctor1<IFC, RETURN, ARG1>::cMethodFunctor1(IFC * pIfc, tMethod pMethod, ARG1 arg1)
 : m_pIfc(CTAddRef(pIfc)),
   m_pMethod(pMethod),
   m_arg1(arg1)
{
}

////////////////////////////////////////

template <typename IFC, typename RETURN, typename ARG1>
cMethodFunctor1<IFC, RETURN, ARG1>::cMethodFunctor1(const cMethodFunctor1 & other)
 : m_pIfc(other.m_pIfc),
   m_pMethod(other.m_pMethod),
   m_arg1(other.m_arg1)
{
}

////////////////////////////////////////

template <typename IFC, typename RETURN, typename ARG1>
cMethodFunctor1<IFC, RETURN, ARG1>::~cMethodFunctor1()
{
}

////////////////////////////////////////

template <typename IFC, typename RETURN, typename ARG1>
const cMethodFunctor1<IFC, RETURN, ARG1> &
cMethodFunctor1<IFC, RETURN, ARG1>::operator =(const cMethodFunctor1 & other)
{
   m_pIfc = other.m_pIfc;
   m_pMethod = other.m_pMethod;
   m_arg1 = other.m_arg1;
   return *this;
}

////////////////////////////////////////

template <typename IFC, typename RETURN, typename ARG1>
void cMethodFunctor1<IFC, RETURN, ARG1>::operator ()()
{
   Assert(!!m_pIfc);
   Assert(m_pMethod != NULL);
   (m_pIfc->*m_pMethod)(m_arg1);
}

///////////////////////////////////////////////////////////////////////////////
//
// TEMPLATE: cMethodFunctor2
//
// Functor that encapsulates a call to a COM interface method taking two
// arguments

template <typename IFC, typename RETURN, typename ARG1, typename ARG2>
class cMethodFunctor2 : public cFunctor
{
protected:
   typedef RETURN (METHODFUNCTORCALLTYPE IFC::*tMethod)(ARG1, ARG2);

public:
   cMethodFunctor2(IFC * pIfc, tMethod pMethod, ARG1 arg1, ARG2 arg2);
   cMethodFunctor2(const cMethodFunctor2 & other);
   ~cMethodFunctor2();

   const cMethodFunctor2 & operator =(const cMethodFunctor2 & other);

   void operator ()();

protected:
   cAutoIPtr<IFC> m_pIfc;
   tMethod m_pMethod;
   ARG1 m_arg1;
   ARG2 m_arg2;
};

////////////////////////////////////////

template <typename IFC, typename RETURN, typename ARG1, typename ARG2>
cMethodFunctor2<IFC, RETURN, ARG1, ARG2>::cMethodFunctor2(IFC * pIfc, tMethod pMethod, ARG1 arg1, ARG2 arg2)
 : m_pIfc(CTAddRef(pIfc)),
   m_pMethod(pMethod),
   m_arg1(arg1),
   m_arg2(arg2)
{
}

////////////////////////////////////////

template <typename IFC, typename RETURN, typename ARG1, typename ARG2>
cMethodFunctor2<IFC, RETURN, ARG1, ARG2>::cMethodFunctor2(const cMethodFunctor2 & other)
 : m_pIfc(other.m_pIfc),
   m_pMethod(other.m_pMethod),
   m_arg1(other.m_arg1),
   m_arg2(other.m_arg2)
{
}

////////////////////////////////////////

template <typename IFC, typename RETURN, typename ARG1, typename ARG2>
cMethodFunctor2<IFC, RETURN, ARG1, ARG2>::~cMethodFunctor2()
{
}

////////////////////////////////////////

template <typename IFC, typename RETURN, typename ARG1, typename ARG2>
const cMethodFunctor2<IFC, RETURN, ARG1, ARG2> &
cMethodFunctor2<IFC, RETURN, ARG1, ARG2>::operator =(const cMethodFunctor2 & other)
{
   m_pIfc = other.m_pIfc;
   m_pMethod = other.m_pMethod;
   m_arg1 = other.m_arg1;
   m_arg2 = other.m_arg2;
   return *this;
}

////////////////////////////////////////

template <typename IFC, typename RETURN, typename ARG1, typename ARG2>
void cMethodFunctor2<IFC, RETURN, ARG1, ARG2>::operator ()()
{
   Assert(!!m_pIfc);
   Assert(m_pMethod != NULL);
   (m_pIfc->*m_pMethod)(m_arg1, m_arg2);
}

///////////////////////////////////////////////////////////////////////////////
//
// TEMPLATE: cMethodFunctor3
//
// Functor that encapsulates a call to a COM interface method taking three
// arguments

template <typename IFC, typename RETURN, typename ARG1, typename ARG2, typename ARG3>
class cMethodFunctor3 : public cFunctor
{
protected:
   typedef RETURN (METHODFUNCTORCALLTYPE IFC::*tMethod)(ARG1, ARG2, ARG3);

public:
   cMethodFunctor3(IFC * pIfc, tMethod pMethod, ARG1 arg1, ARG2 arg2, ARG3 arg3);
   cMethodFunctor3(const cMethodFunctor3 & other);
   ~cMethodFunctor3();

   const cMethodFunctor3 & operator =(const cMethodFunctor3 & other);

   void operator ()();

protected:
   cAutoIPtr<IFC> m_pIfc;
   tMethod m_pMethod;
   ARG1 m_arg1;
   ARG2 m_arg2;
   ARG3 m_arg3;
};

////////////////////////////////////////

template <typename IFC, typename RETURN, typename ARG1, typename ARG2, typename ARG3>
cMethodFunctor3<IFC, RETURN, ARG1, ARG2, ARG3>::cMethodFunctor3(IFC * pIfc, tMethod pMethod,
                                                                ARG1 arg1, ARG2 arg2, ARG3 arg3)
 : m_pIfc(CTAddRef(pIfc)),
   m_pMethod(pMethod),
   m_arg1(arg1),
   m_arg2(arg2),
   m_arg3(arg3)
{
}

////////////////////////////////////////

template <typename IFC, typename RETURN, typename ARG1, typename ARG2, typename ARG3>
cMethodFunctor3<IFC, RETURN, ARG1, ARG2, ARG3>::cMethodFunctor3(const cMethodFunctor3 & other)
 : m_pIfc(other.m_pIfc),
   m_pMethod(other.m_pMethod),
   m_arg1(other.m_arg1),
   m_arg2(other.m_arg2),
   m_arg3(other.m_arg3)
{
}

////////////////////////////////////////

template <typename IFC, typename RETURN, typename ARG1, typename ARG2, typename ARG3>
cMethodFunctor3<IFC, RETURN, ARG1, ARG2, ARG3>::~cMethodFunctor3()
{
}

////////////////////////////////////////

template <typename IFC, typename RETURN, typename ARG1, typename ARG2, typename ARG3>
const cMethodFunctor3<IFC, RETURN, ARG1, ARG2, ARG3> &
cMethodFunctor3<IFC, RETURN, ARG1, ARG2, ARG3>::operator =(const cMethodFunctor3 & other)
{
   m_pIfc = other.m_pIfc;
   m_pMethod = other.m_pMethod;
   m_arg1 = other.m_arg1;
   m_arg2 = other.m_arg2;
   m_arg3 = other.m_arg3;
   return *this;
}

////////////////////////////////////////

template <typename IFC, typename RETURN, typename ARG1, typename ARG2, typename ARG3>
void cMethodFunctor3<IFC, RETURN, ARG1, ARG2, ARG3>::operator ()()
{
   Assert(!!m_pIfc);
   Assert(m_pMethod != NULL);
   (m_pIfc->*m_pMethod)(m_arg1, m_arg2, m_arg3);
}

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_FUNCTOR_H
