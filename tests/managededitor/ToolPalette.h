/////////////////////////////////////////////////////////////////////////////
// $Id$

#if !defined(INCLUDED_TOOLPALETTE_H)
#define INCLUDED_TOOLPALETTE_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

namespace ManagedEditor
{

ref class ToolPaletteGroup;


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: ToolPaletteItem
//

ref class ToolPaletteItem : public System::Object
{
public:
   ToolPaletteItem(System::Object ^ tool, System::String ^ name, int iImage);
   ~ToolPaletteItem();

   virtual bool Equals(System::Object ^ obj) override;

   property System::Object ^ Tool
   {
      System::Object ^ get()
      {
         return m_tool;
      }
   }

   property System::String ^ Name
   {
      System::String ^ get()
      {
         return m_name;
      }
   }

   property int ImageIndex
   {
      int get()
      {
         return m_iImage;
      }
   }

   property bool Enabled
   {
      bool get()
      {
         return m_bEnabled;
      }

      void set(bool bEnabled)
      {
         m_bEnabled = bEnabled;
      }
   }

private:
   System::Object ^ m_tool;
   System::String ^ m_name;
   int m_iImage;
   bool m_bEnabled;
};


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: ToolPaletteGroup
//

ref class ToolPaletteGroup sealed : public System::Object
{
public:
   ToolPaletteGroup(System::String ^ name, System::Windows::Forms::ImageList ^ imageList);
   ~ToolPaletteGroup();

   property System::String ^ Name
   {
      System::String ^ get()
      {
         return m_name;
      }
   }

   property System::Windows::Forms::ImageList ^ ImageList
   {
      System::Windows::Forms::ImageList ^ get()
      {
         return m_imageList;
      }
   }

   property bool Collapsed
   {
      bool get()
      {
         return m_bCollapsed;
      }

      void set(bool bCollapsed)
      {
         m_bCollapsed = bCollapsed;
      }
   }

   property System::Collections::IList ^ Items
   {
      System::Collections::IList ^ get()
      {
         return m_items;
      }
   }

   ToolPaletteItem ^ FindItem(const System::String ^ toolName);

private:
   System::String ^ m_name;
   System::Windows::Forms::ImageList ^ m_imageList;
   System::Collections::ArrayList ^ m_items;
   bool m_bCollapsed;
};


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: ToolSelectEventArgs
//

ref class ToolSelectEventArgs : public System::EventArgs
{
public:
   ToolSelectEventArgs(ToolPaletteItem ^ oldTool, ToolPaletteItem ^ newTool);

   property ToolPaletteItem ^ OldItem
   {
      ToolPaletteItem ^ get()
      {
         return m_oldItem;
      }
   }

   property ToolPaletteItem ^ NewItem
   {
      ToolPaletteItem ^ get()
      {
         return m_newItem;
      }
   }

private:
   ToolPaletteItem ^ m_oldItem;
   ToolPaletteItem ^ m_newItem;
};


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: ToolPalette
//

ref class ToolPalette : System::Windows::Forms::Control
{
public:
   ToolPalette();
   ~ToolPalette();

   property System::Collections::IList ^ Groups
   {
      System::Collections::IList ^ get()
      {
         return m_groups;
      }
   }

   property ToolPaletteItem ^ CurrentItem
   {
      ToolPaletteItem ^ get()
      {
         return m_currentItem;
      }
   }

   property System::Object ^ CurrentTool
   {
      System::Object ^ get()
      {
         return m_currentItem ? m_currentItem->Tool : nullptr;
      }
   }

   property int ItemHeight
   {
      int get();

      void set(int itemHeight)
      {
         m_itemHeight = itemHeight;
      }
   }

   delegate void ToolSelectHandler(System::Object ^ sender, ToolSelectEventArgs ^ e);
   event ToolSelectHandler ^ ToolSelect;

   ToolPaletteGroup ^ AddGroup(System::String ^ groupName, System::Windows::Forms::ImageList ^ imageList);
   ToolPaletteGroup ^ FindGroup(System::String ^ groupName);

   ToolPaletteItem ^ AddTool(ToolPaletteGroup ^ group, System::Object ^ tool, System::String ^ toolName, int iImage);

protected:
   virtual void OnMouseDown(System::Windows::Forms::MouseEventArgs ^ e) override;
   virtual void OnMouseLeave(System::EventArgs ^ e) override;
   virtual void OnMouseMove(System::Windows::Forms::MouseEventArgs ^ e) override;
   virtual void OnMouseUp(System::Windows::Forms::MouseEventArgs ^ e) override;
   virtual void OnPaint(System::Windows::Forms::PaintEventArgs ^ e) override;

private:
   void PaintGroupHeading(ToolPaletteGroup ^ group, System::Drawing::Graphics ^ graphics, System::Drawing::Rectangle ^ itemRect);
   void PaintToolItem(ToolPaletteItem ^ item, System::Drawing::Graphics ^ graphics, System::Drawing::Rectangle ^ itemRect);
   void PaintSelectedItem(ToolPaletteItem ^ item, System::Drawing::Graphics ^ graphics, System::Drawing::Rectangle ^ itemRect);

   System::Object ^ GetHitItem(System::Windows::Forms::MouseEventArgs ^ e);

   void SetMouseOverItem(System::Object ^ item);

   System::Collections::ArrayList ^ m_groups;

   ToolPaletteItem ^ m_currentItem;

   int m_itemHeight;

   System::Object ^ m_mouseOverItem;
   System::Object ^ m_clickCandidateItem;
};


} // namespace ManagedEditor

/////////////////////////////////////////////////////////////////////////////

#endif // !defined(INCLUDED_TOOLPALETTE_H)
