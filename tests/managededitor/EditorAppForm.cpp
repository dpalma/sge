///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "EditorAppForm.h"
#include "EditorMapSettings.h"
#include "EntityWrapper.h"

#include "engine/cameraapi.h"
#include "engine/entityapi.h"
#include "engine/terrainapi.h"
#include "gui/guiapi.h"
#include "platform/sys.h"
#include "render/renderapi.h"

#include "tech/globalobj.h"
#include "tech/schedulerapi.h"

#include <GL/glew.h>

extern void StringConvert(System::String ^ string, cStr * pStr);

namespace ManagedEditor
{
   using namespace System::Windows::Forms;

   ///////////////////////////////////////////////////////////////////////////////
   //
   // CLASS: EditorRenderControl
   //

   EditorRenderControl::EditorRenderControl()
   {
      HandleCreated += gcnew System::EventHandler(this, &EditorRenderControl::OnHandleCreated);
      HandleDestroyed += gcnew System::EventHandler(this, &EditorRenderControl::OnHandleDestroyed);
   }

   EditorRenderControl::~EditorRenderControl()
   {
   }

   void EditorRenderControl::OnHandleCreated(System::Object ^ sender, System::EventArgs ^ e)
   {
      UseGlobal(Renderer);
      if (!!pRenderer)
      {
         pRenderer->DestroyContext();
         HWND hWnd = reinterpret_cast<HWND>(Handle.ToPointer());
         pRenderer->CreateContext(hWnd);
      }
   }

   void EditorRenderControl::OnHandleDestroyed(System::Object ^ sender, System::EventArgs ^ e)
   {
      UseGlobal(Renderer);
      if (!!pRenderer)
      {
         pRenderer->DestroyContext();
      }
   }

   ///////////////////////////////////////////////////////////////////////////////
   //
   // CLASS: EditorAppForm
   //

   EditorAppForm::EditorAppForm()
    : m_resMgr(gcnew System::Resources::ResourceManager("ManagedEditor.Editor", System::Reflection::Assembly::GetExecutingAssembly()))
    , m_pEntityManagerListener(NULL)
   {
      m_toolPalette = gcnew ToolPalette();
      m_toolPalette->Dock = System::Windows::Forms::DockStyle::Fill;
      ToolSplitContainer->Panel1->Controls->Add(m_toolPalette);

      m_toolPalette->ToolSelect += gcnew ToolPalette::ToolSelectHandler(this, &EditorAppForm::toolPalette_OnToolSelect);

      m_propertyGrid = gcnew PropertyGrid();
      m_propertyGrid->Dock = System::Windows::Forms::DockStyle::Fill;
      ToolSplitContainer->Panel2->Controls->Add(m_propertyGrid);

      m_renderControl = gcnew EditorRenderControl();
	   m_renderControl->Dock = System::Windows::Forms::DockStyle::Fill;
      MainPanel->Controls->Add(m_renderControl);

      m_renderControl->Resize += gcnew System::EventHandler(this, &EditorAppForm::renderControl_OnResize);
      m_renderControl->MouseDown += gcnew System::Windows::Forms::MouseEventHandler(this, &EditorAppForm::renderControl_OnMouseDown);
      m_renderControl->MouseUp += gcnew System::Windows::Forms::MouseEventHandler(this, &EditorAppForm::renderControl_OnMouseUp);
      m_renderControl->MouseClick += gcnew System::Windows::Forms::MouseEventHandler(this, &EditorAppForm::renderControl_OnMouseClick);
      m_renderControl->MouseMove += gcnew System::Windows::Forms::MouseEventHandler(this, &EditorAppForm::renderControl_OnMouseMove);
      m_renderControl->MouseHover += gcnew System::EventHandler(this, &EditorAppForm::renderControl_OnMouseHover);
      m_renderControl->MouseWheel += gcnew System::Windows::Forms::MouseEventHandler(this, &EditorAppForm::renderControl_OnMouseWheel);

      m_document = gcnew EditorDocument();
      cTerrainSettings terrainSettings;
      terrainSettings.SetTileSet(_T("defaulttiles.xml")); // TODO: fix hard coded
      m_document->New(terrainSettings);

      m_document->DocumentChange += gcnew EditorDocument::DocumentChangeHandler(this, &EditorAppForm::OnDocumentChange);

      m_originalUndoText = UndoMenuItem->Text;
      m_originalRedoText = RedoMenuItem->Text;

      UndoMenuItem->Click += gcnew System::EventHandler(this, &EditorAppForm::OnUndo);
      RedoMenuItem->Click += gcnew System::EventHandler(this, &EditorAppForm::OnRedo);

      float centerX = static_cast<float>(terrainSettings.GetTileCountX() * terrainSettings.GetTileSize()) / 2;
      float centerZ = static_cast<float>(terrainSettings.GetTileCountZ() * terrainSettings.GetTileSize()) / 2;

      UseGlobal(CameraControl);
      pCameraControl->LookAtPoint(centerX, centerZ);

      Application::Idle += gcnew System::EventHandler(this, &EditorAppForm::OnIdle);
      Application::ApplicationExit += gcnew System::EventHandler(this, &EditorAppForm::OnExit);

      CreateEditorTools();

      m_pEntityManagerListener = new EditorAppFormEntityManagerListener();
      if (m_pEntityManagerListener)
      {
         m_pEntityManagerListener->SetEditorAppForm(this);
         UseGlobal(EntityManager);
         pEntityManager->AddEntityManagerListener(m_pEntityManagerListener);
      }
   }

