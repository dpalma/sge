///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "stripify.h"

#include <algorithm>

#include "dbgalloc.h" // must be last header

LOG_DEFINE_CHANNEL(Stripify);

// REFERENCES
// http://www.cs.queensu.ca/home/jstewart/strips/algorithm/
// http://www.codercorner.com/Strips.htm

///////////////////////////////////////////////////////////////////////////////

struct sAdjTri
{
   int verts[3]; // vertices that make up triangle
   int tris[3]; // neighboring triangles

   int WhichEdge(int v0, int v1) const;
   int OppositeVertex(int v0, int v1) const;
   int NeighborCount() const;
};

///////////////////////////////////////

inline int sAdjTri::WhichEdge(int v0, int v1) const
{
   if ((v0 == verts[0] && v1 == verts[1]) || (v0 == verts[1] && v1 == verts[0]))
      return 0;
   else if ((v0 == verts[0] && v1 == verts[2]) || (v0 == verts[2] && v1 == verts[0]))
      return 1;
   else if ((v0 == verts[1] && v1 == verts[2]) || (v0 == verts[2] && v1 == verts[1]))
      return 2;
   else
      return -1;
}

///////////////////////////////////////

inline int sAdjTri::OppositeVertex(int v0, int v1) const
{
   if ((v0 == verts[0] && v1 == verts[1]) || (v0 == verts[1] && v1 == verts[0]))
      return verts[2];
   else if ((v0 == verts[0] && v1 == verts[2]) || (v0 == verts[2] && v1 == verts[0]))
      return verts[1];
   else if ((v0 == verts[1] && v1 == verts[2]) || (v0 == verts[2] && v1 == verts[1]))
      return verts[0];
   else
      return -1;
}

///////////////////////////////////////

inline int sAdjTri::NeighborCount() const
{
   int nNeighbors = 0;
   if (tris[0] != -1)
      nNeighbors++;
   if (tris[1] != -1)
      nNeighbors++;
   if (tris[2] != -1)
      nNeighbors++;
   return nNeighbors;
}

///////////////////////////////////////////////////////////////////////////////

static void ComputeStrip(int face, int v0, int v1,
                         const std::vector<sAdjTri> & adjTris,
                         std::vector<bool> * pAdjTrisDone,
                         sStripInfo * pStripInfo)
{
   Assert(pStripInfo != NULL);

   pStripInfo->strip.push_back(v0);
   pStripInfo->strip.push_back(v1);

   bool bDone = false;
   while (!bDone)
   {
      int v2 = adjTris[face].OppositeVertex(v0, v1);
      Assert(v2 != -1);

      pStripInfo->strip.push_back(v2);
      pStripInfo->faces.push_back(face);
      (*pAdjTrisDone)[face] = true;

      int edge = adjTris[face].WhichEdge(v1, v2);
      Assert(edge != -1);

      int next = adjTris[face].tris[edge];

      if (next == -1)
      {
         bDone = true;
      }
      else
      {
         DebugMsgEx3(Stripify, "Triangle %d leads to %d through edge %d\n", face, next, edge);
         face = next;

         if ((*pAdjTrisDone)[face])
         {
            bDone = true;
         }
      }

      v0 = v1;
      v1 = v2;
   }
}

///////////////////////////////////////////////////////////////////////////////

struct sAdjEdge
{
   int v0, v1; // vertices that make up the edge
   int tri; // triangle that owns this edge
};

///////////////////////////////////////
// sort on 3 keys: v0, then v1, then face index

struct sLessAdjEdge
{
   bool operator()(const sAdjEdge & e1, const sAdjEdge & e2) const;
};

bool sLessAdjEdge::operator()(const sAdjEdge & e1, const sAdjEdge & e2) const
{
   Assert(e1.v0 < e1.v1);
   Assert(e2.v0 < e2.v1);
   if (e1.v0 < e2.v0)
      return true;
   else if (e1.v1 < e2.v1)
      return true;
   else if (e1.tri < e2.tri)
      return true;
   return false;
}

///////////////////////////////////////////////////////////////////////////////

