////////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "renderapi.h"

#ifdef HAVE_CPPUNITLITE2
#include "CppUnitLite2.h"
#endif

#include <GL/glew.h>

#include <cstdlib>

#include "dbgalloc.h" // must be last header

////////////////////////////////////////////////////////////////////////////////

enum eInterleaveComponent
{
   EMPTY, NOMATCH, V2F, V3F, V4F, C3F, C4F, C4UB, N3F, T2F, T3F, T4F
};

struct sInterleaveLookupEntry
{
   union
   {
      byte components[4];
      uint number;
   } match;
   GLenum format;
};

static int CDECL InterleaveCompare(const void * elem1, const void * elem2)
{
   const sInterleaveLookupEntry * pEntry1 = (const sInterleaveLookupEntry *)elem1;
   const sInterleaveLookupEntry * pEntry2 = (const sInterleaveLookupEntry *)elem2;
   return (pEntry1->match.number - pEntry2->match.number);
}

tResult GetGlInterleaveFormat(const sVertexElement * pElements, uint nElements, GLenum * pFormat)
{
   if (pElements == NULL || pFormat == NULL)
   {
      return E_POINTER;
   }

   if (nElements > 4)
   {
      return S_FALSE;
   }

   static const eInterleaveComponent componentLookup[][8] =
   {
      // kVEU_Position
      {
         NOMATCH, // kVET_Float1
         V2F,     // kVET_Float2
         V3F,     // kVET_Float3
         V4F,     // kVET_Float4
         NOMATCH, // kVET_Color
         NOMATCH, // kVET_UnsignedByte4
         NOMATCH, // kVET_Short2
         NOMATCH, // kVET_Short4
      },
      // kVEU_Normal
      {
         NOMATCH, // kVET_Float1
         NOMATCH, // kVET_Float2
         N3F,     // kVET_Float3
         NOMATCH, // kVET_Float4
         NOMATCH, // kVET_Color
         NOMATCH, // kVET_UnsignedByte4
         NOMATCH, // kVET_Short2
         NOMATCH, // kVET_Short4
      },
      // kVEU_Color
      {
         NOMATCH, // kVET_Float1
         NOMATCH, // kVET_Float2
         C3F,     // kVET_Float3
         C4F,     // kVET_Float4
         NOMATCH, // kVET_Color
         C4UB,    // kVET_UnsignedByte4
         NOMATCH, // kVET_Short2
         NOMATCH, // kVET_Short4
      },
      // kVEU_TexCoord
      {
         NOMATCH, // kVET_Float1
         T2F,     // kVET_Float2
         T3F,     // kVET_Float3
         T4F,     // kVET_Float4
         NOMATCH, // kVET_Color
         NOMATCH, // kVET_UnsignedByte4
         NOMATCH, // kVET_Short2
         NOMATCH, // kVET_Short4
      },
      // kVEU_Weight
      {
         NOMATCH, // kVET_Float1
         NOMATCH, // kVET_Float2
         NOMATCH, // kVET_Float3
         NOMATCH, // kVET_Float4
         NOMATCH, // kVET_Color
         NOMATCH, // kVET_UnsignedByte4
         NOMATCH, // kVET_Short2
         NOMATCH, // kVET_Short4
      },
      // kVEU_Index
      {
         NOMATCH, // kVET_Float1
         NOMATCH, // kVET_Float2
         NOMATCH, // kVET_Float3
         NOMATCH, // kVET_Float4
         NOMATCH, // kVET_Color
         NOMATCH, // kVET_UnsignedByte4
         NOMATCH, // kVET_Short2
         NOMATCH, // kVET_Short4
      },
   };

   static sInterleaveLookupEntry interleaveLookup[] =
   {
      { { V2F,    EMPTY,   EMPTY,   EMPTY }, GL_V2F },               // 0x2A20
      { { V3F,    EMPTY,   EMPTY,   EMPTY }, GL_V3F },               // 0x2A21
      { { C4UB,   V2F,     EMPTY,   EMPTY }, GL_C4UB_V2F },          // 0x2A22
      { { C4UB,   V3F,     EMPTY,   EMPTY }, GL_C4UB_V3F },          // 0x2A23
      { { C3F,    V3F,     EMPTY,   EMPTY }, GL_C3F_V3F },           // 0x2A24
      { { N3F,    V3F,     EMPTY,   EMPTY }, GL_N3F_V3F },           // 0x2A25
      { { C4F,    N3F,     V3F,     EMPTY }, GL_C4F_N3F_V3F },       // 0x2A26
      { { T2F,    V3F,     EMPTY,   EMPTY }, GL_T2F_V3F },           // 0x2A27
      { { T4F,    V4F,     EMPTY,   EMPTY }, GL_T4F_V4F },           // 0x2A28
      { { T2F,    C4UB,    V3F,     EMPTY }, GL_T2F_C4UB_V3F },      // 0x2A29
      { { T2F,    C3F,     V3F,     EMPTY }, GL_T2F_C3F_V3F },       // 0x2A2A
      { { T2F,    N3F,     V3F,     EMPTY }, GL_T2F_N3F_V3F },       // 0x2A2B
      { { T2F,    C4F,     N3F,     V3F   }, GL_T2F_C4F_N3F_V3F },   // 0x2A2C
      { { T4F,    C4F,     N3F,     V4F   }, GL_T4F_C4F_N3F_V4F },   // 0x2A2D
   };
   static bool bInterleaveLookupSorted;

   if (!bInterleaveLookupSorted)
   {
      qsort(interleaveLookup, _countof(interleaveLookup),
         sizeof(interleaveLookup[0]), InterleaveCompare);
      bInterleaveLookupSorted = true;
   }

   sInterleaveLookupEntry entry = { { EMPTY, EMPTY, EMPTY, EMPTY }, 0 };
   for (uint i = 0; i < nElements; i++)
   {
      const sVertexElement & element = pElements[i];
      entry.match.components[i] = componentLookup[element.usage][element.type];
   }

   sInterleaveLookupEntry * pEntry = (sInterleaveLookupEntry *)bsearch(&entry, interleaveLookup,
      _countof(interleaveLookup), sizeof(interleaveLookup[0]), InterleaveCompare);
   if (pEntry != NULL)
   {
      *pFormat = pEntry->format;
      return S_OK;
   }

   return S_FALSE;
}

