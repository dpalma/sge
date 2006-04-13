///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

// Local Includes
#include "EditorAppForm.h"
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
//
// NAMESPACE: ManagedEditor
//

namespace ManagedEditor
{
   static const int kDefStatsX = 25;
   static const int kDefStatsY = 25;
   static const cColor kDefStatsColor(1,1,1,1);

   using namespace System::Windows::Forms;

   ///////////////////////////////////////////////////////////////////////////////
   //
   // CLASS: EditorAppForm
   //

   EditorAppForm::EditorAppForm()
    : m_pFont(NULL)
   {
      m_toolPalette = gcnew ToolPalette();
      m_toolPalette->Dock = System::Windows::Forms::DockStyle::Fill;
      Splitter->Panel1->Controls->Add(m_toolPalette);

      m_toolPalette->ToolSelect += gcnew ToolPalette::ToolSelectHandler(this, &EditorAppForm::OnToolSelect);

      m_glControl = gcnew GlControl();
	   m_glControl->Dock = System::Windows::Forms::DockStyle::Fill;
      Splitter->Panel2->Controls->Add(m_glControl);

      m_document = gcnew EditorDocument();

      System::Windows::Forms::Application::Idle += gcnew System::EventHandler(this, &EditorAppForm::OnIdle);

      System::Reflection::Assembly ^ a = System::Reflection::Assembly::GetExecutingAssembly();
      cli::array<System::Type ^> ^ types = a->GetTypes();
      for each(System::Type ^ type in types)
      {
         if (type->IsSubclassOf(EditorTool::typeid))
         {
            DebugMsg("tool found\n");
         }
      }
   }

   EditorAppForm::~EditorAppForm()
   {
      if (m_pFont != NULL)
      {
         m_pFont->Release();
      }
   }

   void EditorAppForm::OnIdle(System::Object ^ sender, System::EventArgs ^ e)
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

   void EditorAppForm::OnToolSelect(System::Object ^ sender, ToolSelectEventArgs ^ e)
   {
   }

   void EditorAppForm::NewDocument()
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

   void EditorAppForm::OpenDocument()
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
   
   void EditorAppForm::SaveDocument()
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
            SaveDocumentAs();
         }
      }
   }

   void EditorAppForm::SaveDocumentAs()
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

} // namespace ManagedEditor
