#include "RectFlatten.h"
#include "TerrainModifier.h"
#include "heightmap/Heightmap.h"



using namespace Ogre;



	RectFlatten::RectFlatten( const Ogre::Vector3& center, Ogre::Real width, Ogre::Real height )
		: mCenter(center),
		mWidth(width),
		mHeight(height)
	{
		mMin.x = mCenter.x - (mWidth * 0.5f);
		mMin.y = mCenter.z - (mHeight * 0.5f);

		mMax = mMin + Vector2(mWidth,mHeight);

	}

	void RectFlatten::displace( Vec2D<double> vTopLeft, double fWidth, Heightmap* pHeightmap, float fScale ) const
	{
		int iMapWidth = (int)pHeightmap->getWidth();

		float mapWidth = (float)(pHeightmap->getWidth()-1);

		float mapScale = mapWidth / fWidth;

		Vector2 vMin = mMin;
		Vector2 vMax = mMax;

		vMin += (fWidth * 0.5);
		vMax += (fWidth * 0.5);

		vMin *= mapScale;
		vMax *= mapScale;

		Vec2D<int> iMin(Math::IFloor(vMin.x),Math::IFloor(vMin.y));
		Vec2D<int> iMax(Math::ICeil(vMax.x),Math::ICeil(vMax.y));

		iMin.x = std::min(std::max(iMin.x,0),iMapWidth-1);
		iMin.y = std::min(std::max(iMin.y,0),iMapWidth-1);
		iMax.x = std::min(std::max(iMax.x,0),iMapWidth-1);
		iMax.y = std::min(std::max(iMax.y,0),iMapWidth-1);

		Real fDelta = fWidth / Real(iMapWidth-1);
		Real fStartX = vTopLeft.x + (float(iMin.x) * fDelta);
		vTopLeft.y += float(iMin.y) * fDelta;

		HEIGHTMAPTYPE* pData = pHeightmap->getData();
		for (int z=iMin.y;z<=iMax.y;z++)
		{
			vTopLeft.x = fStartX;
			for (int x=iMin.x;x<=iMax.x;x++)
			{
				if (vTopLeft.x >= mMin.x &&
					vTopLeft.x <= mMax.x &&
					vTopLeft.y >= mMin.y &&
					vTopLeft.y <= mMax.y)
					pData[x + z * iMapWidth] = PACK_HEIGHT(std::min(std::max(mCenter.y / fScale,0.0),MAX_INT));
				//else
				//	pData++;


				vTopLeft.x += fDelta;
			}
			vTopLeft.y += fDelta;
		}
	}

	bool RectFlatten::isInBounds( const Vec2D<double>& vMin, const Vec2D<double>& vMax ) const
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



// 		return CRectangle(
// 			( A.Left > B.Left ? A.Left : B.Left ),
// 			( A.Top > B.Top ? A.Top : B.Top ),
// 			( A.Right < B.Right ? A.Right : B.Right ),
// 			( A.Bottom < B.Bottom ? A.Bottom : B.Bottom ) );




		//return false;
	}
