///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_CAMERA_H
#define INCLUDED_CAMERA_H

#include "frustum.h"

#include "matrix4.h"
#include "comtools.h"

#ifdef _MSC_VER
#pragma once
#endif

class cSceneNode;
F_DECLARE_INTERFACE(IRenderDevice);


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cCamera
//

class cCamera
{
   cCamera(const cCamera &);
   const cCamera & operator =(const cCamera &);

public:
   cCamera();
   virtual ~cCamera();

   void Render(IRenderDevice * pDevice, cSceneNode * pScene);

   void SetPerspective(float fov, float aspect, float znear, float zfar);
   void SetOrtho(float left, float right, float bottom, float top, float znear, float zfar);

   const sMatrix4 & GetModelViewMatrix() const;
   void SetModelViewMatrix(const sMatrix4 & modelview);

   const sMatrix4 & GetProjectionMatrix() const;
   void SetProjectionMatrix(const sMatrix4 & projection);

   const sMatrix4 & GetModelViewProjectionMatrix() const;
   const sMatrix4 & GetModelViewProjectionInverseMatrix() const;

private:
   void UpdateCompositeMatrices();

   sMatrix4 m_projection;
   sMatrix4 m_modelView;

   sMatrix4 m_modelViewProjection;
   sMatrix4 m_modelViewProjectionInverse;

   cFrustum m_frustum;
};

///////////////////////////////////////

inline const sMatrix4 & cCamera::GetModelViewMatrix() const
{
   return m_modelView;
}

///////////////////////////////////////

inline void cCamera::SetModelViewMatrix(const sMatrix4 & modelView)
{
   m_modelView = modelView;
   UpdateCompositeMatrices();
}

///////////////////////////////////////

inline const sMatrix4 & cCamera::GetProjectionMatrix() const
{
   return m_projection;
}

///////////////////////////////////////

inline void cCamera::SetProjectionMatrix(const sMatrix4 & projection)
{
   m_projection = projection;
   UpdateCompositeMatrices();
}

///////////////////////////////////////

inline const sMatrix4 & cCamera::GetModelViewProjectionMatrix() const
{
   return m_modelViewProjection;
}

///////////////////////////////////////

inline const sMatrix4 & cCamera::GetModelViewProjectionInverseMatrix() const
{
   return m_modelViewProjectionInverse;
}

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_CAMERA_H
