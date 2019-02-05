#include "CloudDisplacement.h"
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

///////////////////////////////////////////////////////////////////////////////
    CloudDisplacement::CloudDisplacement(Terrain *t)
    : mTerrain(t), mfWidth(t->getRootNode()->getFloatingPointWidth()), mfScale(t->getTerrainHeight())
    {
        mHeightmap = t->getHeightmap();
    }


///////////////////////////////////////////////////////////////////////////////
    void CloudDisplacement::modify(const Ogre::Vector3 &point )
    {
        const int iMapWidth = (int)mHeightmap->getWidth(); // 8193

		const float fMapWidth = (float)(iMapWidth - 1); // 8193 - 1 = 8192

		const float fMapScale = fMapWidth / mfWidth; // 8192/32000 = 0.256

		const float fInvMapScale = 1.0f / fMapScale; // 1.0f/  0.256 = 3.90625

		const float fHalfWidth = mfWidth * 0.5; // 16000

		const Ogre::Real fInvScale = (1.0f / MAX_INT) * mfScale; // = 2248.0

		Ogre::Vector2 vMin(point.x, point.z);

		// [-half,+half] -> [0,width]
		vMin += fHalfWidth;

		// [0,width] -> [0,mapPixels]
		vMin *= fMapScale;

		// Grab the pixel range inside the brush rect
		Vec2D<int> iMin(Math::ICeil(vMin.x),Math::ICeil(vMin.y));

		// Make sure it doesn't go out of bounds
		iMin.x = std::min(std::max(iMin.x,0),iMapWidth-1);
		iMin.y = std::min(std::max(iMin.y,0),iMapWidth-1);

        // Get the world position of pixels
		vMin = Ogre::Vector2(iMin.x,iMin.y) * fInvMapScale;

		vMin -= (mfWidth * 0.5);

        // Get pointer to the heightmap data
		HEIGHTMAPTYPE* pData = mHeightmap->getData();
		pData[iMin.x + iMin.y * iMapWidth] = PACK_HEIGHT(std::min(std::max(point.y / fInvScale,0.0),MAX_INT));
		mTerrain->getRootNode()->checkModifier(this);
    }
