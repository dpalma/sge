///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "EditorMapSettings.h"

#include "terrainapi.h"

#include "globalobj.h"
#include "resourceapi.h"

///////////////////////////////////////////////////////////////////////////////

namespace ManagedEditor
{

   ///////////////////////////////////////////////////////////////////////////////
   //
   // CLASS: EditorMapSettingsDlg
   //

   System::Void EditorMapSettingsDlg::OnValidated(System::Object ^ sender, System::EventArgs ^ e)
   {
      if (sender->Equals(widthComboBox))
      {
      }
      else if (sender->Equals(heightComboBox))
      {
      }
   }

   System::Void EditorMapSettingsDlg::OnLoad(System::Object ^ sender, System::EventArgs ^ e)
   {
      widthComboBox->SelectedText = m_mapSettings->Width.ToString();

      std::vector<cStr> tileSets;
      UseGlobal(ResourceManager);
      if (pResourceManager->ListResources(kRT_TerrainTileSet, &tileSets) == S_OK)
      {
         std::vector<cStr>::const_iterator iter = tileSets.begin();
         for (; iter != tileSets.end(); iter++)
         {
            cStr tileSetName;
            ITerrainTileSet * pTileSet = NULL;
            if (pResourceManager->Load(iter->c_str(), kRT_TerrainTileSet, NULL, (void**)&pTileSet) != S_OK
               || pTileSet->GetName(&tileSetName) != S_OK)
            {
               continue;
            }

            //m_tileSets.push_back(*iter);
            int iItem = tileSetComboBox->Items->Add(gcnew System::String(tileSetName.c_str()));
            //SendDlgItemMessage(IDC_MAP_TILESET, CB_SETITEMDATA, index, (LPARAM)(m_tileSets.size() - 1));
            //if (m_tileSet.Compare(iter->c_str()) == 0)
            //{
            //   iSelection = index;
            //}
         }
      }
   }

} // namespace ManagedEditor
