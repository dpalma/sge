///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "guifontfreetype.h"

#include "filepath.h"
#include "filespec.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shlobj.h>
#endif

#include "dbgalloc.h" // must be last header


///////////////////////////////////////////////////////////////////////////////

#define IsFlagSet(var, bit) (((var) & (bit)) == (bit))


///////////////////////////////////////////////////////////////////////////////

static tResult GetFontPath(cFilePath * pFontPath)
{
   if (pFontPath == NULL)
   {
      return E_POINTER;
   }

#ifdef _WIN32
   tResult result = E_FAIL;

   typedef tResult (STDCALL * tSHGetFolderPath)(HWND, int, HANDLE, DWORD, LPTSTR);

   HMODULE hSHFolder = LoadLibrary(_T("SHFolder.dll"));
   if (hSHFolder != NULL)
   {
      tSHGetFolderPath pfn = reinterpret_cast<tSHGetFolderPath>(GetProcAddress(hSHFolder,
#ifdef _UNICODE
         "SHGetFolderPathW"));
#else
         "SHGetFolderPathA"));
#endif
      if (pfn != NULL)
      {
         tChar szPath[MAX_PATH];
         if ((*pfn)(NULL, CSIDL_FONTS, NULL, 0, szPath) == S_OK)
         {
            *pFontPath = cFilePath(szPath);
            result = S_OK;
         }
      }

      FreeLibrary(hSHFolder);
   }

   return result;
#else
   return E_NOTIMPL;
#endif
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIFontFreetype
//

///////////////////////////////////////

cGUIFontFreetype::cGUIFontFreetype()
 : m_pFont(NULL)
{
}

///////////////////////////////////////

cGUIFontFreetype::~cGUIFontFreetype()
{
}

///////////////////////////////////////

void cGUIFontFreetype::OnFinalRelease()
{
   delete m_pFont, m_pFont = NULL;
}

///////////////////////////////////////

tResult cGUIFontFreetype::Create(const cGUIFontDesc & fontDesc)
{
   if (m_pFont != NULL)
   {
      return E_FAIL;
   }

   if (_tcslen(fontDesc.GetFace()) == 0)
   {
      return E_INVALIDARG;
   }

   cFileSpec fontName(fontDesc.GetFace());
   fontName.SetFileExt(_T("ttf"));

   cFilePath fontPath;
   if (GetFontPath(&fontPath) == S_OK)
   {
      fontName.SetPath(fontPath);
   }

   m_pFont = new FTGLTextureFont(fontName.CStr());
   if (m_pFont == NULL)
   {
      return E_OUTOFMEMORY;
   }

   if (m_pFont->Error() != FT_Err_Ok
      || !m_pFont->FaceSize(fontDesc.GetPointSize()))
   {
      delete m_pFont, m_pFont = NULL;
      return E_FAIL;
   }

   return S_OK;
}

///////////////////////////////////////

tResult cGUIFontFreetype::RenderText(const char * pszText, int /*textLength*/, tRect * pRect,
                                     uint flags, const cColor & color) const
{
   if (pszText == NULL || pRect == NULL)
   {
      return E_POINTER;
   }

   if (m_pFont == NULL)
   {
      return E_FAIL;
   }

   if (IsFlagSet(flags, kRT_CalcRect))
   {
      float llx = 0, lly = 0, llz = 0, urx = 0, ury = 0, urz = 0;
      m_pFont->BBox(pszText, llx, lly, llz, urx, ury, urz);
      pRect->right = pRect->left + Abs(FloatToInt(urx - llx));
      pRect->bottom = pRect->top + Abs(FloatToInt(lly - ury));
   }
   else
   {
   }

   return E_NOTIMPL;
}

///////////////////////////////////////

tResult cGUIFontFreetype::RenderText(const wchar_t * pszText, int textLength, tRect * pRect,
                                     uint flags, const cColor & color) const
{
   return E_NOTIMPL;
}

///////////////////////////////////////////////////////////////////////////////

tResult GUIFontCreateFreetype(const cGUIFontDesc & fontDesc, IGUIFont * * ppFont)
{
   if (ppFont == NULL)
   {
      return E_POINTER;
   }

   cAutoIPtr<cGUIFontFreetype> pFont(new cGUIFontFreetype);
   if (!pFont)
   {
      return E_OUTOFMEMORY;
   }

   tResult result = pFont->Create(fontDesc);
   if (result == S_OK)
   {
      *ppFont = CTAddRef(pFont);
      return S_OK;
   }

   return result;
}

///////////////////////////////////////////////////////////////////////////////