static bool ComputeAdjacencyInfo(const int * pFaces, int nFaces,
                                 std::vector<sAdjTri> * pAdjTris)
{
   Assert(pFaces != NULL && nFaces > 0);
   Assert(pAdjTris != NULL);

   std::vector<sAdjEdge> adjEdges;

   for (int i = 0; i < nFaces; i++)
   {
      int v0 = pFaces[i * 3 + 0];
      int v1 = pFaces[i * 3 + 1];
      int v2 = pFaces[i * 3 + 2];

      //DebugMsgEx4(Stripify, "Triangle %d: %d, %d, %d\n", i, v0, v1, v2);

      sAdjTri adjTri;
      adjTri.verts[0] = v0;
      adjTri.verts[1] = v1;
      adjTri.verts[2] = v2;
      adjTri.tris[0] = -1;
      adjTri.tris[1] = -1;
      adjTri.tris[2] = -1;
      pAdjTris->push_back(adjTri);

      sAdjEdge adjEdge;
      adjEdge.tri = i;

      adjEdge.v0 = Min(v0,v1);
      adjEdge.v1 = Max(v0,v1);
      adjEdges.push_back(adjEdge);

      adjEdge.v0 = Min(v0,v2);
      adjEdge.v1 = Max(v0,v2);
      adjEdges.push_back(adjEdge);

      adjEdge.v0 = Min(v1,v2);
      adjEdge.v1 = Max(v1,v2);
      adjEdges.push_back(adjEdge);
   }

   std::sort(adjEdges.begin(), adjEdges.end(), sLessAdjEdge());

   int count, faces[3], lastv0, lastv1;

   lastv0 = adjEdges[0].v0;
   lastv1 = adjEdges[0].v1;
   count = 0;

   std::vector<sAdjEdge>::iterator iter;
   for (iter = adjEdges.begin(); iter != adjEdges.end(); iter++)
   {
      if (iter->v0 == lastv0 && iter->v1 == lastv1)
      {
         faces[count++] = iter->tri;
         if (count == 3)
         {
            // don't support meshes where an edge can be shared by more than 2 triangles
            pAdjTris->clear();
            return false;
         }
      }
      else
      {
         if (count == 2)
         {
            DebugMsgEx4(Stripify, "Triangles %d, %d share edge %d -> %d\n",
               faces[0], faces[1], lastv0, lastv1);

            int edgeIndex0 = (*pAdjTris)[faces[0]].WhichEdge(lastv0, lastv1);
            int edgeIndex1 = (*pAdjTris)[faces[1]].WhichEdge(lastv0, lastv1);

            if (edgeIndex0 < 0 || edgeIndex1 < 0)
            {
               // each triange should have the edge since it's purported
               // to be shared between the two
               pAdjTris->clear();
               return false;
            }

            (*pAdjTris)[faces[0]].tris[edgeIndex0] = faces[1];
            (*pAdjTris)[faces[1]].tris[edgeIndex1] = faces[0];
         }

         count = 0;
         faces[count++] = iter->tri;
         lastv0 = iter->v0;
         lastv1 = iter->v1;
      }
   }

   return true;
}

///////////////////////////////////////////////////////////////////////////////

