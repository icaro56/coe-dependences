#include "SquareDisplacement.h"
#include "quadtree/QNode.h"
#include "TerrainModifier.h"
#include "heightmap/Heightmap.h"
#include <OgreImage.h>
#include <OgreColourValue.h>
#include <OgreTextureManager.h>
#include <OgreHardwarePixelBuffer.h>
#include <OgreLogManager.h>
#include <OgreStringConverter.h>
#include <OgreManualObject.h>
#include <OgreMaterialManager.h>
#include <OgreEntity.h>

using namespace Ogre;

////////////////////////////////////////////////////////////////////////////////////////
    SquareDisplacement::SquareDisplacement(Terrain *t)
    : mTerrain(t), mfWidth(t->getRootNode()->getFloatingPointWidth()), mfScale(t->getTerrainHeight())
    {
        mHeightmap = t->getHeightmap();
    }

//////////////////////////////////////////////////////////
    void SquareDisplacement::calculateMinMax()
    {
        mMin = mPoints.at(0);
        mMax = mPoints.at(0);

        PointList::iterator itr = mPoints.begin();
        for (itr; itr != mPoints.end() ; ++itr)
        {
            mMin.x = mMin.x > (*itr).x ? (*itr).x : mMin.x;
            mMax.x = mMax.x < (*itr).x ? (*itr).x : mMax.x;

            mMin.y = mMin.y > (*itr).y ? (*itr).y : mMin.y;
            mMax.y = mMax.y < (*itr).y ? (*itr).y : mMax.y;
        }
    }

////////////////////////////////////////////////////////////
    void SquareDisplacement::modify(const Ogre::Vector3 &pivot, float inc)
    {
        mPoints.clear();
        mPoints.push_back(Ogre::Vector2(pivot.x - inc, pivot.z - inc));
        mPoints.push_back(Ogre::Vector2(pivot.x + inc, pivot.z - inc));
        mPoints.push_back(Ogre::Vector2(pivot.x + inc, pivot.z + inc));
        mPoints.push_back(Ogre::Vector2(pivot.x - inc, pivot.z + inc));

        const int iMapWidth = (int)mHeightmap->getWidth(); // 8193

		const float fMapWidth = (float)(iMapWidth - 1); // 8193 - 1 = 8192

		const float fMapScale = fMapWidth / mfWidth; // 8192/32000 = 0.256

		const float fInvMapScale = 1.0f / fMapScale; // 1.0f/  0.256 = 3.90625

		const float fHalfWidth = mfWidth * 0.5; // 16000

		const float fMax = MAX_INT * mfScale; //65535.0 * 2248.0 = 147.322.680

		const float fInvScale = (1.0f / MAX_INT) * mfScale; // = 2248.0

        calculateMinMax();

		Ogre::Vector2 vMin = mMin;
		Ogre::Vector2 vMax = mMax;

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
		vMin = Ogre::Vector2(iMin.x,iMin.y) * fInvMapScale;
		vMax = Ogre::Vector2(iMax.x,iMax.y) * fInvMapScale;

		vMin -= fHalfWidth;
		vMax -= fHalfWidth;

		// Terrain position delta
		const float fDelta = mfWidth / float(iMapWidth-1);

        // Get pointer to the heightmap data
		HEIGHTMAPTYPE* pData = mHeightmap->getData();
		float height = 0.0f;

        //Loop
		for (int y=iMin.y;y<=iMax.y;y++)
		{
			for (int x=iMin.x;x<iMax.x;x++)
			{
                pData[x + y * iMapWidth] = PACK_HEIGHT(std::min(std::max(pivot.y / fInvScale,0.0),MAX_INT));
			}
		}

        mTerrain->getRootNode()->checkModifier(this);
    }

