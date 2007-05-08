///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_RENDERGUIDS_H
#define INCLUDED_RENDERGUIDS_H

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////

// {B1F78800-CEC7-4df4-9AF6-25D8AC296EC2}
DEFINE_GUID(IID_IRenderer, 
0xb1f78800, 0xcec7, 0x4df4, 0x9a, 0xf6, 0x25, 0xd8, 0xac, 0x29, 0x6e, 0xc2);

// {B962C170-8A3A-409e-8031-04CD3E7C44AD}
DEFINE_GUID(IID_IRenderTarget, 
0xb962c170, 0x8a3a, 0x409e, 0x80, 0x31, 0x4, 0xcd, 0x3e, 0x7c, 0x44, 0xad);

// {D82266F8-FD93-49f7-904B-74D7AF37DA24}
DEFINE_GUID(IID_IRenderFont, 
0xd82266f8, 0xfd93, 0x49f7, 0x90, 0x4b, 0x74, 0xd7, 0xaf, 0x37, 0xda, 0x24);

// {747C41BD-488B-4349-95C9-78342D132976}
DEFINE_GUID(IID_IRenderFontFactory, 
0x747c41bd, 0x488b, 0x4349, 0x95, 0xc9, 0x78, 0x34, 0x2d, 0x13, 0x29, 0x76);

// {5065B828-D331-478f-ABA2-8AB171418499}
DEFINE_GUID(IID_IRenderCamera, 
0x5065b828, 0xd331, 0x478f, 0xab, 0xa2, 0x8a, 0xb1, 0x71, 0x41, 0x84, 0x99);

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_RENDERGUIDS_H
