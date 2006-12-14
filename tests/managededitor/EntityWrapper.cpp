///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "EntityWrapper.h"

#include "engine/entityapi.h"


namespace ManagedEditor
{
   using namespace System::ComponentModel;

   ///////////////////////////////////////////////////////////////////////////////
   //
   // CLASS: EntityWrapper
   //

   EntityWrapper::EntityWrapper(IEntity * pEntity)
    : m_pEntity(CTAddRef(pEntity))
   {
   }

   EntityWrapper::~EntityWrapper()
   {
   }

   EntityWrapper::!EntityWrapper()
   {
      if (m_pEntity)
      {
         m_pEntity->Release();
         m_pEntity = NULL;
      }
   }

   AttributeCollection ^ EntityWrapper::GetAttributes()
   {
      return nullptr;
   }

   System::String ^ EntityWrapper::GetClassName()
   {
      return "Entity";
   }

   System::String ^ EntityWrapper::GetComponentName()
   {
      if (m_pEntity)
      {
         return System::String::Format("Entity {0}", m_pEntity->GetId());
      }

      return nullptr;
   }

   TypeConverter ^ EntityWrapper::GetConverter()
   {
      return nullptr;
   }

   EventDescriptor ^ EntityWrapper::GetDefaultEvent()
   {
      return nullptr;
   }

   PropertyDescriptor ^ EntityWrapper::GetDefaultProperty()
   {
      return nullptr;
   }

   System::Object ^ EntityWrapper::GetEditor(System::Type ^ editorBaseType)
   {
      return nullptr;
   }

   EventDescriptorCollection ^ EntityWrapper::GetEvents()
   {
      return nullptr;
   }

   EventDescriptorCollection ^ EntityWrapper::GetEvents(array<System::Attribute ^> ^ attributes)
   {
      return nullptr;
   }

   PropertyDescriptorCollection ^ EntityWrapper::GetProperties()
   {
      return gcnew PropertyDescriptorCollection(gcnew array<PropertyDescriptor ^>
      {
         gcnew EntityPropertyDescriptor("Type", nullptr, EntityWrapper::typeid, true, System::String::typeid),
         gcnew EntityPropertyDescriptor("ID", nullptr, EntityWrapper::typeid, true, System::Int32::typeid),
      });
   }

   PropertyDescriptorCollection ^ EntityWrapper::GetProperties(array<System::Attribute ^> ^ attributes)
   {
      return GetProperties();
   }

   System::Object ^ EntityWrapper::GetPropertyOwner(PropertyDescriptor ^ pd)
   {
      return nullptr;
   }

   ///////////////////////////////////////////////////////////////////////////////
   //
   // CLASS: EntityPropertyDescriptor
   //

   EntityPropertyDescriptor::EntityPropertyDescriptor(System::String ^ name,
                                                      array<System::Attribute ^> ^ attributes,
                                                      System::Type ^ componentType,
                                                      bool bReadOnly,
                                                      System::Type ^ propertyType)
    : PropertyDescriptor(name, attributes)
    , m_componentType(componentType)
    , m_bReadOnly(bReadOnly)
    , m_propertyType(propertyType)
   {
   }

   System::Type ^ EntityPropertyDescriptor::ComponentType::get()
   {
      return m_componentType;
   }

   bool EntityPropertyDescriptor::IsReadOnly::get()
   {
      return m_bReadOnly;
   }

   System::Type ^ EntityPropertyDescriptor::PropertyType::get()
   {
      return m_propertyType;
   }

   bool EntityPropertyDescriptor::CanResetValue(System::Object ^ component)
   {
      return false;
   }

   System::Object ^ EntityPropertyDescriptor::GetValue(System::Object ^ component)
   {
      if (component)
      {
      }
      return nullptr;
   }

   void EntityPropertyDescriptor::ResetValue(System::Object ^ component)
   {
   }

   void EntityPropertyDescriptor::SetValue(System::Object ^ component, System::Object ^ value)
   {
   }

   bool EntityPropertyDescriptor::ShouldSerializeValue(System::Object ^ component)
   {
      return false;
   }

} // namespace ManagedEditor
