///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "animation.h"
#include "comtools.h"

#include <vector>
#include <cfloat>

#include "dbgalloc.h" // must be last header

LOG_DEFINE_CHANNEL(Animation);

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cKeyFrameInterpolator
//

class cKeyFrameInterpolator : public cComObject<IMPLEMENTS(IKeyFrameInterpolator)>
{
public:
   cKeyFrameInterpolator(const char * pszName,
      const sKeyFrameVec3 * pScaleKeys, uint nScaleKeys,
      const sKeyFrameQuat * pRotationKeys, uint nRotationKeys,
      const sKeyFrameVec3 * pTranslationKeys, uint nTranslationKeys);

   virtual tTime GetPeriod() const;

   virtual tResult GetScaleKeys(sKeyFrameVec3 * pScaleKeys, uint * pnScaleKeys) const;
   virtual tResult GetRotationKeys(sKeyFrameQuat * pRotationKeys, uint * pnRotationKeys) const;
   virtual tResult GetTranslationKeys(sKeyFrameVec3 * pTranslationKeys, uint * pnTranslationKeys) const;

   virtual tResult Interpolate(tTime time, tVec3 * pScale, tQuat * pRotation, tVec3 * pTranslation);

private:
   typedef std::vector<sKeyFrameVec3> tVec3Keys;
   typedef std::vector<sKeyFrameQuat> tQuatKeys;

   char m_szName[100];
   tVec3Keys m_scaleKeys;
   tQuatKeys m_rotationKeys;
   tVec3Keys m_translationKeys;
   tTime m_period;
};

///////////////////////////////////////

cKeyFrameInterpolator::cKeyFrameInterpolator(const char * pszName,
   const sKeyFrameVec3 * pScaleKeys, uint nScaleKeys,
   const sKeyFrameQuat * pRotationKeys, uint nRotationKeys,
   const sKeyFrameVec3 * pTranslationKeys, uint nTranslationKeys)
 : m_period(0)
{
   if (pszName != NULL)
   {
      strncpy(m_szName, pszName, _countof(m_szName));
      m_szName[_countof(m_szName) - 1] = 0;
   }
   else
   {
      memset(m_szName, 0, sizeof(m_szName));
   }

   if (pScaleKeys != NULL && nScaleKeys > 0)
   {
      m_scaleKeys.resize(nScaleKeys);
      memcpy(&m_scaleKeys[0], pScaleKeys, sizeof(sKeyFrameVec3) * nScaleKeys);
   }

   if (pRotationKeys != NULL && nRotationKeys > 0)
   {
      m_rotationKeys.resize(nRotationKeys);
      memcpy(&m_rotationKeys[0], pRotationKeys, sizeof(sKeyFrameQuat) * nRotationKeys);
   }

   if (pTranslationKeys != NULL && nTranslationKeys > 0)
   {
      m_translationKeys.resize(nTranslationKeys);
      memcpy(&m_translationKeys[0], pTranslationKeys, sizeof(sKeyFrameVec3) * nTranslationKeys);
   }

   tTime maxScaleTime = FLT_MIN;
   for (uint i = 0; i < nScaleKeys; i++)
   {
      if (pScaleKeys[i].time > maxScaleTime)
         maxScaleTime = pScaleKeys[i].time;
   }

   tTime maxRotationTime = FLT_MIN;
   for (i = 0; i < nRotationKeys; i++)
   {
      if (pRotationKeys[i].time > maxRotationTime)
         maxRotationTime = pRotationKeys[i].time;
   }

   tTime maxTranslationTime = FLT_MIN;
   for (i = 0; i < nTranslationKeys; i++)
   {
      if (pTranslationKeys[i].time > maxTranslationTime)
         maxTranslationTime = pTranslationKeys[i].time;
   }

   m_period = Max(maxScaleTime, Max(maxRotationTime, maxTranslationTime));
}

///////////////////////////////////////

tTime cKeyFrameInterpolator::GetPeriod() const
{
   return m_period;
}

///////////////////////////////////////

tResult cKeyFrameInterpolator::GetScaleKeys(sKeyFrameVec3 * pScaleKeys, uint * pnScaleKeys) const
{
   if (pScaleKeys != NULL && pnScaleKeys != NULL)
   {
      uint nKeyFramesToCopy = Min(m_scaleKeys.size(), *pnScaleKeys);
      memcpy(pScaleKeys, &m_scaleKeys[0], sizeof(sKeyFrameVec3) * nKeyFramesToCopy);
      *pnScaleKeys = nKeyFramesToCopy;
      return (nKeyFramesToCopy < m_scaleKeys.size()) ? S_FALSE : S_OK;
   }
   else if (pScaleKeys == NULL && pnScaleKeys != NULL)
   {
      *pnScaleKeys = m_scaleKeys.size();
      return S_OK;
   }
   return E_FAIL;
}

