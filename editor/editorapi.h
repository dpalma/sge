///////////////////////////////////////////////////////////////////////////////
// $Id$

#if !defined(INCLUDED_EDITORAPI_H)
#define INCLUDED_EDITORAPI_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

F_DECLARE_INTERFACE(IEditorApp);
F_DECLARE_INTERFACE(IEditorLoopClient);

/////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IEditorApp
//

interface __declspec(uuid("2A04E541-6BA1-41e9-92FA-E7B3D493F1A2")) IEditorApp : IUnknown
{
   virtual tResult AddLoopClient(IEditorLoopClient * pLoopClient) = 0;
   virtual tResult RemoveLoopClient(IEditorLoopClient * pLoopClient) = 0;
};

IEditorApp * AccessEditorApp();

/////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IEditorLoopClient
//

interface __declspec(uuid("ED1B3A1A-E2D8-4eec-AABD-648A548729E8")) IEditorLoopClient : IUnknown
{
   virtual void OnFrame(double time, double elapsed) = 0;
};

/////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_EDITORAPI_H
