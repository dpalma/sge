///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_SCENECAMERA_H
#define INCLUDED_SCENECAMERA_H

#include "scenegroup.h"
#include "frustum.h"
#include "matrix4.h"

#ifdef _MSC_VER
#pragma once
#endif

class cSceneNode;

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cSceneCameraGroup
//

class cSceneCameraGroup : public cSceneGroup
{
   cSceneCameraGroup(const cSceneCameraGroup &);
   const cSceneCameraGroup & operator =(const cSceneCameraGroup &);

public:
   cSceneCameraGroup();
   virtual ~cSceneCameraGroup();

   void SetPerspective(float fov, float aspect, float znear, float zfar);
   void SetOrtho(float left, float right, float bottom, float top, float znear, float zfar);

   const sMatrix4 & GetModelViewMatrix() const;
   void SetModelViewMatrix(const sMatrix4 & modelview);

   const sMatrix4 & GetProjectionMatrix() const;
   void SetProjectionMatrix(const sMatrix4 & projection);

   const sMatrix4 & GetModelViewProjectionMatrix() const;
   const sMatrix4 & GetModelViewProjectionInverseMatrix() const;

   virtual void Render();

private:
   void UpdateCompositeMatrices();

   sMatrix4 m_projection;
   sMatrix4 m_modelView;

   sMatrix4 m_modelViewProjection;
   sMatrix4 m_modelViewProjectionInverse;

   cFrustum m_frustum;
};

///////////////////////////////////////

inline const sMatrix4 & cSceneCameraGroup::GetModelViewMatrix() const
{
   return m_modelView;
}

///////////////////////////////////////

inline void cSceneCameraGroup::SetModelViewMatrix(const sMatrix4 & modelView)
{
   m_modelView = modelView;
   UpdateCompositeMatrices();
}

///////////////////////////////////////

inline const sMatrix4 & cSceneCameraGroup::GetProjectionMatrix() const
{
   return m_projection;
}

///////////////////////////////////////

inline void cSceneCameraGroup::SetProjectionMatrix(const sMatrix4 & projection)
{
   m_projection = projection;
   UpdateCompositeMatrices();
}

///////////////////////////////////////

inline const sMatrix4 & cSceneCameraGroup::GetModelViewProjectionMatrix() const
{
   return m_modelViewProjection;
}

///////////////////////////////////////

inline const sMatrix4 & cSceneCameraGroup::GetModelViewProjectionInverseMatrix() const
{
   return m_modelViewProjectionInverse;
}

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_SCENECAMERA_H