///////////////////////////////////////

tResult cKeyFrameInterpolator::GetRotationKeys(sKeyFrameQuat * pRotationKeys, uint * pnRotationKeys) const
{
   if (pRotationKeys != NULL && pnRotationKeys != NULL)
   {
      uint nKeyFramesToCopy = Min(m_rotationKeys.size(), *pnRotationKeys);
      memcpy(pRotationKeys, &m_rotationKeys[0], sizeof(sKeyFrameQuat) * nKeyFramesToCopy);
      *pnRotationKeys = nKeyFramesToCopy;
      return (nKeyFramesToCopy < m_rotationKeys.size()) ? S_FALSE : S_OK;
   }
   else if (pRotationKeys == NULL && pnRotationKeys != NULL)
   {
      *pnRotationKeys = m_rotationKeys.size();
      return S_OK;
   }
   return E_FAIL;
}

///////////////////////////////////////

tResult cKeyFrameInterpolator::GetTranslationKeys(sKeyFrameVec3 * pTranslationKeys, uint * pnTranslationKeys) const
{
   if (pTranslationKeys != NULL && pnTranslationKeys != NULL)
   {
      uint nKeyFramesToCopy = Min(m_translationKeys.size(), *pnTranslationKeys);
      memcpy(pTranslationKeys, &m_translationKeys[0], sizeof(sKeyFrameVec3) * nKeyFramesToCopy);
      *pnTranslationKeys = nKeyFramesToCopy;
      return (nKeyFramesToCopy < m_translationKeys.size()) ? S_FALSE : S_OK;
   }
   else if (pTranslationKeys == NULL && pnTranslationKeys != NULL)
   {
      *pnTranslationKeys = m_scaleKeys.size();
      return S_OK;
   }
   return E_FAIL;
}

///////////////////////////////////////

tResult cKeyFrameInterpolator::Interpolate(tTime time, tVec3 * pScale, tQuat * pRotation, tVec3 * pTranslation)
{
   if (pScale != NULL)
   {
      *pScale = tVec3(1,1,1);

      tVec3Keys::iterator iter, prev;
      for (prev = iter = m_scaleKeys.begin(); iter != m_scaleKeys.end(); prev = iter, iter++)
      {
         if (iter->time >= time)
         {
            if (iter == prev)
            {
               *pScale = iter->value;
            }
            else
            {
               tTime u = (time - prev->time) / (iter->time - prev->time);
               *pScale = Vec3Lerp(prev->value, iter->value, (tVec3::value_type)u);
            }
            break;
         }
      }
   }

   if (pRotation != NULL)
   {
      *pRotation = tQuat(0,0,0,1);

      tQuatKeys::iterator iter, prev;
      for (prev = iter = m_rotationKeys.begin(); iter != m_rotationKeys.end(); prev = iter, iter++)
      {
         if (iter->time >= time)
         {
            if (iter == prev)
            {
               *pRotation = iter->value;
            }
            else
            {
               tTime u = (time - prev->time) / (iter->time - prev->time);
               DebugMsgEx4(Animation, "time %.2f prev %.2f, iter %.2f, u %.2f\n", time, prev->time, iter->time, u);
               *pRotation = QuatSlerp(prev->value, iter->value, u);
            }
            break;
         }
      }
   }

   if (pTranslation != NULL)
   {
      *pTranslation = tVec3(0,0,0);

      tTime prevTime = 0;
      tVec3Keys::iterator iter, prev;
      for (prev = iter = m_translationKeys.begin(); iter != m_translationKeys.end(); prevTime = iter->time, prev = iter, iter++)
      {
         if (iter->time >= time)
         {
            if (iter == prev)
            {
               *pTranslation = iter->value;
            }
            else
            {
               tTime u = (time - prev->time) / (iter->time - prev->time);
               *pTranslation = Vec3Lerp(prev->value, iter->value, (tVec3::value_type)u);
            }
            break;
         }
      }
   }

   return S_OK;
}

///////////////////////////////////////

tResult KeyFrameInterpolatorCreate(const char * pszName,
                                   const sKeyFrameVec3 * pScaleKeys, uint nScaleKeys,
                                   const sKeyFrameQuat * pRotationKeys, uint nRotationKeys,
                                   const sKeyFrameVec3 * pTranslationKeys, uint nTranslationKeys,
                                   IKeyFrameInterpolator * * ppInterpolator)
{
   if (!ppInterpolator)
      return E_INVALIDARG;

   *ppInterpolator = new cKeyFrameInterpolator(pszName,
      pScaleKeys, nScaleKeys,
      pRotationKeys, nRotationKeys,
      pTranslationKeys, nTranslationKeys);

   if (!*ppInterpolator)
      return E_FAIL;

   return S_OK;
}

///////////////////////////////////////////////////////////////////////////////
