///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "font.h"
#include "color.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "comtools.h"
#include "techmath.h"

#include "stdgl.h"

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

class cGLXRasterFont : public cComObject<IMPLEMENTS(IRenderFont)>
{
public:
   cGLXRasterFont();
   virtual ~cGLXRasterFont();

   virtual void OnFinalRelease();

   virtual tResult DrawText(const char * pszText, int textLength, 
                            uint flags, tRect * pRect, const cColor & color) const;

   bool Create(const char * pszFont, int pointSize);

private:
   cGLXRasterFont(const cGLXRasterFont &);
   const cGLXRasterFont & operator=(const cGLXRasterFont &);

   float GetHeight() const { return m_height; }

   XFontStruct * m_pFontInfo;
   Display * m_pDisplay;

   int m_glyphStart;
   int m_listCount;
   int m_listBase;

   int m_charWidths[kDefaultGlyphLast - kDefaultGlyphFirst + 1];
   float m_height;
};

///////////////////////////////////////

cGLXRasterFont::cGLXRasterFont()
 : m_pFontInfo(NULL),
   m_pDisplay(NULL),
   m_glyphStart(0),
   m_listCount(0),
   m_listBase(0)
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

tResult cGLXRasterFont::DrawText(const char * pszText, int textLength, 
                                 uint flags, tRect * pRect, const cColor & color) const
{
   Assert(pszText != NULL);

   if (textLength < 0)
   {
      textLength = strlen(pszText);
   }

   if ((flags & kDT_CalcRect) == kDT_CalcRect)
   {
      // An alternative way...
      //int dir, ascent, descent;
      //XCharStruct charStruct;
      //XTextExtents(m_pFontInfo, pszText, length, &dir, &ascent, &descent, &charStruct);
      //*pWidth = charStruct.width;
      //*pHeight = ascent + descent;

      pRect->right = pRect->left + XTextWidth(m_pFontInfo, pszText, textLength);
      pRect->bottom = pRect->top + GetHeight();
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

bool cGLXRasterFont::Create(const char * pszFont, int pointSize)
{
   Assert(m_listBase == 0);

   bool bSucceeded = false;

   if (GetDisplay(&m_pDisplay) == S_OK)
   {
      Assert(m_pDisplay != NULL);

      m_pFontInfo = XLoadQueryFont(m_pDisplay, pszFont);
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

         memset(m_charWidths, maxWidth, sizeof(m_charWidths)); // @HACK
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

tResult FontCreate(const cFontDesc & fontDesc, IRenderFont * * ppFont)
{
   if (ppFont == NULL)
   {
      return E_POINTER;
   }

   cGLXRasterFont * pFont = new cGLXRasterFont;

   if (!pFont->Create(fontDesc.GetFace(), fontDesc.GetPointSize()))
   {
      SafeRelease(pFont);
      return E_FAIL;
   }

   *ppFont = static_cast<IRenderFont *>(pFont);
   return S_OK;
}

tResult FontGetDefaultDesc(cFontDesc * pFontDesc)
{
   if (pFontDesc == NULL)
   {
      return E_POINTER;
   }
   *pFontDesc = cFontDesc(g_szDefaultFont, g_defaultPointSize, kFE_None, kDefaultGlyphFirst, kDefaultGlyphLast);
   return S_OK;
}

tResult FontCreateDefault(IRenderFont * * ppFont)
{
   cFontDesc fontDesc;
   if (FontGetDefaultDesc(&fontDesc) == S_OK)
   {
      return FontCreate(fontDesc, ppFont);
   }
   return E_FAIL;
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
