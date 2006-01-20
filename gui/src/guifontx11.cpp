///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "guifontx11.h"

#include "comtools.h"
#include "techmath.h"

#include <GL/glew.h>
#include <GL/glxew.h>

#include <X11/Xutil.h>

#include <cstring>

#include "dbgalloc.h" // must be last header

extern tResult GetDisplay(Display * * ppDisplay);

///////////////////////////////////////////////////////////////////////////////

static const char g_szDefaultFont[] = "fixed";
int g_defaultPointSize = 10;

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGLXRasterFont
//

///////////////////////////////////////

cGLXRasterFont::cGLXRasterFont()
 : m_pFontInfo(NULL)
 , m_pDisplay(NULL)
 , m_glyphStart(0)
 , m_listCount(0)
 , m_listBase(0)
{
}

///////////////////////////////////////

cGLXRasterFont::~cGLXRasterFont()
{
}

///////////////////////////////////////

void cGLXRasterFont::OnFinalRelease()
{
   glDeleteLists(m_listBase, m_listCount);
   m_listBase = 0;
   m_listCount = 0;

   if (m_pDisplay != NULL && m_pFontInfo != NULL)
   {
      XFreeFont(m_pDisplay, m_pFontInfo);
      m_pFontInfo = NULL;

      m_pDisplay = NULL;
   }
}

///////////////////////////////////////

tResult cGLXRasterFont::RenderText(const char * pszText, int textLength,
                                   tRect * pRect, uint flags, const cColor & color) const
{
   if (pszText == NULL)
   {
      return E_POINTER;
   }

   if (textLength < 0)
   {
      textLength = strlen(pszText);
   }

   if ((flags & kRT_CalcRect) == kRT_CalcRect)
   {
      // An alternative way...
      //int dir, ascent, descent;
      //XCharStruct charStruct;
      //XTextExtents(m_pFontInfo, pszText, length, &dir, &ascent, &descent, &charStruct);
      //*pWidth = charStruct.width;
      //*pHeight = ascent + descent;

      pRect->right = pRect->left + XTextWidth(m_pFontInfo, pszText, textLength);
      pRect->bottom = Round(pRect->top + GetHeight());
   }
   else
   {
      glRasterPos2f(pRect->left, pRect->top + GetHeight());
      glPushAttrib(GL_LIST_BIT);
      glListBase(m_listBase - m_glyphStart);
      glCallLists(textLength, GL_UNSIGNED_BYTE, pszText);
      glPopAttrib();
   }

   return S_OK;
}

///////////////////////////////////////

tResult cGLXRasterFont::RenderText(const wchar_t * pszText, int textLength,
                                   tRect * pRect, uint flags, const cColor & color) const
{
   return E_NOTIMPL;
}

///////////////////////////////////////

bool cGLXRasterFont::Create(const cGUIFontDesc & fontDesc)
{
   Assert(m_listBase == 0);

   bool bSucceeded = false;

   if (GetDisplay(&m_pDisplay) == S_OK)
   {
      Assert(m_pDisplay != NULL);

      m_pFontInfo = XLoadQueryFont(m_pDisplay, fontDesc.GetFace());
      if (m_pFontInfo != NULL)
      {
         Font id = m_pFontInfo->fid;

         m_glyphStart = m_pFontInfo->min_char_or_byte2;
         m_listCount = m_pFontInfo->max_char_or_byte2 -
                       m_pFontInfo->min_char_or_byte2 + 1;

         /*char * pCharSet = (char *)alloca((m_listCount + 1) * sizeof(char));
         for (int i = m_pFontInfo->min_char_or_byte2; i <= pFontInfo->min_char_or_byte2; i++)
            pCharSet[i] = i;
         pCharSet[m_listCount - 1] = 0;*/

         int maxWidth  = m_pFontInfo->max_bounds.rbearing - m_pFontInfo->min_bounds.lbearing;
         int maxHeight = m_pFontInfo->max_bounds.ascent   + m_pFontInfo->max_bounds.descent;

         m_height = maxHeight;

         m_listBase = glGenLists(m_listCount);
         if (glIsList(m_listBase))
         {
            if (m_listBase > 0)
            {
               glXUseXFont(id, m_glyphStart, m_listCount, m_listBase + m_glyphStart);
               bSucceeded = true;
            }
         }
      }
   }

   return bSucceeded;
}

///////////////////////////////////////////////////////////////////////////////

tResult GUIFontCreateGL(const cGUIFontDesc & fontDesc,
                        IGUIFont * * ppFont)
{
   if (ppFont == NULL)
   {
      return E_POINTER;
   }

   cGLXRasterFont * pFont = new cGLXRasterFont;

   if (pFont == NULL)
   {
      return E_OUTOFMEMORY;
   }

   if (!pFont->Create(fontDesc))
   {
      SafeRelease(pFont);
      return E_FAIL;
   }

   *ppFont = static_cast<IGUIFont *>(pFont);
   return S_OK;
}

///////////////////////////////////////////////////////////////////////////////

static void DumpXFonts()
{
   Display * pDisplay = XOpenDisplay(NULL);
   if (pDisplay != NULL)
   {
      int nFonts;
      char * * ppszFonts = XListFonts(pDisplay,
         "-*-*-*-*-*-*-*-*-*-*-*-*-iso8859-1", 1000, &nFonts);
      if (ppszFonts != NULL)
      {
         DebugMsg1("%d fonts:\n", nFonts);
         char * p = *ppszFonts;
         while (nFonts--)
         {
            DebugMsg1("%s\n", p);
            p += strlen(p) + 1;
         }
         XFreeFontNames(ppszFonts);
      }
      XCloseDisplay(pDisplay);
   }
}

///////////////////////////////////////////////////////////////////////////////
