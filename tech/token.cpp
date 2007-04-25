///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "tech/token.h"

#include "tech/comtools.h"

#ifdef HAVE_UNITTESTPP
#include "UnitTest++.h"
#endif

#include "tech/dbgalloc.h" // must be last header


///////////////////////////////////////////////////////////////////////////////

int ParseTuple(const tChar * pszIn, const tChar * pszDelims,
               tParseTupleCallbackFn pfnCallback, uint_ptr userData)
{
   if (pszIn == NULL || pfnCallback == NULL)
   {
      return E_POINTER;
   }

   if (pszDelims == NULL)
   {
      pszDelims = _T(",;");
   }

   const tChar * psz = pszIn;
   const tChar * pszStop = psz + _tcslen(psz);

   // the open and close bracket strings must "line up"
   // for example, '<' and '>' both appear at index zero
   static const tChar szOpenBrackets[]  = _T("<([{");
   static const tChar szCloseBrackets[] = _T(">)]}");

   const tChar * pszOpenBracket = _tcschr(szOpenBrackets, *psz);
   if (pszOpenBracket != NULL)
   {
      if (*(pszStop - 1) == szCloseBrackets[pszOpenBracket - szOpenBrackets])
      {
         psz = _tcsinc(psz);
         pszStop = _tcsdec(pszIn, pszStop);
      }
      else
      {
         // Unmatched brackets
         return E_INVALIDARG;
      }
   }

   int nParsed = 0;

   while (psz < pszStop)
   {
      while (isspace(*psz))
      {
         psz = _tcsinc(psz);
      }

      const tChar * psz2 = _tcspbrk(psz, pszDelims);
      if (psz2 == NULL)
      {
         psz2 = pszStop;
      }

      int len = psz2 - psz;
      if (len > 0)
      {
         cStr temp(psz, len);
         (*pfnCallback)(temp.c_str(), userData);
         nParsed++;
      }

      psz = _tcsinc(psz2);
   }

   return nParsed;
}


///////////////////////////////////////////////////////////////////////////////

std::string cToken<std::string, tChar>::Token(const tChar * pszToken)
{
#ifdef _UNICODE
   size_t s = wcstombs(NULL, pszToken, 0);
   if (s == 0)
   {
      return std::string();
   }
   char * pszTemp = reinterpret_cast<char*>(alloca(s * sizeof(char)));
   wcstombs(pszTemp, pszToken, s);
   return std::string(pszTemp);
#else
   return std::string(pszToken);
#endif
}


///////////////////////////////////////////////////////////////////////////////

double cToken<double, tChar>::Token(const tChar * pszToken)
{
#ifdef _UNICODE
   return _wtof(pszToken);
#else
   return atof(pszToken);
#endif
}


///////////////////////////////////////////////////////////////////////////////

float cToken<float, tChar>::Token(const tChar * pszToken)
{
#ifdef _UNICODE
   return _wtof(pszToken);
#else
   return atof(pszToken);
#endif
}


///////////////////////////////////////////////////////////////////////////////

int cToken<int, tChar>::Token(const tChar * pszToken)
{
#ifdef __GNUC__
   return strtol(pszToken, NULL, 10);
#else
   return _ttoi(pszToken);
#endif
}


///////////////////////////////////////////////////////////////////////////////

#ifdef HAVE_UNITTESTPP

////////////////////////////////////////

TEST(TokenizerBadArgs)
{
   cTokenizer<std::string, std::vector<std::string>, std::string::value_type> strTok;
   CHECK(strTok.Tokenize(NULL) == E_POINTER);
   CHECK(strTok.Tokenize("[1,2,3") == E_INVALIDARG); // missing end bracket
}

////////////////////////////////////////

TEST(TokenizerSuccessCases)
{
   {
      cTokenizer<std::string, std::vector<std::string>, std::string::value_type> strTok;
      CHECK(strTok.Tokenize("[]") == 0);
      CHECK(strTok.Tokenize("{  }") == 0);
   }

   {
      cTokenizer<double> dblTok;
      CHECK(dblTok.Tokenize("[1,2,3,4]") == 4);
      CHECK(dblTok.m_tokens[0] == 1);
      CHECK(dblTok.m_tokens[1] == 2);
      CHECK(dblTok.m_tokens[2] == 3);
      CHECK(dblTok.m_tokens[3] == 4);
   }

   {
      cTokenizer<double> dblTok;
      CHECK(dblTok.Tokenize("< 10 ; 32.2 ; 48.8 >") == 3);
      CHECK(dblTok.m_tokens[0] == 10.0);
      CHECK(dblTok.m_tokens[1] == 32.2);
      CHECK(dblTok.m_tokens[2] == 48.8);
   }

   {
      cTokenizer<double> dblTok;
      CHECK(dblTok.Tokenize("(1,2,3,4)") == 4);
      CHECK(dblTok.m_tokens[0] == 1);
      CHECK(dblTok.m_tokens[1] == 2);
      CHECK(dblTok.m_tokens[2] == 3);
      CHECK(dblTok.m_tokens[3] == 4);
   }

   {
      cTokenizer<double> dblTok;
      CHECK(dblTok.Tokenize("1000,2000,3000,4000") == 4);
      CHECK(dblTok.m_tokens[0] == 1000);
      CHECK(dblTok.m_tokens[1] == 2000);
      CHECK(dblTok.m_tokens[2] == 3000);
      CHECK(dblTok.m_tokens[3] == 4000);
   }
}

#endif // HAVE_UNITTESTPP

///////////////////////////////////////////////////////////////////////////////
