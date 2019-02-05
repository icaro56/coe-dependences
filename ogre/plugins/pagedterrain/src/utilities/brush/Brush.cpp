#include "Brush.h"
#include <OgreImage.h>
#include <OgreColourValue.h>
#include <OgreTextureManager.h>
#include <OgreHardwarePixelBuffer.h>
#include <OgreLogManager.h>
#include <OgreStringConverter.h>

using namespace Ogre;

Brush::Brush()
: mImageData(0), mImageHeight(0), mImageWidth(0)
{}

Brush::Brush( const Ogre::String& imageName )
: mImageData(0), mImageHeight(0), mImageWidth(0)
{
	mBrushTexName = imageName;
	Image image;
	image.load(imageName,"General");

	mImageWidth = image.getWidth();
	mImageHeight = image.getHeight();

	mImageData = new float[mImageWidth * mImageHeight];
	float* pData = mImageData;

	for (int j=0;j<mImageHeight;j++)
	{
		for (int i=0;i<mImageWidth;i++)
		{
			*pData++ = image.getColourAt(i,j,0).r;
		}
	}
}

Brush::Brush( Ogre::Image& image, const Ogre::String& imageName )
: mImageData(0), mImageHeight(0), mImageWidth(0)
{
    mImageWidth = image.getWidth();
    mImageHeight = image.getHeight();
    mBrushTexName = imageName;

    mImageData = new float[mImageWidth * mImageHeight];
    float* pData = mImageData;

    for (int j=0;j<mImageHeight;j++)
    {
        for (int i=0;i<mImageWidth;i++)
        {
            *pData++ = image.getColourAt(i,j,0).r;
        }
    }
    if (TextureManager::getSingleton().resourceExists(mBrushTexName) == false)
        TextureManager::getSingleton().loadImage(mBrushTexName,"General",image);
}

Brush::Brush( const float* pImageData, size_t width, size_t height, const Ogre::String& imageName )
{}

Brush::~Brush()
{
    if (getTexName() != "")
    {
        TextureManager::getSingleton().remove(getTexName());
    }

    delete[] mImageData;
    mImageData = 0;
}

float Brush::getAt( int x, int y )
{
    assert(mImageData != 0 && "mImageData == 0");

    x = std::min(std::max(x,0),mImageWidth-1);
    y = std::min(std::max(y,0),mImageHeight-1);

    return mImageData[x + y * mImageWidth];
}
