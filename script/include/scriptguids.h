///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_SCRIPTGUIDS_H
#define INCLUDED_SCRIPTGUIDS_H

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////

// {7EABF652-F7D6-42c1-8C5F-AE8564FCC6C6}
DEFINE_GUID(IID_IScriptable,
0x7eabf652, 0xf7d6, 0x42c1, 0x8c, 0x5f, 0xae, 0x85, 0x64, 0xfc, 0xc6, 0xc6);

// {C9F12147-DA9E-45e4-B582-BBA407427403}
DEFINE_GUID(IID_IScriptableFactory, 
0xc9f12147, 0xda9e, 0x45e4, 0xb5, 0x82, 0xbb, 0xa4, 0x7, 0x42, 0x74, 0x3);

// {55819C5F-DFA2-4988-B49D-B772C2DD7532}
DEFINE_GUID(IID_IScriptInterpreter, 
0x55819c5f, 0xdfa2, 0x4988, 0xb4, 0x9d, 0xb7, 0x72, 0xc2, 0xdd, 0x75, 0x32);

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_SCRIPTGUIDS_H
