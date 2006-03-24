///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_EDITORDOCUMENT_H
#define INCLUDED_EDITORDOCUMENT_H

#ifdef _MSC_VER
#pragma once
#endif

class cTerrainSettings;

namespace ManagedEditor
{


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: EditorDocument
//

ref class EditorDocument sealed
{
public:
   EditorDocument();
   ~EditorDocument();

   void Reset();
   bool New(const cTerrainSettings & ts);
   bool Open(System::String ^ fileName);
   bool Save(System::String ^ fileName);

   property bool Modified;
   property System::String ^ FileName;

private:
};


} // namespace ManagedEditor

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_EDITORDOCUMENT_H