   EditorAppForm::~EditorAppForm()
   {
   }

   EditorAppForm::!EditorAppForm()
   {
      StopGlobalObjects();
   }

   void EditorAppForm::OnIdle(System::Object ^ sender, System::EventArgs ^ e)
   {
      UseGlobal(Scheduler);
      pScheduler->NextFrame();

      UseGlobal(Renderer);
      if (pRenderer->BeginScene() == S_OK)
      {
         UseGlobal(TerrainRenderer);
         pTerrainRenderer->Render();

         UseGlobal(EntityManager);
         pEntityManager->RenderAll();

         int width, height;
         if (SysGetWindowSize(&width, &height) == S_OK)
         {
            cAutoIPtr<IRender2D> pRender2D;
            if (pRenderer->Begin2D(width, height, &pRender2D) == S_OK)
            {
               UseGlobal(GUIContext);
               if (!!pGUIContext)
                  pGUIContext->RenderGUI(pRender2D);

               pRenderer->End2D();
            }
         }

         pRenderer->EndScene();
      }
   }

   void EditorAppForm::OnExit(System::Object ^ sender, System::EventArgs ^ e)
   {
      if (m_pEntityManagerListener)
      {
         m_pEntityManagerListener->SetEditorAppForm(nullptr);

         UseGlobal(EntityManager);
         pEntityManager->RemoveEntityManagerListener(m_pEntityManagerListener);

         m_pEntityManagerListener->Release();
         m_pEntityManagerListener = NULL;
      }
   }

   void EditorAppForm::OnEntitySelectionChange()
   {
      System::Object ^ newSelectedObject = nullptr;

      UseGlobal(EntityManager);
      uint nSelected = pEntityManager->GetSelectedCount();
      if (nSelected > 0)
      {
         cAutoIPtr<IEnumEntities> pEntities;
         if (pEntityManager->GetSelected(&pEntities) == S_OK)
         {
            if (nSelected == 1)
            {
               ulong count = 1;
               cAutoIPtr<IEntity> pEntity;
               if (pEntities->Next(1, &pEntity, &count) == S_OK)
               {
                  newSelectedObject = gcnew EntityWrapper(pEntity);
               }
            }
            else // multiple entities selected
            {
            }
         }
      }

      m_propertyGrid->SelectedObject = newSelectedObject;
   }

   void EditorAppForm::OnDocumentChange(System::Object ^ sender, DocumentChangeEventArgs ^ e)
   {
      EditorDocument ^ doc = dynamic_cast<EditorDocument ^>(sender);
      if (!doc)
      {
         return;
      }

      UndoMenuItem->Text = m_originalUndoText;
      if (doc->UndoStack->Count > 0)
      {
         EditorDocumentCommand ^ command = doc->UndoStack->Peek();
         if (command)
         {
            System::String ^ commandLabel = command->Label;
            if (commandLabel)
            {
               UndoMenuItem->Text = System::String::Format(m_resMgr->GetString("undoMenuItemTemplate"), commandLabel);
            }
         }
      }

      RedoMenuItem->Text = m_originalRedoText;
      if (doc->RedoStack->Count > 0)
      {
         EditorDocumentCommand ^ command = doc->RedoStack->Peek();
         if (command)
         {
            System::String ^ commandLabel = command->Label;
            if (commandLabel)
            {
               RedoMenuItem->Text = System::String::Format(m_resMgr->GetString("redoMenuItemTemplate"), commandLabel);
            }
         }
      }
   }

   void EditorAppForm::OnUndo(System::Object ^ sender, System::EventArgs ^ e)
   {
      if (m_document)
      {
         m_document->Undo();
      }
   }

   void EditorAppForm::OnRedo(System::Object ^ sender, System::EventArgs ^ e)
   {
      if (m_document)
      {
         m_document->Redo();
      }
   }

   void EditorAppForm::renderControl_OnResize(System::Object ^ sender, System::EventArgs ^ e)
   {
      if (m_renderControl && (m_renderControl->Height > 0))
      {
         float aspect = static_cast<float>(m_renderControl->Width) / m_renderControl->Height;

         const float kFov = 70;
         const float kZNear = 1;
         const float kZFar = 5000;

         tMatrix4 proj;
         MatrixPerspective(kFov, aspect, kZNear, kZFar, &proj);

         UseGlobal(Renderer);
         pRenderer->SetProjectionMatrix(proj.m);

         glViewport(0, 0, m_renderControl->Width, m_renderControl->Height);
      }
   }