////////////////////////////////////////////////////////////////////////////////

#ifdef HAVE_CPPUNITLITE2

///////////////////////////////////////

#define INTERLEAVE_TEST_BEGIN() \
   do { \
   const sVertexElement decl[] = {

#define INTERLEAVE_TEST_END(fmtExpected) \
   }; \
   GLenum fmtCalculated; \
   CHECK(GetGlInterleaveFormat(decl, _countof(decl), &fmtCalculated) == S_OK); \
   CHECK_EQUAL(fmtCalculated, (fmtExpected)); \
   } while (0)

TEST(GlInterleaveFormatsRecognized)
{
   INTERLEAVE_TEST_BEGIN()
      { kVEU_Position, kVET_Float2 },
   INTERLEAVE_TEST_END(GL_V2F);

   INTERLEAVE_TEST_BEGIN()
      { kVEU_Position, kVET_Float3 },
   INTERLEAVE_TEST_END(GL_V3F);

   INTERLEAVE_TEST_BEGIN()
      { kVEU_Color, kVET_Float3 },
      { kVEU_Position, kVET_Float3 },
   INTERLEAVE_TEST_END(GL_C3F_V3F);

   INTERLEAVE_TEST_BEGIN()
      { kVEU_Color, kVET_Float4 },
      { kVEU_Normal, kVET_Float3 },
      { kVEU_Position, kVET_Float3 },
   INTERLEAVE_TEST_END(GL_C4F_N3F_V3F);

   INTERLEAVE_TEST_BEGIN()
      { kVEU_Color, kVET_UnsignedByte4 },
      { kVEU_Position, kVET_Float2 },
   INTERLEAVE_TEST_END(GL_C4UB_V2F);

   INTERLEAVE_TEST_BEGIN()
      { kVEU_Color, kVET_UnsignedByte4 },
      { kVEU_Position, kVET_Float3 },
   INTERLEAVE_TEST_END(GL_C4UB_V3F);

   INTERLEAVE_TEST_BEGIN()
      { kVEU_Normal, kVET_Float3 },
      { kVEU_Position, kVET_Float3 },
   INTERLEAVE_TEST_END(GL_N3F_V3F);

   INTERLEAVE_TEST_BEGIN()
      { kVEU_TexCoord, kVET_Float2 },
      { kVEU_Position, kVET_Float3 },
   INTERLEAVE_TEST_END(GL_T2F_V3F);

   INTERLEAVE_TEST_BEGIN()
      { kVEU_TexCoord, kVET_Float2 },
      { kVEU_Color, kVET_UnsignedByte4 },
      { kVEU_Position, kVET_Float3 },
   INTERLEAVE_TEST_END(GL_T2F_C4UB_V3F);

   INTERLEAVE_TEST_BEGIN()
      { kVEU_TexCoord, kVET_Float2 },
      { kVEU_Color, kVET_Float3 },
      { kVEU_Position, kVET_Float3 },
   INTERLEAVE_TEST_END(GL_T2F_C3F_V3F);

   INTERLEAVE_TEST_BEGIN()
      { kVEU_TexCoord, kVET_Float2 },
      { kVEU_Normal, kVET_Float3 },
      { kVEU_Position, kVET_Float3 },
   INTERLEAVE_TEST_END(GL_T2F_N3F_V3F);

   INTERLEAVE_TEST_BEGIN()
      { kVEU_TexCoord, kVET_Float2 },
      { kVEU_Color, kVET_Float4 },
      { kVEU_Normal, kVET_Float3 },
      { kVEU_Position, kVET_Float3 },
   INTERLEAVE_TEST_END(GL_T2F_C4F_N3F_V3F);

   INTERLEAVE_TEST_BEGIN()
      { kVEU_TexCoord, kVET_Float4 },
      { kVEU_Position, kVET_Float4 },
   INTERLEAVE_TEST_END(GL_T4F_V4F);

   INTERLEAVE_TEST_BEGIN()
      { kVEU_TexCoord, kVET_Float4 },
      { kVEU_Color, kVET_Float4 },
      { kVEU_Normal, kVET_Float3 },
      { kVEU_Position, kVET_Float4 },
   INTERLEAVE_TEST_END(GL_T4F_C4F_N3F_V4F);
}

