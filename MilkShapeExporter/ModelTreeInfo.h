/////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_MODELTREEINFO_H
#define INCLUDED_MODELTREEINFO_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

typedef struct msModel msModel;
typedef struct msMesh msMesh;

/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cModelTreeInfo
//

class cModelTreeInfo
{
public:
   cModelTreeInfo(msModel * pModel);

   BOOL DisplayModelInfo(CTreeCtrl * pTreeCtrl);

protected:
   BOOL DisplayMaterials(CTreeCtrl * pTreeCtrl, HTREEITEM hParent = TVI_ROOT);
   BOOL DisplayMeshes(CTreeCtrl * pTreeCtrl, HTREEITEM hParent = TVI_ROOT);
   BOOL DisplayMeshVertices(msMesh * pMsMesh, CTreeCtrl * pTreeCtrl, HTREEITEM hParent = TVI_ROOT);

private:
   msModel * m_pModel;
};

/////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_MODELTREEINFO_H
