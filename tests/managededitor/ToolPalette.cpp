/////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "ToolPalette.h"


namespace ManagedEditor
{

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: ToolPaletteItem
//

////////////////////////////////////////

ToolPaletteItem::ToolPaletteItem(System::Object ^ tool, System::String ^ name, int iImage)
 : m_tool(tool)
 , m_name(name)
 , m_iImage(iImage)
 , m_bEnabled(true)
{
}

////////////////////////////////////////

ToolPaletteItem::~ToolPaletteItem()
{
}

////////////////////////////////////////

bool ToolPaletteItem::Equals(System::Object ^ obj)
{
   if (obj == nullptr || GetType() != obj->GetType())
   {
      return false;
   }

   ToolPaletteItem ^ other = dynamic_cast<ToolPaletteItem ^>(obj);

   return m_tool == other->m_tool
      && m_name == other->m_name
      && m_iImage == other->m_iImage
      && m_bEnabled == other->m_bEnabled;
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: ToolPaletteGroup
//

////////////////////////////////////////

ToolPaletteGroup::ToolPaletteGroup(System::String ^ name, System::Windows::Forms::ImageList ^ imageList)
 : m_name(name)
 , m_imageList(imageList)
 , m_items(gcnew System::Collections::ArrayList())
 , m_bCollapsed(false)
{
}

////////////////////////////////////////

ToolPaletteGroup::~ToolPaletteGroup()
{
}

////////////////////////////////////////

ToolPaletteItem ^ ToolPaletteGroup::FindItem(const System::String ^ toolName)
{
   System::Collections::IEnumerator ^ itemEnum = m_items->GetEnumerator();
   while (itemEnum->MoveNext())
   {
      ToolPaletteItem ^ item = dynamic_cast<ToolPaletteItem ^>(itemEnum->Current);
      if (item->Name->CompareTo(const_cast<System::String ^>(toolName)) == 0)
      {
         return item;
      }
   }
   return nullptr;
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: ToolSelectEventArgs
//

////////////////////////////////////////

ToolSelectEventArgs::ToolSelectEventArgs(ToolPaletteItem ^ oldItem, ToolPaletteItem ^ newItem)
 : m_oldItem(oldItem)
 , m_newItem(newItem)
{
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: ToolPalette
//

////////////////////////////////////////

ToolPalette::ToolPalette()
 : m_groups(gcnew System::Collections::ArrayList())
 , m_currentItem(nullptr)
 , m_itemHeight(0)
 , m_mouseOverItem(nullptr)
 , m_clickCandidateItem(nullptr)
{
}

////////////////////////////////////////

ToolPalette::~ToolPalette()
{
}

////////////////////////////////////////

int ToolPalette::ItemHeight::get()
{
   if (m_itemHeight == 0)
   {
      System::Drawing::Font ^ font = System::Drawing::SystemFonts::DefaultFont;
      System::Drawing::Size border = System::Windows::Forms::SystemInformation::BorderSize;
      m_itemHeight = font->Height + (2 * border.Height);
   }
   return m_itemHeight;
}

////////////////////////////////////////

ToolPaletteGroup ^ ToolPalette::AddGroup(System::String ^ groupName,
                                         System::Windows::Forms::ImageList ^ imageList)
{
   if (groupName != nullptr)
   {
      ToolPaletteGroup ^ group = FindGroup(groupName);
      if (group != nullptr)
      {
         return group;
      }

      ToolPaletteGroup ^ newGroup = gcnew ToolPaletteGroup(groupName, imageList);
      m_groups->Add(newGroup);
      return newGroup;
   }

   return nullptr;
}

////////////////////////////////////////

ToolPaletteGroup ^ ToolPalette::FindGroup(System::String ^ groupName)
{
   if (groupName != nullptr)
   {
      System::Collections::IEnumerator ^ groupEnum = m_groups->GetEnumerator();
      while (groupEnum->MoveNext())
      {
         ToolPaletteGroup ^ group = dynamic_cast<ToolPaletteGroup ^>(groupEnum->Current);
         if (System::String::Compare(groupName, group->Name) == 0)
         {
            return group;
         }
      }
   }
   return nullptr;
}

////////////////////////////////////////

ToolPaletteItem ^ ToolPalette::AddTool(ToolPaletteGroup ^ group, System::Object ^ tool, System::String ^ toolName, int iImage)
{
   ToolPaletteItem ^ newTool = gcnew ToolPaletteItem(tool, toolName, iImage);
   group->Items->Add(newTool);
   Invalidate();
   return newTool;
}

////////////////////////////////////////

void ToolPalette::OnMouseDown(System::Windows::Forms::MouseEventArgs ^ e)
{
   Assert(m_clickCandidateItem == nullptr);
   m_clickCandidateItem = GetHitItem(e);
}

////////////////////////////////////////

void ToolPalette::OnMouseLeave(System::EventArgs ^ e)
{
   SetMouseOverItem(nullptr);
}

////////////////////////////////////////

void ToolPalette::OnMouseMove(System::Windows::Forms::MouseEventArgs ^ e)
{
   SetMouseOverItem(GetHitItem(e));
}

////////////////////////////////////////

void ToolPalette::OnMouseUp(System::Windows::Forms::MouseEventArgs ^ e)
{
   if (m_clickCandidateItem != nullptr)
   {
      if (m_clickCandidateItem == GetHitItem(e))
      {
         {
            ToolPaletteGroup ^ group = dynamic_cast<ToolPaletteGroup ^>(m_clickCandidateItem);
            if (group != nullptr)
            {
               group->Collapsed = !group->Collapsed;
               Invalidate();
            }
         }

         {
            ToolPaletteItem ^ item = dynamic_cast<ToolPaletteItem ^>(m_clickCandidateItem);
            if (item != nullptr && item->Enabled)
            {
               if (item != m_currentItem)
               {
                  if (&ToolPalette::ToolSelect != nullptr)
                  {
                     ToolSelect(this, gcnew ToolSelectEventArgs(m_currentItem, item));
                  }

                  m_currentItem = item;

                  Invalidate();
               }
            }
         }
      }

      m_clickCandidateItem = nullptr;
   }
}

////////////////////////////////////////

void ToolPalette::OnPaint(System::Windows::Forms::PaintEventArgs ^ e)
{
   System::Drawing::Rectangle itemRect = ClientRectangle;
   itemRect.Height = ItemHeight;

   System::Collections::IEnumerator ^ groupEnum = Groups->GetEnumerator();
   while (groupEnum->MoveNext())
   {
      ToolPaletteGroup ^ group = dynamic_cast<ToolPaletteGroup ^>(groupEnum->Current);

      if (group->Name->Length > 0)
      {
         PaintGroupHeading(group, e->Graphics, itemRect);
         itemRect.Offset(0, ItemHeight);
      }

      if (!group->Collapsed)
      {
         System::Collections::IEnumerator ^ itemEnum = group->Items->GetEnumerator();
         while (itemEnum->MoveNext())
         {
            ToolPaletteItem ^ item = dynamic_cast<ToolPaletteItem ^>(itemEnum->Current);
            if (item == CurrentItem)
            {
               PaintSelectedItem(item, e->Graphics, itemRect);
            }
            else
            {
               PaintToolItem(item, e->Graphics, itemRect);
            }
            itemRect.Offset(0, ItemHeight);
         }
      }
   }
}

////////////////////////////////////////

void ToolPalette::PaintGroupHeading(ToolPaletteGroup ^ group,
                                    System::Drawing::Graphics ^ graphics,
                                    System::Drawing::Rectangle ^ itemRect)
{
   using namespace System::Drawing;
   graphics->FillRectangle(Brushes::DarkGray, *itemRect);
   graphics->DrawString(group->Name, SystemFonts::DefaultFont, Brushes::White,
      PointF(static_cast<float>(itemRect->Left), static_cast<float>(itemRect->Top)));
}

////////////////////////////////////////

void ToolPalette::PaintToolItem(ToolPaletteItem ^ item,
                                System::Drawing::Graphics ^ graphics,
                                System::Drawing::Rectangle ^ itemRect)
{
   using namespace System::Drawing;
   graphics->FillRectangle(Brushes::LightGray, *itemRect);
   graphics->DrawString(item->Name, SystemFonts::DefaultFont, Brushes::Black,
      PointF(static_cast<float>(itemRect->Left), static_cast<float>(itemRect->Top)));
}

////////////////////////////////////////

void ToolPalette::PaintSelectedItem(ToolPaletteItem ^ item,
                                    System::Drawing::Graphics ^ graphics,
                                    System::Drawing::Rectangle ^ itemRect)
{
   PaintToolItem(item, graphics, itemRect);
}

////////////////////////////////////////

static bool PointInRect(int x, int y, System::Drawing::Rectangle ^ rect)
{
   return x >= rect->Left
      && x <= rect->Right
      && y >= rect->Top
      && y <= rect->Bottom;
}

System::Object ^ ToolPalette::GetHitItem(System::Windows::Forms::MouseEventArgs ^ e)
{
   System::Drawing::Rectangle itemRect = ClientRectangle;
   itemRect.Height = ItemHeight;

   System::Object ^ hitItem = nullptr;

   System::Collections::IEnumerator ^ groupEnum = Groups->GetEnumerator();
   while (groupEnum->MoveNext() && (hitItem == nullptr))
   {
      ToolPaletteGroup ^ group = dynamic_cast<ToolPaletteGroup ^>(groupEnum->Current);

      if (group->Name->Length > 0)
      {
         if (PointInRect(e->X, e->Y, itemRect))
         {
            hitItem = group;
            break;
         }

         itemRect.Offset(0, ItemHeight);
      }

      if (!group->Collapsed)
      {
         System::Collections::IEnumerator ^ toolEnum = group->Items->GetEnumerator();
         while (toolEnum->MoveNext())
         {
            ToolPaletteItem ^ tool = dynamic_cast<ToolPaletteItem ^>(toolEnum->Current);

            if (PointInRect(e->X, e->Y, itemRect))
            {
               hitItem = tool;
               break;
            }

            itemRect.Offset(0, ItemHeight);
         }
      }
   }

   return hitItem;
}

////////////////////////////////////////

void ToolPalette::SetMouseOverItem(System::Object ^ obj)
{
   if (obj != m_mouseOverItem)
   {
      m_mouseOverItem = obj;
      Invalidate();
   }
}


///////////////////////////////////////////////////////////////////////////////

} // namespace ManagedEditor
