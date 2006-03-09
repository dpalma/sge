///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

// Local Includes
#include "EditorForm.h"

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


static const int kDefStatsX = 25;
static const int kDefStatsY = 25;
static const cColor kDefStatsColor(1,1,1,1);


namespace Editor
{

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cEditorForm
//

cEditorForm::cEditorForm()
 : m_pFont(NULL)
{
   InitializeComponent();

   m_glControl = gcnew cGlControl();
   Controls->Add(m_glControl);

   System::Windows::Forms::Application::Idle += gcnew System::EventHandler(this, &cEditorForm::OnIdle);
}

cEditorForm::~cEditorForm()
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

void cEditorForm::OnIdle(System::Object ^ sender, System::EventArgs ^ e)
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

} // namespace Editor

///////////////////////////////////////////////////////////////////////////////
