/////////////////////////////////////////////////////////////////////////////
// $Id$

#if !defined(INCLUDED_EDITORDOC_H)
#define INCLUDED_EDITORDOC_H

#include "editorapi.h"
#include "connptimpl.h"

#include <stack>

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class cTerrain;
struct sTerrainVertex;

F_DECLARE_INTERFACE(IMaterial);

typedef std::stack<IEditorCommand *> tCommandStack;

/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cEditorDoc
//

class cEditorDoc : public CComObjectRoot,
                   public CComCoClass<cEditorDoc, &CLSID_EditorDoc>,
                   public cConnectionPoint<IEditorModel, IEditorModelListener>
{
public:
	cEditorDoc();
	virtual ~cEditorDoc();

   DECLARE_NO_REGISTRY()
   DECLARE_NOT_AGGREGATABLE(cEditorDoc)

   BEGIN_COM_MAP(cEditorDoc)
      COM_INTERFACE_ENTRY(IEditorModel)
   END_COM_MAP()

   // IEditorModel methods
   virtual tResult New(const cMapSettings * pMapSettings);
   virtual tResult Open(IReader * pReader);
   virtual tResult Save(IWriter * pWriter);
   virtual tResult Reset();
   virtual tResult IsModified();

   virtual tResult CanUndo(cStr * pLabel);
   virtual tResult Undo();
   virtual tResult CanRedo(cStr * pLabel);
   virtual tResult Redo();

   IMaterial * AccessMaterial();

   cTerrain * AccessTerrain();

   virtual tResult AddCommand(IEditorCommand * pCommand);

   virtual tResult AddEditorModelListener(IEditorModelListener * pListener);
   virtual tResult RemoveEditorModelListener(IEditorModelListener * pListener);

private:
   bool m_bModified;

   cAutoIPtr<IMaterial> m_pMaterial;

   cTerrain * m_pTerrain;

   tCommandStack m_undoStack, m_redoStack;
};

////////////////////////////////////////

inline IMaterial * cEditorDoc::AccessMaterial()
{
   return m_pMaterial;
}

////////////////////////////////////////

inline cTerrain * cEditorDoc::AccessTerrain()
{
   return m_pTerrain;
}

/////////////////////////////////////////////////////////////////////////////

#endif // !defined(INCLUDED_EDITORDOC_H)