void ComputeStrips(const int * pFaces, int nFaces,
                   std::vector<sStripInfo> * pAllStrips)
{
   Assert(pAllStrips != NULL);

   std::vector<sAdjTri> adjTris;
   if (ComputeAdjacencyInfo(pFaces, nFaces, &adjTris))
   {
      int current = 0;

      std::vector<bool> adjTrisDone;
      adjTrisDone.resize(adjTris.size(), false);

      while (current < adjTris.size())
      {
         while (adjTrisDone[current] && current < adjTris.size())
         {
            current++;
         }

         if (current >= adjTris.size())
            break;

         sStripInfo stripInfo[3];
         int firstPartLen[3];

         std::vector<bool> stripTrisDone;
         stripTrisDone.resize(adjTris.size());

         std::pair<int, int> vertRefs[3];
         vertRefs[0] = std::make_pair(0,1);
         vertRefs[1] = std::make_pair(0,2);
         vertRefs[2] = std::make_pair(1,2);

         for (int i = 0; i < 3; i++)
         {
            std::copy(adjTrisDone.begin(), adjTrisDone.end(), stripTrisDone.begin());

            // compute the first part of the strip, off in one direction
            ComputeStrip(
               current,
               adjTris[current].verts[vertRefs[i].first],
               adjTris[current].verts[vertRefs[i].second],
               adjTris,
               &stripTrisDone,
               &stripInfo[i]);

            firstPartLen[i] = stripInfo[i].strip.size();

            Assert(stripInfo[i].faces.size() == stripInfo[i].strip.size() - 2);

            int nextV0 = stripInfo[i].strip[2];
            int nextV1 = stripInfo[i].strip[1];

            // reverse the first part of the strip
            int j, length = stripInfo[i].strip.size();
            for (j = 0; j < (length / 2); j++)
            {
               std::swap(stripInfo[i].strip[j], stripInfo[i].strip[length - j - 1]);
            }

            // ...and the faces
            length = stripInfo[i].faces.size();
            for (j = 0; j < (length / 2); j++)
            {
               std::swap(stripInfo[i].faces[j], stripInfo[i].faces[length - j - 1]);
            }

            Assert(stripInfo[i].strip.size() >= 3);
            stripInfo[i].strip.resize(stripInfo[i].strip.size() - 3);

            if (stripInfo[i].faces.size() >= 3)
               stripInfo[i].faces.resize(stripInfo[i].faces.size() - 3);

            // compute the second part of the strip, off in another direction
            // from the same starting triangle
            ComputeStrip(
               current,
               nextV0,
               nextV1,
               adjTris,
               &stripTrisDone,
               &stripInfo[i]);
         }

         sStripInfo * pLongest = &stripInfo[0];
         if (stripInfo[1].strip.size() > pLongest->strip.size())
            pLongest = &stripInfo[1];
         if (stripInfo[2].strip.size() > pLongest->strip.size())
            pLongest = &stripInfo[2];

         tIntArray::iterator iter;
         for (iter = pLongest->faces.begin(); iter != pLongest->faces.end(); iter++)
         {
            adjTrisDone[*iter] = true;
         }

         // if the length of the first part of the strip is odd, the strip must be reversed
         if (firstPartLen[pLongest - stripInfo] & 1)
         {
            int length = pLongest->strip.size();
            for (int j = 0; j < (length / 2); j++)
            {
               std::swap(pLongest->strip[j], pLongest->strip[length - j - 1]);
            }

            int firstFaceNewPos = length - firstPartLen[pLongest - stripInfo];

            // if the position of the original face in the reversed strip is odd
            // then replicate the first vertex to preserve the original winding
            if (firstFaceNewPos & 1)
            {
               pLongest->strip.insert(pLongest->strip.begin(), pLongest->strip[0]);
            }
         }

         pAllStrips->push_back(*pLongest);
      }

#ifndef NDEBUG
      int maxStripLen = 0, sum = 0;
      std::vector<sStripInfo>::iterator iter;
      for (iter = pAllStrips->begin(); iter != pAllStrips->end(); iter++)
      {
         maxStripLen = Max(maxStripLen, iter->strip.size());
         sum += iter->strip.size();
      }
      DebugMsgEx3(Stripify, "%d Strips; Average strip length = %d; Max strip length = %d\n",
         pAllStrips->size(), sum / pAllStrips->size(), maxStripLen);
#endif
   }
}

///////////////////////////////////////////////////////////////////////////////

void CombineStrips(const std::vector<sStripInfo> & allStrips,
                   std::vector<int> * pStrip)
{
   std::vector<sStripInfo>::const_iterator iter;
   for (iter = allStrips.begin(); iter != allStrips.end(); iter++)
   {
      if (!pStrip->empty() && (pStrip->back() != iter->strip.front()))
      {
         pStrip->push_back(pStrip->back());
         pStrip->push_back(iter->strip.front());
      }

      if (pStrip->size() & 1)
      {
         pStrip->push_back(iter->strip.front());
      }

      pStrip->insert(pStrip->end(), iter->strip.begin(), iter->strip.end());
   }
}

///////////////////////////////////////////////////////////////////////////////
