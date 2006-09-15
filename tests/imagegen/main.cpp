///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "color.h"
#include "filespec.h"
#include "imageapi.h"
#include "readwriteapi.h"
#include "vec3.h"

#include "dbgalloc.h"

struct sLightSource
{
   tVec3 position;
   cColor color;
};

int main(int argc, char *argv[])
{
   uint imageWidth = 256, imageHeight = 256;

   cAutoIPtr<IImage> pImage;
   if (ImageCreate(imageWidth, imageHeight, kPF_RGBA8888, NULL, &pImage) != S_OK)
   {
      return -1;
   }

   cColor diffuse(0,0,0);
   cColor specular(0,0,1);
   float shininess = .85f;

   uint imageCenterX = imageWidth / 2, imageCenterY = imageHeight / 2;

   float sphereRadius = static_cast<float>(Max(imageWidth, imageHeight));
   tVec3 sphereCenter(static_cast<float>(imageCenterX), static_cast<float>(imageCenterY), -sphereRadius);

   uint imageZ = 0, eyeZ = -static_cast<int>(Max(imageCenterX, imageCenterY));

   tVec3 eye(imageCenterX, imageCenterY, imageZ);

   sLightSource lightSource;
   lightSource.position = tVec3(-static_cast<int>(imageWidth), -static_cast<int>(imageHeight), eyeZ);
   lightSource.color = cColor(1,1,1,1);

   for (uint i = 0; i < pImage->GetHeight(); ++i)
   {
      for (uint j = 0; j < pImage->GetWidth(); ++j)
      {
         tVec3 imagePoint(i, j, imageZ);

         tVec3 n(0, 0, 1);
         n = (imagePoint - sphereCenter) / sphereRadius;
         n.Normalize();

         n.x = 0;

         tVec3 l(lightSource.position - imagePoint);
         l.Normalize();
         float nDotL = n.Dot(l);

         tVec3 v(eye - imagePoint);
         v.Normalize();

         // Calculate the half-way vector
         tVec3 h(l + v);
         h.Normalize();

         float nDotH = n.Dot(h);
         float nDotHToShininess = pow(nDotH, shininess);

         cColor color(0, 0, 0);

         color.r += lightSource.color.r * diffuse.r * nDotL;
         color.g += lightSource.color.g * diffuse.g * nDotL;
         color.b += lightSource.color.b * diffuse.b * nDotL;

         color.r += specular.r * nDotHToShininess;
         color.g += specular.g * nDotHToShininess;
         color.b += specular.b * nDotHToShininess;

         if (pImage->SetPixel(j, i, color) != S_OK)
         {
            return -1;
         }
      }
   }

   cAutoIPtr<IWriter> pWriter;
   if (FileWriterCreate(cFileSpec("test.bmp"), &pWriter) != S_OK)
   {
      return -1;
   }

   if (BmpWrite(pImage, pWriter) != S_OK)
   {
      return -1;
   }

   return 0;
}
