///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_FUNCTOR_H
#define INCLUDED_FUNCTOR_H

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cFunctor
//

class cFunctor
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
   typedef RETURN (* tFn)();

public:
   cFunctor0(tFn pfn);
   cFunctor0(const cFunctor0 & other);
   ~cFunctor0();
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
   typedef RETURN (* tFn)(ARG1);

public:
   cFunctor1(tFn pfn, ARG1 arg1);
   cFunctor1(const cFunctor1 & other);
   ~cFunctor1();
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
   typedef RETURN (* tFn)(ARG1, ARG2);

public:
   cFunctor2(tFn pfn, ARG1 arg1, ARG2 arg2);
   cFunctor2(const cFunctor2 & other);
   ~cFunctor2();
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
   typedef RETURN (* tFn)(ARG1, ARG2, ARG3);

public:
   cFunctor3(tFn pfn, ARG1 arg1, ARG2 arg2, ARG3 arg3);
   cFunctor3(const cFunctor3 & other);
   ~cFunctor3();
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
void cFunctor3<RETURN, ARG1, ARG2, ARG3>::operator ()()
{
   Assert(m_pfn != NULL);
   (*m_pfn)(m_arg1, m_arg2, m_arg3);
}

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_FUNCTOR_H
