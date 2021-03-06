///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#ifdef HAVE_UNITTESTPP // entire file

#include "tech/statemachine.h"
#include "tech/statemachinetem.h"

#include "tech/techstring.h"

#include "UnitTest++.h"

#include <locale>

#include "tech/dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////

LOG_DEFINE_CHANNEL(StateMachineTest);

#define LocalMsg(msg)                  DebugMsgEx(StateMachineTest,(msg))
#define LocalMsg1(msg,a)               DebugMsgEx1(StateMachineTest,(msg),(a))
#define LocalMsg2(msg,a,b)             DebugMsgEx2(StateMachineTest,(msg),(a),(b))
#define LocalMsg3(msg,a,b,c)           DebugMsgEx3(StateMachineTest,(msg),(a),(b),(c))
#define LocalMsg4(msg,a,b,c,d)         DebugMsgEx4(StateMachineTest,(msg),(a),(b),(c),(d))

#define LocalMsgIf(cond,msg)           DebugMsgIfEx(StateMachineTest,(cond),(msg))
#define LocalMsgIf1(cond,msg,a)        DebugMsgIfEx1(StateMachineTest,(cond),(msg),(a))
#define LocalMsgIf2(cond,msg,a,b)      DebugMsgIfEx2(StateMachineTest,(cond),(msg),(a),(b))
#define LocalMsgIf3(cond,msg,a,b,c)    DebugMsgIfEx3(StateMachineTest,(cond),(msg),(a),(b),(c))
#define LocalMsgIf4(cond,msg,a,b,c,d)  DebugMsgIfEx4(StateMachineTest,(cond),(msg),(a),(b),(c),(d))


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cCppIdentifierRecognizer
//

class cCppIdentifierRecognizer : public cStateMachine<cCppIdentifierRecognizer, int>
{
public:
   cCppIdentifierRecognizer();
   ~cCppIdentifierRecognizer();

   void Reset();

   void AddChar(cStr::value_type c);

   bool GetIdentifier(cStr * pIdent) const;

   void OnEnterCharOk();
   void OnExitCharOk();
   void OnUpdateCharOk(int);

private:
   tState m_initialState;
   tState m_charOkState;
   tState m_errorState;
   cStr m_identifier;
};

////////////////////////////////////////

cCppIdentifierRecognizer::cCppIdentifierRecognizer()
 : tStateMachine(&m_initialState)
 , m_charOkState(&cCppIdentifierRecognizer::OnEnterCharOk,
                 &cCppIdentifierRecognizer::OnExitCharOk,
                 &cCppIdentifierRecognizer::OnUpdateCharOk)
{
}

////////////////////////////////////////

cCppIdentifierRecognizer::~cCppIdentifierRecognizer()
{
}

////////////////////////////////////////

void cCppIdentifierRecognizer::Reset()
{
   GotoState(&m_initialState);
   m_identifier.clear();
}

////////////////////////////////////////

void cCppIdentifierRecognizer::AddChar(cStr::value_type c)
{
   if (IsCurrentState(&m_initialState))
   {
      if ((c == _T('_')) || _istalpha(c))
      {
         m_identifier.push_back(c);
         GotoState(&m_charOkState);
      }
      else
      {
         GotoState(&m_errorState);
      }
   }
   else if (IsCurrentState(&m_charOkState))
   {
      if ((c == _T('_')) || _istalpha(c) || _istdigit(c))
      {
         m_identifier.push_back(c);
         // no state change
      }
      else
      {
         GotoState(&m_errorState);
      }
   }
}

////////////////////////////////////////

bool cCppIdentifierRecognizer::GetIdentifier(cStr * pIdent) const
{
   if (IsCurrentState(&m_charOkState))
   {
      *pIdent = m_identifier;
      return true;
   }
   return false;
}

////////////////////////////////////////

void cCppIdentifierRecognizer::OnEnterCharOk()
{
}

////////////////////////////////////////

void cCppIdentifierRecognizer::OnExitCharOk()
{
}

////////////////////////////////////////

void cCppIdentifierRecognizer::OnUpdateCharOk(int)
{
}


///////////////////////////////////////////////////////////////////////////////

TEST(StateMachine)
{
   static const struct
   {
      const tChar * pszIdent;
      bool result;
   }
   tests[] =
   {
      { _T("0000"),           false },
      { _T("i"),              true },
      { _T("_variable"),      true },
      { _T("member1234"),     true },
      { _T("this->foo"),      false },
      { _T("foo.bar"),        false },
   };

   cCppIdentifierRecognizer r;
   for (int i = 0; i < _countof(tests); i++)
   {
      r.Reset();
      int len = _tcslen(tests[i].pszIdent);
      for (int j = 0; j < len; j++)
      {
         r.AddChar(tests[i].pszIdent[j]);
      }
      cStr ident;
      bool result = r.GetIdentifier(&ident);
      if (result)
      {
         CHECK(ident.compare(tests[i].pszIdent) == 0);
      }
      CHECK_EQUAL(result, tests[i].result);
   }
}

///////////////////////////////////////////////////////////////////////////////

#endif // HAVE_UNITTESTPP (entire file)