   void EditorAppForm::InvokeToolMethod(System::String ^ methodName,
                                        array<System::Type ^> ^ paramTypes,
                                        array<System::Object ^> ^ params)
   {
      EditorTool ^ tool = dynamic_cast<EditorTool ^>(m_toolPalette->CurrentTool);
      if (!tool)
      {
         return;
      }

      System::Reflection::MethodInfo ^ methodInfo = tool->GetType()->GetMethod(methodName, paramTypes);
      if (!methodInfo)
      {
         return;
      }

      EditorDocumentCommandArray ^ commands = dynamic_cast<EditorDocumentCommandArray ^>(methodInfo->Invoke(tool, params));
      if (commands && m_document)
      {
         m_document->AddDocumentCommands(commands, true);
      }
   }

   void EditorAppForm::renderControl_OnMouseDown(System::Object ^ sender, MouseEventArgs ^ e)
   {
      InvokeToolMethod(
         "OnMouseDown",
         gcnew array<System::Type ^>{System::Object::typeid, MouseEventArgs::typeid},
         gcnew array<System::Object ^>{sender, e});
   }

   void EditorAppForm::renderControl_OnMouseUp(System::Object ^ sender, MouseEventArgs ^ e)
   {
      InvokeToolMethod(
         "OnMouseUp",
         gcnew array<System::Type ^>{System::Object::typeid, MouseEventArgs::typeid},
         gcnew array<System::Object ^>{sender, e});
   }

   void EditorAppForm::renderControl_OnMouseClick(System::Object ^ sender, MouseEventArgs ^ e)
   {
      InvokeToolMethod(
         "OnMouseClick",
         gcnew array<System::Type ^>{System::Object::typeid, MouseEventArgs::typeid},
         gcnew array<System::Object ^>{sender, e});
   }

   void EditorAppForm::renderControl_OnMouseMove(System::Object ^ sender, MouseEventArgs ^ e)
   {
      InvokeToolMethod(
         "OnMouseMove",
         gcnew array<System::Type ^>{System::Object::typeid, MouseEventArgs::typeid},
         gcnew array<System::Object ^>{sender, e});
   }

   void EditorAppForm::renderControl_OnMouseHover(System::Object ^ sender, System::EventArgs ^ e)
   {
      InvokeToolMethod(
         "OnMouseHover",
         gcnew array<System::Type ^>{System::Object::typeid, System::EventArgs::typeid},
         gcnew array<System::Object ^>{sender, e});
   }

   void EditorAppForm::renderControl_OnMouseWheel(System::Object ^ sender, MouseEventArgs ^ e)
   {
      InvokeToolMethod(
         "OnMouseWheel",
         gcnew array<System::Type ^>{System::Object::typeid, MouseEventArgs::typeid},
         gcnew array<System::Object ^>{sender, e});
   }

   void EditorAppForm::toolPalette_OnToolSelect(System::Object ^ sender, ToolSelectEventArgs ^ e)
   {
      if (e->NewItem)
      {
         EditorTool ^ tool = dynamic_cast<EditorTool ^>(e->NewItem->Tool);
         if (tool)
         {
         }
      }
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
      openDlg->Filter = m_resMgr->GetString("editorFileFilter");
      openDlg->FileName = "";
      openDlg->DefaultExt = m_resMgr->GetString("editorDefaultFileExtension");
      openDlg->CheckFileExists = true;
      openDlg->CheckPathExists = true;
      if (openDlg->ShowDialog() == ::DialogResult::OK)
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
         saveDlg->Filter = m_resMgr->GetString("editorFileFilter");
         saveDlg->FileName = System::String::Format(m_resMgr->GetString("editorUntitledFileTemplate"), 1);
         saveDlg->DefaultExt = m_resMgr->GetString("editorDefaultFileExtension");
         if (saveDlg->ShowDialog() == ::DialogResult::OK)
         {
            m_document->Save(saveDlg->FileName);
         }
      }
   }

   void EditorAppForm::CreateEditorTools()
   {
      System::Reflection::Assembly ^ a = System::Reflection::Assembly::GetExecutingAssembly();
      array<System::Type ^> ^ types = a->GetTypes();
      for each(System::Type ^ type in types)
      {
         if (type->IsSubclassOf(EditorTool::typeid))
         {
            m_toolPalette->AddTool(type, m_resMgr);
         }
      }
   }

   ///////////////////////////////////////////////////////////////////////////////
   //
   // CLASS: EditorAppFormEntityManagerListener
   //

   EditorAppFormEntityManagerListener::EditorAppFormEntityManagerListener()
    : m_editorAppForm(nullptr)
   {
   }

   void EditorAppFormEntityManagerListener::SetEditorAppForm(EditorAppForm ^ editorAppForm)
   {
      m_editorAppForm = editorAppForm;
   }

   void EditorAppFormEntityManagerListener::OnEntitySelectionChange()
   {
      if (m_editorAppForm)
      {
         m_editorAppForm->OnEntitySelectionChange();
      }
   }

} // namespace ManagedEditor
