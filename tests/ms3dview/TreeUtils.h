/////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_TREEUTILS_H
#define INCLUDED_TREEUTILS_H

#ifdef _MSC_VER
#pragma once
#endif

struct sVertexElement;
interface IVertexBuffer;

/////////////////////////////////////////////////////////////////////////////

CString VertexUsageToString(byte usage);
CString VertexDataTypeToString(byte type);
void DescribeVertexElement(const sVertexElement & element, CString * pStr);
void DescribeVertexData(const sVertexElement & element, const byte * pData, CString * pStr);
void AddVertices(uint nVertices, IVertexBuffer * pVertexBuffer, CTreeCtrl * pTreeCtrl, HTREEITEM hParent);

/////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_TREEUTILS_H
