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

/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cCommandStack
//

typedef std::stack<IEditorCommand *> tCommandStack;

class cCommandStack
{
public:
   cCommandStack();
   ~cCommandStack();

   tResult FlushUndo();
   tResult FlushRedo();

   tResult CanUndo(cStr * pLabel);
   tResult Undo();
   tResult CanRedo(cStr * pLabel);
   tResult Redo();

   tResult PushCommand(IEditorCommand * pCommand);

private:
   tCommandStack m_undoStack, m_redoStack;
};


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

   cTerrain * AccessTerrain();

   virtual tResult AddCommand(IEditorCommand * pCommand);

   virtual tResult AddEditorModelListener(IEditorModelListener * pListener);
   virtual tResult RemoveEditorModelListener(IEditorModelListener * pListener);

private:
   bool m_bModified;

   cTerrain * m_pTerrain;

   cCommandStack m_commandStack;
};

////////////////////////////////////////

inline cTerrain * cEditorDoc::AccessTerrain()
{
   return m_pTerrain;
}

/////////////////////////////////////////////////////////////////////////////

#endif // !defined(INCLUDED_EDITORDOC_H)
