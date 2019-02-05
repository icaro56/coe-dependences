#include "CircleDisplacement.h"
#include "TerrainModifier.h"
#include "heightmap/Heightmap.h"

#include <OgreVector2.h>


using namespace Ogre;



	CircleDisplacement::CircleDisplacement( const Ogre::Vector3& center, Ogre::Real radius, Ogre::Real displacement, bool addative /*= true*/ )
		: mCenter(center),
		mRadius(radius),
		mDisplacement(displacement),
		mAddative(addative)
	{

	}

	/*void CircleDisplacement::displace( Ogre::Vector3* pVerts )
	{
		Vector3 pos;
		Real radSquare = mRadius * mRadius;
		Real dist;
		Vector2 center = Vector2(mCenter.x,mCenter.z);
		Real halfDisp = mDisplacement * 0.5f;
		for (size_t z=0;z<MAP_WIDTH;z++)
		{
			for (size_t x=0;x<MAP_WIDTH;x++)
			{
				pos = *pVerts;
				dist = (Vector2(pos.x,pos.z) - center).squaredLength();
				if (dist < radSquare)
				{
					dist = sqrt(dist) / mRadius;
					if (mAddative)
						pos.y += halfDisp + cos(dist * 3.14f) * halfDisp;
					else
						pos.y = halfDisp + cos(dist * 3.14f) * halfDisp;
					*pVerts++ = pos;
				}
				else
					pVerts++;
			}
		}
	}*/

	void CircleDisplacement::displace( Vec2D<double> vTopLeft, double fWidth, Heightmap* pHeightmap, Ogre::Real fScale ) const
	{


		int iMapWidth = (int)pHeightmap->getWidth();

		float mapWidth = (float)(pHeightmap->getWidth()-1);

		float mapScale = mapWidth / fWidth;

		Vector2 center = Vector2(mCenter.x,mCenter.z);

		Vector2 vMin = center - mRadius;
		Vector2 vMax = center + mRadius;

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




		//Real fDelta = fWidth / Real(pHeightmap->getWidth()-1);
		//Real fStartX = vTopLeft.x;
		Real radSquare = mRadius * mRadius;
		Real dist;
		HEIGHTMAPTYPE* pData = pHeightmap->getData();
		float height = 0.0f;

		Real halfDisp = mDisplacement * 0.5f;
		for (int z=iMin.y;z<=iMax.y;z++)
		{
			vTopLeft.x = fStartX;
			for (int x=iMin.x;x<=iMax.x;x++)
			{
				height = UNPACK_HEIGHT(pData[x + z * iMapWidth]) * fScale;
				dist = (Vector2(vTopLeft.x,vTopLeft.y) - center).squaredLength();
				if (dist < radSquare)
				{
					dist = sqrt(dist) / mRadius;
					if (mAddative)
						height += halfDisp + cos(dist * 3.14f) * halfDisp;
					else
						height = halfDisp + cos(dist * 3.14f) * halfDisp;

					pData[x + z * iMapWidth] = PACK_HEIGHT(std::min(std::max(height / fScale,0.0),MAX_INT));
				}
				//else
				//	pData++;

				vTopLeft.x += fDelta;
			}
			vTopLeft.y += fDelta;
		}
	}
	bool CircleDisplacement::isInBounds( const Vec2D<double>& vMin, const Vec2D<double>& vMax ) const
	{
		Vector2 center = Vector2(mCenter.x,mCenter.z);
		Vector2 corner;
		Real radSquare = mRadius * mRadius;

		// Check to see if it's inside
		if (
			(center.x >= vMin.x) ||
			(center.x <= vMax.x) ||
			(center.y >= vMin.y) ||
			(center.y <= vMax.y)
			)
			return true;

		// More aggressive:
		// Check radius
		if (
			((center.x + mRadius) >= vMin.x) ||
			((center.x - mRadius) <= vMax.x) ||
			((center.y + mRadius) >= vMin.y) ||
			((center.y - mRadius) <= vMax.y)
			)
			return true;

		// Ok, maybe the displacement is bigger than this
		// quad-tree node...

		// Top left
		corner = Vector2(vMin.x,vMin.y);
		if ((corner - center).squaredLength() < radSquare)
			return true;

		// Top right
		corner = Vector2(vMax.x,vMin.y);
		if ((corner - center).squaredLength() < radSquare)
			return true;

		// Bottom left
		corner = Vector2(vMin.x,vMax.y);
		if ((corner - center).squaredLength() < radSquare)
			return true;

		// Bottom right
		corner = Vector2(vMax.x,vMax.y);
		if ((corner - center).squaredLength() < radSquare)
			return true;

		// Fail miserably
		return false;


	}
