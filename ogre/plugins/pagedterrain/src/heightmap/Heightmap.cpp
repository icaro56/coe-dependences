#include "Heightmap.h"
#include "HeightmapReader.h"
#include "utilities/vectors/Vec2D.h"

#include <OgreVector3.h>
#include <OgrePlane.h>
#include <OgreImage.h>

using namespace Ogre;





	Heightmap::Heightmap()
		: mWidth(0),
		mAllocatedWidth(0),
		mData(0),
		mMaxLevels(0),
		mStartDepth(-1),
		mHasBorder(true)
	{

	}

	Heightmap::Heightmap( const std::string& strFileName, size_t iWidth /*= 0*/, bool bHasBorder /*= true*/ )
		: mHasBorder(bHasBorder),
		mWidth(0),
		mAllocatedWidth(0),
		mData(0),
		mMaxLevels(0),
		mStartDepth(-1)
	{
		if (strFileName != "")
			HeightmapReader::getSingleton().readHeightmap(strFileName,this,iWidth);
		else
		{
			setWidth(iWidth);
			allocateData();
			memset(mData,0,mAllocatedWidth * mAllocatedWidth * sizeof(HEIGHTMAPTYPE));
		}
	}

	/*Heightmap::Heightmap( float* fData, size_t iWidth )
		: mData(fData),
		mWidth(iWidth),
		mMaxLevels(0),
		mStartDepth(-1)
	{

	}*/

	Heightmap::~Heightmap()
	{
		delete[] mData;
		mData = 0;
	}

	void Heightmap::allocateData()
	{
		if (mData)
			return;

		assert("mAllocatedWidth == 0" && mAllocatedWidth);

		mData = new HEIGHTMAPTYPE[mAllocatedWidth * mAllocatedWidth];
	}

	void Heightmap::getSubdivision( Heightmap* pSubMap, size_t iWidth, size_t iOffsetX, size_t iOffsetY, size_t iSpacing )
	{
		pSubMap->setWidth(iWidth);
		pSubMap->allocateData();

		HEIGHTMAPTYPE* pData = pSubMap->getData();

		int iDelta = iSpacing;
		Vec2D<int> iMapPos;
		iMapPos.x = int(iOffsetX) - iDelta;
		iMapPos.y = int(iOffsetY) - iDelta;

		int x,y;

		for (size_t j=0;j<iWidth;j++)
		{
			iMapPos.x = int(iOffsetX) - iDelta;
			y = std::min(std::max(iMapPos.y,0),int(mWidth)-1);
			for(size_t i=0;i<iWidth;i++)
			{
				x = std::min(std::max(iMapPos.x,0),int(mWidth)-1);
				*pData++ = mData[x + y * mWidth];

				iMapPos.x += iDelta;
			}
			iMapPos.y += iDelta;
		}
	}

	size_t Heightmap::calculateMaxLevels()
	{
		size_t iVal = ((mWidth - 1 + (mHasBorder ? -2 : 0)) / (VERTEX_WIDTH - 1));
		mMaxLevels = 0;
		while (iVal != 1 && mMaxLevels < 100)
		{
			iVal >>= 1;
			mMaxLevels++;
		}

		assert("Max level calculation reached 100, which seems impossible." && mMaxLevels < 100);

		return mMaxLevels;
	}

	float Heightmap::getHeightAt( float fPosX, float fPosZ, size_t iDepth, size_t iOffsetX, size_t iOffsetY)
	{
		int iSpacing = (1 << int(iDepth));

		const float fScale = float((mWidth-1) / iSpacing);

		fPosX *= fScale;
		fPosZ *= fScale;

		float fFloorX0 = floor(fPosX);
		float fFloorY0 = floor(fPosZ);

		int x0 = int(fFloorX0) * iSpacing;
		int y0 = int(fFloorY0) * iSpacing;
		int x1 = x0 + iSpacing;
		int y1 = y0 + iSpacing;


		fFloorX0 = fPosX - fFloorX0;
		fFloorY0 = fPosZ - fFloorY0;


		int iMin = (mHasBorder ? -1 : 0);
		int iMax = mAllocatedWidth - 1;

		x0 = std::min(std::max(x0,iMin),iMax);
		x1 = std::min(std::max(x1,iMin),iMax);
		y0 = std::min(std::max(y0,iMin),iMax);
		y1 = std::min(std::max(y1,iMin),iMax);


		float fx0 = UNPACK_HEIGHT(getHeightAt(x0,y0));
		float fx1 = UNPACK_HEIGHT(getHeightAt(x1,y0));
		float fy0 = UNPACK_HEIGHT(getHeightAt(x0,y1));
		float fy1 = UNPACK_HEIGHT(getHeightAt(x1,y1));



		float fL0 = Lerp(fx0,fx1,fFloorX0);
		float fL1 = Lerp(fy0,fy1,fFloorX0);

		return Lerp(fL0,fL1,fFloorY0);


	}

	HEIGHTMAPTYPE Heightmap::getHeightAt( int x, int y )
	{
		if (mData == 0)
			return (HEIGHTMAPTYPE)0;

		if (mHasBorder)
		{
			x += 1;
			y += 1;
		}

		if (x < 0 || x >= (int)mAllocatedWidth || y < 0 || y >= (int)mAllocatedWidth)
			return (HEIGHTMAPTYPE)0;

		return mData[x + y * (int)mAllocatedWidth];
	}

	void Heightmap::setWidth( size_t iWidth )
	{
		mWidth = iWidth;
		mAllocatedWidth = mWidth + (mHasBorder ? 2 : 0);
	}

	float Heightmap::getHeightAndNormalAt(
		float fPosX,
		float fPosZ,
		const Ogre::Vector3& vScale,
		size_t iDepth,
		size_t iOffsetX,
		size_t iOffsetY,
		Ogre::Vector3& vNormal )
	{
		int iSpacing = (1 << int(iDepth));
		//float fSpacing = float(iSpacing);

		const float fScale = float((mWidth-1) / iSpacing);
		const float fInvScale = 1.0f / float(mWidth-1);;

		const float fScalePosX = fPosX * fScale;
		const float fScalePosZ = fPosZ * fScale;

		float fFloorX0 = floor(fScalePosX);
		float fFloorY0 = floor(fScalePosZ);

		int x0 = int(fFloorX0) * iSpacing;
		int y0 = int(fFloorY0) * iSpacing;
		int x1 = x0 + iSpacing;
		int y1 = y0 + iSpacing;


		fFloorX0 = fScalePosX - fFloorX0;
		fFloorY0 = fScalePosZ - fFloorY0;


		int iMin = (mHasBorder ? -1 : 0);
		int iMax = mAllocatedWidth - 1;

		x0 = std::min(std::max(x0,iMin),iMax);
		x1 = std::min(std::max(x1,iMin),iMax);
		y0 = std::min(std::max(y0,iMin),iMax);
		y1 = std::min(std::max(y1,iMin),iMax);


		float fx0 = UNPACK_HEIGHT(getHeightAt(x0,y0));
		float fx1 = UNPACK_HEIGHT(getHeightAt(x1,y0));
		float fy0 = UNPACK_HEIGHT(getHeightAt(x0,y1));
		float fy1 = UNPACK_HEIGHT(getHeightAt(x1,y1));


		Plane plane;
		if ((fFloorX0 + (1.0f - fFloorY0)) <= 1.0f)
		{
			Vector3 v1 = Vector3(x0 * fInvScale,fx0,y0 * fInvScale) * vScale;
			Vector3 v3 = Vector3(x0 * fInvScale,fy0,y1 * fInvScale) * vScale;
			Vector3 v4 = Vector3(x1 * fInvScale,fy1,y1 * fInvScale) * vScale;

			plane.redefine(v1,v3,v4);
		}
		else
		{
			Vector3 v1 = Vector3(x0 * fInvScale,fx0,y0 * fInvScale) * vScale;
			Vector3 v4 = Vector3(x1 * fInvScale,fy1,y1 * fInvScale) * vScale;
			Vector3 v2 = Vector3(x1 * fInvScale,fx1,y0 * fInvScale) * vScale;
			plane.redefine(v1,v4,v2);
		}

		vNormal = plane.normal;

		return
			(-(plane.normal.x * fPosX * vScale.x)
			- plane.normal.z * fPosZ * vScale.z
			- plane.d) / plane.normal.y;

	}

	void Heightmap::saveHeightmap( const Ogre::String& filePath )
	{
		PixelFormat pf = PF_UNKNOWN;
		switch(sizeof(HEIGHTMAPTYPE))
		{
		case 1:
			pf = PF_L8;
			break;
		case 2:
			pf = PF_L16;
		    break;
		default:
		    break;
		}

		Image img;
		img.loadDynamicImage((uchar *)mData,mAllocatedWidth,mAllocatedWidth,1,pf);

		img.save(filePath);
	}

	void Heightmap::setHeightAt( int x, int y, HEIGHTMAPTYPE val )
	{
		if (mData == 0)
			return;

		if (mHasBorder)
		{
			x += 1;
			y += 1;
		}

		if (x < 0 || x >= (int)mAllocatedWidth || y < 0 || y >= (int)mAllocatedWidth)
			return;

		setHeightAt(x + y * (int)mAllocatedWidth, val);

	}

	void Heightmap::setHeightAt( int pos, HEIGHTMAPTYPE val )
	{
		mData[pos] = val;
	}

