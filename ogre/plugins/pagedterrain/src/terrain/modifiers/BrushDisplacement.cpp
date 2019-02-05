#include "BrushDisplacement.h"
#include "utilities/brush/Brush.h"
#include "TerrainModifier.h"
#include "heightmap/Heightmap.h"
#include <OgreImage.h>
#include <OgreColourValue.h>
#include <OgreTextureManager.h>
#include <OgreHardwarePixelBuffer.h>
#include <OgreLogManager.h>
#include <OgreStringConverter.h>

#include <fstream>

using namespace Ogre;


//////////////////////////////////////////////////////////////////////////
BrushDisplacement::BrushDisplacement(BrushPtr pBrush,  const Ogre::Vector3& center, Ogre::Real width, Ogre::Real height, Ogre::Real intensity )
    : mCenter(center),
    mWidth(width),
    mHeight(height),
    mIntensity(intensity),
    mBrush(pBrush)
{
    mMin.x = mCenter.x - (mWidth * 0.5f);
    mMin.y = mCenter.z - (mHeight * 0.5f);

    mMax = mMin + Vector2(mWidth,mHeight);


}

void BrushDisplacement::displace( Vec2D<double> vTopLeft, double fWidth, Heightmap* pHeightmap, float fScale ) const
{
    const bool bLerp = mBrush->isInterpolated();

    const int iMapWidth = (int)pHeightmap->getWidth();

    const float fMapWidth = (float)(iMapWidth - 1);

    const float fMapScale = fMapWidth / fWidth;

    const float fInvMapScale = 1.0f / fMapScale;

    const float fHalfWidth = fWidth * 0.5;

    //const float fMax = MAX_INT * fScale;

    const float fInvScale = (1.0f / MAX_INT) * fScale;


    Vector2 vMin = mMin;
    Vector2 vMax = mMax;

    // [-half,+half] -> [0,width]
    vMin += fHalfWidth;
    vMax += fHalfWidth;

    // [0,width] -> [0,mapPixels]
    vMin *= fMapScale;
    vMax *= fMapScale;

   /* if(vMax.y !=4096)
        vMax.y = 4096;*/

    // Grab the pixel range inside the brush rect
    Vec2D<int> iMin(Math::ICeil(vMin.x),Math::ICeil(vMin.y));
    Vec2D<int> iMax(Math::IFloor(vMax.x),Math::IFloor(vMax.y));


    // Make sure it doesn't go out of bounds
    iMin.x = std::min(std::max(iMin.x,0),iMapWidth-1);
    iMin.y = std::min(std::max(iMin.y,0),iMapWidth-1);
    iMax.x = std::min(std::max(iMax.x,0),iMapWidth-1);
    iMax.y = std::min(std::max(iMax.y,0),iMapWidth-1);

    // Get the world position of pixels
    vMin = Vector2(iMin.x,iMin.y) * fInvMapScale;
    vMax = Vector2(iMax.x,iMax.y) * fInvMapScale;

    vMin -= (fWidth * 0.5);
    vMax -= (fWidth * 0.5);


    Vector2 vBrushSize(mBrush->getWidth() - 1, mBrush->getHeight() - 1);
    Vector2 vBrushScale(mWidth,mHeight);

    vBrushSize /= vBrushScale;

    // Terrain position delta
    const float fDelta = fWidth / float(iMapWidth-1);

    Vector2 vTerrainPos = vMin;
    float fTerrainStartX = vMin.x;

    Vector2 vBrushPos;

    // Get pointer to the heightmap data
    HEIGHTMAPTYPE* pData = pHeightmap->getData();
    Ogre::Real height = 0.0f;

    // Loop
    for (int y=iMin.y;y<=iMax.y;y++)
    {

        vTerrainPos.x = fTerrainStartX;
        for (int x=iMin.x;x<iMax.x;x++)
        {


            if (
                vTerrainPos.x >= mMin.x &&
                vTerrainPos.x <= mMax.x &&
                vTerrainPos.y >= mMin.y &&
                vTerrainPos.y <= mMax.y
                )
            {
                height = UNPACK_HEIGHT(pData[x + y * iMapWidth]) * fInvScale;

                vBrushPos = vTerrainPos - mMin;

                vBrushPos *= vBrushSize;



                if (bLerp)
                {
                    Vector2 vFloor;
                    vFloor.x = Math::Floor(vBrushPos.x);
                    vFloor.y = Math::Floor(vBrushPos.y);

                    Vector2 vLerp = vBrushPos - vFloor;
                    Vec2D<int> ivPos((int)vFloor.x,(int)vFloor.y);

                    Ogre::Real TL = mBrush->getAt(ivPos.x,ivPos.y);
                    Ogre::Real TR = mBrush->getAt(ivPos.x + 1, ivPos.y);
                    Ogre::Real BL = mBrush->getAt(ivPos.x, ivPos.y + 1);
                    Ogre::Real BR = mBrush->getAt(ivPos.x + 1, ivPos.y + 1);

                    Ogre::Real lerpTop = Lerp(TL,TR,vLerp.x);
                    Ogre::Real lerpBottom = Lerp(BL,BR,vLerp.x);
                    Ogre::Real lerpFinal = Lerp(lerpTop,lerpBottom,vLerp.y);

                    height += lerpFinal * (mIntensity * 0.01f * fScale);
                }
                else
                {
                    height += mBrush->getAt(vBrushPos.x,vBrushPos.y) * (mIntensity * 0.01f * fScale);
                }

                pData[x + y * iMapWidth] = PACK_HEIGHT(std::min(std::max(height / fInvScale,0.0),MAX_INT));

            }
            vTerrainPos.x += fDelta;
        }
        vTerrainPos.y += fDelta;
    }
}

