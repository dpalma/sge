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
#include "schedulerapi.h"
#include "sys.h"
#include "terrainapi.h"

// Tech Includes
#include "globalobj.h"

#include <GL/glew.h>

extern void StringConvert(System::String ^ string, cStr * pStr);

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
   {
      m_toolPalette = gcnew ToolPalette();
      m_toolPalette->Dock = System::Windows::Forms::DockStyle::Fill;
      ToolPanel->Controls->Add(m_toolPalette);

      m_toolPalette->ToolSelect += gcnew ToolPalette::ToolSelectHandler(this, &EditorAppForm::OnToolSelect);

      m_glControl = gcnew GlControl();
	   m_glControl->Dock = System::Windows::Forms::DockStyle::Fill;
      MainPanel->Controls->Add(m_glControl);

      m_document = gcnew EditorDocument();
      cTerrainSettings terrainSettings;
      terrainSettings.SetTileSet(_T("defaulttiles.xml")); // TODO: fix hard coded
      m_document->New(terrainSettings);

      float centerX = static_cast<float>(terrainSettings.GetTileCountX() * terrainSettings.GetTileSize()) / 2;
      float centerZ = static_cast<float>(terrainSettings.GetTileCountZ() * terrainSettings.GetTileSize()) / 2;

      UseGlobal(CameraControl);
      pCameraControl->LookAtPoint(centerX, centerZ);

      System::Windows::Forms::Application::Idle += gcnew System::EventHandler(this, &EditorAppForm::OnIdle);

      CreateEditorTools();
   }

   EditorAppForm::~EditorAppForm()
   {
   }

   void EditorAppForm::OnIdle(System::Object ^ sender, System::EventArgs ^ e)
   {
      UseGlobal(Scheduler);
      pScheduler->NextFrame();

      UseGlobal(Renderer);
      if (pRenderer->BeginScene() == S_OK)
      {
         UseGlobal(Camera);
         pCamera->SetGLState();

         UseGlobal(TerrainRenderer);
         pTerrainRenderer->Render();

         UseGlobal(EntityManager);
         pEntityManager->RenderAll();

         UseGlobal(GUIContext);
         if (!!pGUIContext)
         {
            pGUIContext->RenderGUI();
         }

         pRenderer->EndScene();

         m_glControl->SwapBuffers();
      }
   }

   void EditorAppForm::OnToolSelect(System::Object ^ sender, ToolSelectEventArgs ^ e)
   {
   }

   void EditorAppForm::OnResize(System::EventArgs ^ e)
   {
      EditorForm::OnResize(e);

      float aspect = static_cast<float>(m_glControl->Width) / m_glControl->Height;

      const float kFov = 70;
      const float kZNear = 1;
      const float kZFar = 5000;

      UseGlobal(Camera);
      pCamera->SetPerspective(kFov, aspect, kZNear, kZFar);

      glViewport(0, 0, m_glControl->Width, m_glControl->Height);

      glMatrixMode(GL_PROJECTION);
      glLoadMatrixf(pCamera->GetProjectionMatrix().m);
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
         terrainSettings.SetTileCountX(mapSettings->Width);
         terrainSettings.SetTileCountZ(mapSettings->Height);

         cStr tileSet;
         StringConvert(mapSettings->TileSet, &tileSet);
         terrainSettings.SetTileSet(tileSet.c_str());

         m_document = gcnew EditorDocument();
         m_document->New(terrainSettings);
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

   void EditorAppForm::CreateEditorTools()
   {
      System::Reflection::Assembly ^ a = System::Reflection::Assembly::GetExecutingAssembly();
      cli::array<System::Type ^> ^ types = a->GetTypes();
      for each(System::Type ^ type in types)
      {
         if (type->IsSubclassOf(EditorTool::typeid))
         {
            System::String ^ groupName = nullptr;
            cli::array<System::Object ^> ^ attribs = type->GetCustomAttributes(false);
            for each(System::Object ^ attrib in attribs)
            {
               if (attrib->GetType() == EditorToolGroup::typeid)
               {
                  EditorToolGroup ^ toolGroup = dynamic_cast<EditorToolGroup ^>(attrib);
                  groupName = toolGroup->Group;
               }
            }
            m_toolPalette->AddTool(m_toolPalette->AddGroup(groupName, nullptr), type->FullName, -1);
         }
      }
   }

} // namespace ManagedEditor
