///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_ENTITYWRAPPER_H
#define INCLUDED_ENTITYWRAPPER_H

#include "tech/comtools.h"

#ifdef _MSC_VER
#pragma once
#endif

F_DECLARE_INTERFACE(IEntity);

namespace ManagedEditor
{

   ///////////////////////////////////////////////////////////////////////////////
   //
   // CLASS: EntityWrapper
   //

   ref class EntityWrapper sealed : public System::ComponentModel::ICustomTypeDescriptor
   {
   public:
      EntityWrapper(IEntity * pEntity);
      ~EntityWrapper();
      !EntityWrapper();

      IEntity * AccessEntity() { return m_pEntity; }

      virtual System::ComponentModel::AttributeCollection ^ GetAttributes();
      virtual System::String ^ GetClassName();
      virtual System::String ^ GetComponentName();
      virtual System::ComponentModel::TypeConverter ^ GetConverter();
      virtual System::ComponentModel::EventDescriptor ^ GetDefaultEvent();
      virtual System::ComponentModel::PropertyDescriptor ^ GetDefaultProperty();
      virtual System::Object ^ GetEditor(System::Type ^ editorBaseType);
      virtual System::ComponentModel::EventDescriptorCollection ^ GetEvents();
      virtual System::ComponentModel::EventDescriptorCollection ^ GetEvents(array<System::Attribute ^> ^ attributes);
      virtual System::ComponentModel::PropertyDescriptorCollection ^ GetProperties();
      virtual System::ComponentModel::PropertyDescriptorCollection ^ GetProperties(array<System::Attribute ^> ^ attributes);
      virtual System::Object ^ GetPropertyOwner(System::ComponentModel::PropertyDescriptor ^ pd);

   private:
      IEntity * m_pEntity;
      System::ComponentModel::PropertyDescriptorCollection ^ m_properties;
   };

   ///////////////////////////////////////////////////////////////////////////////
   //
   // CLASS: EntityPropertyDescriptor
   //

   private ref class EntityPropertyDescriptor sealed : public System::ComponentModel::PropertyDescriptor
   {
   public:
      EntityPropertyDescriptor(System::String ^ name, array<System::Attribute ^> ^ attributes, System::Type ^ componentType, bool bReadOnly, System::Type ^ propertyType);

      virtual property System::Type ^ ComponentType { System::Type ^ get() override; }
      virtual property bool IsReadOnly { bool get() override; }
      virtual property System::Type ^ PropertyType { System::Type ^ get() override; }
      virtual bool CanResetValue(System::Object ^ component) override;
      virtual System::Object ^ GetValue(System::Object ^ component) override;
      virtual void ResetValue(System::Object ^ component) override;
      virtual void SetValue(System::Object ^ component, System::Object ^ value) override;
      virtual bool ShouldSerializeValue(System::Object ^ component) override;

   private:
      System::Type ^ m_componentType;
      bool m_bReadOnly;
      System::Type ^ m_propertyType;
   };

} // namespace ManagedEditor

#endif // INCLUDED_ENTITYWRAPPER_H