void BrushDisplacement::displaceTexture( Vec2D<double> vTopLeft, double fWidth, Ogre::TexturePtr pTex, int iChannel /*= 0*/) const
{
    size_t totalComponents = Ogre::PixelUtil::getComponentCount(pTex->getFormat());
    if (totalComponents < iChannel)
        return;

    Ogre::uchar defaultValues[4] = {0,0,0,0};
    switch(totalComponents)
    {
        case 3:
            defaultValues[3] = 255;
            break;
        case 2:
            defaultValues[3] = 255;
            defaultValues[2] = 255;
            break;
        case 1:
            defaultValues[3] = 255;
            defaultValues[2] = 255;
            defaultValues[1] = 255;
            break;
        default:
            break;
    }

    size_t destDepth = Ogre::PixelUtil::getNumElemBytes(pTex->getFormat());

    const bool bLerp = mBrush->isInterpolated();

    const int iMapWidth = (int)pTex->getWidth();

    const float fMapWidth = (float)(iMapWidth - 1);

    const float fMapScale = fMapWidth / fWidth;

    const float fInvMapScale = 1.0f / fMapScale;

    const float fHalfWidth = fWidth * 0.5;

    Vector2 vMin = mMin;
    Vector2 vMax = mMax;

    // [-half,+half] -> [0,width]
    vMin += fHalfWidth;
    vMax += fHalfWidth;

    // [0,width] -> [0,mapPixels]
    vMin *= fMapScale;
    vMax *= fMapScale;

    // Grab the pixel range inside the brush rect
    Vec2D<int> iMin(Math::ICeil(vMin.x),Math::ICeil(vMin.y));
    Vec2D<int> iMax(Math::IFloor(vMax.x),Math::IFloor(vMax.y));


    // Make sure it doesn't go out of bounds
    iMin.x = std::min(std::max(iMin.x,0),iMapWidth-1);
    iMin.y = std::min(std::max(iMin.y,0),iMapWidth-1);
    iMax.x = std::min(std::max(iMax.x,0),iMapWidth-1);
    iMax.y = std::min(std::max(iMax.y,0),iMapWidth-1);


    // Get the world position of pixels
    vMin = Vector2(iMin.x,iMin.y) * fInvMapScale;
    vMax = Vector2(iMax.x,iMax.y) * fInvMapScale;

    vMin -= (fWidth * 0.5);
    vMax -= (fWidth * 0.5);

    Vector2 vBrushSize(mBrush->getWidth() - 1, mBrush->getHeight() - 1);
    Vector2 vBrushScale(mWidth,mHeight);

    vBrushSize /= vBrushScale;

    // Terrain position delta
    const float fDelta = fWidth / float(iMapWidth-1);

    Vector2 vTerrainPos = vMin;
    float fTerrainStartX = vMin.x;

    Vector2 vBrushPos;

    // Get pointer to the heightmap data
    //HEIGHTMAPTYPE* pData = pHeightmap->getData();
    HardwarePixelBufferSharedPtr pBuf = pTex->getBuffer();
    pBuf->lock(HardwareBuffer::HBL_NORMAL);

    const PixelBox& pixelBox = pBuf->getCurrentLock();

    //size_t totalSize = pixelBox.getConsecutiveSize();


    uchar* pData = static_cast<uchar*>(pixelBox.data);



    float height = 0.0f;

    // Loop
    for (int y=iMin.y;y<=iMax.y;y++)
    {
        vTerrainPos.x = fTerrainStartX;
        for (int x=iMin.x;x<iMax.x;x++)
        {
            if (
                vTerrainPos.x >= mMin.x &&
                vTerrainPos.x <= mMax.x &&
                vTerrainPos.y >= mMin.y &&
                vTerrainPos.y <= mMax.y
                )
            {
                int pos = (x * destDepth) + (y * iMapWidth * destDepth);
                height = ((float)pData[pos + iChannel]) / 255.0f;

                vBrushPos = vTerrainPos - mMin;

                vBrushPos *= vBrushSize;

                if (bLerp)
                {
                    Vector2 vFloor;
                    vFloor.x = Math::Floor(vBrushPos.x);
                    vFloor.y = Math::Floor(vBrushPos.y);

                    Vector2 vLerp = vBrushPos - vFloor;
                    Vec2D<int> ivPos((int)vFloor.x,(int)vFloor.y);

                    Ogre::Real TL = mBrush->getAt(ivPos.x,ivPos.y);
                    Ogre::Real TR = mBrush->getAt(ivPos.x + 1, ivPos.y);
                    Ogre::Real BL = mBrush->getAt(ivPos.x, ivPos.y + 1);
                    Ogre::Real BR = mBrush->getAt(ivPos.x + 1, ivPos.y + 1);

                    Ogre::Real lerpTop = Lerp(TL,TR,vLerp.x);
                    Ogre::Real lerpBottom = Lerp(BL,BR,vLerp.x);
                    Ogre::Real lerpFinal = Lerp(lerpTop,lerpBottom,vLerp.y);

                    height += lerpFinal * (mIntensity * 0.01f);
                }
                else
                {
                    height += mBrush->getAt(vBrushPos.x,vBrushPos.y) * (mIntensity * 0.01f);
                }


                //pData[pos] = defaultValues[0];
                //pData[pos + 1] = defaultValues[1];
                //pData[pos + 2] = defaultValues[2];
                //pData[pos + 3] = defaultValues[3];
                pData[pos + iChannel] = static_cast<uchar>(std::min(std::max(255.0f * height,0.0f),255.0f));


            }
            vTerrainPos.x += fDelta;
        }
        vTerrainPos.y += fDelta;
    }

    pBuf->unlock();

    }

