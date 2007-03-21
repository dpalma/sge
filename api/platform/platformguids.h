///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_PLATFORMGUIDS_H
#define INCLUDED_PLATFORMGUIDS_H

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////

// {49698F40-3EDB-44d9-9906-3D3514061897}
DEFINE_GUID(IID_IInput, 
0x49698f40, 0x3edb, 0x44d9, 0x99, 0x6, 0x3d, 0x35, 0x14, 0x6, 0x18, 0x97);

// {A12C2385-265E-49b6-A421-361224CD4F5D}
DEFINE_GUID(IID_IInputListener, 
0xa12c2385, 0x265e, 0x49b6, 0xa4, 0x21, 0x36, 0x12, 0x24, 0xcd, 0x4f, 0x5d);

// {4B0BADD8-B0F1-4aac-9B1B-71D1B5374006}
DEFINE_GUID(IID_IInputKeyBindTarget, 
0x4b0badd8, 0xb0f1, 0x4aac, 0x9b, 0x1b, 0x71, 0xd1, 0xb5, 0x37, 0x40, 0x6);

// {278B57D5-877F-4756-B46C-F95B79A683B2}
DEFINE_GUID(IID_IInputModalListener, 
0x278b57d5, 0x877f, 0x4756, 0xb4, 0x6c, 0xf9, 0x5b, 0x79, 0xa6, 0x83, 0xb2);

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_PLATFORMGUIDS_H
