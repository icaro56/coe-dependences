#ifndef BRUSH_H
#define BRUSH_H

//#pragma once

#include "PTPrerequisites.h"
#include <OgrePrerequisites.h>
#include "DllRequisites.h"

class PAGEDTERRAIN_EXPORT Brush
{
public:
    Brush();
    Brush(const Ogre::String& imageName);
    Brush(Ogre::Image& image, const Ogre::String& imageName);
    Brush(const float* pImageData, size_t width, size_t height, const Ogre::String& imageName);
    ~Brush();

    int getWidth(){return mImageWidth;}
    int getHeight(){return mImageHeight;}

    float* getData(){return mImageData;}

    float getAt(int x, int y);

    const Ogre::String& getTexName(){return mBrushTexName;}

    bool isInterpolated(){return mBilinearInterpolation;}
    void setInterpolated(bool lerp = true){mBilinearInterpolation = lerp;}

private:

    Ogre::String mBrushTexName;

    int mImageWidth;
    int mImageHeight;
    float* mImageData;

    bool mBilinearInterpolation;
};

#endif