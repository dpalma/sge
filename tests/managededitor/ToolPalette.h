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
   ToolPaletteItem(ToolPaletteGroup ^ group, System::String ^ name, int iImage);
   ~ToolPaletteItem();

   virtual bool Equals(System::Object ^ obj) override;

   property ToolPaletteGroup ^ Group
   {
      ToolPaletteGroup ^ get()
      {
         return m_group;
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
   ToolPaletteGroup ^ m_group;
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

   property System::Collections::IList ^ Tools
   {
      System::Collections::IList ^ get()
      {
         return m_tools;
      }
   }

   ToolPaletteItem ^ FindTool(const System::String ^ toolName);

private:
   System::String ^ m_name;
   System::Windows::Forms::ImageList ^ m_imageList;
   System::Collections::ArrayList ^ m_tools;
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

   property ToolPaletteItem ^ FormerTool
   {
      ToolPaletteItem ^ get()
      {
         return m_oldTool;
      }
   }

   property ToolPaletteItem ^ NewTool
   {
      ToolPaletteItem ^ get()
      {
         return m_newTool;
      }
   }

private:
   ToolPaletteItem ^ m_oldTool;
   ToolPaletteItem ^ m_newTool;
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

   property ToolPaletteItem ^ CurrentTool
   {
      ToolPaletteItem ^ get()
      {
         return m_currentTool;
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

   ToolPaletteItem ^ AddTool(ToolPaletteGroup ^ group, System::String ^ toolName, int iImage);

protected:
   virtual void OnMouseDown(System::Windows::Forms::MouseEventArgs ^ e) override;
   virtual void OnMouseLeave(System::EventArgs ^ e) override;
   virtual void OnMouseMove(System::Windows::Forms::MouseEventArgs ^ e) override;
   virtual void OnMouseUp(System::Windows::Forms::MouseEventArgs ^ e) override;
   virtual void OnPaint(System::Windows::Forms::PaintEventArgs ^ e) override;

private:
   void PaintGroupHeading(ToolPaletteGroup ^ group, System::Drawing::Graphics ^ graphics, System::Drawing::Rectangle ^ itemRect);
   void PaintToolItem(ToolPaletteItem ^ tool, System::Drawing::Graphics ^ graphics, System::Drawing::Rectangle ^ itemRect);
   void PaintSelectedTool(ToolPaletteItem ^ tool, System::Drawing::Graphics ^ graphics, System::Drawing::Rectangle ^ itemRect);

   System::Object ^ GetHitItem(System::Windows::Forms::MouseEventArgs ^ e);

   void SetMouseOverItem(System::Object ^ hItem);

   System::Collections::ArrayList ^ m_groups;

   ToolPaletteItem ^ m_currentTool;

   int m_itemHeight;

   System::Object ^ m_mouseOverItem;
   System::Object ^ m_clickCandidateItem;
};


} // namespace ManagedEditor

/////////////////////////////////////////////////////////////////////////////

#endif // !defined(INCLUDED_TOOLPALETTE_H)