///////////////////////////////////////

TEST(GlInterleaveFormatsUnrecognized)
{
   GLenum format;

   // Not in suitable order
   {
      sVertexElement decl[] =
      {
         { kVEU_Position, kVET_Float4 },
         { kVEU_Color, kVET_Float4 },
         { kVEU_TexCoord, kVET_Float4 },
         { kVEU_Normal, kVET_Float3 },
      };
      CHECK(GetGlInterleaveFormat(decl, _countof(decl), &format) != S_OK);
   }

   // No compatible format
   {
      sVertexElement decl[] =
      {
         { kVEU_TexCoord, kVET_Float1 },
         { kVEU_Color, kVET_Float1 },
         { kVEU_Normal, kVET_Float1 },
         { kVEU_Position, kVET_Float1 },
      };
      CHECK(GetGlInterleaveFormat(decl, _countof(decl), &format) != S_OK);
   }

   // No compatible format #2
   {
      sVertexElement decl[] =
      {
         { kVEU_TexCoord, kVET_Float2 },
         { kVEU_Normal, kVET_Float3 },
         { kVEU_Position, kVET_Float3 },
         { kVEU_Index, kVET_UnsignedByte4 },
      };
      CHECK(GetGlInterleaveFormat(decl, _countof(decl), &format) != S_OK);
   }
}

#endif // HAVE_CPPUNITLITE2

////////////////////////////////////////////////////////////////////////////////