//
// 	void BrushDisplacement::displace( Vec2D<double> vTopLeft, double fWidth, Heightmap* pHeightmap, float fScale ) const
// 	{
//
//
//
// 		//size_t startX = 0;
// 		//size_t startZ = 0;
// 		//size_t endX = pHeightmap->getWidth();
// 		//size_t endZ = endX;
//
// 		bool bLerp = mBrush->isInterpolated();
//
// 		int iMapWidth = (int)pHeightmap->getWidth();
//
// 		float mapWidth = (float)(pHeightmap->getWidth()-1);
//
// 		float mapScale = mapWidth / fWidth;
//
// 		Vector2 vMin = mMin;
// 		Vector2 vMax = mMax;
//
// 		vMin += (fWidth * 0.5);
// 		vMax += (fWidth * 0.5);
//
// 		vMin *= mapScale;
// 		vMax *= mapScale;
//
// 		Vec2D<int> iMin(Math::ICeil(vMin.x),Math::ICeil(vMin.y));
// 		Vec2D<int> iMax(Math::IFloor(vMax.x),Math::IFloor(vMax.y));
//
// 		iMin.x = std::min(std::max(iMin.x,0),iMapWidth-1);
// 		iMin.y = std::min(std::max(iMin.y,0),iMapWidth-1);
// 		iMax.x = std::min(std::max(iMax.x,0),iMapWidth-1);
// 		iMax.y = std::min(std::max(iMax.y,0),iMapWidth-1);
//
// 		float invMapScale = 1.0f / mapScale;
//
// 		vMin = Vector2(iMin.x,iMin.y) * invMapScale;
// 		vMax = Vector2(iMax.x,iMax.y) * invMapScale;
//
// 		vMin -= (fWidth * 0.5);
// 		vMax -= (fWidth * 0.5);
//
// 		Vector2 vScaledDimensions = vMax - vMin;
//
//
// 		//Vec2D<double> vBottomRight = vTopLeft + fWidth;
//
// 		Vector2 vBias(-vTopLeft.x,-vTopLeft.y);
//
// 		Vector2 vBrushMin = mMin + vBias;
// 		Vector2 vBrushMax = mMax + vBias;
//
// 		Vector2 vBrushScale(mWidth,mHeight);
// 		vBrushScale /= fWidth;
//
// 		/*vBrushMin *= vBrushScale;
// 		vBrushMax *= vBrushScale;
//
// 		vBrushMin.x = std::min(std::max(vBrushMin.x,0.0f),mWidth);
// 		vBrushMin.y = std::min(std::max(vBrushMin.y,0.0f),mHeight);
// 		vBrushMax.x = std::min(std::max(vBrushMax.x,0.0f),mWidth);
// 		vBrushMax.y = std::min(std::max(vBrushMax.y,0.0f),mHeight);*/
//
// 		Vector2 vBrushWidth(mWidth,mHeight);// = vBrushMax - vBrushMin;
//
// 		Vector2 vBrushDelta = 1.0f / vBrushWidth;
//
// 		//vBrushMin = mMin + vBias;
// 		//vBrushMax = mMax + vBias;
//
// 		vBrushMin *= vBrushScale;
//
// 		Real fBrushStartX = vBrushMin.x;
//
// 		Vector2 vBrushSize(mBrush->getWidth()-1,mBrush->getHeight()-1);
// 		Vector2 vBrushImageSize = vBrushSize;
// 		Vector2 vInvBrushScale(1.0f / mWidth,1.0f / mHeight);
// 		vBrushSize *= vInvBrushScale;
//
// 		Real fDelta = fWidth / Real(iMapWidth-1);
// 		Real fStartX = vTopLeft.x + (float(iMin.x) * fDelta);
// 		vTopLeft.y += float(iMin.y) * fDelta;
//
// 		HEIGHTMAPTYPE* pData = pHeightmap->getData();
// 		float height = 0.0f;
// 		Vec2D<int> iBrushPos;
// 		Vector2 vBrushPos;
// 		for (int z=iMin.y;z<=iMax.y;z++)
// 		{
// 			vTopLeft.x = fStartX;
// 			//vBrushMin.x = fBrushStartX;
// 			for (int x=iMin.x;x<=iMax.x;x++)
// 			{
// 				if (vTopLeft.x >= mMin.x &&
// 					vTopLeft.x <= mMax.x &&
// 					vTopLeft.y >= mMin.y &&
// 					vTopLeft.y <= mMax.y/* &&
// 					vBrushMin.x >= 0.0f &&
// 					vBrushMin.y >= 0.0f &&
// 					vBrushMin.x <= mWidth &&
// 					vBrushMin.y <= mHeight*/)
// 				{
// 					height = UNPACK_HEIGHT(pData[x + z * iMapWidth]) * fScale;
// 					/*iBrushPos.x = vBrushMin.x * vBrushSize.x;
// 					iBrushPos.y = vBrushMin.y * vBrushSize.y;
//
// 					height += mBrush->getAt(iBrushPos.x,iBrushPos.y) * mIntensity;*/
//
// 					vBrushPos.x = vTopLeft.x - vMin.x;
// 					vBrushPos.y = vTopLeft.y - vMin.y;
//
// 					vBrushPos *= vBrushSize;
// 					if (bLerp)
// 					{
// 						Vector2 vFloor;
// 						vFloor.x = Math::Floor(vBrushPos.x);
// 						vFloor.y = Math::Floor(vBrushPos.y);
//
// 						Vector2 vLerp = vBrushPos - vFloor;
// 						Vec2D<int> ivPos((int)vFloor.x,(int)vFloor.y);
//
// 						float TL = mBrush->getAt(ivPos.x,ivPos.y);
// 						float TR = mBrush->getAt(ivPos.x + 1, ivPos.y);
// 						float BL = mBrush->getAt(ivPos.x, ivPos.y + 1);
// 						float BR = mBrush->getAt(ivPos.x + 1, ivPos.y + 1);
//
// 						float lerpTop = Lerp(TL,TR,vLerp.x);
// 						float lerpBottom = Lerp(BL,BR,vLerp.x);
// 						float lerpFinal = Lerp(lerpTop,lerpBottom,vLerp.y);
//
// 						height += lerpFinal * mIntensity;
//
// 					}
// 					else
// 					{
//
// 						height += mBrush->getAt(vBrushPos.x,vBrushPos.y) * mIntensity;
// 					}
//
//
//
//
// 					pData[x + z * iMapWidth] = PACK_HEIGHT(std::min(std::max(height / fScale,0.0f),MAX_INT));
// 				}
//
//
//
// 				vTopLeft.x += fDelta;
// 				//vBrushMin.x += vBrushDelta.x;
// 			}
// 			vTopLeft.y += fDelta;
// 			//vBrushMin.y += vBrushDelta.y;
// 		}
// 	}

bool BrushDisplacement::isInBounds( const Vec2D<double>& vMin, const Vec2D<double>& vMax ) const
{
    // This inside checked node
    if (mMin.x >= vMin.x &&
        mMin.y >= vMin.y &&
        mMax.x <= vMax.x &&
        mMax.y <= vMax.y)
        return true;

    // Node inside this
    if (vMin.x >= mMin.x &&
        vMin.y >= mMin.y &&
        vMax.x <= mMax.x &&
        vMax.y <= mMax.y)
        return true;


    return !
        (
        (mMin.x > vMax.x) ||
        (mMin.y > vMax.y) ||
        (mMax.x < vMin.x) ||
        (mMax.y < vMin.y)
        );


    //return false;
}





