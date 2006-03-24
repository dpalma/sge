///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

// Local Includes
#include "EditorForm.h"
#include "EditorAbout.h"
#include "EditorMapSettings.h"

// GUI Includes
#include "guiapi.h"

// Engine Includes
#include "cameraapi.h"
#include "entityapi.h"
#include "renderapi.h"
#include "simapi.h"
#include "sys.h"
#include "terrainapi.h"

// Tech Includes
#include "globalobj.h"

#include <GL/glew.h>


///////////////////////////////////////////////////////////////////////////////

namespace ManagedEditor
{

static const int kDefStatsX = 25;
static const int kDefStatsY = 25;
static const cColor kDefStatsColor(1,1,1,1);

using namespace System::Windows::Forms;

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: EditorForm
//

EditorForm::EditorForm()
 : m_pFont(NULL)
{
   InitializeComponent();

   m_glControl = gcnew GlControl();
	m_glControl->Dock = System::Windows::Forms::DockStyle::Fill;
   toolStripContainer1->ContentPanel->Controls->Add(m_glControl);

   m_document = gcnew EditorDocument();

   System::Windows::Forms::Application::Idle += gcnew System::EventHandler(this, &EditorForm::OnIdle);
}

EditorForm::~EditorForm()
{
   if (components)
   {
      delete components;
   }

   if (m_pFont != NULL)
   {
      m_pFont->Release();
   }
}

void EditorForm::OnIdle(System::Object ^ sender, System::EventArgs ^ e)
{
   UseGlobal(Sim);
   pSim->NextFrame();

   UseGlobal(Renderer);
   if (pRenderer->BeginScene() == S_OK)
   {
      UseGlobal(Camera);
      glMatrixMode(GL_MODELVIEW);
      glLoadMatrixf(pCamera->GetViewMatrix().m);

      UseGlobal(TerrainRenderer);
      pTerrainRenderer->Render();

      UseGlobal(EntityManager);
      pEntityManager->RenderAll();

      UseGlobal(GUIContext);
      if (!!pGUIContext)
      {
         if (m_pFont == NULL)
         {
            IGUIFont * pFont = NULL;
            if (pGUIContext->GetDefaultFont(&pFont) == S_OK)
            {
               m_pFont = pFont;
            }
         }

         cAutoIPtr<IGUIRenderDeviceContext> pRenderDeviceContext;
         if (pGUIContext->GetRenderDeviceContext(&pRenderDeviceContext) == S_OK)
         {
            pRenderDeviceContext->Begin2D();

            pGUIContext->RenderGUI();

            if (m_pFont != NULL)
            {
               tChar szStats[100];
               SysReportFrameStats(szStats, _countof(szStats));

               tRect rect(kDefStatsX, kDefStatsY, 0, 0);
               m_pFont->RenderText(szStats, _tcslen(szStats), &rect,
                                   kRT_NoClip | kRT_DropShadow, kDefStatsColor);
            }

            pRenderDeviceContext->End2D();
         }
      }

      pRenderer->EndScene();

      m_glControl->SwapBuffers();
   }
}

System::Void EditorForm::OnFileOpen(System::Object ^ sender, System::EventArgs ^ e)
{
   OpenFileDialog ^ openDlg = gcnew OpenFileDialog();
   openDlg->Filter = "SGE Map Files (*.sgm)|*.sgm|All Files (*.*)|*.*";
   openDlg->FileName = "";
   openDlg->DefaultExt = ".sgm";
   openDlg->CheckFileExists = true;
   openDlg->CheckPathExists = true;
   if (openDlg->ShowDialog() == System::Windows::Forms::DialogResult::OK)
   {
      EditorDocument ^ newDoc = gcnew EditorDocument();
      if (newDoc->Open(openDlg->FileName))
      {
         m_document = newDoc;
      }
   }
}

System::Void EditorForm::OnFileNew(System::Object ^ sender, System::EventArgs ^ e)
{
   cTerrainSettings terrainSettings;
   terrainSettings.SetTileSet(_T("defaulttiles.xml")); // TODO: fix hard coded

   EditorMapSettings ^ mapSettings = gcnew EditorMapSettings();
   mapSettings->Width = 128;
   mapSettings->Height = 128;
   mapSettings->TileSet = _T("defaulttiles.xml");

   EditorMapSettingsDlg ^ mapSettingsDlg = gcnew EditorMapSettingsDlg(mapSettings);
   if (mapSettingsDlg->ShowDialog() == System::Windows::Forms::DialogResult::OK)
   {
      m_document = gcnew EditorDocument();
   }
}

System::Void EditorForm::OnFileExit(System::Object ^ sender, System::EventArgs ^ e)
{
   Application::Exit();
}

System::Void EditorForm::OnFileSave(System::Object ^ sender, System::EventArgs ^ e)
{
   if (m_document)
   {
      if (m_document->FileName)
      {
         try
         {
            m_document->Save(m_document->FileName);
         }
         catch (Exception ^ ex)
         {
            MessageBox::Show(ex->ToString());
         }
      }
      else
      {
         OnFileSaveAs(sender, e);
      }
   }
}

System::Void EditorForm::OnFileSaveAs(System::Object ^ sender, System::EventArgs ^ e)
{
   try
   {
      if (m_document)
      {
         SaveFileDialog ^ saveDlg = gcnew SaveFileDialog;
         saveDlg->Filter = "SGE Map Files (*.sgm)|*.sgm|All Files (*.*)|*.*";
         saveDlg->FileName = "Untitled1.sgm";
         saveDlg->DefaultExt = ".sgm";
         if (saveDlg->ShowDialog() == System::Windows::Forms::DialogResult::OK)
         {
            m_document->Save(saveDlg->FileName);
         }
      }
   }
   catch (Exception ^ ex)
   {
      MessageBox::Show(ex->ToString());
   }
}

System::Void EditorForm::OnHelpAbout(System::Object ^ sender, System::EventArgs ^ e)
{
   EditorAbout ^ aboutBox = gcnew EditorAbout();
   aboutBox->ShowDialog();
}

} // namespace ManagedEditor

///////////////////////////////////////////////////////////////////////////////
