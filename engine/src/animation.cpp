///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "animation.h"
#include "comtools.h"

#include <vector>
#include <cfloat>
#include <algorithm>

#include "dbgalloc.h" // must be last header

LOG_DEFINE_CHANNEL(Animation);

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cKeyFrameAnimation
//

class cKeyFrameAnimation : public cComObject<IMPLEMENTS(IKeyFrameAnimation)>
{
   cKeyFrameAnimation(const cKeyFrameAnimation &); // private, un-implemented
   const cKeyFrameAnimation & operator =(const cKeyFrameAnimation &); // private, un-implemented

public:
   cKeyFrameAnimation();
   ~cKeyFrameAnimation();

   bool Create(IKeyFrameInterpolator * * ppInterpolators, uint nInterpolators);

   virtual tTime GetPeriod() const;

   virtual tResult GetInterpolator(int index, IKeyFrameInterpolator * * ppInterpolator) const;

private:
   tTime m_period;

   typedef std::vector<IKeyFrameInterpolator *> tInterpolators;
   tInterpolators m_interpolators;
};

///////////////////////////////////////

cKeyFrameAnimation::cKeyFrameAnimation()
 : m_period(0)
{
}

///////////////////////////////////////

cKeyFrameAnimation::~cKeyFrameAnimation()
{
   std::for_each(m_interpolators.begin(), m_interpolators.end(), CTInterfaceMethod(&IUnknown::Release));
   m_interpolators.clear();
}

///////////////////////////////////////

bool cKeyFrameAnimation::Create(IKeyFrameInterpolator * * ppInterpolators, uint nInterpolators)
{
   Assert(m_interpolators.empty());

   if ((ppInterpolators != NULL) && (nInterpolators > 0))
   {
      m_period = 0;

      m_interpolators.resize(nInterpolators);
      for (uint i = 0; i < nInterpolators; i++)
      {
         m_interpolators[i] = ppInterpolators[i];
         m_interpolators[i]->AddRef();

         if (m_interpolators[i]->GetPeriod() > m_period)
         {
            m_period = m_interpolators[i]->GetPeriod();
         }
      }

      return true;
   }

   return false;
}

///////////////////////////////////////

tTime cKeyFrameAnimation::GetPeriod() const
{
   return m_period;
}

///////////////////////////////////////

tResult cKeyFrameAnimation::GetInterpolator(int index, IKeyFrameInterpolator * * ppInterpolator) const
{
   if (index >= 0 && index < m_interpolators.size() && ppInterpolator != NULL)
   {
      *ppInterpolator = m_interpolators[index];
      (*ppInterpolator)->AddRef();
      return S_OK;
   }
   return E_FAIL;
}

///////////////////////////////////////

tResult KeyFrameAnimationCreate(IKeyFrameInterpolator * * ppInterpolators, uint nInterpolators,
                                IKeyFrameAnimation * * ppAnimation)
{
   if (ppAnimation != NULL && ppInterpolators != NULL && nInterpolators > 0)
   {
      cKeyFrameAnimation * pKeyFrameAnimation = new cKeyFrameAnimation;
      if (!pKeyFrameAnimation->Create(ppInterpolators, nInterpolators))
      {
         delete pKeyFrameAnimation;
         pKeyFrameAnimation = NULL;
      }
      *ppAnimation = static_cast<IKeyFrameAnimation *>(pKeyFrameAnimation);
      if ((*ppAnimation) != NULL)
      {
         return S_OK;
      }
   }
   return E_FAIL;
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cKeyFrameInterpolator
//

class cKeyFrameInterpolator : public cComObject<IMPLEMENTS(IKeyFrameInterpolator)>
{
public:
   cKeyFrameInterpolator(const char * pszName, const sKeyFrame * pKeyFrames, uint nKeyFrames);

   virtual tTime GetPeriod() const;
   virtual tResult GetKeyFrames(sKeyFrame * pKeyFrames, uint * pnKeyFrames) const;
   virtual tResult Interpolate(tTime time, sKeyFrame * pInterpFrame) const;

private:
   typedef std::vector<sKeyFrame> tKeyFrames;

   char m_szName[100];
   tTime m_period;
   tKeyFrames m_keyFrames;
};

///////////////////////////////////////

cKeyFrameInterpolator::cKeyFrameInterpolator(const char * pszName,
                                             const sKeyFrame * pKeyFrames, 
                                             uint nKeyFrames)
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

   if (pKeyFrames != NULL && nKeyFrames > 0)
   {
      m_keyFrames.resize(nKeyFrames);
      memcpy(&m_keyFrames[0], pKeyFrames, sizeof(sKeyFrame) * nKeyFrames);
   }

   m_period = FLT_MIN;
   for (uint i = 0; i < nKeyFrames; i++)
   {
      if (pKeyFrames[i].time > m_period)
         m_period = pKeyFrames[i].time;
   }
}

///////////////////////////////////////

tTime cKeyFrameInterpolator::GetPeriod() const
{
   return m_period;
}

///////////////////////////////////////

tResult cKeyFrameInterpolator::GetKeyFrames(sKeyFrame * pKeyFrames, uint * pnKeyFrames) const
{
   if (pKeyFrames != NULL && pnKeyFrames != NULL)
   {
      uint nKeyFramesToCopy = Min(m_keyFrames.size(), *pnKeyFrames);
      memcpy(pKeyFrames, &m_keyFrames[0], sizeof(sKeyFrame) * nKeyFramesToCopy);
      *pnKeyFrames = nKeyFramesToCopy;
      return (nKeyFramesToCopy < m_keyFrames.size()) ? S_FALSE : S_OK;
   }
   else if (pKeyFrames == NULL && pnKeyFrames != NULL)
   {
      *pnKeyFrames = m_keyFrames.size();
      return S_OK;
   }
   return E_FAIL;
}

///////////////////////////////////////

tResult cKeyFrameInterpolator::Interpolate(tTime time, sKeyFrame * pInterpFrame) const
{
   if (pInterpFrame != NULL)
   {
      tKeyFrames::const_iterator iter, prev;
      for (prev = iter = m_keyFrames.begin(); iter != m_keyFrames.end(); prev = iter, iter++)
      {
         if (iter->time >= time)
         {
            if (iter == prev)
            {
               pInterpFrame->rotation = iter->rotation;
               pInterpFrame->translation = iter->translation;
            }
            else
            {
               tTime u = (time - prev->time) / (iter->time - prev->time);
               DebugMsgEx4(Animation, "time %.2f prev %.2f, iter %.2f, u %.2f\n", time, prev->time, iter->time, u);
               pInterpFrame->rotation = QuatSlerp(prev->rotation, iter->rotation, u);
               pInterpFrame->translation = Vec3Lerp(prev->translation, iter->translation, (tVec3::value_type)u);
            }

            return S_OK;
         }
      }
   }

   return E_FAIL;
}

///////////////////////////////////////

tResult KeyFrameInterpolatorCreate(const char * pszName,
                                   const sKeyFrame * pKeyFrames, uint nKeyFrames,
                                   IKeyFrameInterpolator * * ppInterpolator)
{
   if (ppInterpolator == NULL)
      return E_INVALIDARG;

   *ppInterpolator = new cKeyFrameInterpolator(pszName, pKeyFrames, nKeyFrames);

   if ((*ppInterpolator) == NULL)
      return E_FAIL;

   return S_OK;
}

///////////////////////////////////////////////////////////////////////////////
