///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_SCENECAMERA_H
#define INCLUDED_SCENECAMERA_H

#include "sceneapi.h"
#include "frustum.h"
#include "matrix4.h"

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cSceneCamera
//

class cSceneCamera : public cComObject<IMPLEMENTS(ISceneCamera)>
{
   cSceneCamera(const cSceneCamera &);
   const cSceneCamera & operator =(const cSceneCamera &);

public:
   cSceneCamera();
   ~cSceneCamera();

   void SetPerspective(float fov, float aspect, float znear, float zfar);
   void SetOrtho(float left, float right, float bottom, float top, float znear, float zfar);

   const tMatrix4 & GetViewMatrix() const;
   void SetViewMatrix(const tMatrix4 & view);

   const tMatrix4 & GetProjectionMatrix() const;
   void SetProjectionMatrix(const tMatrix4 & projection);

   const tMatrix4 & GetViewProjectionMatrix() const;
   const tMatrix4 & GetViewProjectionInverseMatrix() const;

private:
   void UpdateCompositeMatrices();

   tMatrix4 m_projection;
   tMatrix4 m_view;

   tMatrix4 m_viewProjection;
   tMatrix4 m_viewProjectionInverse;

   cFrustum m_frustum;
};

///////////////////////////////////////

inline const tMatrix4 & cSceneCamera::GetViewMatrix() const
{
   return m_view;
}

///////////////////////////////////////

inline void cSceneCamera::SetViewMatrix(const tMatrix4 & view)
{
   m_view = view;
   UpdateCompositeMatrices();
}

///////////////////////////////////////

inline const tMatrix4 & cSceneCamera::GetProjectionMatrix() const
{
   return m_projection;
}

///////////////////////////////////////

inline void cSceneCamera::SetProjectionMatrix(const tMatrix4 & projection)
{
   m_projection = projection;
   UpdateCompositeMatrices();
}

///////////////////////////////////////

inline const tMatrix4 & cSceneCamera::GetViewProjectionMatrix() const
{
   return m_viewProjection;
}

///////////////////////////////////////

inline const tMatrix4 & cSceneCamera::GetViewProjectionInverseMatrix() const
{
   return m_viewProjectionInverse;
}

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_SCENECAMERA_H
