/////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "ToolPalette.h"


namespace ManagedEditor
{

   ///////////////////////////////////////////////////////////////////////////////
   //
   // CLASS: ToolPaletteItemAttribute
   //

   ToolPaletteItemAttribute::ToolPaletteItemAttribute()
    : m_label(nullptr)
    , m_image(nullptr)
    , m_group(nullptr)
   {
   }

   System::String ^ ToolPaletteItemAttribute::Label::get()
   {
      return m_label;
   }

   void ToolPaletteItemAttribute::Label::set(System::String ^ label)
   {
      m_label = label;
   }

   System::String ^ ToolPaletteItemAttribute::Image::get()
   {
      return m_image;
   }

   void ToolPaletteItemAttribute::Image::set(System::String ^ image)
   {
      m_image = image;
   }

   System::String ^ ToolPaletteItemAttribute::Group::get()
   {
      return m_group;
   }

   void ToolPaletteItemAttribute::Group::set(System::String ^ group)
   {
      m_group = group;
   }

   ///////////////////////////////////////////////////////////////////////////////
   //
   // CLASS: ToolPaletteItem
   //

   ////////////////////////////////////////

   ToolPaletteItem::ToolPaletteItem(System::Object ^ tool, System::String ^ name)
    : m_tool(tool)
    , m_name(name)
    , m_imageIndex(-1)
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

      if (!other)
      {
         return false; // Should never happen because of the GetType() != obj->GetType() check above
      }

      return m_tool == other->m_tool
         && m_name == other->m_name
         && m_imageIndex == other->m_imageIndex
         && m_bEnabled == other->m_bEnabled;
   }


   ///////////////////////////////////////////////////////////////////////////////
   //
   // CLASS: ToolPaletteGroup
   //

   ////////////////////////////////////////

   ToolPaletteGroup::ToolPaletteGroup(System::String ^ name)
    : m_name(name)
    , m_items(gcnew System::Collections::ArrayList())
    , m_images(gcnew System::Windows::Forms::ImageList())
    , m_bCollapsed(false)
   {
      m_images->TransparentColor = System::Drawing::Color::Magenta;
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

   ToolPaletteGroup ^ ToolPalette::AddGroup(System::String ^ groupName)
   {
      if (groupName != nullptr)
      {
         ToolPaletteGroup ^ group = FindGroup(groupName);
         if (group != nullptr)
         {
            return group;
         }

         ToolPaletteGroup ^ newGroup = gcnew ToolPaletteGroup(groupName);
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

   ToolPaletteItem ^ ToolPalette::AddTool(System::Type ^ type, System::Resources::ResourceManager ^ resMgr)
   {
      if (type)
      {
         ToolPaletteItemAttribute ^ tpi = dynamic_cast<ToolPaletteItemAttribute ^>(
            System::Attribute::GetCustomAttribute(type, ToolPaletteItemAttribute::typeid));
         if (tpi && resMgr)
         {
            System::String ^ groupName = tpi->Group ? resMgr->GetString(tpi->Group) : nullptr;
            if (groupName)
            {
               ToolPaletteGroup ^ group = AddGroup(groupName);
               System::String ^ toolLabel = tpi->Label ? resMgr->GetString(tpi->Label) : nullptr;
               System::Drawing::Image ^ toolImage = tpi->Image ? dynamic_cast<System::Drawing::Image ^>(resMgr->GetObject(tpi->Image)) : nullptr;
               System::Object ^ tool = System::Activator::CreateInstance(type);
               if (group && tool)
               {
                  int imageIndex = -1;
                  if (toolImage)
                  {
                     group->Images->Images->Add(toolImage);
                     imageIndex = group->Images->Images->Count - 1;
                  }
                  ToolPaletteItem ^ newItem = gcnew ToolPaletteItem(tool, toolLabel ? toolLabel : type->FullName);
                  group->Items->Add(newItem);
                  newItem->ImageIndex = imageIndex;
                  Invalidate();
                  return newItem;
               }
            }
         }
      }
      return nullptr;
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
            DrawGroupHeading(e->Graphics, group, itemRect);
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
                  DrawSelectedItem(e->Graphics, group->Images, item, itemRect);
               }
               else
               {
                  DrawToolItem(e->Graphics, group->Images, item, itemRect);
               }
               itemRect.Offset(0, ItemHeight);
            }
         }
      }
   }

   ////////////////////////////////////////

   void ToolPalette::DrawGroupHeading(System::Drawing::Graphics ^ graphics,
                                      ToolPaletteGroup ^ group,
                                      System::Drawing::Rectangle bounds)
   {
      using namespace System::Drawing;
      graphics->FillRectangle(Brushes::DarkGray, bounds);
      graphics->DrawString(group->Name, SystemFonts::DefaultFont, Brushes::White,
         PointF(static_cast<float>(bounds.Left), static_cast<float>(bounds.Top)));
   }

   ////////////////////////////////////////

   void ToolPalette::DrawToolItem(System::Drawing::Graphics ^ graphics,
                                  System::Windows::Forms::ImageList ^ imageList,
                                  ToolPaletteItem ^ item,
                                  System::Drawing::Rectangle bounds)
   {
      using namespace System::Drawing;
      graphics->FillRectangle(SystemBrushes::Control, bounds);
      if (imageList && (item->ImageIndex > -1))
      {
         Rectangle imageRect(bounds.Left, bounds.Top, bounds.Height, bounds.Height);
         imageList->Draw(graphics, Point(bounds.Left, bounds.Top), item->ImageIndex);
         bounds = Rectangle::FromLTRB(bounds.Left + bounds.Height, bounds.Top, bounds.Right, bounds.Bottom);
      }
      graphics->DrawString(item->Name, SystemFonts::DefaultFont, SystemBrushes::ControlText,
         PointF(static_cast<float>(bounds.Left), static_cast<float>(bounds.Top)));
   }

   ////////////////////////////////////////

   void ToolPalette::DrawSelectedItem(System::Drawing::Graphics ^ graphics,
                                      System::Windows::Forms::ImageList ^ imageList,
                                      ToolPaletteItem ^ item,
                                      System::Drawing::Rectangle bounds)
   {
      DrawToolItem(graphics, imageList, item, bounds);
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
